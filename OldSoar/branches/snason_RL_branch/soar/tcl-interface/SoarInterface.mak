# Microsoft Developer Studio Generated NMAKE File, Based on SoarInterface.dsp
!IF "$(CFG)" == ""
CFG=dll - Win32 Release
!MESSAGE No configuration specified. Defaulting to dll - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "dll - Win32 Release" && "$(CFG)" != "dll - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SoarInterface.mak" CFG="dll - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dll - Win32 Release"

OUTDIR=.
INTDIR=.\Release
# Begin Custom Macros
OutDir=.
# End Custom Macros

ALL : "$(OUTDIR)\libsoar8.4.5.dll" "$(OUTDIR)\SoarInterface.bsc"


CLEAN :
	-@erase "$(INTDIR)\soarAgent.obj"
	-@erase "$(INTDIR)\soarAgent.sbr"
	-@erase "$(INTDIR)\soarArgv.obj"
	-@erase "$(INTDIR)\soarArgv.sbr"
	-@erase "$(INTDIR)\soarCommands.obj"
	-@erase "$(INTDIR)\soarCommands.sbr"
	-@erase "$(INTDIR)\soarCommandUtils.obj"
	-@erase "$(INTDIR)\soarCommandUtils.sbr"
	-@erase "$(INTDIR)\soarInterp.obj"
	-@erase "$(INTDIR)\soarInterp.sbr"
	-@erase "$(INTDIR)\soarLog.obj"
	-@erase "$(INTDIR)\soarLog.sbr"
	-@erase "$(INTDIR)\soarMain.obj"
	-@erase "$(INTDIR)\soarMain.sbr"
	-@erase "$(INTDIR)\soarVars.obj"
	-@erase "$(INTDIR)\soarVars.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\libsoar8.4.5.dll"
	-@erase "$(OUTDIR)\libsoar8.4.5.exp"
	-@erase "$(OUTDIR)\libsoar8.4.5.lib"
	-@erase "$(OUTDIR)\SoarInterface.bsc"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MTd /W3 /GX /O2 /I "C:\Tcl\include" /I "..\kernel" /D "NDEBUG" /D "WIN32" /D "USE_TCL" /D "WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\SoarInterface.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SoarInterface.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\soarAgent.sbr" \
	"$(INTDIR)\soarArgv.sbr" \
	"$(INTDIR)\soarCommands.sbr" \
	"$(INTDIR)\soarCommandUtils.sbr" \
	"$(INTDIR)\soarInterp.sbr" \
	"$(INTDIR)\soarLog.sbr" \
	"$(INTDIR)\soarMain.sbr" \
	"$(INTDIR)\soarVars.sbr"

"$(OUTDIR)\SoarInterface.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=soarkernel.lib tcl84.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\libsoar8.4.5.pdb" /machine:I386 /out:"$(OUTDIR)\libsoar8.4.5.dll" /implib:"$(OUTDIR)\libsoar8.4.5.lib" /libpath:"..\kernel" /libpath:"c:\Tcl\lib" 
LINK32_OBJS= \
	"$(INTDIR)\soarAgent.obj" \
	"$(INTDIR)\soarArgv.obj" \
	"$(INTDIR)\soarCommands.obj" \
	"$(INTDIR)\soarCommandUtils.obj" \
	"$(INTDIR)\soarInterp.obj" \
	"$(INTDIR)\soarLog.obj" \
	"$(INTDIR)\soarMain.obj" \
	"$(INTDIR)\soarVars.obj"

"$(OUTDIR)\libsoar8.4.5.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\libsoar8.4.5.dll" "$(OUTDIR)\SoarInterface.bsc"
   copy libsoar8.4.5.dll ..\library
	del libsoar8.4.5.dll
	del libsoar8.4.5.lib
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "dll - Win32 Debug"

OUTDIR=.
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.
# End Custom Macros

ALL : "$(OUTDIR)\libsoar8.4.5.dll" "$(OUTDIR)\SoarInterface.bsc"


CLEAN :
	-@erase "$(INTDIR)\soarAgent.obj"
	-@erase "$(INTDIR)\soarAgent.sbr"
	-@erase "$(INTDIR)\soarArgv.obj"
	-@erase "$(INTDIR)\soarArgv.sbr"
	-@erase "$(INTDIR)\soarCommands.obj"
	-@erase "$(INTDIR)\soarCommands.sbr"
	-@erase "$(INTDIR)\soarCommandUtils.obj"
	-@erase "$(INTDIR)\soarCommandUtils.sbr"
	-@erase "$(INTDIR)\soarInterp.obj"
	-@erase "$(INTDIR)\soarInterp.sbr"
	-@erase "$(INTDIR)\soarLog.obj"
	-@erase "$(INTDIR)\soarLog.sbr"
	-@erase "$(INTDIR)\soarMain.obj"
	-@erase "$(INTDIR)\soarMain.sbr"
	-@erase "$(INTDIR)\soarVars.obj"
	-@erase "$(INTDIR)\soarVars.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\libsoar8.4.5.dll"
	-@erase "$(OUTDIR)\libsoar8.4.5.exp"
	-@erase "$(OUTDIR)\libsoar8.4.5.ilk"
	-@erase "$(OUTDIR)\libsoar8.4.5.lib"
	-@erase "$(OUTDIR)\libsoar8.4.5.pdb"
	-@erase "$(OUTDIR)\SoarInterface.bsc"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "C:\Tcl\include" /I "..\kernel" /D "_DEBUG" /D "WIN32" /D "USE_TCL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\SoarInterface.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SoarInterface.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\soarAgent.sbr" \
	"$(INTDIR)\soarArgv.sbr" \
	"$(INTDIR)\soarCommands.sbr" \
	"$(INTDIR)\soarCommandUtils.sbr" \
	"$(INTDIR)\soarInterp.sbr" \
	"$(INTDIR)\soarLog.sbr" \
	"$(INTDIR)\soarMain.sbr" \
	"$(INTDIR)\soarVars.sbr"

"$(OUTDIR)\SoarInterface.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=soarkernel.lib tcl84.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\libsoar8.4.5.pdb" /debug /machine:I386 /out:"$(OUTDIR)\libsoar8.4.5.dll" /implib:"$(OUTDIR)\libsoar8.4.5.lib" /libpath:"..\kernel" /libpath:"c:\Tcl\lib" 
LINK32_OBJS= \
	"$(INTDIR)\soarAgent.obj" \
	"$(INTDIR)\soarArgv.obj" \
	"$(INTDIR)\soarCommands.obj" \
	"$(INTDIR)\soarCommandUtils.obj" \
	"$(INTDIR)\soarInterp.obj" \
	"$(INTDIR)\soarLog.obj" \
	"$(INTDIR)\soarMain.obj" \
	"$(INTDIR)\soarVars.obj"

"$(OUTDIR)\libsoar8.4.5.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\libsoar8.4.5.dll" "$(OUTDIR)\SoarInterface.bsc"
   copy libsoar8.4.5.dll ..\library
	del libsoar8.4.5.dll
	del libsoar8.4.5.lib
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("SoarInterface.dep")
!INCLUDE "SoarInterface.dep"
!ELSE 
!MESSAGE Warning: cannot find "SoarInterface.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "dll - Win32 Release" || "$(CFG)" == "dll - Win32 Debug"
SOURCE=.\soarAgent.c

"$(INTDIR)\soarAgent.obj"	"$(INTDIR)\soarAgent.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\soarArgv.c

"$(INTDIR)\soarArgv.obj"	"$(INTDIR)\soarArgv.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\soarCommands.c

"$(INTDIR)\soarCommands.obj"	"$(INTDIR)\soarCommands.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\soarCommandUtils.c

"$(INTDIR)\soarCommandUtils.obj"	"$(INTDIR)\soarCommandUtils.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\soarInterp.c

"$(INTDIR)\soarInterp.obj"	"$(INTDIR)\soarInterp.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\soarLog.c

"$(INTDIR)\soarLog.obj"	"$(INTDIR)\soarLog.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\soarMain.c

"$(INTDIR)\soarMain.obj"	"$(INTDIR)\soarMain.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\soarVars.c

"$(INTDIR)\soarVars.obj"	"$(INTDIR)\soarVars.sbr" : $(SOURCE) "$(INTDIR)"



!ENDIF 

