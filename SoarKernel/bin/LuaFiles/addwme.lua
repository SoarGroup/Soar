-- Name: AddWme
--
-- This is a complete test of the addWme call.
--
--
--==================================================
addwme_agentname = "addwme_agent"


----------------------------------------------------
--
----------------------------------------------------
function myOutputFunction(agent, cb_data, call_data)
end


----------------------------------------------------
--
----------------------------------------------------
function myInputFunction(agent, cb_data, call_data)
	--agent = soar_cGetAgentByName(addwme_agentname)
	--id = soar_cGetAgentOutputLinkId(agent)
end

----------------------------------------------------
--
----------------------------------------------------
function checkfailures(expectedval, actualval)

	if (actualval == expectedval) then 
		--printf("PASS\n")
	else
		failures = (failures + 1) 
		printf("*** FAILURE: ")
		printf("ActualVal = ")
		printf("%d", actualval)
		printf("   ExpectedVal = ")
		printf("%d", expectedval)
		printf("\n")
	end
	
end
----------------------------------------------------
--
----------------------------------------------------
function test_add(szId, szAttr, szValue, accept, expected)
	rv, actual = soar_cAddWme(szId, szAttr, szValue, accept)
	if (accept) then
		safe_accept = 1;
	else
		safe_accept = 0;
	end
	if ((expected > 0 and actual <= 0) or (expected < 0 and actual ~= expected)) then
		tprint("\nFAILURE: soar_cAddWme(" .. szId .. ", " .. szAttr .. ", " .. szValue .. ", " .. safe_accept .. ")")
		tprint("\nexpected = " .. expected .. " actual = " .. actual .. "\n")
	end
	return rv, actual
end
----------------------------------------------------
--
----------------------------------------------------
function test_remove(num, expected)
	actual = soar_cRemoveWmeUsingTimetag(num)
	if (actual ~= expected) then
		tprint("\nFAILURE: soar_cRemoveWmeUsingTimetag(" .. num .. ")")
		tprint("\nexpected = " .. expected .. " actual = " .. actual .. "\n")
	end
end
----------------------------------------------------
--
----------------------------------------------------

function AddWmeTest()
   printf("Starting AddWmeTest.\n")
	failures = 0
   --
   -- Here, "nil" is not really "nil", but calling a function
   -- that does not exist is an equivalient enough behavior
   --
   createSoarPlayer(addwme_agentname, "nil", "nil", "addwme.log", TRUE)

   --
   -- Load agent code
   --
   sourceProductionsFromFile( "TestProductions\\test.soar" )	

   --soar_ecCaptureInput("captured_input.s")
	
   soar_ecBuildInfo();
   --soar_ecExcludedBuildInfo();
   --
   -- Add 10 elements to I3
   --
	expectedtimetag = 13

	x,y = y,x


	--Do at least 6 iterations to check each phase
   for i = 0, 6 do
		print_to_screen(TRUE)
		soar_cRun(1, TRUE, go_type_enum.GO_PHASE, soar_apiSlotType.NO_SLOT)
		  --printf("\n soar_cAddWme \n")
		  
		  --VALID CONDITIONS
		  rv, new_wme = test_add("I3", "stringattr", "stringval", nil, 1)
		  test_remove(new_wme, 0);
		  rv, new_wme = test_add("I3", "string_attr_val_w_pref", "yxx", "+", 1) 
		  test_remove(new_wme, 0);
		  rv, new_wme = test_add("I3", "^floatval", "2.255", "+", 1)
		  test_remove(new_wme, 0);
		  rv, new_wme = test_add("I3", "negfloatval", "-2.255", "+", 1) 
		  test_remove(new_wme, 0);
		  rv, new_wme = test_add("I3", "less0val", "-.001", nil, 1) 
		  test_remove(new_wme, 0);
		  rv, new_wme = test_add("I3", "largeintval", "123456789", "+", 1) 
		  test_remove(new_wme, 0);
		  rv, new_wme = test_add("I3", "prefer", "yxx", "-", 1) 
		  test_remove(new_wme, 0);
		  rv, new_wme = test_add("I3", "*", "NewID", nil, 1) 
		  test_remove(new_wme, 0);
		  rv, new_wme = test_add("I3", "NewVal", "*", nil, 1) 
		  test_remove(new_wme, 0);
		  rv, new_wme = test_add("I3", "StateValue", "<s>", nil, 1)
		  test_remove(new_wme, 0);
		  rv, new_wme = test_add("I3", "<s>", "StateAttr", nil, 1)
		  test_remove(new_wme, 0);
		  rv, new_wme = test_add("I3", "VerticalBarVal", "|1|", nil, 1)
		  test_remove(new_wme, 0);
		  rv, new_wme = test_add("I3", "|1.2|", "VerticalBarAttr", nil, 1)
		  test_remove(new_wme, 0);
		  rv, new_wme = test_add("I3", "1.1", "floatattr", nil, 1)
		  test_remove(new_wme, 0);
		  rv, new_wme = test_add("I3", "1", "|\"In Quotes\"|", nil, 1)
		  test_remove(new_wme, 0);
		  rv, new_wme = test_add("I3", "whatsthelargeststringthatisallowed567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890?", "stringval", nil, 1) 
		  test_remove(new_wme, 0);
		  
		  
		  --ERROR CONDITIONS
		  --This wme is created even though there is an error
		  rv, new_wme = test_add("I3", "12345678901234567890", "inttoolarge", nil, -2) 
		  test_remove(new_wme, -1);
	  
		  rv, new_wme = test_add("badid", "xyy", "0", nil, -1) 
		  test_remove(new_wme, -1);

		  rv, new_wme = test_add("I3", "space in attr", "1", nil, -2) 
		  test_remove(new_wme, -1);
		  
		  rv, new_wme = test_add("I3", "<z>", "<BadVarAttr>", nil, -2)
		  test_remove(new_wme, -1);
			  
		  rv, new_wme = test_add("I3", "BadVarValue", "<z>", nil, -3)
		  test_remove(new_wme, -1);

		  rv, new_wme = test_add("I3", "1", "space in value", nil, -3) 
		  test_remove(new_wme, -1);
		  
		  soar_memdump();
	end
	print_to_screen(TRUE);
	soar_Stats("-memory");
	printf("%d", new_wme)

		
	--quit while agents still exist to exercise error code
	soar_cQuit();

	soar_cDestroyAgentByName(addwme_agentname);
	soar_cQuit();


	printf("FAILURES = ")
	printf("%d", failures)
	printf("\n Done with addwmeTest.\n")
end


