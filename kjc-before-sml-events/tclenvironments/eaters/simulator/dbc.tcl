
proc dbcsafe {inpt outpt {key *} } {
  
    if [file exists $outpt] {
	puts "The output file '$outpt' already exists. Aborting."
	return
    }
    set nBegins 0
    set nEnds 0
    
    dbc $inpt $outpt $key
    undbc $outpt dbcBuffer $key
    
    set original [open $inpt r]
    set buffer [open dbcBuffer r]
   
    if { [string compare [read $original] [read $buffer]] } {
	puts "Error"
    } else {
        puts "Files are OK."
    }

    close $original
    close $buffer
    file delete dbcBuffer 
} 
proc dbc { inpt outpt {key *} } {

    if [file exists $outpt] {
	puts "The output file '$outpt' already exists. Aborting."
	return
    }
    set inFile [open $inpt r]
    set outFile [open $outpt w]
    set nBegins 0
    set nEnds 0
    foreach line [split [read $inFile] \n] {

	if [string match "*\#*DBC END*$key*" $line] {
	    incr nEnds
	}

	if { $nBegins > $nEnds } {
	    # inside DBC
	    puts $outFile "\#$line"
	} else {
	    puts $outFile $line
	}

	if [string match "*\#*DBC BEGIN*$key*" $line ] {
	    incr nBegins
	}
    }
    if { $nEnds != $nBegins } {
	puts "DBC ERROR -- EOF before DBC END!"
    }
    close $inFile
    close $outFile
}

proc undbc { inpt outpt {key *}} {
 
    if [file exists $outpt] {
	puts "The output file '$outpt' already exists. Aborting."
	return
    }

    set inFile [open $inpt r]
    set outFile [open $outpt w]
    set nBegins 0
    set nEnds 0

    foreach line [split [read $inFile] \n] {

	if [string match "*\#*DBC END*$key*" $line] {
	    incr nEnds
	}

	if { $nBegins > $nEnds } {
	    # inside DBC
	    puts $outFile [string trimleft $line \#]
	} else {
	    puts $outFile $line
	}

	if [string match "*\#*DBC BEGIN*$key*" $line ] {
	    incr nBegins

	}
    }

    if { $nEnds != $nBegins } {
	puts "DBC ERROR -- EOF before DBC END!"
    }


    close $inFile
    close $outFile
}
	
	    
	    
