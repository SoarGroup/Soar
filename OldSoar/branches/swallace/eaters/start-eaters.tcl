#!/usr/bin/wish

set tcl_pkgPath {} 

set soar_library [file join .. [file join "soar-8.3.5" "library"]]
set tsi_library [file join .. "tsi-3.1.5"]
set soar_doc_dir [file join $soar_library doc]
set doc_dir [file join $soar_library doc]
 

# We need to set the environment variables here because the
# Soar dll uses them

set env(SOAR_LIBRARY) $soar_library
set env(TSI_LIBRARY) $tsi_library

###set auto_path {}
lappend auto_path $tcl_library $tk_library $soar_library $tsi_library
 

# Start with controller hidden, then expose it when it has been altered
set ETCPConfig(AgentName) Eater
set ETCPConfig(AgentFolder) agents

set mode off
if { [llength $argv] > 0 } { set mode [lindex $argv 0] }


if { ![string compare $mode client] } {

  tsi 1 -mode client

  wm deiconify .

} else {

  source [file join simulator PDFileMenus.tcl]
  source [file join simulator et-controlpanel.tcl]
  source [file join simulator eaters-configure.tcl]

  tsi 1 -controlpanel makeETControlPanel

  source [file join simulator process.tcl]
  source [file join simulator eaters-layout.tcl]
  source [file join simulator interface.tcl]


  wm deiconify .
}
