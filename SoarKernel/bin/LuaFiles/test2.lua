-- Name: Test2
--
-- This is a simple test of the input link.
--
--
--==================================================
test2_agentname = "test2"


----------------------------------------------------
--
----------------------------------------------------
function test2()
   printf("Starting test2.\n")

   --
   -- Here, "nil" is not really "nil", but calling a function
   -- that does not exist is an equivalient enough behavior
   --
   createSoarPlayer(test2_agentname, "nil", "nil", "test2.log", TRUE)
   psa = soar_cGetAgentByName( test2_agentname )

   --
   -- Load agent code
   --
   sourceProductionsFromFile( "TestProductions/waterjug.soar" )	

   --soar_ecCaptureInput("captured_input.s")


   --
   -- Add 10 elements to S1
   --
   id="S1"
   for i = 0,10 do
		soar_cRun(1, TRUE, go_type_enum.GO_DECISION, soar_apiSlotType.NO_SLOT)
		x = "ident_"..i

        rv, new_wme = soar_cAddWme(id, x, "val_"..(i+10), nil)

        if(id == "I3") then id = "S1"
        elseif(id == "I2") then id = "I3"
        elseif(id == "S1") then id = "I2" end

        soar_memdump();
   end

   soar_cDestroyAgentByName(test2_agentname);
   soar_cQuit();
   printf("Done with test2.\n")
end


