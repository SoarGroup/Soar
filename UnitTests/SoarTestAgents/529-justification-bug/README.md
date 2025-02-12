# Regression Test Agent for Issue #529

This simple agent triggers the creation of two duplicate justifications with o-support. Soar should apply the RHS changes both times, but it previously would only apply the changes once (though it did run the RHS functions both times).

The "succeeded" RHS function is called once we detect that the effect of the justification has occurred twice. If the bug is still present, this function will not be called and the agent will impasse instead.
