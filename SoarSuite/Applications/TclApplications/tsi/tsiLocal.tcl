
proc tsiOnEnvironmentStop {} {
    global tsiSimulationState
    
    set tsiSimulationState(running) 0
    # workaround for bugzilla bug 398
    puts -nonewline ""
    
}

proc tsiOnEnvironmentRun {} {
    global tsiSimulationState

    set tsiSimulationState(running) 1
}

proc tsiOnEnvironmentStep {} {
    global tsiSimulationState
    
    set tsiSimulationState(running) 1
    
}    
