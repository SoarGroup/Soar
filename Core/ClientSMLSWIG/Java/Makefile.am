###########################################
# Author: Jonathan Voigt, voigtjr@gmail.com
# Date: 2006
###########################################
BASEDIR_PREFIX=../../..
SOAR_LIBRARY=$(BASEDIR_PREFIX)/SoarLibrary
EXTRA_DIST = Java_sml_ClientInterface.i build/readme.txt\
build/Agent.java\
build/AgentEventHandlerPlusData.java\
build/AnalyzeXML.java\
build/ClientAnalyzedXML.java\
build/ClientErrors.java\
build/ClientTraceXML.java\
build/ClientXML.java\
build/ConnectionInfo.java\
build/ElementXML.java\
build/EventHandlerPlusData.java\
build/Events.java\
build/FloatElement.java\
build/Identifier.java\
build/IdentifierSymbol.java\
build/IntElement.java\
build/Kernel.java\
build/OutputEventHandlerPlusData.java\
build/OutputNotificationHandlerPlusData.java\
build/PrintEventHandlerPlusData.java\
build/ProductionEventHandlerPlusData.java\
build/RhsEventHandlerPlusData.java\
build/RunEventHandlerPlusData.java\
build/SWIGTYPE_p_ElementXML_InterfaceStructTag.java\
build/SWIGTYPE_p_f_enum_sml__smlAgentEventId_p_void_p_sml__Agent__void.java\
build/SWIGTYPE_p_f_enum_sml__smlPrintEventId_p_void_p_sml__Agent_p_q_const__char__void.java\
build/SWIGTYPE_p_f_enum_sml__smlProductionEventId_p_void_p_sml__Agent_p_q_const__char_p_q_const__char__void.java\
build/SWIGTYPE_p_f_enum_sml__smlRunEventId_p_void_p_sml__Agent_enum_sml__smlPhase__void.java\
build/SWIGTYPE_p_f_enum_sml__smlStringEventId_p_void_p_sml__Kernel_p_q_const__char__void.java\
build/SWIGTYPE_p_f_enum_sml__smlSystemEventId_p_void_p_sml__Kernel__void.java\
build/SWIGTYPE_p_f_enum_sml__smlUpdateEventId_p_void_p_sml__Kernel_enum_sml__smlRunFlags__void.java\
build/SWIGTYPE_p_f_enum_sml__smlXMLEventId_p_void_p_sml__Agent_p_sml__ClientXML__void.java\
build/SWIGTYPE_p_f_p_void_p_sml__Agent__void.java\
build/SWIGTYPE_p_f_p_void_p_sml__Agent_p_q_const__char_p_sml__WMElement__void.java\
build/SWIGTYPE_p_f_sml__smlRhsEventId_p_void_p_sml__Agent_p_q_const__char_p_q_const__char__std__string.java\
build/SWIGTYPE_p_size_t.java\
build/SWIGTYPE_p_void.java\
build/StringElement.java\
build/StringEventHandlerPlusData.java\
build/SystemEventHandlerPlusData.java\
build/UpdateEventHandlerPlusData.java\
build/WMElement.java\
build/XMLEventHandlerPlusData.java\
build/sml.java\
build/smlAgentEventId.java\
build/smlConstants.java\
build/smlGenericEventId.java\
build/smlInterleaveStepSize.java\
build/smlJNI.java\
build/smlPhase.java\
build/smlPrintEventId.java\
build/smlProductionEventId.java\
build/smlRhsEventId.java\
build/smlRunEventId.java\
build/smlRunFlags.java\
build/smlRunResult.java\
build/smlRunStepSize.java\
build/smlStringEventId.java\
build/smlSystemEventId.java\
build/smlUpdateEventId.java\
build/smlWorkingMemoryEventId.java\
build/smlXMLEventId.java\
build/sml_Names.java

if SWIGJAVA
lib_LTLIBRARIES = libJava_sml_ClientInterface.la
else
lib_LTLIBRARIES = 
endif

libJava_sml_ClientInterface_la_LIBADD = \
$(BASEDIR_PREFIX)/Core/ClientSML/src/libClientSML.la \
$(BASEDIR_PREFIX)/Core/ConnectionSML/src/libConnectionSML.la \
$(BASEDIR_PREFIX)/Core/ElementXML/src/libElementXML.la

nodist_libJava_sml_ClientInterface_la_SOURCES = Java_sml_ClientInterface_wrap.cpp JavaCallbackByHand.h 

JAVASO = @JAVASO@
libJava_sml_ClientInterface_la_LDFLAGS = -shrext $(JAVASO)

SWIG=@SWIG@
JAVAINC= @JAVAINC@
JAVAC= @JAVAC@
JAR=@JAR@
INCLUDES   = -I../../ClientSML/include -I../../ElementXML/include -I../../ConnectionSML/include $(JAVAINC)

if VISIBILITY_AVAILABLE
SWIGCXXFLAGS = $(SOAR_CXXFLAGS) -Wno-unused -fvisibility=default
else
SWIGCXXFLAGS = $(SOAR_CXXFLAGS) -Wno-unused
endif

CXXFLAGS = $(SWIGCXXFLAGS)

CLEANFILES = build/*.java build/*.class Java_sml_ClientInterface_wrap.cpp

SWIGOPT    = -java -Wall -package sml $(INCLUDES) -outdir build

Java_sml_ClientInterface_wrap.cpp: Java_sml_ClientInterface.i
	$(SWIG) $(SWIGOPT) -o $@ $<
	$(JAVAC) build/*.java
	cp -pRP build $(SOAR_LIBRARY)/bin/sml
	$(JAR) cf $(SOAR_LIBRARY)/bin/sml.jar -C $(SOAR_LIBRARY)/bin sml
	rm -rf $(SOAR_LIBRARY)/bin/sml

clean:
	rm -f Java_sml_ClientInterface_wrap.cpp
	rm -f build/*.java
	rm -f build/*.class
