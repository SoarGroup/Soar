# Soar Eval Policy

`policy.json` controls Layer 2 of the eval harness: the judgment layer that decides whether raw metric differences count as improvements, regressions, or noise.

Layer 1 reports facts only: baseline value, candidate value, delta, and percent change. This policy file defines how Layer 2 interprets those facts.

## Fields

### `lower_is_better`

Metrics where a lower candidate value is an improvement and a higher value is a regression.

- `decisions`, `elaboration_cycles`, `production_firings`, `wm_max` — resource-use metrics
- `kernel_cpu_sec`, `total_cpu_sec` — timing metrics (subject to noise thresholds below)

### `neutral`

Metrics reported but not judged directionally. A change is flagged as "changed" rather than "improved" or "regressed."

- `productions_chunks`, `productions_user` — chunk count direction depends on context
- `wm_current`, `wm_mean` — informational

### `timing_noise_floor`

Absolute timing delta (seconds) below which changes are ignored as noise. Default: `0.005` (5ms).

### `timing_relative_threshold`

Relative timing delta (percent) below which changes are ignored as noise. Default: `1.0` (1%).

A timing change is flagged only when it exceeds **both** thresholds. This biases toward review rather than automatic pass.

## Customization

Stricter (flag more): lower the thresholds.
Less sensitive (flag less): raise the thresholds.
Change metric direction: move names between `lower_is_better` and `neutral`.

The maintainer owns this file. Contributors see the raw numbers (Layer 1). The maintainer decides what matters (Layer 2).
