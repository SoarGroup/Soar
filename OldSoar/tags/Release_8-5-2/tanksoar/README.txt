$Id$

TankSoar 3.0.8 is an example environment for the Soar architecture and is used 
for research and to help teach basic and intermediate techniques of Soar in the 
Soar tutorial.  TankSoar 3.0.8 is installed as a component of the Soar-Suite 
package.  For more information, please see the website:

	http://sitemaker.umich.edu/soar/

____________________
File/Directory list:
--------------------
README.txt         - This file.
INSTALL.txt        - Installation and execution notes.
LICENSE.txt        - The license that TankSoar is distributed under.
start-tanksoar.bat - The Windows file used to start TankSoar (see INSTALL file).
init-tanksoar.tcl  - The Tcl/Tk initialization code for TankSoar (see INSTALL file).
agents/            - Example and tutorial Soar agents.
simulator/         - The TankSoar Tcl/Tk source code.

_____________
Known Issues:
-------------
 * Changing maps while agents are active will cause TankSoar to crash. 
 Destroy all active agents before changing maps.

--

Updated for the TankSoar release version 3.0.8 by Jonathan Voigt 
(voigtjr@gmail.com).
