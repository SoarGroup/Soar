-- Name: test7
--
-- This is a simple test of the pwatch. And it uses
-- all of the rhsfun_math functions.
--
--
--==================================================
test7_agentname = "test7"


function run_pwatch_test(fn, iters, operand2Mode, mode)
   printf("\nFile: Test7.lua.  Running test "..fn.."\n")
   --
   -- Here, "nil" is not really nil, but calling a function
   -- that does not exist is an equivalient enough behavior
   --
   createSoarPlayer(test7_agentname, "nil", "nil", fn, operand2Mode)
   psa = soar_cGetAgentByName( test7_agentname )
   
   
   --soar_cSetOperand2(operand2Mode)
   soar_OSupportMode(mode)
   soar_ecBuildInfo();

   --
   -- Load agent code
   --
	sourceProductionsFromFile( "TestProductions/"..fn )	

   for i = 0,iters do
		if (iters == 0) then print_to_screen(TRUE) -- debug_run
		else 
			print_to_screen(FALSE) 
			if (mod(i ,16) == 0) then printf("#") end
		end
		   print_to_screen(TRUE)
			  soar_Pwatch("-off", "")
			  soar_cRun(1, TRUE, go_type_enum.GO_DECISION, soar_apiSlotType.NO_SLOT)
			  soar_Pwatch("-on", "")
			  soar_cRun(1, TRUE, go_type_enum.GO_DECISION, soar_apiSlotType.NO_SLOT)
			  soar_Pwatch("-off", "apply*abs")
			  soar_cRun(1, TRUE, go_type_enum.GO_DECISION, soar_apiSlotType.NO_SLOT)
			  soar_Pwatch("-on", "apply*abs")
           soar_cRun(1, TRUE, go_type_enum.GO_DECISION, soar_apiSlotType.NO_SLOT)
		print_to_screen(TRUE)
		

		if(isHalted(psa)) then 
			printf("\nHalted.\n") 
			break
		end
   end

   closeFile()
   if(diff()) then
	  rm(fn);
   else
	  rm("junkie"..fn)
   end
   
   printf("\nending test "..fn.."\n")

   soar_cDestroyAgentByName(test7_agentname);
   soar_cQuit();
end

----------------------------------------------------
--
----------------------------------------------------
function test7()
   printf("Starting test7.\n")

   setCompression(nil)

   long_run = 1024
   short_run = 32
   debug_run = 0

	
   if(1) then
		run_pwatch_test("rhsfun_math.soar",debug_run, TRUE, 0)
	  
	end

   rm("END")

   printf("\nDone with test7.\n")
end
