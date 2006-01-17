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


set soar_library [file join [pwd] [file join .. [file join "soar-8.5.2" "library"]]]
set tsi_library [file join [pwd] [file join .. "tsi-4.0.1"]]
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
