# Microsoft Developer Studio Project File - Name="SoarKernelTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=SoarKernelTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SoarKernelTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SoarKernelTest.mak" CFG="SoarKernelTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SoarKernelTest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "SoarKernelTest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SoarKernelTest - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(ProjectDir)headers" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "SoarKernelTest - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../build/headers" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Lua+Lib.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\build\debug"

!ENDIF 

# Begin Target

# Name "SoarKernelTest - Win32 Release"
# Name "SoarKernelTest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\diff.cpp
# End Source File
# Begin Source File

SOURCE=.\dummy.cpp
# End Source File
# Begin Source File

SOURCE=.\Loar_interface.cpp
# End Source File
# Begin Source File

SOURCE=.\LuaEnum.cpp
# End Source File
# Begin Source File

SOURCE=.\mainKernelTest.cpp
# End Source File
# Begin Source File

SOURCE=.\new_api.cpp
# End Source File
# Begin Source File

SOURCE=.\new_soar.cpp
# End Source File
# Begin Source File

SOURCE=.\soar_build_info.cpp
# End Source File
# Begin Source File

SOURCE=.\soar_core_api.cpp
# End Source File
# Begin Source File

SOURCE=.\soar_ecore_api.cpp
# End Source File
# Begin Source File

SOURCE=.\soarapi.cpp
# End Source File
# Begin Source File

SOURCE=".\source-cmd.cpp"
# End Source File
# Begin Source File

SOURCE=.\utilities.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\definitions.h
# End Source File
# Begin Source File

SOURCE=.\diff.h
# End Source File
# Begin Source File

SOURCE=.\dummy.h
# End Source File
# Begin Source File

SOURCE=.\lauxlib.h
# End Source File
# Begin Source File

SOURCE=.\Loar_interface.h
# End Source File
# Begin Source File

SOURCE=.\lua.h
# End Source File
# Begin Source File

SOURCE=.\luadebug.h
# End Source File
# Begin Source File

SOURCE=.\LuaEnum.h
# End Source File
# Begin Source File

SOURCE=.\lualib.h
# End Source File
# Begin Source File

SOURCE=.\new_api.h
# End Source File
# Begin Source File

SOURCE=.\new_soar.h
# End Source File
# Begin Source File

SOURCE=.\sk.h
# End Source File
# Begin Source File

SOURCE=.\soar_core_api.h
# End Source File
# Begin Source File

SOURCE=.\soar_ecore_api.h
# End Source File
# Begin Source File

SOURCE=.\soarapi.h
# End Source File
# Begin Source File

SOURCE=.\source_cmd.h
# End Source File
# Begin Source File

SOURCE=.\test2.h
# End Source File
# Begin Source File

SOURCE=.\utilities.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Lua"

# PROP Default_Filter "*.lua"
# Begin Source File

SOURCE=..\build\run\LuaFiles\addwme.lua
# End Source File
# Begin Source File

SOURCE=..\build\run\LuaFiles\common.lua
# End Source File
# Begin Source File

SOURCE=..\build\run\LuaFiles\lua.txt
# End Source File
# Begin Source File

SOURCE=..\build\run\LuaFiles\luac.txt
# End Source File
# Begin Source File

SOURCE=..\build\run\LuaFiles\misc_commands.lua
# End Source File
# Begin Source File

SOURCE=..\build\run\LuaFiles\test1.lua
# End Source File
# Begin Source File

SOURCE=..\build\run\LuaFiles\test2.lua
# End Source File
# Begin Source File

SOURCE=..\build\run\LuaFiles\test3.lua
# End Source File
# Begin Source File

SOURCE=..\build\run\LuaFiles\test4.lua
# End Source File
# Begin Source File

SOURCE=..\build\run\LuaFiles\test4a.lua
# End Source File
# Begin Source File

SOURCE=..\build\run\LuaFiles\test5.lua
# End Source File
# Begin Source File

SOURCE=..\build\run\LuaFiles\test6.lua
# End Source File
# Begin Source File

SOURCE=..\build\run\LuaFiles\test7.lua
# End Source File
# Begin Source File

SOURCE=..\build\run\LuaFiles\test8.lua
# End Source File
# Begin Source File

SOURCE=..\build\run\LuaFiles\testfile.lua
# End Source File
# End Group
# Begin Group "Soar Test Productions"

# PROP Default_Filter "*.soar"
# Begin Source File

SOURCE="..\build\run\TestProductions\blocks-opsub.soar"
# End Source File
# Begin Source File

SOURCE="..\build\run\TestProductions\blocks-world-backtrace.soar"
# End Source File
# Begin Source File

SOURCE="..\build\run\TestProductions\blocks-world.soar"
# End Source File
# Begin Source File

SOURCE=..\build\run\TestProductions\halt.soar
# End Source File
# Begin Source File

SOURCE="..\build\run\TestProductions\operator-subgoaling.soar"
# End Source File
# Begin Source File

SOURCE=..\build\run\TestProductions\rhsfun_math.soar
# End Source File
# Begin Source File

SOURCE=..\build\run\TestProductions\selection.soar
# End Source File
# Begin Source File

SOURCE=..\build\run\TestProductions\simple.soar
# End Source File
# Begin Source File

SOURCE=..\build\run\TestProductions\test.soar
# End Source File
# Begin Source File

SOURCE=..\build\run\TestProductions\tictactoe.soar
# End Source File
# Begin Source File

SOURCE="..\build\run\TestProductions\towers-of-hanoi-fast.soar"
# End Source File
# Begin Source File

SOURCE="..\build\run\TestProductions\towers-of-hanoi-faster.soar"
# End Source File
# Begin Source File

SOURCE="..\build\run\TestProductions\towers-of-hanoi-no-ops.soar"
# End Source File
# Begin Source File

SOURCE="..\build\run\TestProductions\towers-of-hanoi-recur.soar"
# End Source File
# Begin Source File

SOURCE="..\build\run\TestProductions\towers-of-hanoi.soar"
# End Source File
# Begin Source File

SOURCE=..\build\run\TestProductions\waterjug.soar
# End Source File
# Begin Source File

SOURCE=..\build\run\TestProductions\waterjug2.soar
# End Source File
# End Group
# End Target
# End Project
