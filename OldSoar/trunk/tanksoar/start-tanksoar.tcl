#!/usr/bin/wish

#>-=>
# Source library facilities
#>-=>
set tcl_pkgPath {} 
set gameType deathmatch
 
#>-=>
# Set up paths to library then start up game environment
#>-=>
#set tk_library "C:\Tcl\lib\tk8.3"
#set tcl_library "C:\Tcl\lib\tcl8.3"

set soar_library [file join .. [file join .. [file join "soar-8.3.5" "library"]]]
set tsi_library [file join .. [file join .. "tsi-3.1.5"]]
set soar_doc_dir [file join $soar_library doc]
set doc_dir [file join $soar_library doc]

# We need to set the environment variables here because the
# Soar dll uses them
set env(SOAR_LIBRARY) $soar_library
set env(TSI_LIBRARY) $tsi_library

###set auto_path {}
lappend auto_path $tcl_library $tk_library $soar_library $tsi_library
 
cd simulator
source main.tcl
