@ECHO ON
REM Run the Intro Program
REM This command must be run from the MRS root directory
REM in an MRS DOS Command Prompt window.
REM Type Ctrl-C in this window when you want to stop the program.

dsshost -port:50000 -tcpport:50001 -manifest:"Apps\QUT\Intro\Intro.manifest.xml"
