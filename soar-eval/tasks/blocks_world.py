"""Generate random 3-block Blocks World tasks as Soar production rules.

A state is a set of ontop relations over {A, B, C, table}.
For 3 blocks there are 13 distinct legal states.
"""

import random
from itertools import permutations

BLOCKS = ["A", "B", "C"]
TABLE = "table"


def enumerate_states():
    """Return all legal 3-block states as frozensets of (top, bottom) pairs.

    Each block must be on exactly one thing. A block can be on the table
    or on another block, but only if no other block is on that block
    (i.e., the destination must be clear).
    """
    states = []
    # Each block is on something: another block or table
    # Constraint: no two blocks on the same block (but multiple on table is fine)
    options = BLOCKS + [TABLE]
    for a_on in options:
        if a_on == "A":
            continue
        for b_on in options:
            if b_on == "B":
                continue
            for c_on in options:
                if c_on == "C":
                    continue
                # Check: no two blocks on the same non-table thing
                supports = [a_on, b_on, c_on]
                block_supports = [s for s in supports if s != TABLE]
                if len(block_supports) != len(set(block_supports)):
                    continue
                # Check: no cycles (A on B on A)
                placement = {"A": a_on, "B": b_on, "C": c_on}
                if has_cycle(placement):
                    continue
                state = frozenset([("A", a_on), ("B", b_on), ("C", c_on)])
                states.append(state)
    return states


def has_cycle(placement):
    """Check if a placement dict has a cycle among blocks."""
    for start in BLOCKS:
        visited = set()
        current = start
        while current in placement and current != TABLE:
            if current in visited:
                return True
            visited.add(current)
            current = placement[current]
    return False


ALL_STATES = enumerate_states()


def generate_task_pair(rng):
    """Return (initial_state, goal_state) as two different states."""
    pair = rng.sample(ALL_STATES, 2)
    return pair[0], pair[1]


def generate_tasks(n_train, n_transfer, seed):
    """Generate training and transfer task pairs."""
    rng = random.Random(seed)
    train = [generate_task_pair(rng) for _ in range(n_train)]
    transfer = [generate_task_pair(rng) for _ in range(n_transfer)]
    return train, transfer


def state_to_soar(state, var_prefix="s"):
    """Convert a state (frozenset of (top, bottom)) to Soar WME creation actions."""
    lines = []
    ontop_vars = []
    block_vars = {}

    # Create block variables
    for block in BLOCKS:
        var = f"<block{block}>"
        block_vars[block] = var
        lines.append(f'   ({var} ^name {block} ^type block)')
    block_vars[TABLE] = "<table>"
    lines.append(f'   (<table> ^name table ^type table)')

    # Create ontop relations
    for i, (top, bottom) in enumerate(sorted(state)):
        var = f"<ontop{i+1}>"
        ontop_vars.append(var)
        lines.append(f'   ({var} ^top-block {block_vars[top]} ^bottom-block {block_vars[bottom]})')

    return ontop_vars, block_vars, lines


def task_to_soar_file(initial, goal, task_name):
    """Generate a Soar production that sets up a specific BW task.

    Returns the text of a .soar file with a single initialization production.
    """
    block_vars = {b: f"<block{b}>" for b in BLOCKS}
    block_vars[TABLE] = "<table>"

    # Block/table definitions (shared between initial and goal)
    block_defs = []
    for b in BLOCKS:
        block_defs.append(f'   ({block_vars[b]} ^name {b} ^type block)')
    block_defs.append(f'   (<table> ^name table ^type table)')

    # Initial ontop relations
    init_ontop_vars = []
    init_ontop_lines = []
    for i, (top, bottom) in enumerate(sorted(initial)):
        var = f"<ontop{i+1}>"
        init_ontop_vars.append(var)
        init_ontop_lines.append(f'   ({var} ^top-block {block_vars[top]} ^bottom-block {block_vars[bottom]})')

    # Goal ontop relations
    goal_ontop_vars = []
    goal_ontop_lines = []
    for i, (top, bottom) in enumerate(sorted(goal)):
        var = f"<dontop{i+1}>"
        goal_ontop_vars.append(var)
        goal_ontop_lines.append(f'   ({var} ^top-block {block_vars[top]} ^bottom-block {block_vars[bottom]})')

    ontop_refs = " ".join(init_ontop_vars)
    object_refs = " ".join(block_vars[b] for b in BLOCKS) + " <table>"
    dontop_refs = " ".join(goal_ontop_vars)

    # Use a fixed name so the harness can excise it between tasks
    propose_name = "eval*propose*initialize-blocks-world"
    apply_name = "eval*apply*initialize-blocks-world"

    prod = f"""sp {{{propose_name}
   (state <s> ^superstate nil
             -^name)
-->
   (<s> ^operator <o> +)
   (<o> ^name initialize-blocks-world)
}}

sp {{{apply_name}
   (state <s> ^operator.name initialize-blocks-world)
-->
   (<s> ^name blocks-world
        ^ontop {ontop_refs}
        ^object {object_refs}
        ^desired <ds>)
{chr(10).join(block_defs)}
{chr(10).join(init_ontop_lines)}
   (<ds> ^ontop {dontop_refs})
{chr(10).join(goal_ontop_lines)}
}}
"""
    return prod


def write_task_file(filepath, initial, goal, task_name):
    """Write a single task's initialization production to a .soar file."""
    content = task_to_soar_file(initial, goal, task_name)
    with open(filepath, "w") as f:
        f.write(content)
    return filepath


if __name__ == "__main__":
    # Verify state enumeration
    states = enumerate_states()
    print(f"Found {len(states)} legal 3-block states")
    for s in states:
        print(f"  {sorted(s)}")

    # Generate a sample task
    rng = random.Random(42)
    init, goal = generate_task_pair(rng)
    print(f"\nSample task:")
    print(f"  Initial: {sorted(init)}")
    print(f"  Goal:    {sorted(goal)}")
    print(f"\nSoar production:")
    print(task_to_soar_file(init, goal, "eval*task*seed42*train01*initialize"))
