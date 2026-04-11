#!/usr/bin/env python3
"""Per-module eval harness for Soar.

Wraps existing test agents, captures quantitative stats per test,
dumps to JSON, and diffs across builds.

Usage:
    python module_eval.py run --soar BUILD/SoarCLI/soar --suite ChunkingTests --out results.json
    python module_eval.py compare --base upstream.json --candidate pr.json
"""

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path

SOAR_CLI_DEFAULT = Path(__file__).parent.parent / "build" / "SoarCLI" / "soar"
TEST_AGENTS_DIR = Path(__file__).parent.parent / "UnitTests" / "SoarTestAgents"

# Suite definitions: directory glob patterns for test agents
SUITES = {
    "ChunkingTests": {
        "glob": "Chunking/tests/ChunkingDemoTests_*.soar",
        "description": "Chunking demo agents (BW, arithmetic, etc.)",
    },
    "FunctionalTests": {
        "glob": "FunctionalTests_*.soar",
        "description": "Core functional test agents",
    },
    "SMemFunctionalTests": {
        "glob": "smem/SMemFunctionalTests_*.soar",
        "description": "Semantic memory functional tests",
    },
    "EpMemFunctionalTests": {
        "glob": "epmem/EpMemFunctionalTests_*.soar",
        "description": "Episodic memory functional tests",
    },
}

# Metrics we always capture (deterministic, comparable across builds)
DETERMINISTIC_METRICS = [
    "decisions", "elaboration_cycles", "production_firings",
    "wm_current", "wm_mean", "wm_max",
    "productions_user", "productions_chunks",
]

# Metrics that vary (need median of N runs)
TIMING_METRICS = ["kernel_cpu_sec", "total_cpu_sec"]


def parse_stats(output):
    """Parse Soar stats output into a dict of metrics."""
    stats = {"status": "error", "halted": False}

    if "halted" in output.lower():
        stats["halted"] = True
        stats["status"] = "success"
    elif "Run stopped" not in output and "interrupt" not in output.lower():
        stats["status"] = "timeout"

    m = re.search(r"(\d+) decisions?", output)
    if m:
        stats["decisions"] = int(m.group(1))
        stats["status"] = "success"

    m = re.search(r"(\d+) elaboration cycles?", output)
    if m:
        stats["elaboration_cycles"] = int(m.group(1))

    m = re.search(r"(\d+) production firings?", output)
    if m:
        stats["production_firings"] = int(m.group(1))

    m = re.search(r"WM size:\s*(\d+) current,\s*([\d.]+) mean,\s*(\d+) maximum", output)
    if m:
        stats["wm_current"] = int(m.group(1))
        stats["wm_mean"] = float(m.group(2))
        stats["wm_max"] = int(m.group(3))

    m = re.search(r"(\d+) productions? \((\d+) default, (\d+) user, (\d+) chunks?\)", output)
    if m:
        stats["productions_total"] = int(m.group(1))
        stats["productions_default"] = int(m.group(2))
        stats["productions_user"] = int(m.group(3))
        stats["productions_chunks"] = int(m.group(4))

    m = re.search(r"Kernel CPU Time:\s*([\d.]+) sec", output)
    if m:
        stats["kernel_cpu_sec"] = float(m.group(1))

    m = re.search(r"Total\s+CPU Time:\s*([\d.]+) sec", output)
    if m:
        stats["total_cpu_sec"] = float(m.group(1))

    return stats


def run_test(soar_cli, agent_path, max_decisions=10000):
    """Run a single test agent and return parsed stats."""
    try:
        proc = subprocess.run(
            [str(soar_cli), "-s", str(agent_path), f"run {max_decisions}; stats"],
            capture_output=True, text=True, timeout=30
        )
        output = proc.stdout + proc.stderr
        stats = parse_stats(output)
        stats["agent"] = agent_path.name
        stats["exit_code"] = proc.returncode
        return stats
    except subprocess.TimeoutExpired:
        return {"agent": agent_path.name, "status": "timeout", "exit_code": -1}
    except Exception as e:
        return {"agent": agent_path.name, "status": "error", "error": str(e)}


def discover_tests(suite_name):
    """Find test agent files for a suite."""
    if suite_name not in SUITES:
        print(f"Unknown suite: {suite_name}. Available: {list(SUITES.keys())}")
        sys.exit(1)

    pattern = SUITES[suite_name]["glob"]
    agents = sorted(TEST_AGENTS_DIR.glob(pattern))

    # Also check non-nested patterns
    if not agents:
        agents = sorted(TEST_AGENTS_DIR.glob(f"**/{pattern}"))

    return agents


def run_suite(soar_cli, suite_name, max_decisions=10000):
    """Run all tests in a suite, return list of stats dicts."""
    agents = discover_tests(suite_name)
    print(f"  Found {len(agents)} agents in {suite_name}", file=sys.stderr)

    results = []
    for agent in agents:
        stats = run_test(soar_cli, agent, max_decisions)
        results.append(stats)
        status_char = "." if stats["status"] == "success" else "X"
        print(status_char, end="", flush=True, file=sys.stderr)
    print(file=sys.stderr)

    return results


## ---------------------------------------------------------------------------
## VISIBILITY: raw diff — what changed, by how much, no judgment
## ---------------------------------------------------------------------------

def diff_results(base, candidate):
    """Produce a raw diff of two result sets. Facts only, no judgment.

    Returns a list of per-agent diffs. Each metric reports base value,
    candidate value, and delta. No classification, no direction, no
    pass/fail. The consumer decides what matters.
    """
    base_by_agent = {r["agent"]: r for r in base}
    cand_by_agent = {r["agent"]: r for r in candidate}
    all_agents = sorted(set(base_by_agent.keys()) | set(cand_by_agent.keys()))

    diffs = []
    for agent in all_agents:
        b = base_by_agent.get(agent)
        c = cand_by_agent.get(agent)

        entry = {"agent": agent}

        if not b:
            entry["presence"] = "new_in_candidate"
            if c:
                entry["candidate"] = {m: c[m] for m in DETERMINISTIC_METRICS if m in c}
            diffs.append(entry)
            continue
        if not c:
            entry["presence"] = "missing_in_candidate"
            diffs.append(entry)
            continue

        entry["presence"] = "both"
        entry["base_status"] = b.get("status")
        entry["candidate_status"] = c.get("status")
        entry["metrics"] = {}

        for metric in DETERMINISTIC_METRICS + TIMING_METRICS:
            bv = b.get(metric)
            cv = c.get(metric)
            if bv is None and cv is None:
                continue
            m = {"base": bv, "candidate": cv}
            if bv is not None and cv is not None:
                m["delta"] = cv - bv
                if bv != 0:
                    m["pct_change"] = round((cv - bv) / abs(bv) * 100, 1)
            entry["metrics"][metric] = m

        diffs.append(entry)

    return {"agents": diffs}


def print_diff(diff):
    """Print raw diff as a readable table. No judgment words."""
    changed_agents = []
    for entry in diff["agents"]:
        if entry.get("presence") == "new_in_candidate":
            changed_agents.append((entry["agent"], "NEW", {}))
            continue
        if entry.get("presence") == "missing_in_candidate":
            changed_agents.append((entry["agent"], "MISSING", {}))
            continue

        status_changed = entry["base_status"] != entry["candidate_status"]
        metric_changes = {}
        for m, info in entry.get("metrics", {}).items():
            if info.get("delta", 0) != 0:
                metric_changes[m] = info

        if status_changed or metric_changes:
            status = f"{entry['base_status']} -> {entry['candidate_status']}" if status_changed else ""
            changed_agents.append((entry["agent"], status, metric_changes))

    if not changed_agents:
        print("\nNo differences detected.")
        return

    print(f"\n{'Agent':<60} {'Metric':<25} {'Base':>10} {'Candidate':>10} {'Delta':>10} {'%':>8}")
    print("-" * 125)
    for agent, status, metrics in changed_agents:
        if status == "NEW":
            print(f"{agent:<60} {'(new agent)':<25}")
            continue
        if status == "MISSING":
            print(f"{agent:<60} {'(missing)':<25}")
            continue
        if status:
            print(f"{agent:<60} {'status':<25} {status}")
        for m, info in sorted(metrics.items()):
            bv = info.get("base", "—")
            cv = info.get("candidate", "—")
            delta = info.get("delta", "")
            pct = info.get("pct_change", "")
            delta_str = f"{delta:+}" if isinstance(delta, (int, float)) else ""
            pct_str = f"{pct:+.1f}%" if isinstance(pct, (int, float)) else ""
            print(f"{'':60} {m:<25} {str(bv):>10} {str(cv):>10} {delta_str:>10} {pct_str:>8}")


## ---------------------------------------------------------------------------
## DECISION: policy layer — maintainer configures what counts as pass/fail
## ---------------------------------------------------------------------------

# Default policy: lower is better for resource metrics, chunk count is neutral.
# Maintainers can override by providing a policy JSON file.
DEFAULT_POLICY = {
    "lower_is_better": [
        "decisions", "elaboration_cycles", "production_firings",
        "wm_max", "kernel_cpu_sec", "total_cpu_sec",
    ],
    "neutral": [
        "productions_chunks", "productions_user",
        "wm_current", "wm_mean",
    ],
    # Metrics not listed are ignored for pass/fail.
    # Status regressions (success -> timeout/error) always count.
}


def apply_policy(diff, policy=None):
    """Apply a policy to a raw diff. Returns judgment summary.

    This is the ONLY place that says 'regression' or 'improvement'.
    Separated from visibility so the maintainer controls the criteria.
    """
    if policy is None:
        policy = DEFAULT_POLICY

    lower_is_better = set(policy.get("lower_is_better", []))
    neutral = set(policy.get("neutral", []))

    regressions = []
    improvements = []
    neutral_changes = []

    for entry in diff["agents"]:
        agent = entry["agent"]

        if entry.get("presence") == "missing_in_candidate":
            regressions.append({"agent": agent, "reason": "agent missing in candidate"})
            continue

        # Status regression
        bs = entry.get("base_status")
        cs = entry.get("candidate_status")
        if bs == "success" and cs != "success":
            regressions.append({"agent": agent, "reason": f"status {bs} -> {cs}"})

        for metric, info in entry.get("metrics", {}).items():
            delta = info.get("delta", 0)
            if delta == 0:
                continue

            if metric in neutral:
                neutral_changes.append({
                    "agent": agent, "metric": metric,
                    "base": info["base"], "candidate": info["candidate"],
                })
            elif metric in lower_is_better:
                if delta > 0:
                    regressions.append({
                        "agent": agent, "metric": metric,
                        "base": info["base"], "candidate": info["candidate"],
                    })
                else:
                    improvements.append({
                        "agent": agent, "metric": metric,
                        "base": info["base"], "candidate": info["candidate"],
                    })

    return {
        "regressions": regressions,
        "improvements": improvements,
        "neutral_changes": neutral_changes,
        "pareto_pass": len(regressions) == 0 and len(improvements) > 0,
    }


def print_judgment(judgment):
    """Print policy-based judgment. Clearly labeled as policy output."""
    print("\n" + "=" * 60)
    print("POLICY JUDGMENT (maintainer-configured, not harness opinion)")
    print("=" * 60)

    if judgment["regressions"]:
        print(f"\nRegressions ({len(judgment['regressions'])}):")
        for r in judgment["regressions"]:
            if "metric" in r:
                print(f"  {r['agent']}: {r['metric']} {r['base']} -> {r['candidate']}")
            else:
                print(f"  {r['agent']}: {r['reason']}")

    if judgment["improvements"]:
        print(f"\nImprovements ({len(judgment['improvements'])}):")
        for r in judgment["improvements"]:
            print(f"  {r['agent']}: {r['metric']} {r['base']} -> {r['candidate']}")

    if judgment["neutral_changes"]:
        print(f"\nNeutral changes ({len(judgment['neutral_changes'])}):")
        for r in judgment["neutral_changes"]:
            print(f"  {r['agent']}: {r['metric']} {r['base']} -> {r['candidate']}")

    print(f"\nPareto check: {'PASS' if judgment['pareto_pass'] else 'FAIL'}")
    return 0 if not judgment["regressions"] else 1


def cmd_run(args):
    soar_cli = Path(args.soar)
    suites = args.suite if args.suite else list(SUITES.keys())

    all_results = {}
    for suite in suites:
        print(f"Running {suite}...", file=sys.stderr)
        results = run_suite(soar_cli, suite)
        all_results[suite] = results

    output = {
        "soar_binary": str(soar_cli),
        "suites": all_results,
    }

    out_path = Path(args.out)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w") as f:
        json.dump(output, f, indent=2)
    print(f"Results written to {out_path}", file=sys.stderr)


def cmd_compare(args):
    with open(args.base) as f:
        base = json.load(f)
    with open(args.candidate) as f:
        candidate = json.load(f)

    policy = DEFAULT_POLICY
    if args.policy:
        with open(args.policy) as f:
            policy = json.load(f)

    exit_code = 0
    for suite in sorted(set(list(base["suites"].keys()) + list(candidate["suites"].keys()))):
        b = base["suites"].get(suite, [])
        c = candidate["suites"].get(suite, [])
        if not b or not c:
            print(f"\n{suite}: missing from one side, skipping")
            continue

        print(f"\n{'='*60}")
        print(f"Suite: {suite}")
        print(f"{'='*60}")

        # Layer 1: VISIBILITY — raw facts
        diff = diff_results(b, c)
        print_diff(diff)

        # Layer 2: DECISION — policy judgment (optional)
        if not args.facts_only:
            judgment = apply_policy(diff, policy)
            code = print_judgment(judgment)
            if code != 0:
                exit_code = 1

    sys.exit(exit_code)


def main():
    parser = argparse.ArgumentParser(description="Soar per-module eval harness")
    sub = parser.add_subparsers(dest="command")

    run_p = sub.add_parser("run", help="Run test suites and capture stats")
    run_p.add_argument("--soar", default=str(SOAR_CLI_DEFAULT), help="Path to soar CLI")
    run_p.add_argument("--suite", nargs="*", help="Suite(s) to run (default: all)")
    run_p.add_argument("--out", default="results/module_eval.json", help="Output JSON")

    cmp_p = sub.add_parser("compare", help="Compare two result sets")
    cmp_p.add_argument("--base", required=True, help="Baseline JSON")
    cmp_p.add_argument("--candidate", required=True, help="Candidate JSON")
    cmp_p.add_argument("--policy", default=None, help="Policy JSON (default: built-in)")
    cmp_p.add_argument("--facts-only", action="store_true",
                       help="Show raw diff only, no policy judgment")

    args = parser.parse_args()
    if args.command == "run":
        cmd_run(args)
    elif args.command == "compare":
        cmd_compare(args)
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
