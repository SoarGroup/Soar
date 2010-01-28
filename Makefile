all:
	cd Core && scons

checkout:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/Core
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/Java
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/Python

checkout-filterc:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/FilterC

checkout-javamissionaries:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/JavaMissionaries

checkout-javatoh:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/JavaTOH

checkout-loggerjava:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/LoggerJava

checkout-quicklink:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/QuickLink

checkout-soar2d:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/Soar2D

checkout-soartextio:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/SoarTextIO

checkout-sproom:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/Sproom

checkout-sps:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/Sps

checkout-tcl:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/Tcl

checkout-tohsml:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/TOHSML

update:
	svn update *

status:
	svn status *

clean:
	cd Core && scons -c

