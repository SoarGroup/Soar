all:
	cd Core && scons

noscu:
	cd Core && scons --no-scu

up: update
update:
	find . -not -name ".*" -type d -depth 1 -exec echo {} \; -exec svn update {} \;

status:
	find . -not -name ".*" -type d -depth 1 -exec echo {} \; -exec svn status {} \;

clean:
	cd Core && scons -c

co: checkout
checkout:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/Core
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/Java
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/Python

co-filterc: checkout-filterc
checkout-filterc:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/FilterC

co-javamissionaries: checkout-javamissionaries
checkout-javamissionaries:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/JavaMissionaries

co-javatoh: checkout-javatoh
checkout-javatoh:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/JavaTOH

co-loggerjava: checkout-loggerjava
checkout-loggerjava:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/LoggerJava

co-quicklink: checkout-quicklink
checkout-quicklink:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/QuickLink

co-room: checkout-room
checkout-room:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/Room

co-soar2d: checkout-soar2d
checkout-soar2d:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/Soar2D

co-soar2soar: checkout-soar2soar
checkout-soar2soar:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/Soar2Soar

co-soartextio: checkout-soartextio
checkout-soartextio:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/SoarTextIO

co-sproom: checkout-sproom
checkout-sproom:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/Sproom

co-sps: checkout-sps
checkout-sps:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/Sps

co-tcl: checkout-tcl
checkout-tcl:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/Tcl

co-tohsml: checkout-tohsml
checkout-tohsml:
	svn checkout https://soar.googlecode.com/svn/trunk/SoarSuite/TOHSML

