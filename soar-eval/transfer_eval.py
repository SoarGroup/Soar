#!/usr/bin/env python3
"""Soar transfer eval harness.

Runs a chunking agent on generated Blocks World tasks and measures
whether learning on training tasks speeds up transfer tasks.

Usage:
    python eval.py --seeds 5 --train 10 --transfer 10
"""

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

from tasks.blocks_world import generate_tasks, write_task_file

SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parent

SOAR_CLI = REPO_ROOT / "build" / "SoarCLI" / "soar"
BASE_AGENT = SCRIPT_DIR / "agents" / "bw-op-subgoal-base.soar"
RESULTS_DIR = SCRIPT_DIR / "results"
MAX_DECISIONS = 500


def parse_run_line(output):
    """Extract per-run metrics from '--> N decision cycles' line."""
    decisions = 0
    chunks_learned = 0

    m = re.search(r"--> (\d+) decision cycles? executed", output)
    if m:
        decisions = int(m.group(1))

    m = re.search(r"(\d+) new rules? learned", output)
    if m:
        chunks_learned = int(m.group(1))

    return {"decisions": decisions, "chunks_learned": chunks_learned}


def parse_stats(output):
    """Extract cumulative metrics from Soar stats output."""
    chunks = 0
    m = re.search(r"(\d+) chunks?\)", output)
    if m:
        chunks = int(m.group(1))
    return {"chunks_total": chunks}


def run_single_task(soar_cli, base_agent, task_file, chunking=True,
                    extra_sources=None, max_decisions=MAX_DECISIONS):
    """Run agent on a single task in a fresh Soar process.

    extra_sources: list of .soar files to source before the task (e.g., saved chunks)
    Returns stats dict.
    """
    cmd_parts = []
    if not chunking:
        cmd_parts.append("chunk never")
    if extra_sources:
        for s in extra_sources:
            cmd_parts.append(f'source "{s}"')
    cmd_parts.append(f'source "{task_file}"')
    cmd_parts.append(f"run {max_decisions}")
    cmd_parts.append("stats")

    cmd_str = "; ".join(cmd_parts)

    proc = subprocess.run(
        [str(soar_cli), "-s", str(base_agent), cmd_str],
        capture_output=True, text=True, timeout=30, cwd=str(REPO_ROOT)
    )

    output = proc.stdout + proc.stderr
    run_metrics = parse_run_line(output)
    cumulative = parse_stats(output)
    halted = "halted" in output.lower()

    return {
        "decisions": run_metrics["decisions"],
        "chunks_learned": run_metrics["chunks_learned"],
        "chunks_total": cumulative["chunks_total"],
        "halted": halted,
        "status": "success" if halted else "timeout",
    }


def save_chunks(soar_cli, base_agent, task_files, chunking=True,
                max_decisions=MAX_DECISIONS):
    """Run tasks sequentially in one process, save learned chunks to a file.

    Uses the CLI's semicolon-joined command approach which works for
    source+run+save sequences.
    Returns (list of per-task stats, path to saved chunks file).
    """
    chunks_file = tempfile.mktemp(suffix=".soar")

    # Build command: source each task, run, excise, source next, init-soar, run...
    # Then save chunks at the end.
    cmd_parts = []
    if not chunking:
        cmd_parts.append("chunk never")

    for i, task_file in enumerate(task_files):
        cmd_parts.append(f'source "{task_file}"')
        if i > 0:
            cmd_parts.append("init-soar")
        cmd_parts.append(f"run {max_decisions}")
        cmd_parts.append("excise eval*propose*initialize-blocks-world")
        cmd_parts.append("excise eval*apply*initialize-blocks-world")

    # Save all chunks to file
    cmd_parts.append(f'command-to-file "{chunks_file}" print --chunks --full')
    cmd_parts.append("stats")

    cmd_str = "; ".join(cmd_parts)

    proc = subprocess.run(
        [str(soar_cli), "-s", str(base_agent), cmd_str],
        capture_output=True, text=True, timeout=120, cwd=str(REPO_ROOT)
    )

    output = proc.stdout + proc.stderr
    cumulative = parse_stats(output)

    # Check if chunks file was created and has content
    if not os.path.exists(chunks_file) or os.path.getsize(chunks_file) == 0:
        chunks_file = None

    return cumulative, chunks_file


def run_condition(soar_cli, base_agent, task_files, phase_labels,
                  chunking=True, max_decisions=MAX_DECISIONS,
                  chunks_file=None):
    """Run each task as a separate subprocess for reliable stats.

    If chunks_file is provided, it's sourced before each task to simulate
    having learned from prior training.
    """
    extra = [chunks_file] if chunks_file else None
    results = []
    for task_file, label in zip(task_files, phase_labels):
        stats = run_single_task(
            soar_cli, base_agent, task_file,
            chunking=chunking, extra_sources=extra,
            max_decisions=max_decisions
        )
        stats["id"] = label
        results.append(stats)
    return results


def run_eval(seed, n_train, n_transfer, soar_cli=SOAR_CLI,
             base_agent=BASE_AGENT):
    """Run the full three-condition eval for one seed."""
    train_tasks, transfer_tasks = generate_tasks(n_train, n_transfer, seed)

    # Write task files
    tmpdir = tempfile.mkdtemp(prefix="soar_eval_")
    train_files = []
    transfer_files = []
    train_labels = []
    transfer_labels = []

    for i, (init, goal) in enumerate(train_tasks):
        name = f"eval*task*seed{seed}*train{i:02d}"
        path = os.path.join(tmpdir, f"train_{i:02d}.soar")
        write_task_file(path, init, goal, name)
        train_files.append(path)
        train_labels.append(f"train_{i:02d}")

    for i, (init, goal) in enumerate(transfer_tasks):
        name = f"eval*task*seed{seed}*transfer{i:02d}"
        path = os.path.join(tmpdir, f"transfer_{i:02d}.soar")
        write_task_file(path, init, goal, name)
        transfer_files.append(path)
        transfer_labels.append(f"transfer_{i:02d}")

    # Phase 1: Train — run all training tasks in one process, save chunks
    train_cumulative, chunks_file = save_chunks(
        soar_cli, base_agent, train_files, chunking=True
    )

    # Condition 1: trained-transfer (transfer tasks WITH learned chunks)
    trained_transfer = run_condition(
        soar_cli, base_agent, transfer_files, transfer_labels,
        chunking=True, chunks_file=chunks_file
    )

    # Condition 2: fresh baseline (transfer tasks WITHOUT learned chunks)
    baseline_results = run_condition(
        soar_cli, base_agent, transfer_files, transfer_labels, chunking=True
    )

    # Condition 3: no-learning control (no chunking at all)
    nolearn_transfer = run_condition(
        soar_cli, base_agent, transfer_files, transfer_labels, chunking=False
    )

    # Clean up chunks file
    if chunks_file and os.path.exists(chunks_file):
        os.unlink(chunks_file)

    # Compute transfer ratio
    trained_transfer_dcs = sum(r["decisions"] for r in trained_transfer)
    baseline_dcs = sum(r["decisions"] for r in baseline_results)
    trained_train_dcs = 0  # not tracked per-task in this approach
    trained_chunks = train_cumulative.get("chunks_total", 0)

    if baseline_dcs > 0:
        transfer_ratio = (baseline_dcs - trained_transfer_dcs) / baseline_dcs
    else:
        transfer_ratio = 0.0

    trained_success = sum(1 for r in trained_transfer if r["status"] == "success")
    baseline_success = sum(1 for r in baseline_results if r["status"] == "success")
    nolearn_success = sum(1 for r in nolearn_transfer if r["status"] == "success")
    nolearn_dcs = sum(r["decisions"] for r in nolearn_transfer)

    return {
        "seed": seed,
        "n_train": n_train,
        "n_transfer": n_transfer,
        "trained_transfer": {
            "success": f"{trained_success}/{n_transfer}",
            "transfer_decisions": trained_transfer_dcs,
            "chunks": trained_chunks,
            "transfer_ratio": round(transfer_ratio, 3),
        },
        "fresh_baseline": {
            "success": f"{baseline_success}/{n_transfer}",
            "transfer_decisions": baseline_dcs,
        },
        "no_learning": {
            "success": f"{nolearn_success}/{n_transfer}",
            "transfer_decisions": nolearn_dcs,
        },
        "per_task": {
            "trained_transfer": trained_transfer,
            "baseline": baseline_results,
            "no_learning": nolearn_transfer,
        }
    }


def print_summary(results):
    """Print summary table."""
    header = f"{'Seed':>6}  {'Condition':<20}  {'Success':<10}  {'Transfer DCs':>13}  {'Chunks':>7}  {'Transfer Ratio':>15}"
    print(header)
    print("-" * len(header))
    for r in results:
        s = r["seed"]
        t = r["trained_transfer"]
        b = r["fresh_baseline"]
        n = r["no_learning"]
        print(f"{s:>6}  {'trained-transfer':<20}  {t['success']:<10}  {t['transfer_decisions']:>13}  {t['chunks']:>7}  {t['transfer_ratio']:>+15.3f}")
        print(f"{s:>6}  {'fresh-baseline':<20}  {b['success']:<10}  {b['transfer_decisions']:>13}  {'-':>7}  {'-':>15}")
        print(f"{s:>6}  {'no-learning':<20}  {n['success']:<10}  {n['transfer_decisions']:>13}  {'-':>7}  {'-':>15}")
        print()


def main():
    parser = argparse.ArgumentParser(description="Soar transfer eval harness")
    parser.add_argument("--seeds", type=int, default=5, help="Number of seeds")
    parser.add_argument("--train", type=int, default=6, help="Training tasks per seed")
    parser.add_argument("--transfer", type=int, default=6, help="Transfer tasks per seed")
    parser.add_argument("--soar", type=str, default=str(SOAR_CLI), help="Path to soar CLI")
    parser.add_argument("--agent", type=str, default=str(BASE_AGENT), help="Path to base agent")
    parser.add_argument("--output", type=str, default=None, help="JSON output file")
    args = parser.parse_args()

    soar_cli = Path(args.soar)
    base_agent = Path(args.agent)

    if not soar_cli.exists():
        print(f"Error: Soar CLI not found at {soar_cli}", file=sys.stderr)
        sys.exit(1)
    if not base_agent.exists():
        print(f"Error: Base agent not found at {base_agent}", file=sys.stderr)
        sys.exit(1)

    all_results = []
    for seed in range(args.seeds):
        print(f"Running seed {seed}...", file=sys.stderr)
        result = run_eval(seed, args.train, args.transfer, soar_cli, base_agent)
        all_results.append(result)

    print_summary(all_results)

    if args.output:
        output_path = Path(args.output)
    else:
        RESULTS_DIR.mkdir(parents=True, exist_ok=True)
        output_path = RESULTS_DIR / "latest.json"

    with open(output_path, "w") as f:
        json.dump(all_results, f, indent=2)
    print(f"\nResults written to {output_path}", file=sys.stderr)


if __name__ == "__main__":
    main()
