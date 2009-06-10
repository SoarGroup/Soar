all:
	scons

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

