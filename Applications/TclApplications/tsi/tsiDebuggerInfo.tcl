namespace eval InfoPage {
   variable systemText
   variable memoryText
   variable agentInfoText 
   variable soarInfoText 
   variable tsiInfoText 
}

proc InfoPage::create { frame } {
   variable systemText
   variable memoryText
   variable agentInfoText 
   variable soarInfoText 
   variable tsiInfoText 

   # Create contents of info pages
   set paneWin [PanedWindow $frame.pw -side top -weights available]
   set pane1 [$paneWin add -weight 39]
   set pane2 [$paneWin add -weight 61]

   set pw   [PanedWindow $pane1.pw1 -side left -weights available]
   set paneNew      [$pw add -weight 33]
   set agentInfoText [CreateInfoPane $paneNew "Agent"]
   set paneNew      [$pw add -weight 33]
   set soarInfoText [CreateInfoPane $paneNew "Soar Settings"]
   set paneNew      [$pw add -weight 33]
   set tsiInfoText [CreateInfoPane $paneNew "Debugger"]

   pack $pw -fill both -expand yes
   pack $pane1 -fill both -expand yes

   set pw1   [PanedWindow $pane2.pw1 -side left -weights available]
   set paneNew      [$pw1 add -weight 50]
   set systemText [CreateInfoPane $paneNew "System Info"]
   set paneNew      [$pw1 add -weight 50]
   set memoryText [CreateInfoPane $paneNew "Memory Usage and Rete"]

   pack $pw1 -fill both -expand yes
   pack $pane2 -fill both -expand yes
   pack $paneWin -fill both -expand yes
   pack $frame -fill both -expand yes -padx 4 -pady 4
   
   Update
}

proc InfoPage::Update { } {
   variable systemText
   variable memoryText
   variable agentInfoText 
   variable soarInfoText 
   variable tsiInfoText 
   global tsiAgentInfo

   foreach t {$systemText $memoryText $agentInfoText $soarInfoText $tsiInfoText} {
      eval "$t configure -state normal"
      eval "$t delete 1.0 end"
   }

###KJC:
if {0} {
   output-strings-destination -push -append-to-result
   $systemText insert end "[stats -system]"
   $memoryText insert end "Memory Statistics:\n\n"
   $memoryText insert end "[stats -memory]"
   $memoryText insert end "\nRete Statistics:\n\n"
   $memoryText insert end "[stats -rete]"
   
   AddVariableStat $agentInfoText "Agent Name" interp_name
   $agentInfoText insert end "\n"
   if {[info exists ::tsiAgentInfo($::interp_name,sourceFile)]} {
      AddLiteralStat $agentInfoText "Source File" $::tsiAgentInfo($::interp_name,sourceFile) 15
      AddLiteralStat $agentInfoText "Source Path" $::tsiAgentInfo($::interp_name,sourceDir) 15
   }
   $agentInfoText insert end "\n"
   AddLiteralStat $agentInfoText "Number of Total Productions" [llength [print -all]]
   AddLiteralStat $agentInfoText "- User Productions" [llength [print -user]]
   AddLiteralStat $agentInfoText "- Default Productions" [llength [print -defaults]]
   AddLiteralStat $agentInfoText "- Chunks Learned" [llength [print -chunks]]
   AddLiteralStat $agentInfoText "- Justifications Built" [llength [print -justifications]]

   AddLiteralStat $soarInfoText "Soar 8 Mode" [string range [uplevel #0 eval "soar8"] 14 20 ]  
   AddLiteralStat $soarInfoText "Learning" [string range $Debugger::learnSettings 7 9] 
   $soarInfoText insert end "\n"
   AddLiteralStat $soarInfoText "Indifferent Selection Mode" [string range [uplevel #0 eval "indifferent-selection"] 1 20 ]
   AddLiteralStat $soarInfoText "Numeric Indifferent Mode" [string range [uplevel #0 eval "numeric-indifferent-mode"] 1 20 ]  
   AddVariableStat $soarInfoText "Attribute Preference Mode" attribute_preferences_mode
   AddVariableStat $soarInfoText "Operator Support Mode" o_support_mode
   $soarInfoText insert end "\n"
   AddVariableStat $soarInfoText "Default WME depth" default_wme_depth
   AddVariableStat $soarInfoText "Maximum Chunks that can be Learned" max_chunks
   AddVariableStat $soarInfoText "Maximum Elaboration Cycles" max_elaborations
   $soarInfoText insert end "\n"
   AddVariableStat $soarInfoText "Save Backtrace Information" save_backtraces
   AddVariableStat $soarInfoText "Print Alias Switching" print_alias_switch
   AddVariableStat $soarInfoText "Display Warnings" warnings
   # Add learning 
   output-strings-destination -pop
}
   foreach t {$systemText $memoryText $agentInfoText $soarInfoText $tsiInfoText} {
      eval "$t see 1.0"
      eval "$t configure -state disabled"
   }
   update
}

   # TO DO:  Change width of .... depending on window size

proc InfoPage::AddVariableStat { textItem labelName varName {leader 35}} {
   variable variableListbox

   set listText "$labelName[string range "................................." 0 [expr $leader - [string length $labelName]]] [uplevel #0 eval "set $varName"]" 
   $textItem insert end "$listText\n"
}

proc InfoPage::AddLiteralStat { textItem labelName statValue {leader 35}} {
   variable variableListbox

   set listText "$labelName[string range "................................." 0 [expr $leader - [string length $labelName]]] $statValue" 
   $textItem insert end "$listText\n"
}
