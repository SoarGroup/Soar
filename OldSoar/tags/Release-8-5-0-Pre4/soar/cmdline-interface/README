The files in this directory constitue a simple example of how to use
the Soar API.  The application built here is a simple shell interface
to Soar which lacks the standard Tk GUI.  Although the complete
Command Set (as defined by the API) is not implemented in this
example, the shell does allow most of Soar's critical functionality to
be accessed.  Moreover, it supports a mechanism for registering
commands which make it easy for anyone interested to extend the
current functionality.


Also included in this application is an example of how to send and
receive environmental information to and from Soar.  This is
illustrated by running the "adder.soar" agent and watching it count
from 0 to infinity by passing a number from output-link to a
c-function and then back to the input-link.  The c-functions required
for this behavior are defined in the file "io_example.c"


The majority of source code for this application has been extensively
documented to help the learning process.  The only exception to this
occurs in the file "parsing.c" which contains a basic parser, and is
of no real education purpose.  Nonetheless, some documentation does
exist within this file in case you wish to use it as a basis for your
own application.

We suggest that you look over the example code in the following order:

	main.c				- The interface event loop 
	io_example.c			- IO functions used w/ adder agent
	soarInterfaceCommands.c		- Interface command registry
	parsing.c			- parsing utilities






documented:
	main.c
	io_example.c
	parsing.c
	soarInterfaceCommands.c