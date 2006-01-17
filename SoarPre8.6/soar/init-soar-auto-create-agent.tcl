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

# This next line is revision information for this file and can be ignored.
# $Id$

##  The next two lines are needed to be able to load Soar into the
##  wish executable.  

set soar_library [file join [pwd] library]
set env(SOAR_LIBRARY) $soar_library

### The next line sets the location of the Tcl-Soar Interface library.
### If you are not using the TSI, you can comment this line.
set tsi_library  [file join [file join [pwd] ..] tsi-4.0.1]

#### This line tells Tcl where it can find the Soar sharable library.
#### It also indicates where the TSI is, you can leave $tsi_library off
#### if you are not using the TSI.
lappend auto_path  $soar_library $tsi_library

##### The next line tells Soar where it can find its help files
set soar_doc_dir [file join $soar_library ../doc]

##### This line invokes the TSI.  You could invoke your own application
##### instead.  If you don't want to run the TSI, and you want to 
##### start a single agent, change "tsi" to "package require Soar"

if { ![info exists batchMode] } { set batchMode off }

if { [llength $argv] > 0 } {
	set batchMode on
	set batchFile [lindex $argv 1]
}

#tsi 0 -batchMode $batchMode
tsi

if { [string match "on" $batchMode] } {
	source $batchFile

}

createNewAgent "" [pwd] ""