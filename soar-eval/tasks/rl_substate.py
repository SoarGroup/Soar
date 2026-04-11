"""Generate RL-substate tasks.

Each task sets a goal-value (high or low) and max-episodes.
The agent runs max-episodes episodes of RL-driven substate decisions.
With enough episodes, RL converges and PR #577's gate enables chunking.
"""

import random
import os


def generate_task(goal_value, max_episodes=20):
    """Generate a task production."""
    return f"""sp {{eval*propose*init-rl-task
   (state <s> ^superstate nil -^name)
-->
   (<s> ^operator <o> +)
   (<o> ^name init-rl-task)
}}

sp {{eval*apply*init-rl-task
   (state <s> ^operator.name init-rl-task)
-->
   (<s> ^name rl-substate-task ^goal-value {goal_value}
        ^episode 1 ^max-episodes {max_episodes})
}}
"""


def generate_tasks(n_train, n_transfer, seed, max_episodes=20):
    """Generate training and transfer task goal lists."""
    rng = random.Random(seed)
    train = [rng.choice(["high", "low"]) for _ in range(n_train)]
    transfer = [rng.choice(["high", "low"]) for _ in range(n_transfer)]
    return train, transfer


def write_task_file(filepath, goal_value, task_name, max_episodes=20):
    """Write a task .soar file."""
    content = generate_task(goal_value, max_episodes)
    with open(filepath, "w") as f:
        f.write(content)
    return filepath


if __name__ == "__main__":
    train, transfer = generate_tasks(6, 6, seed=42)
    print(f"Train goals: {train}")
    print(f"Transfer goals: {transfer}")
    print()
    print(generate_task("high", 20))
"""
"""
