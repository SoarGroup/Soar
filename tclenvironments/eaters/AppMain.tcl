set env(LD_LIBRARY_PATH) "/Applications/Soar-8.6.1/lib"
set env(DYLID_LIBRARY_PATH) "/Applications/Soar-8.6.1/lib"
set env(TCLLIBPATH) "/Applications/Soar-8.6.1/lib"
set auto_path "$auto_path /Applications/Soar-8.6.1/lib"

set argv ""
cd /Applications/Soar-8.6.1/soar-library
exec wish /Applications/Soar-8.6.1/tclenvironments/eaters/init-eaters.tcl &
#source /Applications/Soar-8.6.1/tclenvironments/eaters/init-eaters.tcl
exit