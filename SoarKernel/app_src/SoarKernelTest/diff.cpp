// -------------------------------------------------------------------
//	MODULE:		diff.cpp
//
//	PURPOSE:	
//
// CREATE:		2001/09/26
//
// (c) 2001 Soar Technology.  All Rights Reserved.
//
// -------------------------------------------------------------------
#include <fstream>
#include <iterator>
#include <iostream>
#include <string.h>

typedef char Bool;
// 
/**
 *
 */

Bool _diff(const char *f1, const char *f2) {

   std::ifstream c_dump(f1,std::ios::binary|std::ios::in);
   std::ifstream cpp_dump(f2,std::ios::binary|std::ios::in);

   if(!c_dump) {
      std::cout << "Cannot open log file " << f1 << "." << std::endl;
#ifdef _WIN32
      getchar();
#endif
      return false;
   }
   if(!cpp_dump) {
      std::cout << "Cannot open log file " << f2 << "." << std::endl;
#ifdef _WIN32
      getchar();
#endif
      return false;
   }

   c_dump.seekg (0, std::ios::end);
   long c_end = c_dump.tellg();
   c_dump.seekg (0, std::ios::beg);
   long c_begin = c_dump.tellg();
   long c_length = c_end - c_begin;

   cpp_dump.seekg (0, std::ios::end);
   long cpp_end = cpp_dump.tellg();
   cpp_dump.seekg (0, std::ios::beg);
   long cpp_begin = cpp_dump.tellg();
   long cpp_length = cpp_end - cpp_begin;
   
   if(cpp_length != c_length){
      std::cout << std::endl << 
                   "=======================================================" << std::endl;
      std::cout << "=======================================================" << std::endl;
      std::cout << "==  Files do Not Match.  They are different lengths. ==" << std::endl;
      std::cout << "=======================================================" << std::endl;
      std::cout << "=======================================================" << std::endl;
#ifdef _WIN32
      getchar();
#endif
      return false;
   }

   c_dump.seekg (0, std::ios::beg);
   cpp_dump.seekg(0, std::ios::beg);

   char *c_mem   = new char[c_length];
   char *cpp_mem = new char[c_length];

   c_dump.read(c_mem, c_length);
   cpp_dump.read(cpp_mem, c_length);

   Bool match = (memcmp(c_mem, cpp_mem, c_length) == 0);
   delete[](c_mem);  
   delete[](cpp_mem);
   
   if(!match){
      std::cout << std::endl << 
                   "==========================================================" << std::endl;
      std::cout << "==========================================================" << std::endl;
      std::cout << "==  Files do Not Match.  Same Size, Different Contents. ==" << std::endl;
      std::cout << "==========================================================" << std::endl;
      std::cout << "==========================================================" << std::endl;
#ifdef _WIN32
      getchar();
#endif
      return false;
   } else {
      std::cout << std::endl << 
                   "===================" << std::endl;
      std::cout << "==  Files Match. ==" << std::endl;
      std::cout << "===================" << std::endl;
      return true;
   }
      
   return true;
}
