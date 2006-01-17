# Microsoft Developer Studio Generated NMAKE File, Based on SoarInterface.dsp
!IF "$(CFG)" == ""
CFG=dll - Win32 Debug_tcl76
!MESSAGE No configuration specified. Defaulting to dll - Win32 Debug_tcl76.
!ENDIF 

!IF "$(CFG)" != "dll - Win32 Release" && "$(CFG)" != "dll - Win32 Debug" &&\
 "$(CFG)" != "dll - Win32 Release_tcl76" && "$(CFG)" !=\
 "dll - Win32 Debug_tcl76"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SoarInterface.mak" CFG="dll - Win32 Debug_tcl76"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dll - Win32 Release_tcl76" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dll - Win32 Debug_tcl76" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "dll - Win32 Release"

OUTDIR=.
INTDIR=.\Release
# Begin Custom Macros
OutDir=.
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\libsoar8.2.dll" "$(OUTDIR)\SoarInterface.bsc"

!ELSE 

ALL : "$(OUTDIR)\libsoar8.2.dll" "$(OUTDIR)\SoarInterface.bsc"

!ENDIF 

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
	-@erase "$(INTDIR)\soarScheduler.obj"
	-@erase "$(INTDIR)\soarScheduler.sbr"
	-@erase "$(INTDIR)\soarVars.obj"
	-@erase "$(INTDIR)\soarVars.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\libsoar8.2.dll"
	-@erase "$(OUTDIR)\libsoar8.2.exp"
	-@erase "$(OUTDIR)\libsoar8.2.lib"
	-@erase "$(OUTDIR)\SoarInterface.bsc"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "C:\Program Files\Tcl\include" /I\
 "..\kernel" /D "NDEBUG" /D "WIN32" /D "USE_TCL" /FR"$(INTDIR)\\"\
 /Fp"$(INTDIR)\SoarInterface.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\Release/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
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
	"$(INTDIR)\soarScheduler.sbr" \
	"$(INTDIR)\soarVars.sbr"

"$(OUTDIR)\SoarInterface.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=..\kernel\soarkernel.lib tcl80vc.lib kernel32.lib user32.lib\
 gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib\
 oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll\
 /incremental:no /pdb:"$(OUTDIR)\libsoar8.2.pdb" /machine:I386\
 /nodefaultlib:"libcmt.lib" /out:"$(OUTDIR)\libsoar8.2.dll"\
 /implib:"$(OUTDIR)\libsoar8.2.lib" 
LINK32_OBJS= \
	"$(INTDIR)\soarAgent.obj" \
	"$(INTDIR)\soarArgv.obj" \
	"$(INTDIR)\soarCommands.obj" \
	"$(INTDIR)\soarCommandUtils.obj" \
	"$(INTDIR)\soarInterp.obj" \
	"$(INTDIR)\soarLog.obj" \
	"$(INTDIR)\soarMain.obj" \
	"$(INTDIR)\soarScheduler.obj" \
	"$(INTDIR)\soarVars.obj"

"$(OUTDIR)\libsoar8.2.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE=$(InputPath)
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.
# End Custom Macros

$(DS_POSTBUILD_DEP) : "$(OUTDIR)\libsoar8.2.dll" "$(OUTDIR)\SoarInterface.bsc"
   copy libsoar8.2.dll ..\library
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "dll - Win32 Debug"

OUTDIR=.
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\soar71.dll" "$(OUTDIR)\SoarInterface.bsc"

!ELSE 

ALL : "$(OUTDIR)\soar71.dll" "$(OUTDIR)\SoarInterface.bsc"

!ENDIF 

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
	-@erase "$(INTDIR)\soarScheduler.obj"
	-@erase "$(INTDIR)\soarScheduler.sbr"
	-@erase "$(INTDIR)\soarVars.obj"
	-@erase "$(INTDIR)\soarVars.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\soar71.dll"
	-@erase "$(OUTDIR)\soar71.exp"
	-@erase "$(OUTDIR)\soar71.ilk"
	-@erase "$(OUTDIR)\soar71.lib"
	-@erase "$(OUTDIR)\soar71.pdb"
	-@erase "$(OUTDIR)\SoarInterface.bsc"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "C:\Program Files\Tcl\include" /I\
 "..\kernel" /D "_DEBUG" /D "WIN32" /D "USE_TCL" /FR"$(INTDIR)\\"\
 /Fp"$(INTDIR)\SoarInterface.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
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
	"$(INTDIR)\soarScheduler.sbr" \
	"$(INTDIR)\soarVars.sbr"

"$(OUTDIR)\SoarInterface.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=..\kernel\soarkernel.lib tcl76.lib kernel32.lib user32.lib\
 gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib\
 oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll\
 /incremental:yes /pdb:"$(OUTDIR)\soar71.pdb" /debug /machine:I386\
 /nodefaultlib:"libcmtd.lib" /out:"$(OUTDIR)\soar71.dll"\
 /implib:"$(OUTDIR)\soar71.lib" 
LINK32_OBJS= \
	"$(INTDIR)\soarAgent.obj" \
	"$(INTDIR)\soarArgv.obj" \
	"$(INTDIR)\soarCommands.obj" \
	"$(INTDIR)\soarCommandUtils.obj" \
	"$(INTDIR)\soarInterp.obj" \
	"$(INTDIR)\soarLog.obj" \
	"$(INTDIR)\soarMain.obj" \
	"$(INTDIR)\soarScheduler.obj" \
	"$(INTDIR)\soarVars.obj"

"$(OUTDIR)\soar71.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dll - Win32 Release_tcl76"

OUTDIR=.\dll___Wi
INTDIR=.\dll___Wi
# Begin Custom Macros
OutDir=.\dll___Wi
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : ".\libsoar8.2.dll" "$(OUTDIR)\SoarInterface.bsc"

!ELSE 

ALL : ".\libsoar8.2.dll" "$(OUTDIR)\SoarInterface.bsc"

!ENDIF 

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
	-@erase "$(INTDIR)\soarScheduler.obj"
	-@erase "$(INTDIR)\soarScheduler.sbr"
	-@erase "$(INTDIR)\soarVars.obj"
	-@erase "$(INTDIR)\soarVars.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\libsoar8.2.exp"
	-@erase "$(OUTDIR)\libsoar8.2.lib"
	-@erase "$(OUTDIR)\SoarInterface.bsc"
	-@erase ".\libsoar8.2.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "C:\Program Files\Tcl\include" /I\
 "..\kernel" /D "NDEBUG" /D "WIN32" /D "USE_TCL" /FR"$(INTDIR)\\"\
 /Fp"$(INTDIR)\SoarInterface.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\dll___Wi/
CPP_SBRS=.\dll___Wi/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
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
	"$(INTDIR)\soarScheduler.sbr" \
	"$(INTDIR)\soarVars.sbr"

"$(OUTDIR)\SoarInterface.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=..\kernel\soarkernel.lib tcl80vc.lib kernel32.lib user32.lib\
 gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib\
 oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll\
 /incremental:no /pdb:"$(OUTDIR)\libsoar8.2.pdb" /machine:I386\
 /nodefaultlib:"libcmt.lib" /out:".\libsoar8.2.dll"\
 /implib:"$(OUTDIR)\libsoar8.2.lib" 
LINK32_OBJS= \
	"$(INTDIR)\soarAgent.obj" \
	"$(INTDIR)\soarArgv.obj" \
	"$(INTDIR)\soarCommands.obj" \
	"$(INTDIR)\soarCommandUtils.obj" \
	"$(INTDIR)\soarInterp.obj" \
	"$(INTDIR)\soarLog.obj" \
	"$(INTDIR)\soarMain.obj" \
	"$(INTDIR)\soarScheduler.obj" \
	"$(INTDIR)\soarVars.obj"

".\libsoar8.2.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE=$(InputPath)
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\dll___Wi
# End Custom Macros

$(DS_POSTBUILD_DEP) : ".\libsoar8.2.dll" "$(OUTDIR)\SoarInterface.bsc"
   copy libsoar8.2.dll ..\library
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "dll - Win32 Debug_tcl76"

OUTDIR=.\dll___W0
INTDIR=.\dll___W0
# Begin Custom Macros
OutDir=.\dll___W0
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : ".\soar71.dll" "$(OUTDIR)\SoarInterface.bsc"

!ELSE 

ALL : ".\soar71.dll" "$(OUTDIR)\SoarInterface.bsc"

!ENDIF 

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
	-@erase "$(INTDIR)\soarScheduler.obj"
	-@erase "$(INTDIR)\soarScheduler.sbr"
	-@erase "$(INTDIR)\soarVars.obj"
	-@erase "$(INTDIR)\soarVars.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\soar71.exp"
	-@erase "$(OUTDIR)\soar71.lib"
	-@erase "$(OUTDIR)\soar71.pdb"
	-@erase "$(OUTDIR)\SoarInterface.bsc"
	-@erase ".\soar71.dll"
	-@erase ".\soar71.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "C:\Program Files\Tcl\include" /I\
 "..\kernel" /D "_DEBUG" /D "WIN32" /D "USE_TCL" /FR"$(INTDIR)\\"\
 /Fp"$(INTDIR)\SoarInterface.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\dll___W0/
CPP_SBRS=.\dll___W0/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
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
	"$(INTDIR)\soarScheduler.sbr" \
	"$(INTDIR)\soarVars.sbr"

"$(OUTDIR)\SoarInterface.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=..\kernel\soarkernel.lib tcl76.lib kernel32.lib user32.lib\
 gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib\
 oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll\
 /incremental:yes /pdb:"$(OUTDIR)\soar71.pdb" /debug /machine:I386\
 /nodefaultlib:"libcmtd.lib" /out:".\soar71.dll" /implib:"$(OUTDIR)\soar71.lib" 
LINK32_OBJS= \
	"$(INTDIR)\soarAgent.obj" \
	"$(INTDIR)\soarArgv.obj" \
	"$(INTDIR)\soarCommands.obj" \
	"$(INTDIR)\soarCommandUtils.obj" \
	"$(INTDIR)\soarInterp.obj" \
	"$(INTDIR)\soarLog.obj" \
	"$(INTDIR)\soarMain.obj" \
	"$(INTDIR)\soarScheduler.obj" \
	"$(INTDIR)\soarVars.obj"

".\soar71.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "dll - Win32 Release" || "$(CFG)" == "dll - Win32 Debug" ||\
 "$(CFG)" == "dll - Win32 Release_tcl76" || "$(CFG)" ==\
 "dll - Win32 Debug_tcl76"
SOURCE=.\soarAgent.c

!IF  "$(CFG)" == "dll - Win32 Release"

DEP_CPP_SOARA=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\kernel\callback.h"\
	"..\kernel\scheduler.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommands.h"\
	

"$(INTDIR)\soarAgent.obj"	"$(INTDIR)\soarAgent.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug"

DEP_CPP_SOARA=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\scheduler.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommands.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarAgent.obj"	"$(INTDIR)\soarAgent.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Release_tcl76"

DEP_CPP_SOARA=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\scheduler.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommands.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarAgent.obj"	"$(INTDIR)\soarAgent.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug_tcl76"

DEP_CPP_SOARA=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\scheduler.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommands.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarAgent.obj"	"$(INTDIR)\soarAgent.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARA) "$(INTDIR)"


!ENDIF 

SOURCE=.\soarArgv.c

!IF  "$(CFG)" == "dll - Win32 Release"

DEP_CPP_SOARAR=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	

"$(INTDIR)\soarArgv.obj"	"$(INTDIR)\soarArgv.sbr" : $(SOURCE) $(DEP_CPP_SOARAR)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug"

DEP_CPP_SOARAR=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarArgv.obj"	"$(INTDIR)\soarArgv.sbr" : $(SOURCE) $(DEP_CPP_SOARAR)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Release_tcl76"

DEP_CPP_SOARAR=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarArgv.obj"	"$(INTDIR)\soarArgv.sbr" : $(SOURCE) $(DEP_CPP_SOARAR)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug_tcl76"

DEP_CPP_SOARAR=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarArgv.obj"	"$(INTDIR)\soarArgv.sbr" : $(SOURCE) $(DEP_CPP_SOARAR)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\soarCommands.c

!IF  "$(CFG)" == "dll - Win32 Release"

DEP_CPP_SOARC=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommands.h"\
	".\soarCommandUtils.h"\
	".\soarScheduler.h"\
	

"$(INTDIR)\soarCommands.obj"	"$(INTDIR)\soarCommands.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug"

DEP_CPP_SOARC=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommands.h"\
	".\soarCommandUtils.h"\
	".\soarScheduler.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarCommands.obj"	"$(INTDIR)\soarCommands.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Release_tcl76"

DEP_CPP_SOARC=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommands.h"\
	".\soarCommandUtils.h"\
	".\soarScheduler.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarCommands.obj"	"$(INTDIR)\soarCommands.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug_tcl76"

DEP_CPP_SOARC=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommands.h"\
	".\soarCommandUtils.h"\
	".\soarScheduler.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarCommands.obj"	"$(INTDIR)\soarCommands.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARC) "$(INTDIR)"


!ENDIF 

SOURCE=.\soarCommandUtils.c

!IF  "$(CFG)" == "dll - Win32 Release"

DEP_CPP_SOARCO=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommandUtils.h"\
	

"$(INTDIR)\soarCommandUtils.obj"	"$(INTDIR)\soarCommandUtils.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARCO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug"

DEP_CPP_SOARCO=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommandUtils.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarCommandUtils.obj"	"$(INTDIR)\soarCommandUtils.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARCO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Release_tcl76"

DEP_CPP_SOARCO=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommandUtils.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarCommandUtils.obj"	"$(INTDIR)\soarCommandUtils.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARCO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug_tcl76"

DEP_CPP_SOARCO=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommandUtils.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarCommandUtils.obj"	"$(INTDIR)\soarCommandUtils.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARCO) "$(INTDIR)"


!ENDIF 

SOURCE=.\soarInterp.c

!IF  "$(CFG)" == "dll - Win32 Release"

DEP_CPP_SOARI=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommandUtils.h"\
	".\soarScheduler.h"\
	

"$(INTDIR)\soarInterp.obj"	"$(INTDIR)\soarInterp.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug"

DEP_CPP_SOARI=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommandUtils.h"\
	".\soarScheduler.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarInterp.obj"	"$(INTDIR)\soarInterp.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Release_tcl76"

DEP_CPP_SOARI=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommandUtils.h"\
	".\soarScheduler.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarInterp.obj"	"$(INTDIR)\soarInterp.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug_tcl76"

DEP_CPP_SOARI=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommandUtils.h"\
	".\soarScheduler.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarInterp.obj"	"$(INTDIR)\soarInterp.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARI) "$(INTDIR)"


!ENDIF 

SOURCE=.\soarLog.c

!IF  "$(CFG)" == "dll - Win32 Release"

DEP_CPP_SOARL=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	

"$(INTDIR)\soarLog.obj"	"$(INTDIR)\soarLog.sbr" : $(SOURCE) $(DEP_CPP_SOARL)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug"

DEP_CPP_SOARL=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarLog.obj"	"$(INTDIR)\soarLog.sbr" : $(SOURCE) $(DEP_CPP_SOARL)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Release_tcl76"

DEP_CPP_SOARL=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarLog.obj"	"$(INTDIR)\soarLog.sbr" : $(SOURCE) $(DEP_CPP_SOARL)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug_tcl76"

DEP_CPP_SOARL=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarLog.obj"	"$(INTDIR)\soarLog.sbr" : $(SOURCE) $(DEP_CPP_SOARL)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\soarMain.c

!IF  "$(CFG)" == "dll - Win32 Release"

DEP_CPP_SOARM=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	

"$(INTDIR)\soarMain.obj"	"$(INTDIR)\soarMain.sbr" : $(SOURCE) $(DEP_CPP_SOARM)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug"

DEP_CPP_SOARM=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarMain.obj"	"$(INTDIR)\soarMain.sbr" : $(SOURCE) $(DEP_CPP_SOARM)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Release_tcl76"

DEP_CPP_SOARM=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarMain.obj"	"$(INTDIR)\soarMain.sbr" : $(SOURCE) $(DEP_CPP_SOARM)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug_tcl76"

DEP_CPP_SOARM=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarMain.obj"	"$(INTDIR)\soarMain.sbr" : $(SOURCE) $(DEP_CPP_SOARM)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\soarScheduler.c

!IF  "$(CFG)" == "dll - Win32 Release"

DEP_CPP_SOARS=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommands.h"\
	".\soarCommandUtils.h"\
	".\soarScheduler.h"\
	

"$(INTDIR)\soarScheduler.obj"	"$(INTDIR)\soarScheduler.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug"

DEP_CPP_SOARS=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommands.h"\
	".\soarCommandUtils.h"\
	".\soarScheduler.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarScheduler.obj"	"$(INTDIR)\soarScheduler.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Release_tcl76"

DEP_CPP_SOARS=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommands.h"\
	".\soarCommandUtils.h"\
	".\soarScheduler.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarScheduler.obj"	"$(INTDIR)\soarScheduler.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug_tcl76"

DEP_CPP_SOARS=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	".\soarCommands.h"\
	".\soarCommandUtils.h"\
	".\soarScheduler.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarScheduler.obj"	"$(INTDIR)\soarScheduler.sbr" : $(SOURCE)\
 $(DEP_CPP_SOARS) "$(INTDIR)"


!ENDIF 

SOURCE=.\soarVars.c

!IF  "$(CFG)" == "dll - Win32 Release"

DEP_CPP_SOARV=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	

"$(INTDIR)\soarVars.obj"	"$(INTDIR)\soarVars.sbr" : $(SOURCE) $(DEP_CPP_SOARV)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug"

DEP_CPP_SOARV=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarVars.obj"	"$(INTDIR)\soarVars.sbr" : $(SOURCE) $(DEP_CPP_SOARV)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Release_tcl76"

DEP_CPP_SOARV=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarVars.obj"	"$(INTDIR)\soarVars.sbr" : $(SOURCE) $(DEP_CPP_SOARV)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dll - Win32 Debug_tcl76"

DEP_CPP_SOARV=\
	"..\..\program files\tcl\include\tcl.h"\
	"..\..\Program Files\Tcl\include\tk.h"\
	"..\..\Program Files\Tcl\include\X11\X.h"\
	"..\..\Program Files\Tcl\include\X11\Xfuncproto.h"\
	"..\..\Program Files\Tcl\include\X11\Xlib.h"\
	"..\..\Program Files\Tcl\include\X11\Xutil.h"\
	"..\kernel\callback.h"\
	"..\kernel\soarkernel.h"\
	".\soar.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\soarVars.obj"	"$(INTDIR)\soarVars.sbr" : $(SOURCE) $(DEP_CPP_SOARV)\
 "$(INTDIR)"


!ENDIF 


!ENDIF 

