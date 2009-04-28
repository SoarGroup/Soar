all:
	scons

32:
	scons m64=no

clean:
	scons -c

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

