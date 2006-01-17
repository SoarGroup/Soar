# Microsoft Developer Studio Generated NMAKE File, Format Version 40001
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=kernel - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to kernel - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "kernel - Win32 Release" && "$(CFG)" != "kernel - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Kernel.mak" CFG="kernel - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "kernel - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "kernel - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "kernel - Win32 Debug"
CPP=cl.exe

!IF  "$(CFG)" == "kernel - Win32 Release"

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
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\soarkernel.lib"

CLEAN : 
	-@erase ".\soarkernel.lib"
	-@erase ".\Release\trace.obj"
	-@erase ".\Release\print.obj"
	-@erase ".\Release\chunk.obj"
	-@erase ".\Release\tempmem.obj"
	-@erase ".\Release\symtab.obj"
	-@erase ".\Release\ma_interface.obj"
	-@erase ".\Release\mem.obj"
	-@erase ".\Release\lexer.obj"
	-@erase ".\Release\decide.obj"
	-@erase ".\Release\io.obj"
	-@erase ".\Release\backtrace.obj"
	-@erase ".\Release\main.obj"
	-@erase ".\Release\interface.obj"
	-@erase ".\Release\parser.obj"
	-@erase ".\Release\rete.obj"
	-@erase ".\Release\init_soar.obj"
	-@erase ".\Release\reorder.obj"
	-@erase ".\Release\callback.obj"
	-@erase ".\Release\recmem.obj"
	-@erase ".\Release\rhsfun_math.obj"
	-@erase ".\Release\production.obj"
	-@erase ".\Release\agent.obj"
	-@erase ".\Release\wmem.obj"
	-@erase ".\Release\explain.obj"
	-@erase ".\Release\rhsfun.obj"
	-@erase ".\Release\osupport.obj"
	-@erase ".\Release\tilde.obj"
	-@erase ".\Release\prefmem.obj"
	-@erase ".\Release\scheduler.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "USE_TCL" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "USE_TCL"\
 /Fp"$(INTDIR)/Kernel.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Kernel.bsc" 
BSC32_SBRS=
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"soarkernel.lib"
LIB32_FLAGS=/nologo /out:"soarkernel.lib" 
LIB32_OBJS= \
	"$(INTDIR)/trace.obj" \
	"$(INTDIR)/print.obj" \
	"$(INTDIR)/chunk.obj" \
	"$(INTDIR)/tempmem.obj" \
	"$(INTDIR)/symtab.obj" \
	"$(INTDIR)/ma_interface.obj" \
	"$(INTDIR)/mem.obj" \
	"$(INTDIR)/lexer.obj" \
	"$(INTDIR)/decide.obj" \
	"$(INTDIR)/io.obj" \
	"$(INTDIR)/backtrace.obj" \
	"$(INTDIR)/main.obj" \
	"$(INTDIR)/interface.obj" \
	"$(INTDIR)/parser.obj" \
	"$(INTDIR)/rete.obj" \
	"$(INTDIR)/init_soar.obj" \
	"$(INTDIR)/reorder.obj" \
	"$(INTDIR)/callback.obj" \
	"$(INTDIR)/recmem.obj" \
	"$(INTDIR)/rhsfun_math.obj" \
	"$(INTDIR)/production.obj" \
	"$(INTDIR)/agent.obj" \
	"$(INTDIR)/wmem.obj" \
	"$(INTDIR)/explain.obj" \
	"$(INTDIR)/rhsfun.obj" \
	"$(INTDIR)/osupport.obj" \
	"$(INTDIR)/tilde.obj" \
	"$(INTDIR)/prefmem.obj" \
	"$(INTDIR)/scheduler.obj"

"$(OUTDIR)\soarkernel.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\soarkernel.lib" "$(OUTDIR)\Debug\Kernel.bsc"

CLEAN : 
	-@erase ".\Debug\Kernel.bsc"
	-@erase ".\Debug\agent.sbr"
	-@erase ".\Debug\interface.sbr"
	-@erase ".\Debug\recmem.sbr"
	-@erase ".\Debug\main.sbr"
	-@erase ".\Debug\rhsfun.sbr"
	-@erase ".\Debug\ma_interface.sbr"
	-@erase ".\Debug\reorder.sbr"
	-@erase ".\Debug\init_soar.sbr"
	-@erase ".\Debug\rete.sbr"
	-@erase ".\Debug\callback.sbr"
	-@erase ".\Debug\io.sbr"
	-@erase ".\Debug\print.sbr"
	-@erase ".\Debug\osupport.sbr"
	-@erase ".\Debug\wmem.sbr"
	-@erase ".\Debug\scheduler.sbr"
	-@erase ".\Debug\lexer.sbr"
	-@erase ".\Debug\decide.sbr"
	-@erase ".\Debug\tempmem.sbr"
	-@erase ".\Debug\production.sbr"
	-@erase ".\Debug\mem.sbr"
	-@erase ".\Debug\tilde.sbr"
	-@erase ".\Debug\trace.sbr"
	-@erase ".\Debug\rhsfun_math.sbr"
	-@erase ".\Debug\explain.sbr"
	-@erase ".\Debug\parser.sbr"
	-@erase ".\Debug\prefmem.sbr"
	-@erase ".\Debug\symtab.sbr"
	-@erase ".\Debug\chunk.sbr"
	-@erase ".\Debug\backtrace.sbr"
	-@erase ".\soarkernel.lib"
	-@erase ".\Debug\rhsfun_math.obj"
	-@erase ".\Debug\explain.obj"
	-@erase ".\Debug\parser.obj"
	-@erase ".\Debug\prefmem.obj"
	-@erase ".\Debug\symtab.obj"
	-@erase ".\Debug\chunk.obj"
	-@erase ".\Debug\backtrace.obj"
	-@erase ".\Debug\agent.obj"
	-@erase ".\Debug\interface.obj"
	-@erase ".\Debug\recmem.obj"
	-@erase ".\Debug\main.obj"
	-@erase ".\Debug\rhsfun.obj"
	-@erase ".\Debug\ma_interface.obj"
	-@erase ".\Debug\reorder.obj"
	-@erase ".\Debug\init_soar.obj"
	-@erase ".\Debug\rete.obj"
	-@erase ".\Debug\callback.obj"
	-@erase ".\Debug\io.obj"
	-@erase ".\Debug\print.obj"
	-@erase ".\Debug\osupport.obj"
	-@erase ".\Debug\wmem.obj"
	-@erase ".\Debug\scheduler.obj"
	-@erase ".\Debug\lexer.obj"
	-@erase ".\Debug\decide.obj"
	-@erase ".\Debug\tempmem.obj"
	-@erase ".\Debug\production.obj"
	-@erase ".\Debug\mem.obj"
	-@erase ".\Debug\tilde.obj"
	-@erase ".\Debug\trace.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /D "_DEBUG" /D "WIN32" /D "USE_TCL" /FR /YX /c
CPP_PROJ=/nologo /MLd /W3 /GX /Z7 /Od /D "_DEBUG" /D "WIN32" /D "USE_TCL"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/Kernel.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Kernel.bsc" 
BSC32_SBRS= \
	"$(INTDIR)/agent.sbr" \
	"$(INTDIR)/interface.sbr" \
	"$(INTDIR)/recmem.sbr" \
	"$(INTDIR)/main.sbr" \
	"$(INTDIR)/rhsfun.sbr" \
	"$(INTDIR)/ma_interface.sbr" \
	"$(INTDIR)/reorder.sbr" \
	"$(INTDIR)/init_soar.sbr" \
	"$(INTDIR)/rete.sbr" \
	"$(INTDIR)/callback.sbr" \
	"$(INTDIR)/io.sbr" \
	"$(INTDIR)/print.sbr" \
	"$(INTDIR)/osupport.sbr" \
	"$(INTDIR)/wmem.sbr" \
	"$(INTDIR)/scheduler.sbr" \
	"$(INTDIR)/lexer.sbr" \
	"$(INTDIR)/decide.sbr" \
	"$(INTDIR)/tempmem.sbr" \
	"$(INTDIR)/production.sbr" \
	"$(INTDIR)/mem.sbr" \
	"$(INTDIR)/tilde.sbr" \
	"$(INTDIR)/trace.sbr" \
	"$(INTDIR)/rhsfun_math.sbr" \
	"$(INTDIR)/explain.sbr" \
	"$(INTDIR)/parser.sbr" \
	"$(INTDIR)/prefmem.sbr" \
	"$(INTDIR)/symtab.sbr" \
	"$(INTDIR)/chunk.sbr" \
	"$(INTDIR)/backtrace.sbr"

"$(OUTDIR)\Debug\Kernel.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"soarkernel.lib"
LIB32_FLAGS=/nologo /out:"soarkernel.lib" 
LIB32_OBJS= \
	"$(INTDIR)/rhsfun_math.obj" \
	"$(INTDIR)/explain.obj" \
	"$(INTDIR)/parser.obj" \
	"$(INTDIR)/prefmem.obj" \
	"$(INTDIR)/symtab.obj" \
	"$(INTDIR)/chunk.obj" \
	"$(INTDIR)/backtrace.obj" \
	"$(INTDIR)/agent.obj" \
	"$(INTDIR)/interface.obj" \
	"$(INTDIR)/recmem.obj" \
	"$(INTDIR)/main.obj" \
	"$(INTDIR)/rhsfun.obj" \
	"$(INTDIR)/ma_interface.obj" \
	"$(INTDIR)/reorder.obj" \
	"$(INTDIR)/init_soar.obj" \
	"$(INTDIR)/rete.obj" \
	"$(INTDIR)/callback.obj" \
	"$(INTDIR)/io.obj" \
	"$(INTDIR)/print.obj" \
	"$(INTDIR)/osupport.obj" \
	"$(INTDIR)/wmem.obj" \
	"$(INTDIR)/scheduler.obj" \
	"$(INTDIR)/lexer.obj" \
	"$(INTDIR)/decide.obj" \
	"$(INTDIR)/tempmem.obj" \
	"$(INTDIR)/production.obj" \
	"$(INTDIR)/mem.obj" \
	"$(INTDIR)/tilde.obj" \
	"$(INTDIR)/trace.obj"

"$(OUTDIR)\soarkernel.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "kernel - Win32 Release"
# Name "kernel - Win32 Debug"

!IF  "$(CFG)" == "kernel - Win32 Release"

!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\wmem.c
DEP_CPP_WMEM_=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_WMEM_=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\wmem.obj" : $(SOURCE) $(DEP_CPP_WMEM_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\wmem.obj" : $(SOURCE) $(DEP_CPP_WMEM_) "$(INTDIR)"

"$(INTDIR)\wmem.sbr" : $(SOURCE) $(DEP_CPP_WMEM_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\backtrace.c
DEP_CPP_BACKT=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\explain.h"\
	".\callback.h"\
	
NODEP_CPP_BACKT=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\backtrace.obj" : $(SOURCE) $(DEP_CPP_BACKT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\backtrace.obj" : $(SOURCE) $(DEP_CPP_BACKT) "$(INTDIR)"

"$(INTDIR)\backtrace.sbr" : $(SOURCE) $(DEP_CPP_BACKT) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\callback.c
DEP_CPP_CALLB=\
	".\soarkernel.h"\
	".\callback.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	
NODEP_CPP_CALLB=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\callback.obj" : $(SOURCE) $(DEP_CPP_CALLB) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\callback.obj" : $(SOURCE) $(DEP_CPP_CALLB) "$(INTDIR)"

"$(INTDIR)\callback.sbr" : $(SOURCE) $(DEP_CPP_CALLB) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\chunk.c
DEP_CPP_CHUNK=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\explain.h"\
	".\callback.h"\
	
NODEP_CPP_CHUNK=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\chunk.obj" : $(SOURCE) $(DEP_CPP_CHUNK) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\chunk.obj" : $(SOURCE) $(DEP_CPP_CHUNK) "$(INTDIR)"

"$(INTDIR)\chunk.sbr" : $(SOURCE) $(DEP_CPP_CHUNK) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\decide.c
DEP_CPP_DECID=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_DECID=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\decide.obj" : $(SOURCE) $(DEP_CPP_DECID) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\decide.obj" : $(SOURCE) $(DEP_CPP_DECID) "$(INTDIR)"

"$(INTDIR)\decide.sbr" : $(SOURCE) $(DEP_CPP_DECID) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\explain.c
DEP_CPP_EXPLA=\
	".\soarkernel.h"\
	".\explain.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_EXPLA=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\explain.obj" : $(SOURCE) $(DEP_CPP_EXPLA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\explain.obj" : $(SOURCE) $(DEP_CPP_EXPLA) "$(INTDIR)"

"$(INTDIR)\explain.sbr" : $(SOURCE) $(DEP_CPP_EXPLA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\init_soar.c
DEP_CPP_INIT_=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_INIT_=\
	".\Menus.h"\
	".\TextEdit.h"\
	".\Dialogs.h"\
	".\Fonts.h"\
	".\Events.h"\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\init_soar.obj" : $(SOURCE) $(DEP_CPP_INIT_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\init_soar.obj" : $(SOURCE) $(DEP_CPP_INIT_) "$(INTDIR)"

"$(INTDIR)\init_soar.sbr" : $(SOURCE) $(DEP_CPP_INIT_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\interface.c
DEP_CPP_INTER=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\scheduler.h"\
	".\callback.h"\
	
NODEP_CPP_INTER=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\interface.obj" : $(SOURCE) $(DEP_CPP_INTER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\interface.obj" : $(SOURCE) $(DEP_CPP_INTER) "$(INTDIR)"

"$(INTDIR)\interface.sbr" : $(SOURCE) $(DEP_CPP_INTER) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\io.c
DEP_CPP_IO_C10=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_IO_C10=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\io.obj" : $(SOURCE) $(DEP_CPP_IO_C10) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\io.obj" : $(SOURCE) $(DEP_CPP_IO_C10) "$(INTDIR)"

"$(INTDIR)\io.sbr" : $(SOURCE) $(DEP_CPP_IO_C10) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\lexer.c
DEP_CPP_LEXER=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_LEXER=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\lexer.obj" : $(SOURCE) $(DEP_CPP_LEXER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\lexer.obj" : $(SOURCE) $(DEP_CPP_LEXER) "$(INTDIR)"

"$(INTDIR)\lexer.sbr" : $(SOURCE) $(DEP_CPP_LEXER) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ma_interface.c
DEP_CPP_MA_IN=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\scheduler.h"\
	".\callback.h"\
	
NODEP_CPP_MA_IN=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\ma_interface.obj" : $(SOURCE) $(DEP_CPP_MA_IN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\ma_interface.obj" : $(SOURCE) $(DEP_CPP_MA_IN) "$(INTDIR)"

"$(INTDIR)\ma_interface.sbr" : $(SOURCE) $(DEP_CPP_MA_IN) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\main.c
DEP_CPP_MAIN_=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_MAIN_=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"

"$(INTDIR)\main.sbr" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mem.c
DEP_CPP_MEM_C=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_MEM_C=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\mem.obj" : $(SOURCE) $(DEP_CPP_MEM_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\mem.obj" : $(SOURCE) $(DEP_CPP_MEM_C) "$(INTDIR)"

"$(INTDIR)\mem.sbr" : $(SOURCE) $(DEP_CPP_MEM_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\osupport.c
DEP_CPP_OSUPP=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_OSUPP=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\osupport.obj" : $(SOURCE) $(DEP_CPP_OSUPP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\osupport.obj" : $(SOURCE) $(DEP_CPP_OSUPP) "$(INTDIR)"

"$(INTDIR)\osupport.sbr" : $(SOURCE) $(DEP_CPP_OSUPP) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\parser.c
DEP_CPP_PARSE=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_PARSE=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\parser.obj" : $(SOURCE) $(DEP_CPP_PARSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\parser.obj" : $(SOURCE) $(DEP_CPP_PARSE) "$(INTDIR)"

"$(INTDIR)\parser.sbr" : $(SOURCE) $(DEP_CPP_PARSE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\prefmem.c
DEP_CPP_PREFM=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_PREFM=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\prefmem.obj" : $(SOURCE) $(DEP_CPP_PREFM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\prefmem.obj" : $(SOURCE) $(DEP_CPP_PREFM) "$(INTDIR)"

"$(INTDIR)\prefmem.sbr" : $(SOURCE) $(DEP_CPP_PREFM) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\print.c
DEP_CPP_PRINT=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_PRINT=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\print.obj" : $(SOURCE) $(DEP_CPP_PRINT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\print.obj" : $(SOURCE) $(DEP_CPP_PRINT) "$(INTDIR)"

"$(INTDIR)\print.sbr" : $(SOURCE) $(DEP_CPP_PRINT) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\production.c
DEP_CPP_PRODU=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_PRODU=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\production.obj" : $(SOURCE) $(DEP_CPP_PRODU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\production.obj" : $(SOURCE) $(DEP_CPP_PRODU) "$(INTDIR)"

"$(INTDIR)\production.sbr" : $(SOURCE) $(DEP_CPP_PRODU) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\recmem.c
DEP_CPP_RECME=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_RECME=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\recmem.obj" : $(SOURCE) $(DEP_CPP_RECME) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\recmem.obj" : $(SOURCE) $(DEP_CPP_RECME) "$(INTDIR)"

"$(INTDIR)\recmem.sbr" : $(SOURCE) $(DEP_CPP_RECME) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\reorder.c
DEP_CPP_REORD=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_REORD=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\reorder.obj" : $(SOURCE) $(DEP_CPP_REORD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\reorder.obj" : $(SOURCE) $(DEP_CPP_REORD) "$(INTDIR)"

"$(INTDIR)\reorder.sbr" : $(SOURCE) $(DEP_CPP_REORD) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\rete.c
DEP_CPP_RETE_=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_RETE_=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\rete.obj" : $(SOURCE) $(DEP_CPP_RETE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\rete.obj" : $(SOURCE) $(DEP_CPP_RETE_) "$(INTDIR)"

"$(INTDIR)\rete.sbr" : $(SOURCE) $(DEP_CPP_RETE_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\rhsfun.c
DEP_CPP_RHSFU=\
	".\soarkernel.h"\
	".\rhsfun_math.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_RHSFU=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\rhsfun.obj" : $(SOURCE) $(DEP_CPP_RHSFU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\rhsfun.obj" : $(SOURCE) $(DEP_CPP_RHSFU) "$(INTDIR)"

"$(INTDIR)\rhsfun.sbr" : $(SOURCE) $(DEP_CPP_RHSFU) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\rhsfun_math.c
DEP_CPP_RHSFUN=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_RHSFUN=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\rhsfun_math.obj" : $(SOURCE) $(DEP_CPP_RHSFUN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\rhsfun_math.obj" : $(SOURCE) $(DEP_CPP_RHSFUN) "$(INTDIR)"

"$(INTDIR)\rhsfun_math.sbr" : $(SOURCE) $(DEP_CPP_RHSFUN) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\scheduler.c
DEP_CPP_SCHED=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_SCHED=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\scheduler.obj" : $(SOURCE) $(DEP_CPP_SCHED) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\scheduler.obj" : $(SOURCE) $(DEP_CPP_SCHED) "$(INTDIR)"

"$(INTDIR)\scheduler.sbr" : $(SOURCE) $(DEP_CPP_SCHED) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\symtab.c
DEP_CPP_SYMTA=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_SYMTA=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\symtab.obj" : $(SOURCE) $(DEP_CPP_SYMTA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\symtab.obj" : $(SOURCE) $(DEP_CPP_SYMTA) "$(INTDIR)"

"$(INTDIR)\symtab.sbr" : $(SOURCE) $(DEP_CPP_SYMTA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tempmem.c
DEP_CPP_TEMPM=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_TEMPM=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\tempmem.obj" : $(SOURCE) $(DEP_CPP_TEMPM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\tempmem.obj" : $(SOURCE) $(DEP_CPP_TEMPM) "$(INTDIR)"

"$(INTDIR)\tempmem.sbr" : $(SOURCE) $(DEP_CPP_TEMPM) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\tilde.c
DEP_CPP_TILDE=\
	{$(INCLUDE)}"\sys\Types.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\tilde.obj" : $(SOURCE) $(DEP_CPP_TILDE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\tilde.obj" : $(SOURCE) $(DEP_CPP_TILDE) "$(INTDIR)"

"$(INTDIR)\tilde.sbr" : $(SOURCE) $(DEP_CPP_TILDE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\trace.c
DEP_CPP_TRACE=\
	".\soarkernel.h"\
	{$(INCLUDE)}"\sys\Types.h"\
	".\callback.h"\
	
NODEP_CPP_TRACE=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\trace.obj" : $(SOURCE) $(DEP_CPP_TRACE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\trace.obj" : $(SOURCE) $(DEP_CPP_TRACE) "$(INTDIR)"

"$(INTDIR)\trace.sbr" : $(SOURCE) $(DEP_CPP_TRACE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\agent.c
DEP_CPP_AGENT=\
	{$(INCLUDE)}"\sys\Types.h"\
	".\soarkernel.h"\
	".\scheduler.h"\
	".\callback.h"\
	
NODEP_CPP_AGENT=\
	".\ThinkCPosix.h"\
	".\sys\times.h"\
	".\soar_mpw.h"\
	".\..\pc_support\gui\winstubs.h"\
	

!IF  "$(CFG)" == "kernel - Win32 Release"


"$(INTDIR)\agent.obj" : $(SOURCE) $(DEP_CPP_AGENT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "kernel - Win32 Debug"


"$(INTDIR)\agent.obj" : $(SOURCE) $(DEP_CPP_AGENT) "$(INTDIR)"

"$(INTDIR)\agent.sbr" : $(SOURCE) $(DEP_CPP_AGENT) "$(INTDIR)"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
