proc __PDFMReplaceSpace { orig } {


	regsub -all " " "$orig" PDFileMenuSPACESTRING modified
	
	return $modified 

}

proc __PDFMUnreplaceSpace { mod } {

	regsub -all PDFileMenuSPACESTRING "$mod" " " original
	
	return $original
}

proc __PDFMUnreplaceSpaceList {modList} {
	
	foreach element $modList {
		regsub -all PDFileMenuSPACESTRING "$element" " " newElement
		lappend origList "$newElement"
	}
	return $origList
}

proc PDFileMenus { fileMenu fileVar args }  {

    set emptyValue "**NO FILES**"
    set filter "*.*"
    set pathVar ""
    set pathMenu ""
    set path [__PDFMReplaceSpace [pwd]]
    set initialPath [__PDFMReplaceSpace [pwd]]
    set filenameDisplay standard
    set pathnameDisplay relative
    set maxDepth -1

    foreach {key value} $args {
	switch -exact -- $key {

	    -pathMenu		{ set pathMenu $value }
	    -pathVar		{ set pathVar $value }
	    -filter		{ set filter  $value }
	    -emptyValue		{ set emptyValue $value }
	    -path		{ set path $value }
	    -maxDepth		{ set maxDepth $value }
	    -filenameDisplay	{ set filenameDisplay $value }
	    -pathnameDisplay	{ set pathnameDisplay $value }
	}
    }
    
    
    if [catch {cd "[__PDFMUnreplaceSpace $path]"} err] {
	error "Could not find the directory [__PDFMUnreplaceSpace $path]: $err"
	return
    }

    set filesAndDirs  [__PDFMGetFilesAndDirs [__PDFMReplaceSpace [pwd]] $filenameDisplay \
			   $pathnameDisplay $filter $maxDepth]

    set files [lindex $filesAndDirs 0]
    set dirs [lindex $filesAndDirs 1]
    set fulldirs [lindex $filesAndDirs 2]

    if { ![winfo exists $fileMenu] } {	

	if { $files == {} } {
	    tk_optionMenu $fileMenu $fileVar $emptyValue
	    $fileMenu configure -state disabled
	} else {
	    eval tk_optionMenu $fileMenu $fileVar $files
	}
    
    } else {
	error "$fileMenu already exists."
	if [catch {cd "[__PDFMUnreplaceSpace $initialPath]"} msg] {
	    error "Could not return to the original path '[__PDFMUnreplaceSpace $initialPath]': $msg"
	}
	return
    }
    
    if { [string compare $pathMenu ""] != 0 && \
	     [string compare $pathVar ""] != 0 } {
	# create the path menu as well...

	if { ![winfo exists $pathMenu] } {
	    eval tk_optionMenu $pathMenu __PDFMInternalPath$fileMenu [__PDFMUnreplaceSpaceList $dirs]
	    uplevel #0 set $pathVar \"[__PDFMUnreplaceSpace [lindex $fulldirs 0]]\"
	    set dirLen [llength $dirs]
	    for {set i 0} {$i < $dirLen } {incr i} {
		[$pathMenu cget -menu] entryconfigure $i -command \
		    [list __PDFMLoadFiles [__PDFMReplaceSpace [pwd]] [lindex $fulldirs $i] \
			 $fileMenu $fileVar $filenameDisplay $pathMenu \
			 $pathVar $pathnameDisplay $filter $maxDepth \
			 $emptyValue]
	   }
		
 
	} else {
	    error "$pathMenu already exists."
	}
    } else {
	# We want to keep track of the path anyway
	
	#uplevel #0 set __PDFMInternalPath$fileMenu [__PDFMUnreplaceSpace "$path"]
    }

    if [catch {cd "[__PDFMUnreplaceSpace $initialPath]"} msg] {
	error "Could not return to the original path '[__PDFMUnreplaceSpace $initialPath]': $msg"
    }
    
    return
}

proc __PDFMLoadFiles {topDir workingDir fileMenu fileVar filenameDisplay \
			  pathMenu pathVar pathnameDisplay filter \
			  maxDepth emptyValue } {

    set cPath [__PDFMReplaceSpace [pwd]]
    cd "[__PDFMUnreplaceSpace $workingDir]"
    

    set filesAndDirs [__PDFMGetFilesAndDirs $topDir $filenameDisplay \
			  $pathnameDisplay $filter $maxDepth]

    [$pathMenu cget -menu] delete 0 end
    [$fileMenu cget -menu] delete 0 end
    
    set files [lindex $filesAndDirs 0]
    set dirs [lindex $filesAndDirs 1]
    set fulldirs [lindex $filesAndDirs 2]

    set nDirs [llength $dirs]
    
    if { $files == {} } {
	uplevel #0 [list set $fileVar "$emptyValue"]
	$fileMenu configure -state disabled
    } else {
	foreach file $files {
	    [$fileMenu cget -menu] add command -label $file -command \
		[list set $fileVar $file]
	}
	uplevel #0 set $fileVar [lindex $files 0]
	$fileMenu configure -state normal
    }

    
    for {set i 0} {$i < $nDirs} {incr i} {
	[$pathMenu cget -menu] add command -label "[__PDFMUnreplaceSpace [lindex $dirs $i]]" \
	    -command [list __PDFMLoadFiles $topDir [lindex $fulldirs $i] \
			 $fileMenu $fileVar $filenameDisplay $pathMenu \
			 $pathVar $pathnameDisplay $filter $maxDepth \
			 $emptyValue ]
    }
    uplevel #0 set __PDFMInternalPath$fileMenu \"[__PDFMUnreplaceSpace [lindex $dirs 0]]\"
    uplevel #0 set $pathVar \"[__PDFMUnreplaceSpace [lindex $fulldirs 0]]\"

    cd "[__PDFMUnreplaceSpace $cPath]"
    
}
    
proc __PDFMGetFilesAndDirs { topDir filenameDisplay \
			     pathnameDisplay filter maxDepth} {



    set currDir [__PDFMReplaceSpace [pwd]]
    if { $maxDepth < 0 } {
	set cDepth 1
    } else {
	set cDepth [expr $maxDepth - [llength [file split $currDir]] \
			      + [llength [file split $topDir]]]
    }
   

    switch -exact $filenameDisplay {
	
	standard	{ 
	    foreach i [glob -nocomplain $filter] { 
		lappend files [file tail $i] 
	    }
	}
	root		{ 
	    foreach i [glob -nocomplain $filter] { 
		lappend files [file rootname [file tail $i]] 
	    }
	}
	default		{
	    set files [glob -nocomplain $filter]
        }
    }
    
    switch -exact $pathnameDisplay {
	base		{
	    if { $topDir == $currDir } {
		set dirs .
		set fulldirs $topDir
	    } else {
		set dirs [list . .. ]
		set fulldirs [list $currDir [file join $currDir ..]]
	    }
	    if { $cDepth > 0 } {
		foreach file [glob -nocomplain *] {
		    if [file isdirectory $file] {
			lappend dirs [file join . [file tail $file]]
			lappend fulldirs [file join $currDir $file]
		    }
		}
	    }
	}
	relative        {
	    if { $topDir == $currDir } {
		set thisPath .
		set dirs .
		set fulldirs $topDir
	    } else {
		set thisPath [eval file join . [lrange [file split $currDir] \
					       [llength [file split $topDir]] \
						     end]]
		set dirs [list $thisPath [file join $thisPath ..]]
		set fulldirs [list $currDir [file join $currDir ..]]
	    } 
	    if { $cDepth > 0 } {
		foreach file [glob -nocomplain *] {
		    if [file isdirectory $file] {
			lappend dirs [file join $thisPath $file]
			lappend fulldirs [file join $currDir $file]
		    }
		}
	    }
		
	}
	default		{
	    if { $topDir == $currDir } {
		set dirs $currDir
		set fulldirs $currDir
	    } else {
		set dirs [list $currDir [file join $currDir ..]]
		set fulldirs [list $currDir [file join $currDir ..]]
	    }
	    if { $cDepth > 0 } {
		foreach file [glob -nocomplain *] {
		    if [file isdirectory $file] {
			lappend dirs $file
			lappend fulldirs [file join $currDir $file]
		    }
		}
	    }
	}
    }

    set topDir [__PDFMReplaceSpace [pwd]]
    

    
    if [info exists files] { 
	return [list [lsort -ascii $files] $dirs $fulldirs]
    } else {
	return [list {} $dirs $fulldirs]
    }
    
}


