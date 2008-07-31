#!/usr/bin/env python

#TODO:
#  - Add an option to specify whether we're building libplayerxdr (whose
#    header gets installed for general use, has copyright boilerplate,
#    etc.) or a user XDR lib

import re
import string
import sys
import os

USAGE = 'USAGE: playerxdrgen.y [-distro] <interface-spec.h> [<extra_interface-spec.h>] <pack.c> <pack.h>'



hasdynamic = []    # A list of types that contain dynamic data. During pass 1,
                    # if a type is found to have dynamic data it will be added
                    # to this list. Then, during other passes for this and
                    # other types, necessary deep copy/clean up functions can
                    # be created and called.

class DataTypeMember:
  arraypattern = re.compile('\[(.*?)\]')
  pointerpattern = re.compile('\*')
  
  def __init__(self, body):
    # is it an array or a scalar?
    self.array = False
    self.pointer = False
    self.arraysize = self.arraypattern.findall(body)
    pointers = self.pointerpattern.findall(body)
    if len(self.arraysize) > 0:
        self.array = True
        self.arraysize = self.arraysize[0]
        body = self.arraypattern.sub('', body)
    elif len(pointers) > 1:  # This checks for things like "uint8_t* *data"
      raise 'Illegal pointer declaration in struct\n' + body
    elif len(pointers) > 0:
      self.array = True
      self.arraysize = ''
      self.pointer = True
      body = self.pointerpattern.sub('', body)

    self.Name = body.strip()
    self.pointervar = self.Name + '_p'
    self.countvar = self.Name + '_count'
  
# multiple variables can occur with single type, i.e. int a,b,c so we first get the type and then add each variable to the entry
class DataTypeMemberSet:
  typepattern = re.compile('\s*\w+')
  variablepattern = re.compile('\s*([^,;]+?)\s*[,;]')
  
  def __init__(self, body):
    self.haspointer = False
    self.hasarray = False
    self.variables = []
    self.typename = self.typepattern.findall(body)[0].strip()
    if self.typename in hasdynamic:
      self.dynamic = True
    else:
      self.dynamic = False
    
    # find the variable names
    body = self.typepattern.sub('', body, 1)
    vars = self.variablepattern.findall(body)
    
    
    # iterate through each variable
    for varstring in vars:
      member = DataTypeMember(varstring)
      self.variables.append(member)
      if member.pointer:
        self.haspointer = True
      if member.array:
        self.hasarray = True
          

class DataType:
  contentspattern = re.compile('.*\{\s*(.*?)\s*\}', re.MULTILINE | re.DOTALL)
  declpattern = re.compile('\s*([^;]*?;)', re.MULTILINE)
  
  def __init__(self, body):
    split = string.split(body)
    self.prefix = split[2]
    self.typename = split[-1]
    self.dynamic = False
    self.hasarray = False
    self.haspointer = False

    self.members = []
    # pick out the contents of the struct
    varpart = self.contentspattern.findall(body)
    if len(varpart) != 1:
      print 'skipping nested / empty struct ' + typename
      raise "Empty Struct"
    # separate the variable declarations
    decls = self.declpattern.findall(varpart[0])
    for dstring in decls:
      ms = DataTypeMemberSet(dstring)
      self.members.append(ms)
      if ms.haspointer or ms.dynamic:
        self.dynamic = True
      if ms.hasarray:
        self.hasarray = True
      if ms.haspointer:
        self.haspointer = True
      
    if self.dynamic:
      hasdynamic.append (self.typename)

      
  def GetVarNames(self):
    varnames = []
    for m in self.members:
      for v in m.variables:
        varnames.append(v.Name)
    return varnames

  def HasDynamicArray(self):
    for m in self.members:
      if m.dynamic:
        for v in m.variables:
          if v.array:
            return True
    return False


class MethodGenerator:
  def __init__(self,headerfile,sourcefile):
    self.headerfile = headerfile
    self.sourcefile = sourcefile
    
          
  def gen_internal_pack(self,datatype):
    self.headerfile.write("int xdr_%(typename)s (XDR* xdrs, %(typename)s * msg);\n" % {"typename":datatype.typename})
    
    self.sourcefile.write("""
int xdr_%(typename)s (XDR* xdrs, %(typename)s * msg)
{ """ % {"typename":datatype.typename})
    
    sourcefile = self.sourcefile

    for member in datatype.members:
      # Do some name mangling for common types
      if member.typename == 'long long':
        xdr_proc = 'xdr_longlong_t'
      elif member.typename == 'int64_t':
        xdr_proc = 'xdr_longlong_t'
      elif member.typename == 'uint64_t':
        xdr_proc = 'xdr_u_long'
      elif member.typename == 'int32_t':
        xdr_proc = 'xdr_int'
      elif member.typename == 'uint32_t':
        xdr_proc = 'xdr_u_int'
      elif member.typename == 'int16_t':
        xdr_proc = 'xdr_short'
      elif member.typename == 'uint16_t':
        xdr_proc = 'xdr_u_short'
      elif member.typename == 'int8_t' or member.typename == 'char':
        xdr_proc = 'xdr_char'
      elif member.typename == 'uint8_t':
        xdr_proc = 'xdr_u_char'
      elif member.typename == 'bool_t':
        xdr_proc = 'xdr_bool'
      else:
        # rely on a previous declaration of an xdr_ proc for this type
        xdr_proc = 'xdr_' + member.typename
      
      for var in member.variables:
        if var.array:
          if var.arraysize == '':  # Handle a dynamically allocated array
            # Check for a matching count variable, because compulsory for dynamic arrays
            # FIXME: Add this test back
            if var.countvar not in datatype.GetVarNames():
              raise 'Missing count var "' + countvar + '" in\n' + s
            # First put in a check to see if unpacking, and if so allocate memory for the array
            sourcefile.write('  if(xdrs->x_op == XDR_DECODE)\n  {\n')
            sourcefile.write('    if((msg->' + var.Name + ' = malloc(msg->' + var.countvar + '*sizeof(' + member.typename + '))) == NULL)\n      return(0);\n')
            #sourcefile.write('    memset(msg->' + varstring + ', 0, msg->' + countvar + '*sizeof(' + typestring + '));\n')
            sourcefile.write('  }\n')
            # Write the lines to (un)pack/convert
            # Is it an array of bytes?  If so, then we'll encode
            # it a packed opaque bytestring, rather than an array of 4-byte-aligned
            # chars.
            if xdr_proc == 'xdr_u_char' or xdr_proc == 'xdr_char':
              sourcefile.write('  {\n')
              sourcefile.write('    ' + member.typename + '* ' + var.pointervar + ' = msg->' + var.Name + ';\n')
              sourcefile.write('    if(xdr_bytes(xdrs, (char**)&' + var.pointervar + ', &msg->' + var.countvar + ', msg->' + var.countvar + ') != 1)\n      return(0);\n')
              sourcefile.write('  }\n')
            else:
              sourcefile.write('  {\n')
              sourcefile.write('    ' + member.typename + '* ' + var.pointervar + ' = msg->' + var.Name + ';\n')
              sourcefile.write('    if(xdr_array(xdrs, (char**)&' + var.pointervar + ', &msg->' + var.countvar + ', msg->' + var.countvar +', sizeof(' + member.typename + '), (xdrproc_t)' + xdr_proc + ') != 1)\n      return(0);\n')
              sourcefile.write('  }\n')
          else:           # Handle a static array
            # Was a _count variable declared? If so, we'll encode as a
            # variable-length array (with xdr_array); otherwise we'll
            # do it fixed-length (with xdr_vector).  xdr_array is picky; we
            # have to declare a pointer to the array, then pass in the
            # address of this pointer.  Passing the address of the array
            # does NOT work.
            if var.countvar in datatype.GetVarNames():
      
              # Is it an array of bytes?  If so, then we'll encode
              # it a packed opaque bytestring, rather than an array of 4-byte-aligned
              # chars.
              if xdr_proc == 'xdr_u_char' or xdr_proc == 'xdr_char':
                sourcefile.write('  {\n')
                sourcefile.write('    ' + member.typename + '* ' + var.pointervar +
                                ' = msg->' + var.Name + ';\n')
                sourcefile.write('    if(xdr_bytes(xdrs, (char**)&' + var.pointervar +
                                ', &msg->' + var.countvar +
                                ', ' + var.arraysize + ') != 1)\n      return(0);\n')
                sourcefile.write('  }\n')
              else:
                sourcefile.write('  {\n')
                sourcefile.write('    ' + member.typename + '* ' + var.pointervar +
                                ' = msg->' + var.Name + ';\n')
                sourcefile.write('    if(xdr_array(xdrs, (char**)&' + var.pointervar +
                                ', &msg->' + var.countvar +
                                ', ' + var.arraysize + ', sizeof(' + member.typename + '), (xdrproc_t)' +
                                xdr_proc + ') != 1)\n      return(0);\n')
                sourcefile.write('  }\n')
            else:
              # Is it an array of bytes?  If so, then we'll encode
              # it a packed opaque bytestring, rather than an array of 4-byte-aligned
              # chars.
              if xdr_proc == 'xdr_u_char' or xdr_proc == 'xdr_char':
                sourcefile.write('  if(xdr_opaque(xdrs, (char*)&msg->' +
                                  var.Name + ', ' + var.arraysize + ') != 1)\n    return(0);\n')
              else:
                sourcefile.write('  if(xdr_vector(xdrs, (char*)&msg->' +
                                  var.Name + ', ' + var.arraysize +
                                  ', sizeof(' + member.typename + '), (xdrproc_t)' +
                                  xdr_proc + ') != 1)\n    return(0);\n')
        else:
          sourcefile.write('  if(' + xdr_proc + '(xdrs,&msg->' +
                              var.Name + ') != 1)\n    return(0);\n')
        #varlist.append(varstring)
    sourcefile.write('  return(1);\n}\n')
    
    
  def gen_external_pack(self,datatype):
    self.headerfile.write("int %(prefix)s_pack(void* buf, size_t buflen, %(typename)s * msg, int op);\n" % {"typename":datatype.typename, "prefix":datatype.prefix})
    
    self.sourcefile.write("""int 
%(prefix)s_pack(void* buf, size_t buflen, %(typename)s * msg, int op)
{
  XDR xdrs;
  int len;
  if(!buflen)
    return 0;
  xdrmem_create(&xdrs, buf, buflen, op);
  if(xdr_%(typename)s(&xdrs,msg) != 1)
    return(-1);
  if(op == PLAYERXDR_ENCODE)
    len = xdr_getpos(&xdrs);
  else
    len = sizeof(%(typename)s);
  xdr_destroy(&xdrs);
  return(len);
} """ % {"typename":datatype.typename, "prefix":datatype.prefix})

    
  def gen_copy(self,datatype):
    # If type is not in hasdynamic, not going to write a function so may as well just continue with the next struct
    self.headerfile.write("unsigned int %(typename)s_copy(%(typename)s *dest, const %(typename)s *src);\n" % {"typename":datatype.typename, "prefix":datatype.prefix})
    if datatype.typename not in hasdynamic:
      self.sourcefile.write("""
unsigned int %(typename)s_copy(%(typename)s *dest, const %(typename)s *src)
{
  if (dest == NULL || src == NULL)
    return 0;
  memcpy(dest,src,sizeof(%(typename)s));
  return sizeof(%(typename)s);
} """ % {"typename":datatype.typename})
    else:
      if datatype.HasDynamicArray():
        itrdec = "unsigned ii;"
      else:
        itrdec = ""
      self.sourcefile.write("""
unsigned int %(typename)s_copy(%(typename)s *dest, const %(typename)s *src)
{      
  %(itrdec)s
  unsigned int size = 0;
  if(src == NULL)
    return(0);""" % {"typename":datatype.typename, "itrdec" : itrdec} )
    
      for member in datatype.members:
        for var in member.variables:
          subs = {"varstring" : var.Name, "countvar" : var.countvar, "typestring" : member.typename, "arraysize" : var.arraysize }
          if var.pointer: # Handle a dynamically allocated array
            sourcefile.write("""
  if(src->%(varstring)s != NULL && src->%(countvar)s > 0)
  {
    if((dest->%(varstring)s = malloc(src->%(countvar)s*sizeof(%(typestring)s))) == NULL)
      return(0);
  }
  else
    dest->%(varstring)s = NULL;""" % subs)
    
          if var.array:
            subs["amp"] = ""
            subs["index"] = "[ii]"
            if var.countvar in datatype.GetVarNames():
              subs["arraysize"] = "src->"+var.countvar
            else:
              subs["arraysize"] = var.arraysize
          else:
            subs["arraysize"] = 1
            subs["amp"] = "&"
            subs["index"] = ""
          
          if member.dynamic:
            if var.array:
              sourcefile.write("""
  for(ii = 0; ii < %(arraysize)s; ii++)""" % subs)
            sourcefile.write("""
  {size += %(typestring)s_copy(&dest->%(varstring)s%(index)s, &src->%(varstring)s%(index)s);}""" % subs)
  
  
          else: #plain old variable or array
            sourcefile.write("""
  size += sizeof(%(typestring)s)*%(arraysize)s;
  memcpy(%(amp)sdest->%(varstring)s,%(amp)ssrc->%(varstring)s,sizeof(%(typestring)s)*%(arraysize)s); """ % subs)
      
      sourcefile.write("""
  return(size);
}""")
  
  
  
  def gen_cleanup(self,datatype):
    # If type is not in hasdynamic, not going to write a function so may as well just continue with the next struct
    self.headerfile.write("void %(typename)s_cleanup(const %(typename)s *msg);\n" % {"typename":datatype.typename})
    if datatype.typename not in hasdynamic:
      self.sourcefile.write("""
void %(typename)s_cleanup(const %(typename)s *msg)
{
} """ % {"typename":datatype.typename})      
    else:
      if datatype.HasDynamicArray():
        itrdec = "unsigned ii;"
      else:
        itrdec = ""
      self.sourcefile.write("""
void %(typename)s_cleanup(const %(typename)s *msg)
{      
  %(itrdec)s
  if(msg == NULL)
    return;""" % {"typename":datatype.typename, "itrdec" : itrdec} )
    
      for member in datatype.members:
        for var in member.variables:
          subs = {"varstring" : var.Name, "countvar" : var.countvar, "typestring" : member.typename, "arraysize" : var.arraysize }
          if member.dynamic:
            if var.array:
              if var.countvar in datatype.GetVarNames():
                formax = "msg->"+var.countvar
              else:
                formax = var.arraysize
              subs["formax"] = formax
              sourcefile.write("""
  for(ii = 0; ii < %(formax)s; ii++)
  {
    %(typestring)s_cleanup(&msg->%(varstring)s[ii]);
  }""" % subs)
            else:
              sourcefile.write("""
  %(typestring)s_cleanup(&msg->%(varstring)s); """ % subs)
          if var.pointer: 
            sourcefile.write("""
  free(msg->%(varstring)s); """ % subs)
  
      sourcefile.write("\n}")
    
  def gen_clone(self,datatype):
    # If type is not in hasdynamic, not going to write a function so may as well just continue with the next struct
    self.headerfile.write("%(typename)s * %(typename)s_clone(const %(typename)s *msg);\n" % {"typename":datatype.typename})
    self.sourcefile.write("""
%(typename)s * %(typename)s_clone(const %(typename)s *msg)
{      
  %(typename)s * clone = malloc(sizeof(%(typename)s));
  if (clone)
    %(typename)s_copy(clone,msg);
  return clone;
}""" % {"typename":datatype.typename})
    
  def gen_free(self,datatype):
    # If type is not in hasdynamic, not going to write a function so may as well just continue with the next struct
    self.headerfile.write("void %(typename)s_free(%(typename)s *msg);\n" % {"typename":datatype.typename})
    self.sourcefile.write("""
void %(typename)s_free(%(typename)s *msg)
{      
  %(typename)s_cleanup(msg);
  free(msg);
}""" % {"typename":datatype.typename})


  def gen_sizeof(self,datatype):
    self.headerfile.write("unsigned int %(typename)s_sizeof(%(typename)s *msg);\n" % {"typename":datatype.typename})
    if datatype.typename not in hasdynamic:
      self.sourcefile.write("""
unsigned int %(typename)s_sizeof(%(typename)s *msg)
{
  return sizeof(%(typename)s);
} """ % {"typename":datatype.typename})
    else:
      if datatype.HasDynamicArray():
        itrdec = "unsigned ii;"
      else:
        itrdec = ""
      self.sourcefile.write("""
unsigned int %(typename)s_sizeof(%(typename)s *msg)
{
  %(itrdec)s
  unsigned int size = 0;
  if(msg == NULL)
    return(0);""" % {"typename":datatype.typename, "itrdec" : itrdec} )
    
      for member in datatype.members:
        for var in member.variables:
          subs = {"varstring" : var.Name, "countvar" : var.countvar, "typestring" : member.typename, "arraysize" : var.arraysize }
          if var.array:
            subs["amp"] = ""
            subs["index"] = "[ii]"
            if var.countvar in datatype.GetVarNames():
              subs["arraysize"] = "msg->"+var.countvar
            else:
              subs["arraysize"] = var.arraysize
          else:
            subs["arraysize"] = 1
            subs["amp"] = "&"
            subs["index"] = ""
          
          if member.dynamic:
            if var.array:
              sourcefile.write("""
  for(ii = 0; ii < %(arraysize)s; ii++)""" % subs)
            sourcefile.write("""
  {size += %(typestring)s_sizeof(&msg->%(varstring)s%(index)s);}""" % subs)
  
  
          else: #plain old variable or array
            sourcefile.write("""
  size += sizeof(%(typestring)s)*%(arraysize)s; """ % subs)
      
      sourcefile.write("""
  return(size);
}""")
  
    
if __name__ == '__main__':

  if len(sys.argv) < 4:
    print USAGE
    sys.exit(-1)

  distro = 0

  idx = 1
  if sys.argv[1] == '-distro':
    if len(sys.argv) < 5:
      print USAGE
      sys.exit(-1)
    distro = 1
    idx += 1

  infilenames = [sys.argv[idx],]
  idx += 1
  sourcefilename = sys.argv[idx]
  idx += 1
  headerfilename = sys.argv[idx]
  idx += 1
  if len(sys.argv) > idx:
    for opt in sys.argv[idx:]:
      infilenames.append(opt)
      print "processeing extra file ", opt


  # Read in the entire file
  instream = ""
  for f in infilenames:
    infile = open(f, 'r')
    instream += infile.read()
    infile.close()

  sourcefile = open(sourcefilename, 'w+')
  headerfile = open(headerfilename, 'w+')

  if distro:
    headerfile.write("""
/** @ingroup libplayerxdr
@{ */
#ifndef _PLAYERXDR_PACK_H_
#define _PLAYERXDR_PACK_H_

#include <rpc/types.h>
#include <rpc/xdr.h>

#include <libplayercore/player.h>
#include <libplayerxdr/functiontable.h>
#ifdef __cplusplus
  extern "C" {
#endif
#ifndef XDR_ENCODE
  #define XDR_ENCODE 0
#endif
#ifndef XDR_DECODE
  #define XDR_DECODE 1
#endif
#define PLAYERXDR_ENCODE XDR_ENCODE
#define PLAYERXDR_DECODE XDR_DECODE

#define PLAYERXDR_MSGHDR_SIZE 40
#define PLAYERXDR_MAX_MESSAGE_SIZE (4*PLAYER_MAX_MESSAGE_SIZE)
""")
    sourcefile.write("""
#include <libplayerxdr/%(headerfilename)s>
#include <string.h>

#include <stdlib.h>
""" % {"headerfilename":headerfilename})
  else:
    ifndefsymbol = '_' + infilenames[0].replace('.','_').replace('/','_').upper() + '_XDR_'
    headerfile.write('#ifndef ' + ifndefsymbol + '\n')
    headerfile.write('#define ' + ifndefsymbol + '\n\n')
    headerfile.write('#include <libplayerxdr/playerxdr.h>\n\n')
    headerfile.write('#include "' + os.path.split(infilenames[0])[-1] + '"\n\n')
    headerfile.write('#ifdef __cplusplus\nextern "C" {\n#endif\n\n')
    
    sourcefile.write('#include <rpc/types.h>\n')
    sourcefile.write('#include <rpc/xdr.h>\n\n')
    sourcefile.write('#include "' + os.path.split(headerfilename)[-1] + '"\n')
    sourcefile.write('#include <string.h>\n')
    sourcefile.write('#include <stdlib.h>\n\n')


  # strip C++-style comments
  pattern = re.compile('//.*')
  instream = pattern.sub('', instream)

  # strip C-style comments
  pattern = re.compile('/\*.*?\*/', re.MULTILINE | re.DOTALL)
  instream = pattern.sub('', instream)

  # strip blank lines
  pattern = re.compile('^\s*?\n', re.MULTILINE)
  instream = pattern.sub('', instream)

  # find structs
  pattern = re.compile('typedef\s+struct\s+player_\w+[^}]+\}[^;]+',
                   re.MULTILINE)
  structs = pattern.findall(instream)

  print 'Found ' + `len(structs)` + ' struct(s)'
  
  #arraypattern = re.compile('\[\s*(\w*?)\s*\]')
#  pointerpattern = re.compile('[A-Za-z0-9_]+\*|\*[A-Za-z0-9_]+')

  gen = MethodGenerator(headerfile,sourcefile)
  
  for s in structs:
    # extract prefix for packing function name and type of struct
    current = DataType(s)

    # Generate the methods
    gen.gen_internal_pack(current)
    gen.gen_external_pack(current)
    gen.gen_copy(current)
    gen.gen_cleanup(current)
    gen.gen_clone(current)
    gen.gen_free(current)    
    gen.gen_sizeof(current)    
    sourcefile.write('\n')
    
  headerfile.write('\n#ifdef __cplusplus\n}\n#endif\n\n')
  headerfile.write('#endif\n')
  if distro:
    headerfile.write('/** @} */\n')

  sourcefile.close()
  headerfile.close()
  