-- Name: Common.lua
--
--
--
--

TRUE = 1
FALSE = nil


----------------------------------------------------
--
----------------------------------------------------
function createSoarPlayer(agentname, inputfn, outputfn, logfile, op2mode)
   soar_cInitializeSoar()

   soar_cCreateAgent(agentname ,logfile, op2mode)

   psa = soar_cGetAgentByName( agentname)
   soar_cSetCurrentAgent( psa )


   --soar_cAddInputFunction( psa,  inputfn, NIL,  NIL, "input-link" )
   --soar_cAddOutputFunction(psa,  outputfn, NIL,  NIL, "output-link" )
end
