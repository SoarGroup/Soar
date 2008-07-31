#!/usr/bin/env python

import re
import string
import sys
import os

USAGE = 'USAGE: parse.y <player.h> <playercore_casts.i> <playercore_arraysofclasses.i> <Jplayercore> <playercore> <player.java>'

if __name__ == '__main__':

  if len(sys.argv) != 7:
    print USAGE
    sys.exit(-1)

  infilename = sys.argv[1]
  outfilename = sys.argv[2]
  aofcfilename = sys.argv[3]
  outdir = sys.argv[4]
  pcoutdir = sys.argv[5]
  pcjfilename = sys.argv[6]
  os.system('mkdir -p ' + outdir)
  os.system('mkdir -p ' + pcoutdir)

  # Read in the entire file
  infile = open(infilename, 'r')
  instream = infile.read()
  infile.close()

  outfile = open(outfilename, 'w+')
  aofcfile = open(aofcfilename, 'w+')
  pcjfile = open(pcoutdir + '/' + pcjfilename, 'w+')

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

  contentspattern = re.compile('.*\{\s*(.*?)\s*\}', re.MULTILINE | re.DOTALL)
  declpattern = re.compile('\s*([^;]*?;)', re.MULTILINE)
  typepattern = re.compile('\s*\S+')
  variablepattern = re.compile('\s*([^,;]+?)\s*[,;]')
  #arraypattern = re.compile('\[\s*(\w*?)\s*\]')
  arraypattern = re.compile('\[(.*?)\]')

  outfile.write('%inline\n%{\n\n')

  pcjfile.write('package net.sourceforge.playerstage.Jplayercore;\n')
  pcjfile.write('public class player {\n\n')

  for s in structs:
    # extract type of struct
    split = string.split(s)
    typename = split[-1]

    # pick out the contents of the struct
    varpart = contentspattern.findall(s)
    if len(varpart) != 1:
      print 'skipping nested / empty struct ' + typename
      continue

    # SWIG macro that lets us access arrays of this non-primitive type
    # as Java arrays
    aofcfile.write('JAVA_ARRAYSOFCLASSES(' + typename +')\n')

    buf_to_name = 'buf_to_' + typename
    buf_from_name = typename + '_to_buf'
    buf_to_Jname = 'buf_to_J' + typename
    buf_from_Jname = 'J' + typename + '_to_buf'
    sizeof_name = typename + '_sizeof'

    # function to return the size of the underlying C structure
    outfile.write('size_t ' + sizeof_name + '(void)\n')
    outfile.write('{\n')
    outfile.write('  return(sizeof(' + typename + '));\n')
    outfile.write('}\n')
    
    # JNI cast from a void* to a pointer to this type
    outfile.write(typename + '* ' + buf_to_name + '(void* buf)\n')
    outfile.write('{\n')
    outfile.write('  return((' + typename + '*)(buf));\n')
    outfile.write('}\n')

    # JNI cast from a pointer to this type to a void*
    outfile.write('void* ' + buf_from_name + '(' + typename + '* msg)\n')
    outfile.write('{\n')
    outfile.write('  return((void*)(msg));\n')
    outfile.write('}\n')

    # Equivalent non-JNI Java class
    jclass = 'J' + typename
    jfile = open(outdir + '/' + jclass + '.java', 'w+')
    jfile.write('package net.sourceforge.playerstage.Jplayercore;\n')
    jfile.write('import java.io.Serializable;\n')
    jfile.write('public class ' + jclass + ' implements Serializable {\n')
    jfile.write('  public final static long serialVersionUID = ' + `hash(s)` + 'L;\n')
    jclass_constructor = '  public ' + jclass + '() {\n';

    # Static method in class player to convert from JNI Java object to
    # non-JNI java object
    pcj_data_to_jdata = ''
    pcj_data_to_jdata += '  public static ' + jclass + ' ' + typename + '_to_' + jclass + '(' + typename + ' data) {\n'
    pcj_data_to_jdata += '    ' + jclass + ' Jdata = new ' + jclass + '();\n'

    # Static method in class player to convert from non-JNI Java object to
    # JNI java object
    pcj_jdata_to_data = ''
    pcj_jdata_to_data += '  public static ' + typename + ' ' + jclass + '_to_' + typename + '(' + jclass + ' Jdata) {\n'
    pcj_jdata_to_data += '    ' + typename + ' data = new ' + typename + '();\n'

    # Static method in class playercore to convert from SWIGTYPE_p_void 
    # to non-JNI Java object.
    pcjfile.write('  public static ' + jclass + ' ' + buf_to_Jname + '(SWIGTYPE_p_void buf) {\n')
    pcjfile.write('    ' + typename + ' data = playercore_java.' + buf_to_name + '(buf);\n')
    pcjfile.write('    return(' + typename + '_to_' + jclass + '(data));\n')
    pcjfile.write('  }\n\n')

    # Static method in class playercore to convert non-JNI Java object to
    # SWIGTYPE_p_void.
    pcjfile.write('  public static SWIGTYPE_p_void ' + buf_from_Jname + '(' + jclass + ' Jdata) {\n')
    pcjfile.write('    ' + typename + ' data = ' + jclass + '_to_' + typename + '(Jdata);\n')
    pcjfile.write('    return(playercore_java.' + buf_from_name + '(data));\n')
    pcjfile.write('  }\n\n')

    # separate the variable declarations
    decls = declpattern.finditer(varpart[0])
    for d in decls:
      # find the type and variable names in this declaration
      dstring = d.string[d.start(1):d.end(1)]
      type = typepattern.findall(dstring)[0]
      dstring = typepattern.sub('', dstring, 1)
      vars = variablepattern.finditer(dstring)

      # Do some name mangling for common types
      builtin_type = 1
      if type == 'int64_t':
        jtype = 'long'
      elif type == 'uint64_t':
        jtype = 'long'
      elif type == 'int32_t':
        jtype = 'int'
      elif type == 'uint32_t':
        jtype = 'long'
      elif type == 'int16_t':
        jtype = 'short'
      elif type == 'uint16_t':
        jtype = 'int'
      elif type == 'int8_t':
        jtype = 'byte'
      elif type == 'uint8_t':
        jtype = 'short'
      elif type == 'char':
        jtype = 'char'
      elif type == 'bool_t':
        jtype = 'boolean'
      elif type == 'double':
        jtype = 'double'
      elif type == 'float':
        jtype = 'float'
      else:
        # rely on a previous declaration of a J class for this type
        jtype = 'J' + type
        builtin_type = 0

      # iterate through each variable
      for var in vars:
        varstring = var.string[var.start(1):var.end(1)]
        # is it an array or a scalar?
        arraysize = arraypattern.findall(varstring)
        if len(arraysize) > 0:
          arraysize = arraysize[0]
          varstring = arraypattern.sub('', varstring)
          if jtype == 'char':
            jfile.write('  public String ' + varstring + ';\n')
          else:
            jfile.write('  public ' + jtype + '[] ' + varstring + ';\n')

          #if builtin_type == 0:
          if jtype != 'char':
            if arraysize.isdigit():
              jclass_constructor += '    ' + varstring + ' = new ' + jtype + '[' + arraysize + '];\n'
            else:
              jclass_constructor += '    ' + varstring + ' = new ' + jtype + '[playercore_javaConstants.' + arraysize + '];\n'
        else:
          arraysize = ''
          jfile.write('  public ' + jtype + ' ' + varstring + ';\n')
          if builtin_type == 0:
            jclass_constructor += '    ' + varstring + ' = new ' + jtype + '();\n'

        capvarstring = string.capitalize(varstring[0]) + varstring[1:]
        if builtin_type:
          pcj_data_to_jdata += '    Jdata.' + varstring + ' = data.get' + capvarstring + '();\n'
          pcj_jdata_to_data += '    data.set' + capvarstring + '(Jdata.' + varstring +');\n'
        else:
          if arraysize == '':
            pcj_data_to_jdata += '    Jdata.' + varstring + ' = ' + type + '_to_' + jtype + '(data.get' + capvarstring + '());\n'
            pcj_jdata_to_data += '    data.set' + capvarstring + '(' + jtype + '_to_' + type + '(Jdata.' + varstring + '));\n'
          else:
            try:
              asize = int(arraysize)
            except:
              arraysize = 'playercore_javaConstants.' + arraysize
            pcj_data_to_jdata += '    {\n'
            pcj_data_to_jdata += '      ' + type + ' foo[] = data.get' + capvarstring + '();\n'
            pcj_data_to_jdata += '      for(int i=0;i<' + arraysize + ';i++)\n'
            pcj_data_to_jdata += '        Jdata.' + varstring + '[i] = ' + type + '_to_' + jtype + '(foo[i]);\n'
            pcj_data_to_jdata += '    }\n'

            pcj_jdata_to_data += '    {\n'
            pcj_jdata_to_data += '      ' + type + ' foo[] = new ' + type + '[' + arraysize + '];\n'
            pcj_jdata_to_data += '      for(int i=0;i<' + arraysize + ';i++)\n'
            pcj_jdata_to_data += '        foo[i] = ' + jtype + '_to_' + type + '(Jdata.' + varstring + '[i]);\n'
            pcj_jdata_to_data += '      data.set' + capvarstring + '(foo);\n'
            pcj_jdata_to_data += '    }\n'
    pcj_data_to_jdata += '    return(Jdata);\n'
    pcj_data_to_jdata += '  }\n\n'
    pcjfile.write(pcj_data_to_jdata)

    pcj_jdata_to_data += '    return(data);\n'
    pcj_jdata_to_data += '  }\n\n'
    pcjfile.write(pcj_jdata_to_data)

    jclass_constructor += '  }\n'
    jfile.write(jclass_constructor)
    jfile.write('}\n')
    jfile.close()

  outfile.write('\n%}\n')
  outfile.close()

  pcjfile.write('\n}\n')
  pcjfile.close()
  
  aofcfile.close()

