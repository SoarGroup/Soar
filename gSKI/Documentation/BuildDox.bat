rem ********************************************************************
rem  @file BuildDox.bat 
rem ***************************************************************************
rem  @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
rem  The U.S. government has non-exclusive license to this software 
rem  for government purposes. 
rem *************************************************************************** 
rem  created:	   5/31/2002   11:14
rem 
rem  purpose: 
rem ********************************************************************/

@echo on
echo Changing Directories
cd ..\..\

echo Building Doxygen
bin\doxygen

echo DONE!!!
