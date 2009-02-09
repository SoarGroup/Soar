/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
 *                      
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/********************************************************************
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ********************************************************************/

/*
 * $Id: configfile.h 4419 2008-03-16 08:41:58Z thjc $
 */
#ifndef CONFFILE_H
#define CONFFILE_H

#include <stdio.h>

#include <libplayercore/player.h>

/** @brief Class for loading configuration file information.

This class is used to load and configure drivers from a configuration text
file.  This file is divided into sections, with section having a set of
key/value fields.

Example file format is as follows:
@code
# This is a comment

# The keyword 'driver' begins a section
driver
(
  # This line is used to identify which driver to
  # instantiate
  name "mydriver"

  # This line lists the interfaces that the driver
  # will support
  provides ["position2d:0" "laser:0"]

  # This line lists the devices to which the driver
  # will subscribe
  requires ["ptz:0"]

  # An integer
  key1  0             
  # A string
  key2 "foo"          
  # A tuple of strings
  key3 ["foo" "bar"]  
  # A tuple of multiple types
  key4 ["foo" 3.14 42]
)
@endcode

The most common use of this class is for a driver to extract configuration
file options.  All drivers are passed a ConfigFile pointer @b cf and an
integer @b section in their contructor (see Driver::Driver).  The driver
code can use this information to retrieve key/value information that was
given inside the appropriate @b driver() section in the configuration file.
For example:
@code
  int maximum_speed = cf->ReadInt(section, "max_speed", -1);
  if(maximum_speed == -1)
  {
    // Since the default value of -1 was assigned,
    // no "max_speed" key/value was given in the configuration
    // file.
  }
@endcode

For each type Foo, there is a method ReadFoo() that looks for a key/value
pair in the indicated section and with the type Foo.  The last argument
specifies a default value to return if the given key is not found.  The
default should either be a reasonable value or a recognizably invalid value
(so that you can determine when the user did not specify a value).

Always use ReadLength for linear dimensions, positions, and time
derivatives (velocities, accelerations, etc.) ; this method will return
values in meters, regardless of what local units are being used in the
configuration file.  Similarly, always use ReadAngle for angular quanities;
this method will always return values in radians.

Always use ReadFilename for filenames; this method will return absolute
paths, appropriately converting any relative paths that are given in the
configuration file.  Non-absolute paths in the configuration file are
assumed to be relative to the directory in which the configuration file
itself resides.

Always use ReadColor for packed 24-bit color values.

Drivers that support multiple interfaces must use ReadDeviceAddr to find
out which interfaces they have been asked to support (single-interface
drivers specify their one interface in the Driver constructor).  For
example, to check whether the driver has been asked to support a @ref
interface_position2d interface, and if so, to add that interface:
@code
  player_devaddr_t position_addr;
  if(cf->ReadDeviceAddr(&position_addr, section, "provides",
                        PLAYER_POSITION2D_CODE, -1, NULL) == 0)
  {
    if(this->AddInterface(position_addr) != 0)
    {
      this->SetError(-1);
      return;
    }
  }
@endcode
A driver can support more than interface of the same type.  For example,
the @ref driver_p2os driver supports 3 @ref interface_position2d interfaces:
one for wheelmotors/odometry, one for compass, and one for gyro.  In this
case, the last argument to ReadDeviceAddr is used to differentiate them,
according to key that was given in the "provides" line.  For example, if
in the configuration file we have:
@code
driver
(
  name "mydriver"
  provides ["odometry:::position2d:0" "gyro:::position2d:1"]
)
@endcode
then in the driver code we can do something like this:
@code
  player_devaddr_t odom_addr, gyro_addr;

  // Do we create an odometry position interface?
  if(cf->ReadDeviceAddr(&odom_addr, section, "provides",
                        PLAYER_POSITION2D_CODE, -1, "odometry") == 0)
  {
    if(this->AddInterface(odom_addr) != 0)
    {
      this->SetError(-1);
      return;
    }
  }

  // Do we create a gyro position interface?
  if(cf->ReadDeviceAddr(&gyro_addr, section, "provides",
                        PLAYER_POSITION2D_CODE, -1, "gyro") == 0)
  {
    if(this->AddInterface(gyro_addr) != 0)
    {
      this->SetError(-1);
      return;
    }
  }
@endcode

Drivers that subscribe to other devices use ReadDeviceAddr in the same way
to retrieve information from the "requires" line of the configuration file.

*/

class ConfigFile
{
  /// @brief Standard constructor
  public: ConfigFile(uint32_t _default_host, uint32_t _default_robot);

  /// @brief Alternate constructor, to specify the host as a string
  public: ConfigFile(const char* _default_host, uint32_t _default_robot);

  /// Alternate constructor, used when not loading from a file
  public: ConfigFile();

  /// @brief Standard destructor
  public: ~ConfigFile();

  /// @internal Intitialization helper
  private: void InitFields();

  /// @brief Load config from file
  /// @param filename Name of file; can be relative or fully qualified path.
  /// @returns Returns true on success.
  public: bool Load(const char *filename);

  /// @brief Add a (name,value) pair directly into the database, without
  /// reading from a file.  The (name,value) goes into the "global" section.
  /// Can be called multiple times with different index to create a tuple
  /// field.
  /// @param index Index of the value within the field (0 unless the field
  ///              is a tuple)
  /// @param name Name of the field
  /// @param value Value to be assigned
  public: void InsertFieldValue(int index,
                                const char* name, 
                                const char* value);

  // Save config back into file
  // Set filename to NULL to save back into the original file
  private: bool Save(const char *filename);

  /// @brief Check for unused fields and print warnings.
  /// @returns Returns true if there are unused fields.
  public: bool WarnUnused();

  /// @brief Read a boolean value (one of: yes, no, true, false, 1, 0)
  /// @param section Section to read.
  /// @param name Field name
  /// @param value Default value if this field is not present in the file
  /// @return Returns the field value
  public: bool ReadBool(int section, const char *name, bool value);

  // Write a bool as "yes" or "no"
  private: void WriteBool(int section, const char* name, bool value);

  // Write a bool as "1" or "0" (for backward compatability)
  private: void WriteBool_Compat(int section, const char* name, bool value);


  /// @brief Read a string value
  /// @param section Section to read.
  /// @param name Field name
  /// @param value Default value if the field is not present in the file.
  /// @returns Returns the field value.
  public: const char *ReadString(int section, 
                                 const char *name, 
                                 const char *value);

  // Write a string
  private: void WriteString(int section, 
                            const char *name, 
                            const char *value);

  /// @brief Read an integer value
  /// @param section Section to read.
  /// @param name Field name
  /// @param value Default value if the field is not present in the file.
  /// @returns Returns the field value.
  public: int ReadInt(int section, 
                      const char *name, 
                      int value);

  // Write an integer
  private: void WriteInt(int section, 
                         const char *name, 
                         int value);

  /// @brief Read a floating point (double) value.
  /// @param section Section to read.
  /// @param name Field name
  /// @param value Default value if the field is not present in the file.
  /// @returns Returns the field value.
  public: double ReadFloat(int section, 
                           const char *name, 
                           double value);

  // Write a float
  private: void WriteFloat(int section, 
                           const char *name, 
                           double value);

  /// @brief Read a length (includes unit conversion, if any).
  /// @param section Section to read.
  /// @param name Field name
  /// @param value Default value if the field is not present in the file.
  /// @returns Returns the field value.
  public: double ReadLength(int section, 
                            const char *name, 
                            double value);

  // Write a length (includes units conversion)
  private: void WriteLength(int section, 
                            const char *name, 
                            double value);
  
  /// @brief Read an angle (includes unit conversion).
  ///
  /// In the configuration file, angles are specified in degrees; this
  /// method will convert them to radians.
  ///
  /// @param section Section to read.
  /// @param name Field name
  /// @param value Default value if the field is not present in the file (radians).
  /// @returns Returns the field value.
  public: double ReadAngle(int section, const char *name, double value);

  /// @brief Read a color (includes text to RGB conversion)
  ///
  /// In the configuration file colors may be specified with sybolic
  /// names; e.g., "blue" and "red".  This function will convert them
  /// to an RGB value using the X11 rgb.txt file.
  ///
  /// @param section Section to read.
  /// @param name Field name
  /// @param value Default value if the field is not present in the file (RGB).
  /// @returns Returns the field value.
  public: uint32_t ReadColor(int section, 
                             const char *name, 
                             uint32_t value);

  /// @brief Read a filename.
  ///
  /// Always returns an absolute path.  If the filename is entered as
  /// a relative path, we prepend the config file's path to it.
  ///
  /// @param section Section to read.
  /// @param name Field name
  /// @param value Default value if the field is not present in the file.
  /// @returns Returns the field value.
  public: const char *ReadFilename(int section, 
                                   const char *name, 
                                   const char *value);

  /// @brief Get the number of values in a tuple.
  /// @param section Section to read.
  /// @param name Field name.
  public: int GetTupleCount(int section, const char *name);

  /// @brief Read a string from a tuple field
  /// @param section Section to read.
  /// @param name Field name
  /// @param index Tuple index (zero-based)
  /// @param value Default value if the field is not present in the file.
  /// @returns Returns the tuple element value.
  public: const char *ReadTupleString(int section, 
                                      const char *name,
                                      int index, 
                                      const char *value);
  
  // Write a string to a tuple
  private: void WriteTupleString(int section, 
                                const char *name,
                                int index, 
                                const char *value);
  
  /// @brief Read an integer from a tuple field
  /// @param section Section to read.
  /// @param name Field name
  /// @param index Tuple index (zero-based)
  /// @param value Default value if the field is not present in the file.
  /// @returns Returns the tuple element value.
  public: int ReadTupleInt(int section, 
                           const char *name,
                           int index, 
                           int value);

  // Write a int to a tuple
  private: void WriteTupleInt(int section, 
                             const char *name,
                             int index, 
                             int value);
  

  /// @brief Read a float (double) from a tuple field
  /// @param section Section to read.
  /// @param name Field name
  /// @param index Tuple index (zero-based)
  /// @param value Default value if the field is not present in the file.
  /// @returns Returns the tuple element value.
  public: double ReadTupleFloat(int section, 
                                const char *name,
                                int index, 
                                double value);

  // Write a float to a tuple
  private: void WriteTupleFloat(int section, 
                               const char *name,
                               int index, 
                               double value);

  /// @brief Read a length from a tuple (includes units conversion)
  /// @param section Section to read.
  /// @param name Field name
  /// @param index Tuple index (zero-based)
  /// @param value Default value if the field is not present in the file.
  /// @returns Returns the tuple element value.
  public: double ReadTupleLength(int section, 
                                 const char *name,
                                 int index, 
                                 double value);

  // Write a to a tuple length (includes units conversion)
  private: void WriteTupleLength(int section, 
                                const char *name,
                                int index, 
                                double value);

  /// @brief Read an angle form a tuple (includes units conversion)
  ///
  /// In the configuration file, angles are specified in degrees; this
  /// method will convert them to radians.
  ///
  /// @param section Section to read.
  /// @param name Field name
  /// @param index Tuple index (zero-based)
  /// @param value Default value if the field is not present in the file.
  /// @returns Returns the tuple element value.
  public: double ReadTupleAngle(int section, 
                                const char *name,
                                int index, 
                                double value);

  // Write an angle to a tuple (includes units conversion)
  private: void WriteTupleAngle(int section, 
                               const char *name,
                               int index, 
                               double value);

  /// @brief Read a color (includes text to RGB conversion)
  ///
  /// In the configuration file colors may be specified with sybolic
  /// names; e.g., "blue" and "red".  This function will convert them
  /// to an RGB value using the X11 rgb.txt file.
  ///
  /// @param section Section to read.
  /// @param name Field name
  /// @param index Tuple index (zero-based)  
  /// @param value Default value if the field is not present in the file (RGB).
  /// @returns Returns the field value.
  public: uint32_t ReadTupleColor(int section, 
                                  const char *name,
                                  int index, 
                                  uint32_t value); 

  /// @brief Read a device id.
  ///
  /// Reads a device id from the named field of the given section.
  /// The returned id will match the given code, index and key values.
  //
  /// @param addr address field to be filled in.
  /// @param section File section.
  /// @param name Field name.
  /// @param code Interface type code (use 0 to match all interface types).
  /// @param index Tuple index (use -1 to match all indices).
  /// @param key Device key value (use NULL to match all key vales).
  /// @return Non-zero on error.
  public: int ReadDeviceAddr(player_devaddr_t* addr, int section, 
                             const char *name, int code, int index, 
                             const char *key);

  // Parse a driver block, and update the deviceTable accordingly
  public: bool ParseDriver(int section);

  // Parse an interface block, and update the interface systems accordingly
  public: bool ParseInterface(int section);

  // Parse all driver blocks
  public: bool ParseAllDrivers();

  // Parse all interface blocks
  public: bool ParseAllInterfaces();

  /// @brief Get the number of sections.
  public: int GetSectionCount();

  /// @brief Get a section type name.
  public: const char *GetSectionType(int section);

  /// @brief Lookup a section number by section type name.
  /// @return Returns -1 if there is no section with this type.
  public: int LookupSection(const char *type);
  
  /// @brief Get a section's parent section.
  /// @returns Returns -1 if there is no parent.
  public: int GetSectionParent(int section);


  ////////////////////////////////////////////////////////////////////////////
  // Private methods used to load stuff from the config file
  
  // Load tokens from a file.
  private: bool LoadTokens(FILE *file, int include);

  // Read in a comment token
  private: bool LoadTokenComment(FILE *file, int *line, int include);

  // Read in a word token
  private: bool LoadTokenWord(FILE *file, int *line, int include);

  // Load an include token; this will load the include file.
  private: bool LoadTokenInclude(FILE *file, int *line, int include);

  // Read in a number token
  private: bool LoadTokenNum(FILE *file, int *line, int include);

  // Read in a string token
  private: bool LoadTokenString(FILE *file, int *line, int include);

  // Read in a whitespace token
  private: bool LoadTokenSpace(FILE *file, int *line, int include);

  // Save tokens to a file.
  private: bool SaveTokens(FILE *file);

  // Clear the token list
  private: void ClearTokens();

  // Add a token to the token list
  private: bool AddToken(int type, const char *value, int include);

  // Set a token in the token list
  private: bool SetTokenValue(int index, const char *value);

  // Get the value of a token
  private: const char *GetTokenValue(int index);

  /// @brief Dump the token list (for debugging).
  public: void DumpTokens();

  // Parse a line
  private: bool ParseTokens();

  // Parse an include statement
  private: bool ParseTokenInclude(int *index, int *line);

  // Parse a macro definition
  private: bool ParseTokenDefine(int *index, int *line);

  // Parse a macro definition
  private: bool ParseTokenPlugin(int *index, int *line);

  // Parse an word (could be a section or an field) from the token list.
  private: bool ParseTokenWord(int section, int *index, int *line);

  // Parse a section from the token list.
  private: bool ParseTokenSection(int section, int *index, int *line);

  // Parse an field from the token list.
  private: bool ParseTokenField(int section, int *index, int *line);

  // Parse a tuple.
  private: bool ParseTokenTuple(int section, int field, 
                                int *index, int *line);

  // Clear the macro list
  private: void ClearMacros();

  // Add a macro
  private: int AddMacro(const char *macroname, const char *sectionname,
                        int line, int starttoken, int endtoken);

  // Lookup a macro by name
  // Returns -1 if there is no macro with this name.
  private: int LookupMacro(const char *macroname);

  // Dump the macro list for debugging
  private: void DumpMacros();

  // Clear the section list
  private: void ClearSections();

  // Add a section
  private: int AddSection(int parent, const char *type);

  /// @brief Dump the section list for debugging
  public: void DumpSections();

  // Clear the field list
  private: void ClearFields();

  // Add a field
  private: int AddField(int section, const char *name, int line);

  // Add a field value.
  private: void AddFieldValue(int field, int index, int value_token);
  
  // Get a field
  private: int GetField(int section, const char *name);

  // Get the number of elements for this field
  private: int GetFieldValueCount(int field);

  // Get the value of an field element
  // Set flag_used to true mark the field element as read.
  private: const char *GetFieldValue(int field, int index, bool flag_used = true);

  // Set the value of an field.
  private: void SetFieldValue(int field, int index, const char *value);

  /// @brief Dump the field list for debugging
  public: void DumpFields();

  // Look up the color in a data based (transform color name -> color value).
  private: uint32_t LookupColor(const char *name);

  /// Name of the file we loaded
  public: char *filename;

  // Token types.
  private: enum
    {
      TokenComment,
      TokenWord, TokenNum, TokenString, TokenBool,
      TokenOpenSection, TokenCloseSection,
      TokenOpenTuple, TokenCloseTuple,
      TokenSpace, TokenEOL
    };

  // Token structure.
  private: struct Token
  {
    // Non-zero if token is from an include file.
    int include;
    
    // Token type (enumerated value).
    int type;

    // Token value
    char *value;
  };

  // A list of tokens loaded from the file.
  // Modified values are written back into the token list.
  private: int token_size, token_count;
  private: Token *tokens;

  // Private macro class
  private: struct CMacro
  {
    // Name of macro
    const char *macroname;

    // Name of section
    const char *sectionname;

    // Line the macro definition starts on.
    int line;
    
    // Range of tokens in the body of the macro definition.
    int starttoken, endtoken;
  };

  // Macro list
  private: int macro_size;
  private: int macro_count;
  private: CMacro *macros;
  
  // Private section class
  private: struct Section
  {
    // Parent section
    int parent;

    // Type of section (i.e. position, laser, etc).
    const char *type;
  };

  // Section list
  private: int section_size;
  private: int section_count;
  private: Section *sections;

  // Private field class
  private: struct Field
  {
    // Index of section this field belongs to
    int section;

    // Name of field
    const char *name;
    
    // A list of token indexes
    int value_count;
    int *values;

    // Flag set if field value has been used
    bool *useds;

    // Line this field came from
    int line;
  };
  
  // Field list
  private: int field_size;
  private: int field_count;
  private: Field *fields;
  private: uint32_t default_host;
  private: uint32_t default_robot;

  // Conversion units
  private: double unit_length;
  private: double unit_angle;
};

#endif
