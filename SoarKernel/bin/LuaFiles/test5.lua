-- Name: Test5
--
-- This is exercises the explain and backtracing code.
--
--
--==================================================
test5_agentname = "test5"


function run_backtrace_test(fn, iters, explain)
	printf("\nFile: Test5.lua.  Running test "..fn.."\n")
	--
	-- Here, "nil" is not really nil, but calling a function
	-- that does not exist is an equivalient enough behavior
	--
	createSoarPlayer(test5_agentname, "nil", "nil", fn, TRUE)
	psa = soar_cGetAgentByName( test5_agentname )

	--
	-- Load agent code
	--
	sourceProductionsFromFile( "TestProductions/"..fn )	
  
	for i = 0,iters do
		if (mod(i ,16) == 0) then printf("#") end
			print_to_screen(FALSE)
			
			soar_ExplainBacktraces("before-any-are-created")
			soar_cRun(1, TRUE, go_type_enum.GO_DECISION , soar_apiSlotType.NO_SLOT)
		
			--print_to_screen(TRUE)				
			if(isHalted(psa)) then 
			printf("\nHalted.\n") 
					
			break
		end
	end

	--do the explain thing
	soar_ExplainBacktraces("justification-1")
	soar_ExplainBacktraces("-1*d12*opnochange*1")
	soar_ExplainBacktraces("chunk-1*d12*opnochange*1")
	soar_ExplainBacktraces("non-exist")

	--Do the re-init to "Free" the chunks
	soar_cReInitSoar();
		
	closeFile()
	if(diff()) then
		rm(fn);
	else
		rm("junkie"..fn)
	end
   
	print_to_screen(TRUE)
	printf("\nending test "..fn.."\n")

	soar_cDestroyAgentByName(test5_agentname);
	
	soar_cQuit();
end

----------------------------------------------------
--
----------------------------------------------------
function test5()
   printf("Starting test5\n")

   setCompression(nil)

   long_run = 1024
   short_run = 32
   debug_run = 0

	
   if(1) then
      run_backtrace_test("blocks-world-backtrace.soar",short_run, 1) 
   end

   rm("END")

   printf("\nDone with test5.\n")
end
