#!/usr/tcl/bin/tclsh

## use update-man-page to generate cat pages for all commands in man/


set cmds [exec ls -c1 man]

foreach f $cmds {
   set name [file rootname $f]
    if {[file exists [file join man ${name}.n]]}  {
       exec update-man-page -cat $name
       exec update-man-page -html $name
   }   
}
