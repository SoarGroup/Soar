all:
	scons

noscu:
	scons scu=no

prof:
	scons gprof=yes

32:
	scons m64=no python=no

clean:
	scons tcl=yes python=yes java=yes -c

distclean: clean
	rm -f config.log
	rm -f .sconsign.dblite
	rm -f SoarSCons.pyc
	rm -rf .sconf_temp
	rm -f SoarLibrary/lib/swt.jar
	rm -f Environments/Soar2D/preferences
	rm -f Environments/Soar2D/config/taxi-rl.cnf
	rm -f Environments/Soar2D/config/tanksoar.cnf
	rm -f Environments/Soar2D/config/tanksoar-console.cnf
	rm -f Environments/Soar2D/config/eaters.cnf
	rm -f Environments/Soar2D/config/eaters-console.cnf
	rm -f Environments/Soar2D/config/taxi.cnf
	rm -f SoarLibrary/bin/testjavasml-success.txt
	rm -f SoarLibrary/bin/clog-test.txt
	rm -f SoarLibrary/bin/testCommandToFile-output.soar
	rm -rf Environments/JavaMissionaries/bin
	rm -f Environments/Soar2D/default-layout-9_0_0.dlf
	rm -rf Environments/Soar2D/bin
	rm -f Environments/Soar2D/default-text-9_0_0.dlf
	rm -rf Environments/JavaTOH/bin
	rm -f SoarLibrary/bin/default-layout-9_0_0.dlf
	rm -f SoarLibrary/bin/default-text-9_0_0.dlf
	rm -rf Tools/LoggerJava/bin
	rm -f Tools/VisualSoar/VSPreferences.txt
	rm -rf Tools/TestJavaSML/bin
	rm -rf Tools/SoarJavaDebugger/bin
	rm -rf SoarLibrary/defaultbin

