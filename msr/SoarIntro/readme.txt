Intro Program Readme

This file contains information about the Intro Program for MSRS,
including installation instructions. It has been updated to work
with Version 1.5 release (July 2007).

NOTE: This program requires the Maze Simulator. Make sure
that you have installed the Maze Simulator first and that
it is running successfully before you try to run this program.
All of the services are supplied in a single ZIP file now.

Also, the manifest uses my modified Dashboard. If you do not
want to use this, then you will have to change the manifest
to use the Simple Dashboard that comes with MSRS.


Quick Start Instructions

Download the ZIP file and unzip it into the root directory
of your MSRS installation. It will create all the services
under the folder:
<MSRS>\Apps\QUT
where MSRS is the root of your MSRS installation, e.g.
C:\Microsoft Robotics Studio (1.5)

NOTE: The ZIP file contains all of the services. If you
have installed it already for one of the other services
then you can run the Intro program immediately.

Now open a MSRS DOS Command Prompt window. You need to
compile all of the services. Simply enter the command:

RebuildQUTApps

This runs RebuildQUTApps.cmd which executes all of the
necessary compilation commands and copies the required
textures to the store\media directory.

Once the rebuild has completed, you can run the Intro
program using the following command:

dsshost -port:50000 -tcpport:50001 -manifest:"Apps/QUT/Intro/Intro.manifest.xml"

However, this is a lot of typing so a batch file has
been provided that contains this command. To run the
batch file, just type:

RunIntro

(DOS commands are not case-sensitive, but it looks better
in mixed upper and lower case.)


To edit the code, you can locate Intro.sln in Windows
Explorer and double-click on it. Alternatively, open a
MSRS DOS Command Prompt window. Enter the following two
commands:

cd Apps\QUT\Intro
devenv Intro.sln

This will start Visual Studio.

If your MSRS is NOT installed on the C: drive, then you
will need to change the following Project Property settings:
Change Output Path in Build
Change Post-Build Events in Build Events
Change Start External Program and Working Directory in Debug
Change the Reference Paths

All you need to do is change C: to your drive letter unless
you installed into a different directory, and then it is more
complicated.

Now you can rebuild the code, and when it has finished you
can run it using the debugger.

Note that MSRS is a multi-threaded environment. You might
find the Threads window useful in the debugger if you have
never used it before.


Please note: There is a slightly longer set of Qs/As and Tips
in the Maze Simulator readme.txt. Sorry for the overlap.


Troubleshooting

Q. I don't get a Dashboard.

A. The Dashboard listed in the manifest is my modified one.
If you have not installed it, then obviously you will not get
a Dashboard. Alternatively, modify the manifest to use the
Simple Dashboard that comes with MSRS. Have a look in the
manifest for the Maze Simulator for the appropriate command.


Q. When I run the batch file it says "Invalid manifest identifier".
What's wrong?

A. You are running the batch file from the wrong directory.
Consequently Dsshost can't find the manifest! Make sure that
you are in the MRS root directory before you run the batch
file. (You should have copied the batch file here when you
installed the Maze Simulator.) Note that the MRS root has to
be the Working Directory in the Debug settings in Visual Studio
Project Properties as well or the debugger won't start up.


Q. What is a manifest anyway?

A. A manifest is an XML file that describes the services that
are required to run an application. You can open it and look
at it -- there is nothing magic about a manifest. However,
it allows applications to be associated with different
services at runtime without changing the code. For instance,
you could change from a real robot to the simulator or vice
versa, provided they both implemented the same types of services.


Q. Then I drive the robot into a wall nothing happens. Why?

A. The most likely reason is that the Simulator is running
but the Intro program is not. Have a look in the DOS window
and make sure that there is a startup message from Intro.


Q. Why does the robot sometimes stop?

A. If you look carefully, you will probably see that it is
jammed up against a wall and its wheels are not touching the
ground. This happens because of the bump sensors.

Another problem that happens occassionally is if you hold
the motors on for too long in the Dashboard after running
into a wall. Then when you release the mouse button a stop
command is sent to the robot. If it has just moved away from
the wall, then it stops. Since there are no more bump messages,
it just sits there.

And there is also a very subtle bug somewhere that causes the
robot to stop occassionally. This has to do with the fact that
the code is multi-threaded. You can drive it into a wall again
to re-start it.


Q. If the robot only rotates on the spot and moves forwards
or backwards, why do I sometimes see it moving in an arc?

A. This puzzled me for a while too, until I did some experimenting
using the Dashboard. There are two issues:
1. The robot has inertia. This means it takes a while to get
going, and once it is going, it takes a while to stop. It's
one of Newton's Laws.
2. The motors take a while to come up to speed, especially when
they have to change direction. So immediately after a rotate,
one of the wheels is always going the wrong way when the robot
decides to drive off. This wheel will slowly come up to the
same speed as the other one, and in the meantime the robot
drives in an arc. Ain't physics wonderful?
These are real physical issues, and they show how well the
simulator actually emulates the real world. Well done Microsoft!


Q. The Microsoft documentation talks about the config directory.
Do I have to put my manifest in there?

A. No. The command line that you use for Dsshost specifies
where the manifest is. Putting it into config is only for
convenience.


Q. Why do you use port 50000?

A. It's in the Microsoft examples :-)

Seriously, you can use any available port. However, it is highly
recommended that you use a value above 32768 because many of the
ports with low numbers are "well known ports" and serve special
functions such as FTP, e-mail, web server, etc.



Tips

When Dsshost is running, you can examine a lot of details using
a web browser by simply going to the port that was specified in
the command line.

For example, if you used the batch file included with this code
you would browse to:
http://localhost:50000

I strongly recommend that you become familiar with all of the
information available from the Dsshost page! What is happening
is that Dsshost acts like a web server while it is running.



Trevor Taylor
Faculty of IT, QUT
July 2007
