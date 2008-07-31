#!/usr/bin/env python

import re
import string
import sys
import os
import glob

HEADER_MODE = "--header"
FUNCTIONTABLE_MODE = "--functiontable"
UTILS_MODE = "--utils"

PLUGIN_MODE = "--plugin"

USAGE = "USAGE: playerinterfacegen.py [%s | %s | %s] [%s] [<interface file>|<interface dir>]" % (HEADER_MODE, FUNCTIONTABLE_MODE, UTILS_MODE, PLUGIN_MODE)

class msg:
  pass


def get_interface(filename):
  interface_filename = os.path.splitext(os.path.split(filename)[-1])[0]
  interface_code = int(interface_filename.split("_")[0])
  interface_name = string.join(interface_filename.split("_")[1:],'_').lower()
  return (interface_code, interface_name)

def processfile(mode, filename, plugin):
  interface_code, interface_name = get_interface(filename)
  interface_def = "PLAYER_%s_CODE" % interface_name.upper()

  print >> sys.stderr, "Processing interface: ", interface_code, interface_name
  
  # now we process the input file
  interface_file = open(filename, 'r')
  input_data = interface_file.read()
  interface_file.close()
  
  interface_types = ""
  interface_messages = []
  
  # Read in the interface comment, and remove it
  interface_comment = ""
  pattern = re.compile('^\s*description\s*{(.*?)}[^\n]*\n',re.M|re.S)
  m = pattern.search(input_data)
  if m:
    interface_comment = m.group(1)
  pattern = re.compile('^\s*description\s*{(.*?)}[^\n]*\n',re.M|re.S)
  input_data = pattern.sub('', input_data)
      
  # read in the interface messages, and remove them
  pattern = re.compile('(?P<comment>/\*\*[^{}]*?\*/)?\s*message\s*{\s*(?P<type>\w*)\s*,\s*(?P<subtype>\w*)\s*,\s*(?P<subtypecode>\w*)\s*,\s*(?P<datatype>\w*)\s*}\s*;\s*',re.M|re.S)
  for m in pattern.finditer(input_data):
    mess = msg()
    mess.msg_type = "PLAYER_MSGTYPE_%s" % m.group("type").upper()
    mess.msg_subtype_string = "PLAYER_%s_%s_%s" % (interface_name.upper(), m.group("type").upper(), m.group("subtype").upper())
    mess.msg_subtype_code = int(m.group("subtypecode"))
    mess.datatype = m.group("datatype")
    if m.group("comment"):
      mess.comment = m.group("comment")
    else:
      mess.comment = ""
    interface_messages.append(mess)
  input_data = pattern.sub('', input_data)
    
  # strip out single line c++ comments at the start of lines, these can then be used to document the interface file
  pattern = re.compile('^\s*//.*',re.M)
  input_data = pattern.sub('', input_data)

    
  # the remaining data is the interface data types
  interface_types = input_data
    
  # Now we dump the data, what gets displayed depends on the mode
  if mode == HEADER_MODE:
    message_string = ""
    for m in interface_messages:
      message_string += """
%(comment)s
#define %(string)s %(code)d
""" % {"comment": m.comment, "string": m.msg_subtype_string, "code": m.msg_subtype_code}

    ifndefsymbol = '_' + interface_name.upper() + '_INTERFACE_H_'
    if plugin:
      print '#ifndef ' + ifndefsymbol
      print '#define ' + ifndefsymbol + '\n'
      
      print "#include <libplayercore/player.h>"
    
    
    print """
/** @ingroup message_codes
 * @{ */
#define %(interface_def)s %(interface_code)d
/** @} 
 *  @ingroup message_strings
 * @{ */
#define PLAYER_%(interface_name_upper)s_STRING "%(interface_name)s" 
/** @} */
// /////////////////////////////////////////////////////////////////////////////
/** @ingroup interfaces
  @defgroup interface_%(interface_name)s %(interface_name)s
  %(interface_comment)s
*/
/**
  @ingroup interface_%(interface_name)s
 * @{ */
 
%(interface_messages)s

%(interface_types)s
 
/** @} */ 
""" % {
  "interface_name" : interface_name, 
  "interface_name_upper" : interface_name.upper(), 
  "interface_def" : interface_def,
  "interface_code" : interface_code,  
  "interface_messages" : message_string,  
  "interface_types" : interface_types,  
  "interface_comment" : interface_comment}  
    if plugin:
      print "#endif // " + ifndefsymbol
  elif mode == FUNCTIONTABLE_MODE:
    if plugin:
      print """
#include "%(interface_name)s_interface.h"
#include "%(interface_name)s_xdr.h"

// Function table for this interface
static playerxdr_function_t %(interface_name)s_ftable[] =
{
""" % {"interface_name": interface_name}
    print "\n  /* %s messages */" % interface_name
    for m in interface_messages:
      print "  {", interface_def, ",", m.msg_type, ",", m.msg_subtype_string, ","
      print "    (player_pack_fn_t)%(dt_base)s_pack, (player_copy_fn_t)%(dt)s_copy, (player_cleanup_fn_t)%(dt)s_cleanup,(player_clone_fn_t)%(dt)s_clone,(player_free_fn_t)%(dt)s_free,(player_sizeof_fn_t)%(dt)s_sizeof}," % { "dt_base": m.datatype[:-2], "dt": m.datatype}
    if plugin:
      print """
  /* This NULL element signals the end of the list */
  {0,0,0,NULL,NULL,NULL}
};

playerxdr_function_t* player_plugininterf_gettable (void)
{
    return %(interface_name)s_ftable;
}
""" % {"interface_name": interface_name}
    

def process_for_utils(targetfile):
  interfaces = []
  if os.path.isdir(targetfile):
    for file in glob.glob(os.path.join(targetfile ,"*.def")):
      if not os.path.isdir(file):
        interfaces.append(get_interface(file))
  print """  
  /* this array lists the interfaces that Player knows how to load
  * It is important that this list is kept in strict numerical order
  * with respect to the interface numeric codes.
  *
  * NOTE: the last element must be NULL
  */
static player_interface_t interfaces[] = {  
  """
  interfaces.sort()
  last_code = -1 # start at -1 so that we generate and entry for 0
  for interface in interfaces:
    last_code += 1
    while interface[0] > last_code:
      print """  {0xFFFF, "nointerf%d"},""" % last_code
      last_code += 1
    print """  {PLAYER_%(i)s_CODE, PLAYER_%(i)s_STRING},""" % {"i" : interface[1].upper() }
  print """
  {0,NULL}
};""" 

if __name__ == '__main__':
  mode = HEADER_MODE
  targetfile = "interfaces"
  plugin = False

  for option in sys.argv[1:]:
    if option == "-h" or option == "--help":
      print USAGE
      sys.exit(-1)
    elif option == HEADER_MODE or option == FUNCTIONTABLE_MODE or option == UTILS_MODE:
      mode = option
    elif option == PLUGIN_MODE:
      plugin = True
    else:
      targetfile = option
      
  print "/* START OF AUTOGENERATED CODE */"
  if plugin:
    print "/* This file or section was automatically generated by playerinterfacegen.py */"
  else:
    print "/* This file or section was automatically generated by playerinterfacegen.py"
    print "To modify the interfaces in this file please edit their interface definition in libplayercore/interfaces/ */"

  if mode == UTILS_MODE:
    process_for_utils(targetfile)
  else:
    if os.path.isdir(targetfile):
      for file in glob.glob(os.path.join(targetfile ,"*.def")):
        if not os.path.isdir(file):
          processfile(mode, file, plugin)
    else:
      processfile(mode, targetfile, plugin)

  print "/* END OF AUTOGENERATED CODE */"

