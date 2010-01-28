all:
	cd Core && scons

checkout:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/Core
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/Java
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/Python

checkout-filterc:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/FilterC

checkout-javamissionaries:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/JavaMissionaries

checkout-javatoh:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/JavaTOH

checkout-loggerjava:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/LoggerJava

checkout-quicklink:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/QuickLink

checkout-soar2d:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/Soar2D

checkout-soartextio:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/SoarTextIO

checkout-sproom:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/Sproom

checkout-sps:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/Sps

checkout-tcl:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/Tcl

checkout-tohsml:
	svn checkout https://soar.googlecode.com/svn/branches/smem-reorg/TOHSML

update:
	svn update *

status:
	svn status *

clean:
	cd Core && scons -c

