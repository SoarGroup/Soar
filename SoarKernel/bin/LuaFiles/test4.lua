-- Name: Test4
--
-- This is a simple test of the different run options.
--
--
--==================================================
test4_agentname = "test4"


function run_options_test(fn, iters, go_type)

   printf("\nFile: Test4.lua.  Running test "..fn.."\n")
   --
   -- Here, "nil" is not really nil, but calling a function
   -- that does not exist is an equivalient enough behavior
   --

   createSoarPlayer(test4_agentname, "nil", "nil", fn, TRUE)
   psa = soar_cGetAgentByName( test4_agentname )

	soar_ecBuildInfo();
   --
   -- Load agent code
   --
  sourceProductionsFromFile( "TestProductions/"..fn )	


   for i = 0,iters do
		if (mod(i ,16) == 0) then printf("#") end
		print_to_screen(TRUE)
		printf(go_type) 
        print_to_screen(FALSE)
			
			if (go_type == "GO_DECISION") then t= go_type_enum.GO_DECISION 
			elseif (go_type == "GO_ELABORATION") then t = go_type_enum.GO_ELABORATION
			elseif (go_type == "GO_STATE") then t = go_type_enum.GO_STATE
			elseif (go_type == "GO_PHASE") then t = go_type_enum.GO_PHASE
			elseif (go_type == "GO_OPERATOR") then t = go_type_enum.GO_OPERATOR
			elseif (go_type == "GO_OUTPUT") then t= go_type_enum.GO_OUTPUT
			end
	
			if (go_type == "GO_SLOT") then
				--first run 1 decision to create a STATE
				soar_cRun(1, TRUE, go_type_enum.GO_DECISION, soar_apiSlotType.NO_SLOT)
				--then attempt to run the others
				soar_cRun(1, TRUE, go_type_enum.GO_SLOT, soar_apiSlotType.STATE_SLOT)
				soar_cRun(1, TRUE, go_type_enum.GO_SLOT, soar_apiSlotType.OPERATOR_SLOT)
				soar_cRun(1, TRUE, go_type_enum.GO_SLOT, soar_apiSlotType.SUPERSTATE_SLOT)
				soar_cRun(1, TRUE, go_type_enum.GO_SLOT, soar_apiSlotType.SUPEROPERATOR_SLOT)
				soar_cRun(1, TRUE, go_type_enum.GO_SLOT, soar_apiSlotType.SUPERSUPERSTATE_SLOT)
				soar_cRun(1, TRUE, go_type_enum.GO_SLOT, soar_apiSlotType.SUPERSUPEROPERATOR_SLOT)
			else
				soar_cRun(1, TRUE, t , soar_apiSlotType.NO_SLOT)
			end
  	 
		if(isHalted(psa)) then 
			printf("\nHalted.\n") 
			break
		end
   end


	print_to_screen(TRUE)	
   closeFile()
   if(diff()) then
	  rm(fn);
   else
	  rm("junkie"..fn)
   end
   
   printf("\nending test "..fn.."\n")

   soar_cDestroyAgentByName(test4_agentname);
   soar_cQuit();

end

----------------------------------------------------
--
----------------------------------------------------
function test4()
   printf("Starting test4\n")

   setCompression(nil)

   long_run = 1024
   short_run = 32
   debug_run = 0
	
	

	
   if(1) then
		--1 cycle
	   run_options_test("waterjug.soar",debug_run, "GO_DECISION")
 	   run_options_test("waterjug.soar",debug_run, "GO_ELABORATION")
 	   run_options_test("waterjug.soar",debug_run, "GO_STATE")
	   run_options_test("waterjug.soar",debug_run, "GO_PHASE")
	   run_options_test("waterjug.soar",debug_run, "GO_OPERATOR")
		run_options_test("waterjug.soar",debug_run, "GO_OUTPUT")
	   run_options_test("waterjug.soar",debug_run, "GO_SLOT")

		-- n cycles
		run_options_test("waterjug.soar",short_run, "GO_DECISION")
		run_options_test("waterjug.soar",short_run, "GO_ELABORATION")
 	   run_options_test("waterjug.soar",short_run, "GO_STATE")
	   run_options_test("waterjug.soar",short_run, "GO_PHASE")
	   run_options_test("waterjug.soar",short_run, "GO_OPERATOR")
		run_options_test("waterjug.soar",short_run, "GO_OUTPUT")
	   run_options_test("waterjug.soar",short_run, "GO_SLOT")
		

   end

   rm("END")

   printf("\nDone with test4.\n")
end
