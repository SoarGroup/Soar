#!/bin/sh 
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

#load the sml stuff
load libTcl_sml_ClientInterface.so
#lappend auto_path .
#package require tcl_sml_clientinterface

#create an embedded kernel running in the kernel's thread (so we don't have to poll for messages)
set kernel [Kernel_CreateKernelInCurrentThread KernelSML 0]
#create an agent named Soar1
set agent [$kernel CreateAgent Soar1]

cd demos/towers-of-hanoi

#load the TOH productions
set result [$kernel ExecuteCommandLine "source towers-of-hanoi.soar" Soar1]
#set the watch level to 0
set result [$kernel ExecuteCommandLine "watch 0" Soar1]
#excise the monitor production
set result [$kernel ExecuteCommandLine "excise towers-of-hanoi*monitor*operator-execution*move-disk" Soar1]

#run TOH and time it using Tcl's built-in timer
set speed [time {set result [$kernel ExecuteCommandLine "run 2048" Soar1]}]

cd ../..
puts $speed

#give Tcl object ownership of underlying C++ object so when we delete it they both get deleted
set result [$kernel -acquire]
#delete kernel object (automatically deletes the agent)
set result [$kernel -delete]
#don't leave bad pointers around
unset agent
unset kernel
