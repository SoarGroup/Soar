# Microsoft Developer Studio Project File - Name="kernel" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=kernel - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Kernel.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Kernel.mak" CFG="kernel - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "kernel - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "kernel - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "kernel - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W4 /GX /O2 /D "NDEBUG" /D "WIN32" /D "USE_TCL" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\soarkernel.lib"

!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W4 /GX /Z7 /Od /D "_DEBUG" /D "WIN32" /D "USE_TCL" /D "WINDOWS" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\soarkernel.lib"

!ENDIF 

# Begin Target

# Name "kernel - Win32 Release"
# Name "kernel - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\agent.c
# End Source File
# Begin Source File

SOURCE=.\backtrace.c
# End Source File
# Begin Source File

SOURCE=.\callback.c
# End Source File
# Begin Source File

SOURCE=.\chunk.c
# End Source File
# Begin Source File

SOURCE=.\consistency.c
# End Source File
# Begin Source File

SOURCE=.\debugutil.c
# End Source File
# Begin Source File

SOURCE=.\decide.c
# End Source File
# Begin Source File

SOURCE=.\explain.c
# End Source File
# Begin Source File

SOURCE=.\init_soar.c
# End Source File
# Begin Source File

SOURCE=.\io.c
# End Source File
# Begin Source File

SOURCE=.\legacy.c
# End Source File
# Begin Source File

SOURCE=.\lexer.c
# End Source File
# Begin Source File

SOURCE=.\mem.c
# End Source File
# Begin Source File

SOURCE=.\osupport.c
# End Source File
# Begin Source File

SOURCE=.\parser.c
# End Source File
# Begin Source File

SOURCE=.\prefmem.c
# End Source File
# Begin Source File

SOURCE=.\print.c
# End Source File
# Begin Source File

SOURCE=.\production.c
# End Source File
# Begin Source File

SOURCE=.\recmem.c
# End Source File
# Begin Source File

SOURCE=.\reorder.c
# End Source File
# Begin Source File

SOURCE=.\rete.c
# End Source File
# Begin Source File

SOURCE=.\rhsfun.c
# End Source File
# Begin Source File

SOURCE=.\rhsfun_examples.c
# End Source File
# Begin Source File

SOURCE=.\rhsfun_math.c
# End Source File
# Begin Source File

SOURCE=.\scheduler.c
# End Source File
# Begin Source File

SOURCE=.\soar_core_api.c
# End Source File
# Begin Source File

SOURCE=.\soar_core_utils.c
# End Source File
# Begin Source File

SOURCE=.\soar_ecore_api.c
# End Source File
# Begin Source File

SOURCE=.\soar_ecore_utils.c
# End Source File
# Begin Source File

SOURCE=.\soarapi.c
# End Source File
# Begin Source File

SOURCE=.\soarapi_datatypes.c
# End Source File
# Begin Source File

SOURCE=.\soarapiCallbacks.c
# End Source File
# Begin Source File

SOURCE=.\soarapiUtils.c
# End Source File
# Begin Source File

SOURCE=.\soarBuildInfo.c
# End Source File
# Begin Source File

SOURCE=.\symtab.c
# End Source File
# Begin Source File

SOURCE=.\sysdep.c
# End Source File
# Begin Source File

SOURCE=.\tempmem.c
# End Source File
# Begin Source File

SOURCE=.\timers.c
# End Source File
# Begin Source File

SOURCE=.\trace.c
# End Source File
# Begin Source File

SOURCE=.\wmem.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\callback.h
# End Source File
# Begin Source File

SOURCE=.\debugutil.h
# End Source File
# Begin Source File

SOURCE=.\explain.h
# End Source File
# Begin Source File

SOURCE=.\rete.h
# End Source File
# Begin Source File

SOURCE=.\rhsfun.h
# End Source File
# Begin Source File

SOURCE=.\rhsfun_examples.h
# End Source File
# Begin Source File

SOURCE=.\rhsfun_math.h
# End Source File
# Begin Source File

SOURCE=.\scheduler.h
# End Source File
# Begin Source File

SOURCE=.\soar_core_api.h
# End Source File
# Begin Source File

SOURCE=.\soar_core_utils.h
# End Source File
# Begin Source File

SOURCE=.\soar_ecore_api.h
# End Source File
# Begin Source File

SOURCE=.\soar_ecore_utils.h
# End Source File
# Begin Source File

SOURCE=.\soarapi.h
# End Source File
# Begin Source File

SOURCE=.\soarapi_datatypes.h
# End Source File
# Begin Source File

SOURCE=.\soarapiCallbacks.h
# End Source File
# Begin Source File

SOURCE=.\soarapiUtils.h
# End Source File
# Begin Source File

SOURCE=.\soarBuildOptions.h
# End Source File
# Begin Source File

SOURCE=.\soarkernel.h
# End Source File
# Begin Source File

SOURCE=.\sysdep.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
