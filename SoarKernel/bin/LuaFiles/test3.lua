-- Name: Test3
--
-- This is a simple test of the input link.
--
--
--==================================================
test3_agentname = "test3"


function run_test(fn, iters, operand2Mode, mode)
   printf("\nFile: Test3.lua.  Running test "..fn.."\n")
   --
   -- Here, "nil" is not really nil, but calling a function
   -- that does not exist is an equivalient enough behavior
   --
   createSoarPlayer(test3_agentname, "nil", "nil", fn, operand2Mode)
   psa = soar_cGetAgentByName( test3_agentname )
   
   
   --soar_cSetOperand2(operand2Mode)
   soar_OSupportMode(mode)
   --soar_ecBuildInfo();

   --
   -- Load agent code
   --
  sourceProductionsFromFile( "TestProductions/"..fn )	


   for i = 0,iters do
		if (iters == 1) then print_to_screen(TRUE) -- debug_run
		else 
			print_to_screen(FALSE) 
			if (mod(i ,16) == 0) then printf("#") end
		end
		   print_to_screen(TRUE)
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
	  rm(""..fn)
   end
   
   printf("ending test "..fn.."\n")

   soar_cDestroyAgentByName(test3_agentname);
   soar_cQuit();
end

----------------------------------------------------
--
----------------------------------------------------
function test3()
   printf("Starting test3.\n")

   setCompression(nil)

   long_run = 1024
   short_run = 32
   debug_run = 0

	
   if(1) then
	
	--Jens, 
	--The operand mode TRUE is SOAR 8 mode and FALSE is SOAR 7 mode
	--Do we need/want to test SOAR7 mode?

 	   run_test("waterjug.soar", short_run, TRUE, 0)
 	   run_test("waterjug.soar", short_run, TRUE, 1)
 	   run_test("waterjug.soar", short_run, TRUE, 2)
 	   run_test("waterjug.soar", short_run, TRUE, 3)
	   run_test("waterjug.soar", short_run, FALSE, 0)
 	   run_test("waterjug.soar", short_run, FALSE, 1)
 	   run_test("waterjug.soar", short_run, FALSE, 2)
 	   run_test("waterjug.soar", short_run, FALSE, 3)
	   run_test("waterjug.soar",long_run, TRUE, 0)
	   run_test("waterjug.soar",long_run, TRUE, 1)
	   run_test("waterjug.soar",long_run, TRUE, 2)
	   run_test("waterjug.soar",long_run, TRUE, 3)
       run_test("simple.soar",long_run, TRUE, 0)
	   run_test("simple.soar",long_run, TRUE, 1)
	   run_test("simple.soar",long_run, TRUE, 2)
	   run_test("simple.soar",long_run, TRUE, 3)
       run_test("halt.soar", long_run, TRUE, 0)
	   run_test("towers-of-hanoi.soar", long_run, TRUE, 0)
	   run_test("towers-of-hanoi-fast.soar",long_run, TRUE, 0)
	   run_test("towers-of-hanoi-fast.soar",long_run, TRUE, 0)
	   run_test("towers-of-hanoi-no-ops.soar",long_run, TRUE, 0)
	   run_test("towers-of-hanoi-recur.soar",long_run, TRUE, 0)
			--selection.soar ??
			--run_test("selection.soar",short_run, TRUE, 0)
	   run_test("waterjug2.soar",long_run, TRUE, 0)
	   run_test("waterjug2.soar",long_run, TRUE, 0)
	   run_test("blocks-world.soar",long_run, TRUE, 0)
	   run_test("blocks-opsub.soar",long_run, TRUE, 0)
	   run_test("blocks-opsub.soar",long_run, TRUE, 1)
	   run_test("blocks-opsub.soar",long_run, TRUE, 2)
	   run_test("blocks-opsub.soar",long_run, TRUE, 3)
	   
	   -- new examples
	   --run_test("mac.soar",long_run, TRUE, 0)
	   --run_test("mac2.soar",long_run, TRUE, 0)
	   --run_test("eight-puzzle.soar",long_run, TRUE, 0)
	   --run_test("load.soar",long_run, TRUE, 0)


	   --From Bob Wray - using main.tcl right now
	   --run_test("test1.soar",debug_run, TRUE, 0)
	   --run_test("test1.soar",debug_run, TRUE, 0)
	   --run_test("test2.soar",debug_run, TRUE, 0)
	   --run_test("test3.soar",debug_run, TRUE, 0)
	   --run_test("test4.soar",debug_run, TRUE, 0)
	   --run_test("test5.soar",debug_run, TRUE, 0)
	   --run_test("test6.soar",debug_run, TRUE, 0)
	   --run_test("test7.soar",debug_run, TRUE, 0)
	   --run_test("test-chunk1.soar",debug_run, TRUE, 0)
	   --run_test("operand.i.i.chunk.soar",debug_run, TRUE, 0)
	   --run_test("choice.soar",debug_run, TRUE, 0)
	   --run_test("choice-bug.soar",debug_run, TRUE, 0)
	   --run_test("chunk2-problem.soar",debug_run, TRUE, 0)
	   --run_test("chunk-backtrace-persist.soar",debug_run, TRUE, 0)
	   --wait.soar needs input link
	   --run_test("wait.soar",debug_run, TRUE, 0)
	   --Why doesn't this do anything?
	   --run_test("operand-simple0.soar",3, TRUE, 0)
	   --run_test("operand-random.soar",debug_run, TRUE, 0)
   end

   rm("END")

   printf("\nDone with test3.\n")
end
