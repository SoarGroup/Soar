# GDS support regression cases

Build `kernel tests` with SCons, or a CMake `Release-test` preset. Run from the
output directory containing UnitTests and SoarUnitTests:

```
UnitTests -t testGDSCloneSupport -t testGDSIndependentDependencies -t testGDSPromotedSupport
```

Each test explicitly exercises `chunk never` and `chunk always` in fresh
kernels. Fresh kernels keep semantic memory, identifier counters and learned
productions independent between controls.

* `testGDSCloneSupport`: standalone ASCII semantic-memory retrieval reduced
  from the original Rosie-based reproducer. Asserts the returned format,
  handle, feature and type after 40 decisions. No Rosie installation, Python,
  external databases or cyclic concept graph is needed. The fixture retains
  the BSD notice for the reused Rosie rules.
* `testGDSIndependentDependencies`: simultaneous, A-first and B-first inputs;
  withdraw A, withdraw B, or introduce an unrelated input. Both premises must
  appear in the GDS before mutation. Either withdrawal must remove the original
  substate; unrelated input must preserve it. Includes shared-result support
  controls: 22 cases total over both chunking modes.
* `testGDSPromotedSupport`: cleanup of a locally supported promoted persistent
  effect must preserve its producer's parent substate. Withdrawal of the true
  external premise must still remove that state. A separate i-supported
  promoted entailment must remain valid and retract on premise withdrawal.

Also run the existing `Teach_Soar_90_Games` and full upstream test suite. That
test exposed an additional instance of the clone-selection defect after
dependency registration was restored: enumerating upper-level o-supported
clones of a local result queued level-1/2/3 justifications with no GDS even
though a level-4 clone was available. Prefer that local clone when present;
retain the previous upper-level behavior when no such clone exists. Do not
silence a null GDS or allocate artificial GDSs for those justifications.

The promoted-effect classification correction applies only to same-level
o-supported clones. Unconditionally assigning the clone level for i-support
is unsafe and is intentionally not used.

The existing preference-deallocation fixture now halts at exactly decision 4
instead of 6: removing its own promoted `bar` effect no longer erroneously
retracts S2 and requires rebuilding it. The test asserts the new exact count,
the final count of 2, absence of `bar`, preservation of S2's `hello world`,
and retains its original reinitialization/reference-count checks.

## Scope of validation

The original Windows integration used 23 acceptance stages, eight exact Rosie
output comparisons and 28 focused GDS cases. Full Rosie acceptance also used
independent compatibility rules, including acquisition-proposal deduplication;
those rules are not C++ fixes and are not part of this contribution. Those
historical results must not be described as tests of the later clone-selection
refinement or as all-platform/upstream-CI validation. Consult the PR for results
on its actual submitted revision and for CI status.
