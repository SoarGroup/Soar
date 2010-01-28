$Id$

	     The Tcl/Tk Soar Interface (TSI) Version 4.0.1
	     ---------------------------------------------

The TSI provides a graphical user interface for interacting with Soar. It is 
based on ideas from Frank Ritter's DSI, and the SCA2 work done by Doug Pearson. 
The Soar Command Line Interface window is essentially the one produced by Randy 
Jones as a development of Karl Schwamb's original GUI for Soar.  There are still 
some known bugs, but things appear to be getting into shape.

Comments, questions, suggestions and discussions relevant to the TSI should be 
sent to soar-group@lists.sourceforge.net.

==============================================================================

The TSI is normally found in the $soar_library directory (folder), but can be 
moved anywhere, provided the global variable, tsi_library, is properly and 
accurately defined when running Wish.  However, if the TSI is moved, it's 
possible (likely) that it will not be able to find the Soar help files or Soar 
demo files.  Users should become quite familiar with the TSI code before trying 
to move it to another location.  In $tsi_library  is the main "tcl library" of 
code that implements the TSI.  All tsi commands will be autoloaded from this 
directory or folder (assuming your tcl interpreter knows where this directory or 
folder is).  See the file "init-soar.tcl" in the top-level directory of the Soar 
distribution for more information on what needs to be defined when starting 
Soar.

The TSI consists of a number of files:


         README.txt	This file

   tsi-defaults.tcl	This specifies some default values for the
			configuration of the TSI.  To override the defaults,
			you can make a file named tsi-defaults.tcl in the
			directory or folder from which you run Soar, and 
			specify the values you want to override.

        tsiInit.tcl	Provides the hooks to start up the TSI and to create
			new agent (and other) interpreters.

tsiControlPanel.tcl	Creates a simple control panel to drive Soar.
			Allows the creation of mutiple Soar agents.

     tsiDialogs.tcl	A set of "standard" dialogs.

       tsiPopUp.tcl     Implements PopUp menus for interacting with Soar.

       termText.tcl     A simple terminal widget for tcl/tk (which the
			TSI agent windows use).

   tsiAgentText.tcl     Enhancements to termText for Soar agents

 tsiAgentWindow.tcl     Implements a window for interacting with Soar agents.
                        Uses tsiAgentText for the main window, and adds a
                        bunch of menus, buttons, and whirligigs.

       tsiUtils.tcl     A collection of utilities supporting the interface,
			including agent windows as well as other types
			of windows (such as monitor windows).

           tclIndex	This file is used by Tcl to autoload various TSI
			commands and procedures.  Do not mess with it or
			move it (unless you are making enhancements to the
			TSI code and know what you are doing).

tsi/demos subdirectory or subfolder:
			This directory or folder contains a few Soar programs 
			that make use (to varying degrees) of some of the
			features in the TSI.  The TSI code assumes that
			this demos directory (folder) is a subdirectory 
			(subfolder) of the directory (folder) holding the main
			TSI code.  If you move this directory or folder
			somewhere else, or rename it, the menus for selecting 
			these demo programs will probably fail to function.

******************************************************************************

The TSI consists of a set of optional windows that include:

1. A control panel to provide easy access to the basic functions that
   are necessary or useful to run a Soar model.  It also supports running
   multiple Soar agents at the same time.

2. An interaction window for Soar, providing equivalent functionality
   to the old command line interface, but with a set of menus and buttons
   available. 

3. A display of the match set of rules about to fire.

4. A continuous display of the current state of the goal stack.

5. A print window that supports the examination of objects selected
   by the user.  (The user either presses p, or double clicks with the
   middle mouse button once an object has been selected.)


Associated with each window are a set of menus, to facilitate use by less 
experienced users.  More advanced users are supported by keystroke shortcuts.

Also included in the package are a set of "standard" dialogs which
support:

1. Simple Confirmation, e.g., by pressing an Ok button.

2. Binary choice, e.g., by pressing Ok or Cancel.

3. The return of a single value.

******************************************************************************

LOADING THE TSI 

Since Soar version 7.2, the TSI has been packaged with the Soar distribution. It 
will be invoked if Soar is started using the "init-soar.tcl" script found in the 
top-level directory (folder) of the Soar distribution. The "init-soar.tcl" 
script defines the $tsi_library variable and adds it to the auto_path so that 
the TSI commands are automatically available in Soar.

If creating a separate application directory and starting Soar by some method 
other than by invoking "init-soar.tcl" you will need to set the tcl variable 
"tsi_library" and invoke the TSI from the main Tcl interpreter by typing "tsi" 
or including the "tsi" command in your new startup script.  Note that this must 
be done _before_ the Soar package is loaded into the Tcl interpreter, or an 
error will occur.  See the "init-soar.tcl" file in the Soar distribution for 
more information on what is required to start Soar.

If you wish to modify the TSI, it is recommended that you create a new folder or 
subdirectory in the $soar_library folder, and copy the TSI files into the new 
folder, and then redefine your $tsi_library variable.  That way you always have 
a distribution copy of the TSI, and there will be less confusion if you send 
your modified TSI to other Soar users or TSI developers.  If you move your 
working copy of the TSI out of the $soar_library location, the TSI will not be 
able to find the Soar help pages or the Soar demos, unless you specifically 
modify the TSI to look in the proper locations.

--

Please feel free to send any suggestions, comments etc. you may have regarding 
this package to soar-tsi@umich.edu.

This file edited for the TSI version 2.5 release with Soar 7.3 and 8.2 by Karen 
Coulter, kcoulter@umich.edu

This file edited for the TSI version 4.0.1 release with Soar 8.5.2 by Jonathan 
Voigt (voigtjr@gmail.com)
