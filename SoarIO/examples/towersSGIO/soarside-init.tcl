# switch to the location of the version of Soar we want to use

cd C:\\Progra~1\\Soar\\Soar-Suite-8.5.2\\soar-8.5.2\\

# start Soar
source init-soar.tcl

# switch to the Agents directory we want to use
#
# we have two directories listed below, for two different example programs
# normally we would create two different init files, one for each program,
#  but we chose to do this instead so the soarside program could be run
#  without any parameters (i.e. without specifying which init file to load)
# instead, all you need to do is uncomment the line for the program you want to run
#
#cd ..\\sgio-1.0.5\\examples\\sgiotest\\agents\\
#cd ..\\sgio-1.0.5\\examples\\simple\\agents\\
cd C:\\Docume~1\\stokesd\\Desktop\\SoarProjects\\SoarIO\\examples\\towersSGIO\\

puts "Error is $errorInfo"