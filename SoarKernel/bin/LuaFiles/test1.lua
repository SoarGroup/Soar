-- Name: Test1
--
-- This is a simple test of the input link.
--
--
--==================================================
test1_agentname = "test1"


----------------------------------------------------
--
----------------------------------------------------
function myOutputFunction(agent, cb_data, call_data)
end


----------------------------------------------------
--
----------------------------------------------------
function myInputFunction(agent, cb_data, call_data)
	--agent = soar_cGetAgentByName(test1_agentname)
	--id = soar_cGetAgentOutputLinkId(agent)
end

----------------------------------------------------
--
----------------------------------------------------
function test1()
   printf("Starting test1.\n")
   
   --
   -- Here, "nil" is not really "nil", but calling a function
   -- that does not exist is an equivalient enough behavior
   --
   createSoarPlayer(test1_agentname, "nil", "nil", "test1.log", TRUE)

   --
   -- Load agent code
   --
   sourceProductionsFromFile( "TestProductions/test.soar" )	

   --soar_ecCaptureInput("captured_input.s")

   soar_ecBuildInfo()
   --soar_ecExcludedBuildInfo()

   --
   -- Add 10 elements to I3
   --
   for i = 0,10 do
		print_to_screen(TRUE)
		soar_cRun(1, TRUE, go_type_enum.GO_DECISION, soar_apiSlotType.NO_SLOT)
        rv, new_wme = soar_cAddWme("I3", "xyy", "yxx", nil)
        tprint("test")
        soar_memdump();
   end

   soar_Stats("-memory");
	
   --quit while agents still exist to exercise error code
   soar_cQuit();

   soar_cDestroyAgentByName(test1_agentname);
   soar_cQuit();
   printf("Done with test1.\n")
end
