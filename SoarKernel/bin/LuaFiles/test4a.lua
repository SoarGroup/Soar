-- Name: test4a
--
-- This is a simple test of the "run forever" option.
--
--
--==================================================
test4a_agentname = "test4a"


function run_forever_test(fn, iters, go_type)
   printf("\nFile: Test4a.lua.  Running test "..fn.."\n")
   --
   -- Here, "nil" is not really nil, but calling a function
   -- that does not exist is an equivalient enough behavior
   --

   createSoarPlayer(test4a_agentname, "nil", "nil", fn, TRUE)
   psa = soar_cGetAgentByName( test4a_agentname )

	soar_ecBuildInfo();
   --
   -- Load agent code
   --
  sourceProductionsFromFile( "TestProductions/"..fn )	


   for i = 0,iters do
		if (mod(i ,16) == 0) then printf("#") end
		print_to_screen(TRUE)
		printf(go_type) 
        --print_to_screen(FALSE)
			
			if (go_type == "GO_DECISION") then t= go_type_enum.GO_DECISION 
			elseif (go_type == "GO_ELABORATION") then t = go_type_enum.GO_ELABORATION
			elseif (go_type == "GO_STATE") then t = go_type_enum.GO_STATE
			elseif (go_type == "GO_PHASE") then t = go_type_enum.GO_PHASE
			elseif (go_type == "GO_OPERATOR") then t = go_type_enum.GO_OPERATOR
			elseif (go_type == "GO_OUTPUT") then t= go_type_enum.GO_OUTPUT
			elseif (go_type == "GO_SLOT") then t = go_type_enum.GO_SLOT 
			end
			--This runs forever
			soar_cRun(-1, TRUE, t , soar_apiSlotType.NO_SLOT)
			

		if(isHalted(psa)) then 
			printf("\nHalted.\n") 
			break
		end
   end

	soar_cReInitSoar()
sourceProductionsFromFile( "TestProductions/"..fn )	


   for i = 0,iters do
		if (mod(i ,16) == 0) then printf("#") end
		print_to_screen(TRUE)
		printf(go_type) 
        --print_to_screen(FALSE)
			
			if (go_type == "GO_DECISION") then t= go_type_enum.GO_DECISION 
			elseif (go_type == "GO_ELABORATION") then t = go_type_enum.GO_ELABORATION
			elseif (go_type == "GO_STATE") then t = go_type_enum.GO_STATE
			elseif (go_type == "GO_PHASE") then t = go_type_enum.GO_PHASE
			elseif (go_type == "GO_OPERATOR") then t = go_type_enum.GO_OPERATOR
			elseif (go_type == "GO_OUTPUT") then t= go_type_enum.GO_OUTPUT
			elseif (go_type == "GO_SLOT") then t = go_type_enum.GO_SLOT 
			end
			
			soar_cRun(-1, TRUE, t , soar_apiSlotType.NO_SLOT)
			

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

   soar_cDestroyAgentByName(test4a_agentname);
   soar_cQuit();

end

----------------------------------------------------
--
----------------------------------------------------
function test4a()
   printf("Starting test4a\n")

   setCompression(nil)

   long_run = 1024
   short_run = 32
   debug_run = 0
	

	
   if(1) then
		--1 cycle
	   run_forever_test("waterjug.soar",debug_run, "GO_DECISION")
 	   run_forever_test("waterjug.soar",debug_run, "GO_ELABORATION")
 	   run_forever_test("waterjug.soar",debug_run, "GO_STATE")
	   run_forever_test("waterjug.soar",debug_run, "GO_PHASE")
	   run_forever_test("waterjug.soar",debug_run, "GO_OPERATOR")
		run_forever_test("waterjug.soar",debug_run, "GO_OUTPUT")
	   run_forever_test("waterjug.soar",debug_run, "GO_SLOT")
		
   end

   rm("END")

   printf("\nDone with test4a.\n")
end
