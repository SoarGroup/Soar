Building MaC in Eclipse
1. Add SWT as a Java Library to Eclipse, if you have not already done so.
2. Build sml.jar if you have not already done so.
3. Select the MaC project and select Project->Properties from the menu bar.
4. Choose "Java Build Path" and select the "Libraries" tab.
5. Click "Add Library...", select "User Library" and choose the "SWT" library.
6. Click "Add external JARs...", navigate to the SoarIO/ClientSMLSWIG/Java
   directory and choose sml.jar

Running MaC in Eclipse
1. Select "Run->Run..." from the menu bar.
2. Choose "Java Application" and click "New".
3. Change the configuration name from "New_configuration" to "MaC".
4. In the "Main" tab, change the project name to "mac" and the Main class to
   'edu.umich.mac.MissionariesAndCannibals'.
5. In the "Arguments" tab, add the following to "VM arguments":
   -Djava.library.path=${system:ECLIPSE_HOME}/plugins/org.eclipse.swt.${system:WS}_3.0.2/os/${system:OS}/${system:ARCH}:/path/to/installed/SML/libraries
   Make sure the "Working directory" is set to "${workspace_loc:mac}"
6. In the "Classpath" tab, select "User Entries" and click on the "Advanced..."
   button. Choose "Add Variable String" and in the associated text box enter
   "${workspace_loc:mac}". Click "OK".
7. In the "Environment" tab, click "New..." to create a new environment
   variable. On *NIX-like systems, the environment variable should be named
   LD_LIBRARY_PATH (or equivalent) and have the value
   /path/to/installed/SML/libraries.
8. Click "Apply", and the "Run" to test.

Adding SWT as a Java Library to Eclipse.
1. Open the Eclipse Preferences
2. In the preferences pane, select Java->Build Path->User Libraries
3. Click the "New..." button.
4. Enter "SWT" for the User library name and click "OK".
5. Select the "SWT" library and click "Add JARs...".
6. In the file selection dialog, navigate to the Eclipse directory, and then
   into plugins->org.eclipse.swt.[platform]_3.0.X, where [platform] is the name 
   of the platform you are working on ('win32' for Windows, 'carbon' for Mac OS
   X) and X is the minor version number of SWT you have installed (ex 3.0.2).
   Then choose ws->[platform] and add all of the JARs in that directory to the
   SWT library.

Building sml.jar
1. Build the SoarKernel, gSKI, and SoarIO modules. If configuring SoarIO on a 
   *NIX-like system, use the --enable-java flag when configuring and be sure
   that SWIG 1.3.24 or later is installed.
2. Enter the SoarIO/ClientSMLSWIG/Java as your current directory.
3. Make a directory named 'sml' and copy all of the class files into the sml
   directory.
4. Type 'jar cf sml.jar sml' to create the sml.jar archive.
