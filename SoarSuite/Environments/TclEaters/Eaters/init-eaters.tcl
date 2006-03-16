#!/usr/bin/wish

# If you are not on a Unix system, you can ignore this block of comments.
#
# The first line of this file invokes wish on unix systems.  If you 
# are running Soar on a Unix system, you must edit the above line to 
# reflect the full pathname of your Wish executable if it is not in 
# the default location /usr/bin.
#
# If you installed ActiveTcl to its default location, the first line 
# of this file should change to:
#!/usr/local/ActiveTcl/bin/wish

# $Id$

#set tcl_pkgPath {} 

set soar_library [file join [pwd]]
set tsi_library  [file join [pwd] .. .. Tools TSI]
set eaters_dir   [file join [pwd] .. .. Environments TclEaters Eaters]
set agents_dir   [file join $eaters_dir agents]

lappend auto_path ~/sandbox/lib

# We need to set the environment variables here because the
# Soar dll uses them

set env(SOAR_LIBRARY) $soar_library
set env(TSI_LIBRARY) $tsi_library

###set auto_path {}
#lappend auto_path $tcl_library $tk_library $soar_library $tsi_library
 lappend auto_path  $soar_library $tsi_library


# Start with controller hidden, then expose it when it has been altered
set ETCPConfig(AgentName) Eater
set ETCPConfig(SimulatorPath) [file join $eaters_dir simulator]
set ETCPConfig(AgentFolder) $agents_dir

set mode off
if { [llength $argv] > 0 } { set mode [lindex $argv 0] }

  
wm deiconify .


if { ![string compare $mode client] } {

  tsi 1 -mode client

  wm deiconify .

} else {

  source [file join [file join $eaters_dir simulator] PDFileMenus.tcl]
  source [file join [file join $eaters_dir simulator] et-controlpanel.tcl]
  source [file join [file join $eaters_dir simulator] eaters-configure.tcl]

  tsi 1 -controlpanel makeETControlPanel -hideagentwin 1

  source [file join [file join $eaters_dir simulator] process.tcl]
  source [file join [file join $eaters_dir simulator] eaters-layout.tcl]
  source [file join [file join $eaters_dir simulator] interface.tcl]


  wm deiconify .
}
