1. handle_numerical-operations.pl <input-kif>
	The output is <input-kif>.modified
2. python LoadKif.py <input-kif>.modified <input-kif>.modified.soar
3. handle_build_soar.pl <input-kif>.modified.soar > final.soar
4. Should source math-common.soar