-- Name: Test6
--
-- This is exercises the rete-net routines.
--
--
--==================================================
test6_agentname = "test6"


function run_rete_net_test(fn, iters)
   printf("\nFile: Test6.lua.  Running test "..fn.."\n")
   --
   -- Here, "nil" is not really nil, but calling a function
   -- that does not exist is an equivalient enough behavior
   --
   createSoarPlayer(test6_agentname, "nil", "nil", fn, TRUE)
   psa = soar_cGetAgentByName( test6_agentname )

	--
	-- Load agent code
	-- And run it as normal
	--
	sourceProductionsFromFile( "TestProductions/"..fn )	
   for i = 0,iters do
		if (mod(i ,16) == 0) then printf("#") end
			print_to_screen(FALSE)
			
			soar_cRun(1, TRUE, go_type_enum.GO_DECISION , soar_apiSlotType.NO_SLOT)
		
		print_to_screen(TRUE)				
		if(isHalted(psa)) then 
		printf("\nHalted.\n") 
			
		break
		end
   end
	printf("\nIt should run normally.\n")

	--
	-- Save the rete-net it to a file
	--
	soar_cReInitSoar()
	soar_cSaveReteNet( "myretenet" )

	--
	-- Remove all productions from memory
	-- And attempt to Run it - it should not run anything!
	--
	soar_cExciseAllProductions("-all")
   for i = 0,iters do
		if (mod(i ,16) == 0) then printf("#") end
			print_to_screen(FALSE)
			
			soar_cRun(1, TRUE, go_type_enum.GO_DECISION , soar_apiSlotType.NO_SLOT)
		
		print_to_screen(TRUE)				
		if(isHalted(psa)) then 
		printf("\nHalted.\n") 
			
		break
		end
   end
	printf("\nHopefully nothing ran.\n")


	--
	-- Restore it from the rete-net file
	-- Run it from the rete-net
	--
	soar_cReInitSoar()
	soar_cLoadReteNet( "myretenet" )
   for i = 0,iters do
		if (mod(i ,16) == 0) then printf("#") end
			print_to_screen(FALSE)
			
			soar_cRun(1, TRUE, go_type_enum.GO_DECISION , soar_apiSlotType.NO_SLOT)
		
		print_to_screen(TRUE)				
		if(isHalted(psa)) then 
		printf("\nHalted.\n") 
			
		break
		end
   end
	printf("\nNow, it should run as normal.\n")

   closeFile()
   if(diff()) then
	  rm(fn);
   else
	  rm("junkie"..fn)
   end
   
	print_to_screen(TRUE)
   printf("\nending test "..fn.."\n")

   soar_cDestroyAgentByName(test6_agentname);
	
   soar_cQuit();

end

----------------------------------------------------
--
----------------------------------------------------
function test6()
   printf("Starting test6\n")

   setCompression(nil)

   long_run = 1024
   short_run = 32
   debug_run = 0

	
   if(1) then
		run_rete_net_test("waterjug.soar",debug_run)
		run_rete_net_test("towers-of-hanoi.soar", debug_run)
		run_rete_net_test("blocks-world.soar", debug_run)
		run_rete_net_test("waterjug2.soar", debug_run)
		run_rete_net_test("simple.soar", long_run)
		run_rete_net_test("halt.soar", long_run)
   end

   rm("END")

   printf("\nDone with test6.\n")
end
