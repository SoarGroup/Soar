#!/usr/bin/wish

# the above line invokes wish on unix systems.  If you are running Soar
# on a Unix system, you must edit the above line to reflect the full
# pathname of your Wish executable, if it is not in /usr/tcl/bin.
# If you are not on a Unix system, the above line is ignored.


##  The next two lines are needed to be able to load Soar into the
##  Wish executable.  If you desire to create an application-specific
##  folder or directory for running Soar, you need to copy this
##  file (start-soar.tcl) and edit the next line to change "[pwd]"
##  to point to where you have installed Soar-8.4.  On Macs, you
##  would specify something like: 
##  set soar_library [file join "hard drive:Desk Top:Soar-8.4" library]
##  Windows:  set soar_library [file join "C:/Soar-8.4" library]
##  Unix   :  set soar_library [file join "/home/soar/Soar-8.4" library]
##  It's a good idea to always use double quotes around the path name.

set soar_library [file join [pwd] library]
set env(SOAR_LIBRARY) $soar_library


### The next line sets the location of the Tcl-Soar Interface library.
### If you are not using the TSI, you can comment this line.
set tsi_library  [file join [file join [pwd] ..] tsi]

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

#  KJC 3/31/03
