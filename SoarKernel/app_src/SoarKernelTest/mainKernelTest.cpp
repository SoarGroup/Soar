/********************************************************************
	created:	2001/09/04
	created:	4:9:2001   23:18
	filename: 	c:\dev\soar-84\port\main.cpp
	file path:	c:\dev\soar-84\port
	file base:	main
	file ext:	cpp
	author:		Jens Wessling
	
	purpose:	
*********************************************************************/


#include <assert.h>
#include <malloc.h>

#include <stdio.h>

#include "soarapi.h"
#include "Loar_interface.h"

#include "agent.h"
#include "init_soar.h"
#include "kernel_struct.h"

/**
*
*
*
*/
/*
#include <cppunit/TextTestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TextTestRunner.h>
#include <cppunit/TestResult.h>
*/
#include <string>

//CPPUNIT_TEST_SUITE_REGISTRATION( WMETest );

typedef char Bool;

Kernel * SKT_kernel;

agent* glbAgent = 0;

int main(int, char **)
{
//   CppUnit::TextTestRunner runner;
//   runner.addTest (CppUnit::TestFactoryRegistry::getRegistry().makeTest());
//   runner.run ();

  SKT_kernel = create_kernel();
  //init_soar(SKT_kernel);

  lua_State *Lua = init_Lua_Soar();

  /*
   *  Initialize Soar, this must be called once (and only once) before
   *  any of Soar's functionality is used.  It basically allocates 
   *  memory and initializes some of Soar's internal data structures  
   */

  Bool b = (lua_dofile(Lua, "LuaFiles/testfile.lua")==0);

  destroy_kernel(SKT_kernel);

  if(!b){
     printf("************************************************************\n");
     printf("************************************************************\n");
     printf("****************     FAILED      ***************************\n");
     printf("************************************************************\n");
     printf("************************************************************\n");
  }

#ifdef WIN32
  getchar();
#endif
  return 0;
}

