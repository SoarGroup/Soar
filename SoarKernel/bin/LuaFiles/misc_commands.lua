-- Name: misc_commands
--
-- This is a test of various interface function calls
--
--
--==================================================
misc_agentname = "the_agent"


----------------------------------------------------
--
----------------------------------------------------
function myOutputFunction(agent, cb_data, call_data)
end


----------------------------------------------------
--
----------------------------------------------------
function myInputFunction(agent, cb_data, call_data)
	--agent = soar_cGetAgentByName(misc_agentname)
	--id = soar_cGetAgentOutputLinkId(agent)
end

----------------------------------------------------
--
----------------------------------------------------
function test_getAgentByName(name, expected)
	actual = soar_cGetAgentByName(name)
	if ((expected and not actual) or (not expected and actual)) then
		tprint("\nFAILURE: soar_cGetAgentByName(" .. name .. ")")
		tprint("\nexpected ")
		if (expected) then
			tprint("non-void return value\n")
		else
			tprint("void return value\n")
		end
	end
	return actual
end

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

function TestMiscCommands()
	printf("Starting misc_commands.\n")
	-- 
	-- Here, "nil" is not really "nil", but calling a function
	-- that does not exist is an equivalient enough behavior
	--
	createSoarPlayer(misc_agentname, "nil", "nil", "misc_commands.log", TRUE)

	--soar_ecCaptureInput("captured_input.s")

	soar_ecBuildInfo();
	--soar_ecExcludedBuildInfo();
	--
	-- Add 10 elements to I3
	--

	soar_SaveBacktraces("-on")

	--
	-- Load agent code
	--
	sourceProductionsFromFile( "TestProductions\\test.soar" )
	
	-- START TESTS

	-- test of soar8 command
	soar_cSetOperand2(nil)
	soar_cSetOperand2(1)

	-- test of version command
	soar_Version("")
	soar_Version("junk")

	-- test of soarnews command
	soar_Soarnews("")
	soar_Soarnews("junk")

	print_to_screen(TRUE)
	soar_cRun(1, TRUE, go_type_enum.GO_PHASE, soar_apiSlotType.NO_SLOT)

	-- test of sp command
	soar_Sp("secondtest*production\n(state <s> ^superstate nil)\n(<s> ^io.output-link <ol>)\n-->\n(<ol> ^junk 5)")
	
	-- test explain-backtraces
	soar_ExplainBacktraces("")
	soar_ExplainBacktraces("test*production")
	
	-- test of init-attention-lapse
	--soar_InitAttentionLapse()

	-- test of attention-lapse command
	--soar_AttentionLapse("")
	--soar_AttentionLapse("-on")
	--soar_AttentionLapse("")

	-- test of start-attention-lapse command
	--soar_StartAttentionLapse(10)

	-- test of wake-from-attention-lapse command
	--soar_WakeFromAttentionLapse()

	-- test of attention-lapse command
	--soar_AttentionLapse("")
	--soar_AttentionLapse("-off")
	--soar_AttentionLapse("")

	-- test of stats command
	soar_Stats("")
	soar_Stats("-system")
	soar_Stats("-memory")
	soar_Stats("-rete")

	-- test of pwatch -on for just one production
	soar_Pwatch("-on", "test*production")

	-- test of watch command
	soar_Watch("")
	soar_Watch("0")
	soar_Watch("")
	soar_Watch("1")
	soar_Watch("")
	soar_Watch("2")
	soar_Watch("")
	soar_Watch("3")
	soar_Watch("")
	soar_Watch("4")
	soar_Watch("")
	soar_Watch("5")
	soar_Watch("")
	soar_Watch("productions -off")
	soar_Watch("")

	-- test of firing-counts command
	soar_FiringCounts("test*production")
	soar_FiringCounts("sectest*production")
	soar_FiringCounts("secondtest*production")
	soar_FiringCounts("1")
	soar_FiringCounts("2")
	soar_FiringCounts("3")

	-- test of format-watch command
	soar_FormatWatch("-stack")
	soar_FormatWatch("-object")

	-- test of attribute-preferences-mode command
	soar_AttributePreferencesMode("");
	soar_AttributePreferencesMode("0");
	soar_AttributePreferencesMode("1");
	soar_AttributePreferencesMode("2");
    soar_AttributePreferencesMode("");

	-- test of preferences command
	soar_Preferences("-none")
	soar_Preferences("-names")
	soar_Preferences("-timetags")
	soar_Preferences("-wmes")
	soar_Preferences("0")
	soar_Preferences("1")
	soar_Preferences("2")
	soar_Preferences("3")

	-- test of soar_cGetAgentByName	
	test_getAgentByName("the_agent", 1)
	--test_getAgentByName("nonexistent_agent", nil)

	-- test of print command
	soar_cPrint("test*production")
	soar_cPrint("secondtest*production")
	soar_cPrint("nonexistent*production")
	
	-- test of chunk-name-format command
	soar_ChunkNameFormat("")
	soar_ChunkNameFormat("-short")
	soar_ChunkNameFormat("-long")
	soar_ChunkNameFormat("-prefix CHUNK")
	soar_ChunkNameFormat("-count 42")

	-- test of max-chunks command
	soar_MaxChunks("")
	soar_MaxChunks("22")
	soar_MaxChunks("")

	-- test of max-elaborations command
	soar_MaxElaborations("")
	soar_MaxElaborations("44")
	soar_MaxElaborations("")

	-- test of max-nil-output-cycles command
	soar_MaxNilOutputCycles("")
	soar_MaxNilOutputCycles("36")
	soar_MaxNilOutputCycles("")

	-- test of default-wme-depth command
	soar_DefWmeDepth("")
	soar_DefWmeDepth("12")
	soar_DefWmeDepth("")

	-- test of gds-print command
	soar_GDS_Print("")

	-- test of indifferent-selection command
	soar_IndifferentSelection("")
	soar_IndifferentSelection("-random")
	soar_IndifferentSelection("-ask")
	soar_IndifferentSelection("-last")
	soar_IndifferentSelection("-first")
	soar_IndifferentSelection("")
	soar_IndifferentSelection("-random")

	-- test of the rete-net, excise, and memories commands
	soar_cSaveReteNet("rete-net.ret")
	soar_Memories("test*production")
	soar_Memories("secondtest*production")
	soar_cExciseAllProductions("blah")
	soar_Memories("test*production")
	soar_Memories("secondtest*production")
	soar_cLoadReteNet("rete-net.ret")
	soar_Memories("")

	-- test of the input-period command
	soar_InputPeriod("")
	soar_InputPeriod("2")
	soar_InputPeriod("")
	
	-- test of the internal-symbols command
	soar_InternalSymbols("")
	
	-- test of the stop-soar command
	soar_cStopAllAgents()

	-- test of the soar_cCreateAgent, soar_cGetAgent, and soar_cDestroyAgentById
	--test_getAgentByName("pointless_agent", 1)
	soar_cCreateAgent("pointless_agent", "pointless_agent.log", TRUE)
	test_getAgentByName("pointless_agent", 1)
        soar_cDestroyAgentByName("pointless_agent");

	-- test of multi-attributes command
	soar_MultiAttributes("")
	soar_MultiAttributes("nonexistent*symbol")
	soar_MultiAttributes("nonexistent*symbol*20 20")
	soar_MultiAttributes("")

	-- test of matches command
	soar_Matches("-assertions")
	soar_Matches("-assertions -names")
	soar_Matches("-assertions -timetags")
	soar_Matches("-assertions -wmes")
	soar_Matches("-retractions")
	soar_Matches("-retractions -names")
	soar_Matches("-retractions -timetags")
	soar_Matches("-retractions -wmes")
	soar_Matches("test*production -n")
	soar_Matches("test*production -t")
	soar_Matches("test*production -w")
	soar_Matches("nonexistent*production -n")

	-- test of production-find command
	soar_ProductionFind("")

	-- test of warnings command
	soar_Warnings("")
	soar_Warnings("-off")
	soar_Warnings("")
	soar_Warnings("-on")
	soar_Warnings("")

	-- test of echo command
	soar_Echo("Testing echo command")
	soar_Echo("with newline")
	soar_Echo("-nonewline Now testing echo command")
	soar_Echo("with no newline")

	-- test of learn command
	soar_Learn("-on")
	soar_Learn("-off")
	soar_Learn("-except")
	soar_Learn("-only")
	soar_Learn("-list")
	soar_Learn("-all-levels")
	soar_Learn("-bottom-up")
	soar_Learn("-badcommand")

	-- test pf the o-support command
	soar_OSupportMode(0)
	soar_OSupportMode(1)
	soar_OSupportMode(2)
	soar_OSupportMode(3)

	-- test of soar_Watch
	soar_Watch("3")

	-- test of soar_Pwatch -off for all productions
	soar_Pwatch("-off", "test*production")

	-- END TESTS
		  
	soar_memdump();

	print_to_screen(TRUE);
	soar_Stats("-memory");
			
	--quit while agents still exist to exercise error code
	soar_cQuit();

	soar_cDestroyAgentByName(misc_agentname);
	soar_cQuit();

	printf("\n Done with misc_commands.\n")
end
