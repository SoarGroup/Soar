
proc tsiOnEnvironmentStop {} {
    global tsiSimulationState
    
    set tsiSimulationState(running) 0

}

proc tsiOnEnvironmentRun {} {
    global tsiSimulationState

    set tsiSimulationState(running) 1
}

proc tsiOnEnvironmentStep {} {
    global tsiSimulationState

    set tsiSimulationState(running) 1
}    
