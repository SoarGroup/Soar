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
 * $Id: configfile.cc 4419 2008-03-16 08:41:58Z thjc $
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <locale.h>

#include <replace/replace.h>

#include <libplayercore/player.h>
#include <libplayercore/playercommon.h>
#include <libplayercore/error.h>
#include <libplayercore/configfile.h>
#include <libplayercore/interface_util.h>
#include <libplayercore/driver.h>
#include <libplayercore/drivertable.h>
#include <libplayercore/devicetable.h>
#include <libplayercore/globals.h>
#include <libplayercore/plugins.h>
#include <libplayercore/addr_util.h>
#include <libplayerxdr/functiontable.h>

//extern int global_playerport;

const char * COLOR_DATABASE[5] = {"/usr/X11R6/lib/X11/rgb.txt","/usr/share/X11/rgb.txt","/etc/X11/rgb.txt","/usr/lib/X11/rgb.txt", NULL};

///////////////////////////////////////////////////////////////////////////
// the isblank() macro is not standard - it's a GNU extension
// and it doesn't work for me, so here's an implementation - rtv
#ifndef isblank
  #define isblank(a) (a == ' ' || a == '\t')
#endif

///////////////////////////////////////////////////////////////////////////
// Useful macros for dumping parser errors
#define TOKEN_ERR(z, l) \
  fprintf(stderr, "%s:%d error: " z "\n", this->filename, l)
#define PARSE_ERR(z, l) \
  fprintf(stderr, "%s:%d error: " z "\n", this->filename, l)

#define CONFIG_WARN1(z, line, a) \
  fprintf(stderr, "%s:%d warning: " z "\n", this->filename, line, a)
#define CONFIG_WARN2(z, line, a, b) \
  fprintf(stderr, "%s:%d warning: " z "\n", this->filename, line, a, b)

#define CONFIG_ERR1(z, line, a) \
  fprintf(stderr, "%s:%d error: " z "\n", this->filename, line, a)
#define CONFIG_ERR2(z, line, a, b) \
  fprintf(stderr, "%s:%d error: " z "\n", this->filename, line, a, b)


///////////////////////////////////////////////////////////////////////////
// Default constructor
ConfigFile::ConfigFile(uint32_t _default_host, uint32_t _default_robot) 
{
  this->default_host = _default_host;
  this->default_robot = _default_robot;
  this->InitFields();
}

/// Alternate constructor, to specify the host as a string
ConfigFile::ConfigFile(const char* _default_host, uint32_t _default_robot)
{
  if(hostname_to_packedaddr(&this->default_host, _default_host) < 0)
  {
    PLAYER_WARN1("name lookup failed on \"%s\"", _default_host);
    this->default_host = 0;
  }
  this->default_robot = _default_robot;
  this->InitFields();
}

/// Alternate constructor, used when not loading from a file
ConfigFile::ConfigFile()
{
  this->default_host = 0;
  this->default_robot = 0;
  this->filename = strdup("");
  this->InitFields();
}

void
ConfigFile::InitFields()
{
  this->filename = NULL;

  this->token_size = 0;
  this->token_count = 0;
  this->tokens = NULL;

  this->macro_count = 0;
  this->macro_size = 0;
  this->macros = NULL;

  this->section_count = 0;
  this->section_size = 0;
  this->sections = NULL;

  this->field_count = 0;
  this->field_size = 0;
  this->fields = NULL;

  // Set defaults units
  this->unit_length = 1.0;
  this->unit_angle = M_PI / 180;

  if(!setlocale(LC_ALL,"POSIX"))
    fputs("Warning: failed to setlocale(); config file may not be parse correctly\n", stderr);
}


///////////////////////////////////////////////////////////////////////////
// Destructor
ConfigFile::~ConfigFile()
{
  ClearFields();
  ClearMacros();
  ClearSections();
  ClearTokens();

  if (this->filename)
    free(this->filename);
}


///////////////////////////////////////////////////////////////////////////
// Load world from file
bool ConfigFile::Load(const char *filename)
{
  // Shouldnt call load more than once,
  // so this should be null.
  assert(this->filename == NULL);
  this->filename = strdup(filename);

  // Open the file
  FILE *file = fopen(this->filename, "r");
  if (!file)
  {
    PLAYER_ERROR2("unable to open world file %s : %s",
               this->filename, strerror(errno));
    //fclose(file);
    return false;
  }

  ClearTokens();
  
  // Read tokens from the file
  if (!LoadTokens(file, 0))
  {
    //DumpTokens();
    fclose(file);
    return false;
  }

  // Parse the tokens to identify sections
  if (!ParseTokens())
  {
    //DumpTokens();
    fclose(file);
    return false;
  }

  // Dump contents and exit if this file is meant for debugging only.
  if (ReadInt(0, "test", 0) != 0)
  {
    PLAYER_ERROR("this is a test file; quitting");
    DumpTokens();
    DumpMacros();
    DumpSections();
    DumpFields();
    fclose(file);
    return false;
  }
  
  // Work out what the length units are
  const char *unit = ReadString(0, "unit_length", "m");
  if (strcmp(unit, "m") == 0)
    this->unit_length = 1.0;
  else if (strcmp(unit, "cm") == 0)
    this->unit_length = 0.01;
  else if (strcmp(unit, "mm") == 0)
    this->unit_length = 0.001;

  // Work out what the angle units are
  unit = ReadString(0, "unit_angle", "degrees");
  if (strcmp(unit, "degrees") == 0)
    this->unit_angle = M_PI / 180;
  else if (strcmp(unit, "radians") == 0)
    this->unit_angle = 1;
  
  //DumpTokens();
  fclose(file);  
  return true;
}

///////////////////////////////////////////////////////////////////////////
/// Add a (name,value) pair directly into the database, without
/// reading from a file.  The (name,value) goes into the "global" section.
void ConfigFile::InsertFieldValue(int index,
                                  const char* name, 
                                  const char* value)
{
  // AddField checks whether the field already exists
  int field = this->AddField(-1,name,0);
  this->AddToken(ConfigFile::TokenWord, value, 0);
  this->AddFieldValue(field, index, this->token_count-1);
}


///////////////////////////////////////////////////////////////////////////
// Save world to file
bool ConfigFile::Save(const char *filename)
{
  // Debugging
  //DumpFields();
  
  // If no filename is supplied, use default
  if (!filename)
    filename = this->filename;

  // Open file
  FILE *file = fopen(filename, "w+");
  if (!file)
  {
    PLAYER_ERROR2("unable to open world file %s : %s",
               filename, strerror(errno));
    return false;
  }

  // Write the current set of tokens to the file
  if (!SaveTokens(file))
  {
    fclose(file);
    return false;
  }

  fclose(file);
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Check for unused fields and print warnings
bool ConfigFile::WarnUnused()
{
  bool unused = false;
  for (int i = 0; i < this->field_count; i++)
  {
    Field *field = this->fields + i;

    if (field->value_count <= 1)
    {
      if (!field->useds[0])
      {
        unused = true;
        CONFIG_WARN1("field [%s] is defined but not used",
                     field->line, field->name);
      }
    }
    else
    {
      for (int j = 0; j < field->value_count; j++)
      {
        if (!field->useds[j])
        {
          unused = true;
          CONFIG_WARN2("field [%s] has unused element %d",
                       field->line, field->name, j);
        }
      }
    }
  }
  return unused;
}


///////////////////////////////////////////////////////////////////////////
// Load tokens from a file.
bool ConfigFile::LoadTokens(FILE *file, int include)
{
  int ch;
  int line;
  char token[256];
  
  line = 1;

  while (true)
  {
    ch = fgetc(file);
    if (ch == EOF)
      break;
    
    if ((char) ch == '#')
    {
      ungetc(ch, file);
      if (!LoadTokenComment(file, &line, include))
        return false;
    }
    else if (isalpha(ch))
    {
      ungetc(ch, file);
      if (!LoadTokenWord(file, &line, include))
        return false;
    }
    else if (strchr("+-.0123456789", ch))
    {
      ungetc(ch, file);
      if (!LoadTokenNum(file, &line, include))
        return false;
    }
    else if (isblank(ch))
    {
      ungetc(ch, file);
      if (!LoadTokenSpace(file, &line, include))
        return false;
    }
    else if (ch == '"')
    {
      ungetc(ch, file);
      if (!LoadTokenString(file, &line, include))
        return false;
    }
    else if (strchr("(", ch))
    {
      token[0] = ch;
      token[1] = 0;
      AddToken(TokenOpenSection, token, include);
    }
    else if (strchr(")", ch))
    {
      token[0] = ch;
      token[1] = 0;
      AddToken(TokenCloseSection, token, include);
    }
    else if (strchr("[", ch))
    {
      token[0] = ch;
      token[1] = 0;
      AddToken(TokenOpenTuple, token, include);
    }
    else if (strchr("]", ch))
    {
      token[0] = ch;
      token[1] = 0;
      AddToken(TokenCloseTuple, token, include);
    }
    else if ( 0x0d == ch )
    {
      ch = fgetc(file);
      if ( 0x0a != ch ) 
        ungetc(ch, file);
      line++;
      AddToken(TokenEOL, "\n", include);
    }
    else if ( 0x0a == ch )
    {
      ch = fgetc(file);
      if ( 0x0d != ch ) 
        ungetc(ch, file);
      line++;
      AddToken(TokenEOL, "\n", include);
    }
    else
    {
      TOKEN_ERR("syntax error", line);
      return false;
    }
  }

  return true;
}


///////////////////////////////////////////////////////////////////////////
// Read in a comment token
bool ConfigFile::LoadTokenComment(FILE *file, int *line, int include)
{
  char token[1024];
  int len;
  int ch;

  len = 0;
  memset(token, 0, sizeof(token));
  
  while (true)
  {
    ch = fgetc(file);

    if (ch == EOF)
    {
      AddToken(TokenComment, token, include);
      return true;
    }
    else if ( 0x0a == ch || 0x0d == ch )
    {
      ungetc(ch, file);
      AddToken(TokenComment, token, include);
      return true;
    }
    else
      token[len++] = ch;
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Read in a word token
bool ConfigFile::LoadTokenWord(FILE *file, int *line, int include)
{
  char token[1024];
  int len;
  int ch;
  
  len = 0;
  memset(token, 0, sizeof(token));
  
  while (true)
  {
    ch = fgetc(file);

    if (ch == EOF)
    {
      AddToken(TokenWord, token, include);
      return true;
    }
    else if (isalpha(ch) || isdigit(ch) || strchr(".-_[]:", ch))
    {
      token[len++] = ch;
    }
    else
    {
      if (strcmp(token, "include") == 0)
      {
        ungetc(ch, file);
        AddToken(TokenWord, token, include);
        if (!LoadTokenInclude(file, line, include))
          return false;
      }
      else if(strcmp(token, "true") == 0 || strcmp(token, "false") == 0 || 
              strcmp(token, "yes") == 0  || strcmp(token, "no") == 0) 
      {
        ungetc(ch, file);
        AddToken(TokenBool, token, include);
      }
      else
      {
        ungetc(ch, file);
        AddToken(TokenWord, token, include);
      }
      return true;
    }
  }
  assert(false);
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Load an include token; this will load the include file.
bool ConfigFile::LoadTokenInclude(FILE *file, int *line, int include)
{
  int ch;
  const char *filename;
  char *fullpath;
  
  ch = fgetc(file);

  if (ch == EOF)
  {
    TOKEN_ERR("incomplete include statement", *line);
    return false;
  }
  else if (!isblank(ch))
  {
    TOKEN_ERR("syntax error in include statement", *line);
    return false;
  }

  ungetc(ch, file);
  if (!LoadTokenSpace(file, line, include))
    return false;

  ch = fgetc(file);
  
  if (ch == EOF)
  {
    TOKEN_ERR("incomplete include statement", *line);
    return false;
  }
  else if (ch != '"')
  {
    TOKEN_ERR("syntax error in include statement", *line);
    return false;
  }

  ungetc(ch, file);
  if (!LoadTokenString(file, line, include))
    return false;

  // This is the basic filename
  filename = GetTokenValue(this->token_count - 1);

  // Now do some manipulation.  If its a relative path,
  // we append the path of the world file.
  if (filename[0] == '/' || filename[0] == '~')
  {
    fullpath = strdup(filename);
  }
  else if (this->filename[0] == '/' || this->filename[0] == '~')
  {
    // Note that dirname() modifies the contents, so
    // we need to make a copy of the filename.
    // There's no bounds-checking, but what the heck.
    char *tmp = strdup(this->filename);
    fullpath = (char*) malloc(PATH_MAX);
    memset(fullpath, 0, PATH_MAX);
    strcat( fullpath, dirname(tmp));
    strcat( fullpath, "/" ); 
    strcat( fullpath, filename );
    assert(strlen(fullpath) + 1 < PATH_MAX);
    free(tmp);
  }
  else
  {
    // Note that dirname() modifies the contents, so
    // we need to make a copy of the filename.
    // There's no bounds-checking, but what the heck.
    char *tmp = strdup(this->filename);
    fullpath = (char*) malloc(PATH_MAX);
    getcwd(fullpath, PATH_MAX);
    strcat( fullpath, "/" ); 
    strcat( fullpath, dirname(tmp));
    strcat( fullpath, "/" ); 
    strcat( fullpath, filename );
    assert(strlen(fullpath) + 1 < PATH_MAX);
    free(tmp);
  }

  // Open the include file
  FILE *infile = fopen(fullpath, "r");
  if (!infile)
  {
    PLAYER_ERROR2("unable to open include file %s : %s",
               fullpath, strerror(errno));
    free(fullpath);
    return false;
  }

  // Add an EOL to stop parsing of the include statement
  AddToken(TokenEOL, "\n", include);

  // Read tokens from the file
  if (!LoadTokens(infile, include + 1))
  {
    //DumpTokens();
    free(fullpath);
    return false;
  }

  free(fullpath);
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Read in a number token
bool ConfigFile::LoadTokenNum(FILE *file, int *line, int include)
{
  char token[1024];
  int len;
  int ch;
  
  len = 0;
  memset(token, 0, sizeof(token));
  
  while (true)
  {
    ch = fgetc(file);

    if (ch == EOF)
    {
      AddToken(TokenNum, token, include);
      return true;
    }
    else if (strchr("+-.0123456789", ch))
    {
      token[len++] = ch;
    }
    else
    {
      AddToken(TokenNum, token, include);
      ungetc(ch, file);
      return true;
    }
  }
  assert(false);
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Read in a string token
bool ConfigFile::LoadTokenString(FILE *file, int *line, int include)
{
  int ch;
  int len;
  char token[1024];
  
  len = 0;
  memset(token, 0, sizeof(token));

  ch = fgetc(file);
      
  while (true)
  {
    ch = fgetc(file);

    if (ch == EOF || ch == '\n')
    {
      TOKEN_ERR("unterminated string constant", *line);
      return false;
    }
    else if (ch == '"')
    {
      AddToken(TokenString, token, include);
      return true;
    }
    else
    {
      token[len++] = ch;
    }
  }
  assert(false);
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Read in a whitespace token
bool ConfigFile::LoadTokenSpace(FILE *file, int *line, int include)
{
  int ch;
  int len;
  char token[1024];
  
  len = 0;
  memset(token, 0, sizeof(token));
  
  while (true)
  {
    ch = fgetc(file);

    if (ch == EOF)
    {
      AddToken(TokenSpace, token, include);
      return true;
    }
    else if (isblank(ch))
    {
      token[len++] = ch;
    }
    else
    {
      AddToken(TokenSpace, token, include);
      ungetc(ch, file);
      return true;
    }
  }
  assert(false);
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Save tokens to a file.
bool ConfigFile::SaveTokens(FILE *file)
{
  int i;
  Token *token;
  
  for (i = 0; i < this->token_count; i++)
  {
    token = this->tokens + i;

    if (token->include > 0)
      continue;
    if (token->type == TokenString)
      fprintf(file, "\"%s\"", token->value);  
    else
      fprintf(file, "%s", token->value);
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Clear the token list
void ConfigFile::ClearTokens()
{
  int i;
  Token *token;

  for (i = 0; i < this->token_count; i++)
  {
    token = this->tokens + i;
    free(token->value);
  }
  free(this->tokens);
  this->tokens = 0;
  this->token_size = 0;
  this->token_count = 0;
}


///////////////////////////////////////////////////////////////////////////
// Add a token to the token list
bool ConfigFile::AddToken(int type, const char *value, int include)
{
  if (this->token_count >= this->token_size)
  {
    this->token_size += 1000;
    this->tokens = (Token*) realloc(this->tokens, this->token_size * sizeof(this->tokens[0]));
  }

  this->tokens[this->token_count].include = include;  
  this->tokens[this->token_count].type = type;
  this->tokens[this->token_count].value = strdup(value);
  this->token_count++;
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Set a token value in the token list
bool ConfigFile::SetTokenValue(int index, const char *value)
{
  assert(index >= 0 && index < this->token_count);

  free(this->tokens[index].value);
  this->tokens[index].value = strdup(value);
  
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Get the value of a token
const char *ConfigFile::GetTokenValue(int index)
{
  assert(index >= 0 && index < this->token_count);

  return this->tokens[index].value;
}


///////////////////////////////////////////////////////////////////////////
// Dump the token list (for debugging).
void ConfigFile::DumpTokens()
{
  int line;

  line = 1;
  printf("\n## begin tokens\n");
  printf("## %4d : ", line);
  for (int i = 0; i < this->token_count; i++)
  {
    if (this->tokens[i].value[0] == '\n')
      printf("[\\n]\n## %4d : %02d ", ++line, this->tokens[i].include);
    else
      printf("[%s] ", this->tokens[i].value);
  }
  printf("\n");
  printf("## end tokens\n");
}


///////////////////////////////////////////////////////////////////////////
// Parse tokens into sections and fields.
bool ConfigFile::ParseTokens()
{
  int i;
  int section;
  int line;
  Token *token;

  ClearSections();
  ClearFields();
  
  // Add in the "global" section.
  section = AddSection(-1, "");
  line = 1;
  
  for (i = 0; i < this->token_count; i++)
  {
    token = this->tokens + i;

    switch (token->type)
    {
      case TokenWord:
        if (strcmp(token->value, "include") == 0)
        {
          if (!ParseTokenInclude(&i, &line))
            return false;
        }
        else if (strcmp(token->value, "define") == 0)
        {
          if (!ParseTokenDefine(&i, &line))
            return false;
        }
        else if (strcmp(token->value, "plugin") == 0)
        {
          if (!ParseTokenPlugin(&i, &line))
            return false;
        }
        else
        {
          if (!ParseTokenWord(section, &i, &line))
            return false;
        }
        break;
      case TokenComment:
        break;
      case TokenSpace:
        break;
      case TokenEOL:
        line++;
        break;
      default:
        PARSE_ERR("syntax error 1", line);
        return false;
    }
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Parse an include statement
bool ConfigFile::ParseTokenInclude(int *index, int *line)
{
  int i;
  Token *token;

  for (i = *index + 1; i < this->token_count; i++)
  {
    token = this->tokens + i;

    switch (token->type)
    {
      case TokenString:
        break;
      case TokenSpace:
        break;
      case TokenEOL:
        *index = i;
        (*line)++;
        return true;
      default:
        PARSE_ERR("syntax error in include statement", *line);
    }
  }
  PARSE_ERR("incomplete include statement", *line);
  return false;
}

///////////////////////////////////////////////////////////////////////////
// Parse a plugin statement
bool ConfigFile::ParseTokenPlugin(int *index, int *line)
{
  int i;
  Token *token;

  lt_dlhandle handle;

  for (i = *index + 1; i < this->token_count; i++)
  {
    token = this->tokens + i;

    switch (token->type)
    {
      case TokenString:
        // Load the plugin
        if((handle = LoadPlugin(token->value,this->filename)) == NULL)
        {
          PLAYER_ERROR1("failed to load plugin: %s", token->value);
          return false;
        }
        InitDriverPlugin(handle);
        break;
      case TokenSpace:
        break;
      case TokenEOL:
        *index = i;
        (*line)++;
        return true;
      default:
        PARSE_ERR("syntax error in plugin statement", *line);
    }
  }
  PARSE_ERR("incomplete include statement", *line);
  return false;
}

///////////////////////////////////////////////////////////////////////////
// Parse a macro definition
bool ConfigFile::ParseTokenDefine(int *index, int *line)
{
  int i;
  int count;
  const char *macroname, *sectionname;
  int starttoken;
  Token *token;

  count = 0;
  macroname = NULL;
  sectionname = NULL;
  starttoken = -1;

  for (i = *index + 1; i < this->token_count; i++)
  {
    token = this->tokens + i;

    switch (token->type)
    {
      case TokenWord:
        if (count == 0)
        {
          if (macroname == NULL)
            macroname = GetTokenValue(i);
          else if (sectionname == NULL)
          {
            sectionname = GetTokenValue(i);
            starttoken = i;
          }
          else
          {
            PARSE_ERR("extra tokens in macro definition", *line);
            return false;
          }
        }
        else
        {
          if (macroname == NULL)
          {
            PARSE_ERR("missing name in macro definition", *line);
            return false;
          }
          if (sectionname == NULL)
          {
            PARSE_ERR("missing name in macro definition", *line);
            return false;
          }
        }
        break;
      case TokenOpenSection:
        count++;
        break;
      case TokenCloseSection:
        count--;
        if (count == 0)
        {
          AddMacro(macroname, sectionname, *line, starttoken, i);
          *index = i;
          return true;
        }
        if (count < 0)
        {
          PARSE_ERR("misplaced ')'", *line);
          return false;
        }
        break;
      default:
        break;
    }
  }
  PARSE_ERR("missing ')'", *line);
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Parse something starting with a word; could be a section or an field.
bool ConfigFile::ParseTokenWord(int section, int *index, int *line)
{
  int i;
  Token *token;

  for (i = *index + 1; i < this->token_count; i++)
  {
    token = this->tokens + i;

    switch (token->type)
    {
      case TokenComment:
        break;
      case TokenSpace:
        break;
      case TokenEOL:
        (*line)++;
        break;
      case TokenOpenSection:
        return ParseTokenSection(section, index, line);
      case TokenNum:
      case TokenString:
      case TokenOpenTuple:
      case TokenBool:
        return ParseTokenField(section, index, line);
      default:
        PARSE_ERR("syntax error 2", *line);
        return false;
    }
  }
  
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Parse a section from the token list.
bool ConfigFile::ParseTokenSection(int section, int *index, int *line)
{
  int i;
  int macro;
  int name;
  Token *token;

  name = *index;
  macro = LookupMacro(GetTokenValue(name));
  
  // If the section name is a macro...
  if (macro >= 0)
  {
    // This is a bit of a hack
    int nsection = this->section_count;
    int mindex = this->macros[macro].starttoken;
    int mline = this->macros[macro].line;
    if (!ParseTokenSection(section, &mindex, &mline))
      return false;
    section = nsection;

    for (i = *index + 1; i < this->token_count; i++)
    {
      token = this->tokens + i;

      switch (token->type)
      {
        case TokenOpenSection:
          break;
        case TokenWord:
          if (!ParseTokenWord(section, &i, line))
            return false;
          break;
        case TokenCloseSection:
          *index = i;
          return true;
        case TokenComment:
          break;
        case TokenSpace:
          break;
        case TokenEOL:
          (*line)++;
          break;
        default:
          PARSE_ERR("syntax error 3", *line);
          return false;
      }
    }
    PARSE_ERR("missing ')'", *line);
  }
  // If the section name is not a macro...
  else
  {
    for (i = *index + 1; i < this->token_count; i++)
    {
      token = this->tokens + i;

      switch (token->type)
      {
        case TokenOpenSection:
          section = AddSection(section, GetTokenValue(name));
          break;
        case TokenWord:
          if (!ParseTokenWord(section, &i, line))
            return false;
          break;
        case TokenCloseSection:
          *index = i;
          return true;
        case TokenComment:
          break;
        case TokenSpace:
          break;
        case TokenEOL:
          (*line)++;
          break;
        default:
          PARSE_ERR("syntax error 3", *line);
          return false;
      }
    }
    PARSE_ERR("missing ')'", *line);
  }
  return false;
}


///////////////////////////////////////////////////////////////////////////
// Parse an field from the token list.
bool ConfigFile::ParseTokenField(int section, int *index, int *line)
{
  int i, field;
  int name, value, count;
  Token *token;

  name = *index;
  value = -1;
  count = 0;
  
  for (i = *index + 1; i < this->token_count; i++)
  {
    token = this->tokens + i;

    switch (token->type)
    {
      case TokenNum:
        field = AddField(section, GetTokenValue(name), *line);
        AddFieldValue(field, 0, i);
        *index = i;
        return true;
      case TokenString:
        field = AddField(section, GetTokenValue(name), *line);
        AddFieldValue(field, 0, i);
        *index = i;
        return true;
      case TokenOpenTuple:
        field = AddField(section, GetTokenValue(name), *line);
        if (!ParseTokenTuple(section, field, &i, line))
          return false;
        *index = i;
        return true;
      case TokenBool:
        field = AddField(section, GetTokenValue(name), *line);
        AddFieldValue(field, 0, i);
        *index = i;
        return true;
      case TokenSpace:
        break;
      default:
        PARSE_ERR("syntax error 4", *line);
        return false;
    }
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Parse a tuple.
bool ConfigFile::ParseTokenTuple(int section, int field, int *index, int *line)
{
  int i, count;
  Token *token;

  count = 0;
  
  for (i = *index + 1; i < this->token_count; i++)
  {
    token = this->tokens + i;

    switch (token->type)
    {
      case TokenNum:
        AddFieldValue(field, count++, i);
        *index = i;
        break;
      case TokenString:
        AddFieldValue(field, count++, i);
        *index = i;
        break;
      case TokenCloseTuple:
        *index = i;
        return true;
      case TokenSpace:
      case TokenEOL:
        break;
      default:
        PARSE_ERR("syntax error 5", *line);
        return false;
    }
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////
// Clear the macro list
void ConfigFile::ClearMacros()
{
  free(this->macros);
  this->macros = NULL;
  this->macro_size = 0;
  this->macro_count = 0;
}


///////////////////////////////////////////////////////////////////////////
// Add a macro
int ConfigFile::AddMacro(const char *macroname, const char *sectionname,
                         int line, int starttoken, int endtoken)
{
  if (this->macro_count >= this->macro_size)
  {
    this->macro_size += 100;
    this->macros = (CMacro*)
      realloc(this->macros, this->macro_size * sizeof(this->macros[0]));
  }

  int macro = this->macro_count;
  this->macros[macro].macroname = macroname;
  this->macros[macro].sectionname = sectionname;
  this->macros[macro].line = line;
  this->macros[macro].starttoken = starttoken;
  this->macros[macro].endtoken = endtoken;
  this->macro_count++;
  
  return macro;
}


///////////////////////////////////////////////////////////////////////////
// Lookup a macro by name
// Returns -1 if there is no macro with this name.
int ConfigFile::LookupMacro(const char *macroname)
{
  int i;
  CMacro *macro;
  
  for (i = 0; i < this->macro_count; i++)
  {
    macro = this->macros + i;
    if (strcmp(macro->macroname, macroname) == 0)
      return i;
  }
  return -1;
}


///////////////////////////////////////////////////////////////////////////
// Dump the macro list for debugging
void ConfigFile::DumpMacros()
{
  printf("\n## begin macros\n");
  for (int i = 0; i < this->macro_count; i++)
  {
    CMacro *macro = this->macros + i;

    printf("## [%s][%s]", macro->macroname, macro->sectionname);
    for (int j = macro->starttoken; j <= macro->endtoken; j++)
    {
      if (this->tokens[j].type == TokenEOL)
        printf("[\\n]");
      else
        printf("[%s]", GetTokenValue(j));
    }
    printf("\n");
  }
  printf("## end macros\n");
}


///////////////////////////////////////////////////////////////////////////
// Clear the section list
void ConfigFile::ClearSections()
{
  free(this->sections);
  this->sections = NULL;
  this->section_size = 0;
  this->section_count = 0;
}


///////////////////////////////////////////////////////////////////////////
// Add a section
int ConfigFile::AddSection(int parent, const char *type)
{
  if (this->section_count >= this->section_size)
  {
    this->section_size += 100;
    this->sections = (Section*)
      realloc(this->sections, this->section_size * sizeof(this->sections[0]));
  }

  int section = this->section_count;
  this->sections[section].parent = parent;
  this->sections[section].type = type;
  this->section_count++;
  
  return section;
}


///////////////////////////////////////////////////////////////////////////
// Get the number of sections
int ConfigFile::GetSectionCount()
{
  return this->section_count;
}


///////////////////////////////////////////////////////////////////////////
// Get a section's parent section
int ConfigFile::GetSectionParent(int section)
{
  if (section < 0 || section >= this->section_count)
    return -1;
  return this->sections[section].parent;
}


///////////////////////////////////////////////////////////////////////////
// Get a section (returns the section type value)
const char *ConfigFile::GetSectionType(int section)
{
  if (section < 0 || section >= this->section_count)
    return NULL;
  return this->sections[section].type;
}

///////////////////////////////////////////////////////////////////////////
// Lookup a section number by type name
// Returns -1 if there is no section with this type
int ConfigFile::LookupSection(const char *type)
{
  for (int section = 0; section < GetSectionCount(); section++)
  {
    if (strcmp(GetSectionType(section), type) == 0)
      return section;
  }
  return -1;
}


///////////////////////////////////////////////////////////////////////////
// Dump the section list for debugging
void ConfigFile::DumpSections()
{
  printf("\n## begin sections\n");
  for (int i = 0; i < this->section_count; i++)
  {
    Section *section = this->sections + i;

    printf("## [%d][%d]", i, section->parent);
    printf("[%s]\n", section->type);
  }
  printf("## end sections\n");
}


///////////////////////////////////////////////////////////////////////////
// Clear the field list
void ConfigFile::ClearFields()
{
  int i;
  Field *field;

  for (i = 0; i < this->field_count; i++)
  {
    field = this->fields + i;
    free(field->values);
    free(field->useds);
    field->values = NULL;
    field->useds = NULL;
  }
  free(this->fields);
  this->fields = NULL;
  this->field_size = 0;
  this->field_count = 0;
}


///////////////////////////////////////////////////////////////////////////
// Add an field
int ConfigFile::AddField(int section, const char *name, int line)
{
  int i;
  Field *field;
  
  // See if this field already exists; if it does, we dont need to
  // add it again.
  for (i = 0; i < this->field_count; i++)
  {
    field = this->fields + i;
    if (field->section != section)
      continue;
    if (strcmp(field->name, name) == 0)
      return i;
  }

  // Expand field array if necessary.
  if (i >= this->field_size)
  {
    this->field_size += 100;
    this->fields = (Field*)
      realloc(this->fields, this->field_size * sizeof(this->fields[0]));
  }

  field = this->fields + i;
  memset(field, 0, sizeof(Field));
  field->section = section;
  field->name = name;
  field->value_count = 0;
  field->values = NULL;
  field->useds = NULL;
  field->line = line;

  this->field_count++;

  return i;
}


///////////////////////////////////////////////////////////////////////////
// Add a field value
void ConfigFile::AddFieldValue(int field, int index, int value_token)
{
  assert(field >= 0);
  Field *pfield = this->fields + field;

  // Expand the array if it's too small
  if (index >= pfield->value_count)
  {
    pfield->value_count = index + 1;
    pfield->values = (int*) realloc(pfield->values, pfield->value_count * sizeof(int));
    pfield->useds = (bool*) realloc(pfield->useds, pfield->value_count * sizeof(bool));
    pfield->useds[pfield->value_count-1] = false;
  }

  // Set the relevant value
  pfield->values[index] = value_token;
  pfield->useds[index] = false;
}


///////////////////////////////////////////////////////////////////////////
// Get a field 
int ConfigFile::GetField(int section, const char *name)
{
  // Find first instance of field
  for (int i = 0; i < this->field_count; i++)
  {
    Field *field = this->fields + i;
    if (field->section != section)
      continue;
    if (strcmp(field->name, name) == 0)
      return i;
  }
  return -1;
}


///////////////////////////////////////////////////////////////////////////
// Get the number of elements for this field
int ConfigFile::GetFieldValueCount(int field)
{
  Field *pfield = this->fields + field;
  return pfield->value_count;
}


///////////////////////////////////////////////////////////////////////////
// Set the value of an field
void ConfigFile::SetFieldValue(int field, int index, const char *value)
{
  //assert(field >= 0 && field < this->field_count);
  Field *pfield = this->fields + field;
  assert(index >= 0 && index < pfield->value_count);

  // Set the relevant value
  SetTokenValue(pfield->values[index], value);
}


///////////////////////////////////////////////////////////////////////////
// Get the value of an field 
const char *ConfigFile::GetFieldValue(int field, int index, bool flag_used)
{
  assert(field >= 0);
  Field *pfield = this->fields + field;
  
  if(index >= pfield->value_count)
    return NULL;

  if (flag_used)
    pfield->useds[index] = true;
  
  return GetTokenValue(pfield->values[index]);
}


///////////////////////////////////////////////////////////////////////////
// Dump the field list for debugging
void ConfigFile::DumpFields()
{
  printf("\n## begin fields\n");
  for (int i = 0; i < this->field_count; i++)
  {
    Field *field = this->fields + i;
    Section *section = this->sections + field->section;
    
    printf("## [%d]", field->section);
    printf("[%s]", section->type);
    printf("[%s]", field->name);
    for (int j = 0; j < field->value_count; j++)
      printf("[%s]", GetTokenValue(field->values[j]));
    printf("\n");
  }
  printf("## end fields\n");
}


///////////////////////////////////////////////////////////////////////////
// Read a string
const char *ConfigFile::ReadString(int section, const char *name, const char *value)
{
  int field = GetField(section, name);
  if (field < 0)
    return value;
  return GetFieldValue(field, 0);
}


///////////////////////////////////////////////////////////////////////////
// Write a string
void ConfigFile::WriteString(int section, const char *name, const char *value)
{
  int field = GetField(section, name);
  if (field < 0)
    return;
  SetFieldValue(field, 0, value);  
}


///////////////////////////////////////////////////////////////////////////
// Read an int
int ConfigFile::ReadInt(int section, const char *name, int value)
{
  int field = GetField(section, name);
  if (field < 0)
    return value;
  return atoi(GetFieldValue(field, 0));
}


///////////////////////////////////////////////////////////////////////////
// Write an int
void ConfigFile::WriteInt(int section, const char *name, int value)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%d", value);
  WriteString(section, name, default_str);
}

///////////////////////////////////////////////////////////////////////////
// Read a boolean
bool ConfigFile::ReadBool(int section, const char* name, bool value)
{
  int field = GetField(section, name);
  if(field < 0) return value;
  const char* s = GetFieldValue(field, 0);
  if( strcmp(s, "yes") == 0 || strcmp(s, "true") == 0 || strcmp(s, "1") == 0)
    return true;
  else if( strcmp(s, "no") == 0 || strcmp(s, "false") == 0 || strcmp(s, "0") == 0)
    return false;
  else
  {
    printf("player: Warning: error in config file section %d field %s: Invalid boolean value.", section, name);
    return value;
  }
}

///////////////////////////////////////////////////////////////////////////
// Write a boolean
void ConfigFile::WriteBool(int section, const char *name, bool value)
{
  char str[4];
  snprintf(str, sizeof(str), "%s", value ? "yes" : "no");
  WriteString(section, name, str);
}

///////////////////////////////////////////////////////////////////////////
// Write a boolean compatibly
void ConfigFile::WriteBool_Compat(int section, const char *name, bool value)
{
  char str[2];
  snprintf(str, sizeof(str), "%s", value ? "1" : "0");
  WriteString(section, name, str);
}

///////////////////////////////////////////////////////////////////////////
// Read a float
double ConfigFile::ReadFloat(int section, const char *name, double value)
{
  int field = GetField(section, name);
  if (field < 0)
    return value;
  return atof(GetFieldValue(field, 0));
}


///////////////////////////////////////////////////////////////////////////
// Read a length (includes unit conversion)
double ConfigFile::ReadLength(int section, const char *name, double value)
{
  int field = GetField(section, name);
  if (field < 0)
    return value;
  return atof(GetFieldValue(field, 0)) * this->unit_length;
}


///////////////////////////////////////////////////////////////////////////
// Write a length (includes units conversion)
void ConfigFile::WriteLength(int section, const char *name, double value)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%.3f", value / this->unit_length);
  WriteString(section, name, default_str);
}


///////////////////////////////////////////////////////////////////////////
// Read an angle (includes unit conversion)
double ConfigFile::ReadAngle(int section, const char *name, double value)
{
  int field = GetField(section, name);
  if (field < 0)
    return value;
  return atof(GetFieldValue(field, 0)) * this->unit_angle;
}


///////////////////////////////////////////////////////////////////////////
// Read a color (included text -> RGB conversion).
// We look up the color in one of the common color databases.
uint32_t ConfigFile::ReadColor(int section, const char *name, uint32_t value)
{
  int field;
  const char *color;
  
  field = GetField(section, name);
  if (field < 0)
    return value;
  color = GetFieldValue(field, 0);

  return LookupColor(color);
}


///////////////////////////////////////////////////////////////////////////
// Read a file name.
// Always returns an absolute path.
// If the filename is entered as a relative path, we prepend
// the world files path to it.
const char *ConfigFile::ReadFilename(int section, const char *name, const char *value)
{
  int field = GetField(section, name);
  if (field < 0)
    return value;

  const char *filename = GetFieldValue(field, 0);
  
  if( filename[0] == '/' || filename[0] == '~' )
    return filename;

  else if (this->filename[0] == '/' || this->filename[0] == '~')
  {
    // Note that dirname() modifies the contents, so
    // we need to make a copy of the filename.
    // There's no bounds-checking, but what the heck.
    char *tmp = strdup(this->filename);
    char *fullpath = (char*) malloc(MAX_FILENAME_SIZE);
    memset(fullpath, 0, MAX_FILENAME_SIZE);
    strcat( fullpath, dirname(tmp));
    strcat( fullpath, "/" ); 
    strcat( fullpath, filename );
    assert(strlen(fullpath) + 1 < MAX_FILENAME_SIZE);

    SetFieldValue(field, 0, fullpath);
    
    free(fullpath);
    free(tmp);
  }
  else
  {
    // Prepend the path
    // Note that dirname() modifies the contents, so
    // we need to make a copy of the filename.
    // There's no bounds-checking, but what the heck.
    char *tmp = strdup(this->filename);
    char *fullpath = (char*) malloc(MAX_FILENAME_SIZE);
    getcwd(fullpath, MAX_FILENAME_SIZE);
    strcat( fullpath, "/" ); 
    strcat( fullpath, dirname(tmp));
    strcat( fullpath, "/" ); 
    strcat( fullpath, filename );
    assert(strlen(fullpath) + 1 < MAX_FILENAME_SIZE);

    SetFieldValue(field, 0, fullpath);
    
    free(fullpath);
    free(tmp);
  }

  filename = GetFieldValue(field, 0);
  assert(filename[0] == '/' || filename[0] == '~');

  return filename;
}


///////////////////////////////////////////////////////////////////////////
// Get the number of values in a tuple
int ConfigFile::GetTupleCount(int section, const char *name)
{
  int field = GetField(section, name);
  if (field < 0)
    return 0;
  return GetFieldValueCount(field);
}


///////////////////////////////////////////////////////////////////////////
// Read a string from a tuple
const char *ConfigFile::ReadTupleString(int section, const char *name,
                                        int index, const char *value)
{
  int field = GetField(section, name);
  if (field < 0)
    return value;
  const char* svalue = GetFieldValue(field, index);
  if(!svalue)
    return value;
  return svalue;
}


///////////////////////////////////////////////////////////////////////////
// Write a string to a tuple
void ConfigFile::WriteTupleString(int section, const char *name,
                                  int index, const char *value)
{
  int field = GetField(section, name);
  /* TODO
  if (field < 0)
    field = InsertField(section, name);
  */
  SetFieldValue(field, index, value);  
}


///////////////////////////////////////////////////////////////////////////
// Read a int from a tuple
int ConfigFile::ReadTupleInt(int section, const char *name,
                             int index, int value)
{
  int field = GetField(section, name);
  if (field < 0)
    return value;
  const char* svalue = GetFieldValue(field, index);
  if(!svalue)
    return value;
  return atoi(svalue);
}


///////////////////////////////////////////////////////////////////////////
// Write a int to a tuple
void ConfigFile::WriteTupleInt(int section, const char *name,
                                 int index, int value)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%d", value);
  WriteTupleString(section, name, index, default_str);
}


///////////////////////////////////////////////////////////////////////////
// Read a float from a tuple
double ConfigFile::ReadTupleFloat(int section, const char *name,
                                  int index, double value)
{
  int field = GetField(section, name);
  if (field < 0)
    return value;
  const char* svalue = GetFieldValue(field, index);
  if(!svalue)
    return value;
  return atof(svalue);
}


///////////////////////////////////////////////////////////////////////////
// Write a float to a tuple
void ConfigFile::WriteTupleFloat(int section, const char *name,
                                 int index, double value)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%.3f", value);
  WriteTupleString(section, name, index, default_str);
}


///////////////////////////////////////////////////////////////////////////
// Read a length from a tuple (includes unit conversion)
double ConfigFile::ReadTupleLength(int section, const char *name,
                                   int index, double value)
{
  int field = GetField(section, name);
  if (field < 0)
    return value;
  const char* svalue = GetFieldValue(field, index);
  if(!svalue)
    return value;
  return atof(svalue) * this->unit_length;
}


///////////////////////////////////////////////////////////////////////////
// Write a length to a tuple (includes unit conversion)
void ConfigFile::WriteTupleLength(int section, const char *name,
                                 int index, double value)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%.3f", value / this->unit_length);
  WriteTupleString(section, name, index, default_str);
}


///////////////////////////////////////////////////////////////////////////
// Read an angle from a tuple (includes unit conversion)
double ConfigFile::ReadTupleAngle(int section, const char *name,
                                  int index, double value)
{
  int field = GetField(section, name);
  if (field < 0)
    return value;
  const char* svalue = GetFieldValue(field, index);
  if(!svalue)
    return value;
  return atof(svalue) * this->unit_angle;
}


///////////////////////////////////////////////////////////////////////////
// Write an angle to a tuple (includes unit conversion)
void ConfigFile::WriteTupleAngle(int section, const char *name,
                                 int index, double value)
{
  char default_str[64];
  snprintf(default_str, sizeof(default_str), "%.3f", value / this->unit_angle);
  WriteTupleString(section, name, index, default_str);
}



///////////////////////////////////////////////////////////////////////////
// Read a color (included text -> RGB conversion).
// We look up the color in one of the common color databases.
uint32_t ConfigFile::ReadTupleColor(int section, const char *name, int index, uint32_t value)
{
  int field;
  const char *color;

  field = GetField(section, name);
  if (field < 0)
    return value;
  color = GetFieldValue(field, index);
  if (!color)
    return value;

  return LookupColor(color);
}


///////////////////////////////////////////////////////////////////////////
// Look up the color in a data based (transform color name -> color value).
uint32_t ConfigFile::LookupColor(const char *name)
{
  FILE *file = NULL;

  for (int i=0; COLOR_DATABASE[i] != NULL; ++i)
  {
    file = fopen(COLOR_DATABASE[i], "r");
    if (file)
      break;
  }
  if (!file)
  {
    PLAYER_ERROR("unable to open color database: tried the following files");
    for (int i=0; COLOR_DATABASE[i] != NULL; ++i)
    {
      PLAYER_ERROR1("\t: %s", COLOR_DATABASE[i]);
    }
    fclose(file);
    return 0xFFFFFF;
  }
  
  
  while (true)
  {
    char line[1024];
    if (!fgets(line, sizeof(line), file))
      break;

    // it's a macro or comment line - ignore the line
    if (line[0] == '!' || line[0] == '#' || line[0] == '%') 
      continue;

    // Trim the trailing space
    while (strchr(" \t\n", line[strlen(line)-1]))
      line[strlen(line)-1] = 0;

    // Read the color
    int r, g, b;
    int chars_matched = 0;
    sscanf( line, "%d %d %d %n", &r, &g, &b, &chars_matched );
      
    // Read the name
    char* nname = line + chars_matched;

    // If the name matches
    if (strcmp(nname, name) == 0)
    {
      fclose(file);
      return ((r << 16) | (g << 8) | b);
    }
  }
  PLAYER_WARN1("unable to find color [%s]; using default (red)", name);
  fclose(file);
  return 0xFF0000;
}


///////////////////////////////////////////////////////////////////////////
// Read a device id.
int ConfigFile::ReadDeviceAddr(player_devaddr_t *addr, int section,
                               const char *name, int interf_code, int index,
                               const char *key)
{
  int prop;
  int i, j, count;
  char str[128];
  char *tokens[5];
  int token_count;
  unsigned int robot, host;
  int ind;
  const char *s, *k;
  player_interface_t interf;

  // Get the field index
  if ((prop = GetField(section, name)) < 0)
  {
    CONFIG_ERR1("missing field [%s]", 0, name);
    return -1;
  }

  // Find the number of values in the field
  count = GetFieldValueCount(prop);
  
  // Consider all the values, looking for a match
  for (i = 0; i < count; i++)
  {
    assert(sizeof(str) > strlen(GetFieldValue(prop, i, false)));
    strcpy(str, GetFieldValue(prop, i, false));

    memset(tokens, 0, sizeof(tokens));
    token_count = 5;

    // Split the string inplace using ':' as the delimiter.
    // Note that we do this backwards (leading fields are optional).
    // The expected syntax is key:host:robot:interface:index.
    for (j = strlen(str) - 1; j >= 0 && token_count > 0; j--)
    {
      if (str[j] == ':')
      {
        tokens[--token_count] = str + j + 1;
        str[j] = 0;
      }
    }
    if (token_count > 0)
      tokens[--token_count] = str;

    // We require at least an interface:index pair
    if(!(tokens[3] && tokens[4]))
    {
      CONFIG_ERR1("missing interface or index in field [%s]", this->fields[prop].line, name);
      return -1;
    }

    // Extract the fields from the tokens (with default values)
    k = tokens[0];
    if(tokens[1] && strlen(tokens[1]))
    {
      // Try to be smart about reading the host part of the address.
      for(j=0;j<(int)strlen(tokens[1]);j++)
      {
        if(!isdigit(tokens[1][j]))
          break;
      }
      // Are all the characters digits?
      if(j == (int)strlen(tokens[1]))
      {
        // Yes; assume it's a 32-bit packed address
        host = atoi(tokens[1]);
      }
      else
      {
        // No; assume it's a string containing a hostname or IP address
        if(hostname_to_packedaddr((uint32_t *) &host, tokens[1]) < 0)
        {
          PLAYER_ERROR1("name lookup failed for host \"%s\"", tokens[1]);
          return -1;
        }
      }
    }
    else
    {
      host = this->default_host;
    }
    if(tokens[2] && strlen(tokens[2]))
      robot = atoi(tokens[2]);
    else
      robot = this->default_robot;
    s = tokens[3];
    ind = atoi(tokens[4]);

    // Find the interface
    if (::lookup_interface(s, &interf) != 0)
    {
      CONFIG_ERR1("unknown interface: [%s]", this->fields[prop].line, s);
      return -1;
    }

    // Match the interface
    if (interf_code > 0 && interf.interf != interf_code)
      continue;

    // Match the tuple index (< 0 matches all indices)
    if (index >= 0 && i != index)
      continue;

    // If we are expecting a key, but there is non in the file, this
    // is no match.
    if (key && k == NULL)
      continue;
    
    // If the key is expected and present in the file, it must match
    if (key && k && strcmp(key, k) != 0)
      continue;

    // Read the field again, just to mark it as read
    GetFieldValue(prop, i, true);

    addr->host = host;
    addr->robot = robot;
    addr->interf = interf.interf;
    addr->index = ind;
    return 0;
 
  }

  return -1;
}

// Parse all driver blocks
bool 
ConfigFile::ParseAllDrivers()
{
  for(int i = 1; i < this->GetSectionCount(); i++)
  {
    // Check for new-style device block
    if(strcmp(this->GetSectionType(i), "driver") == 0)
    {
      if(!this->ParseDriver(i))
        return false;
    }
  }
  return(true);
}

// Parse a driver block, and update the deviceTable accordingly
bool 
ConfigFile::ParseDriver(int section)
{
  const char *pluginname;  
  const char *drivername;
  DriverEntry *entry;
  Driver *driver;
  Device *device;
  int count;
  lt_dlhandle handle;

  entry = NULL;
  driver = NULL;
  
  // Load any required plugins
  pluginname = this->ReadString(section, "plugin", NULL);
  if (pluginname != NULL)
  {
    if((handle = LoadPlugin(pluginname,this->filename)) == NULL)
    {
      PLAYER_ERROR1("failed to load plugin: %s", pluginname);
      return (false);
    }
    if(!InitDriverPlugin(handle))
    {
      PLAYER_ERROR1("failed to initialise plugin: %s", pluginname);
      return (false);
    }
  }

  // Get the driver name
  drivername = this->ReadString(section, "name", NULL);
  if (drivername == NULL)
  {
    PLAYER_ERROR1("No driver name specified in section %d", section);
    return (false);
  }
  
  // Look for the driver
  entry = driverTable->GetDriverEntry(drivername);
  if (entry == NULL)
  {
    PLAYER_ERROR1("Couldn't find driver \"%s\"", drivername);
    return (false);
  }

  // Create a new-style driver
  if (entry->initfunc == NULL)
  {
    PLAYER_ERROR1("Driver has no initialization function \"%s\"", drivername);
    return (false);
  }

  // Create a driver; the driver will add entries into the device
  // table
  driver = (*(entry->initfunc)) (this, section);
  if (driver == NULL || driver->GetError() != 0)
  {
    PLAYER_ERROR1("Initialization failed for driver \"%s\"", drivername);
    return (false);
  }

  // Fill out the driver name in the device table and count the number
  // of devices for this driver
  count = 0;
  for (device = deviceTable->GetFirstDevice(); device != NULL;
       device = deviceTable->GetNextDevice(device))
  {
    if (device->driver == driver)
    {
      strncpy(device->drivername, drivername, sizeof(device->drivername));
      count++;
    }
  }

  // We must have at least one interface per driver
  if (count == 0)
  {
    PLAYER_ERROR1("Driver has no (usable) interfaces \"%s\"", drivername);
    return false;
  }
  
  // Should this device be "always on"?  
  if (driver)
    driver->alwayson = this->ReadInt(section, "alwayson", driver->alwayson);

  return true;
}

// Parse all interface blocks
bool
ConfigFile::ParseAllInterfaces()
{
  for(int i = 1; i < this->GetSectionCount(); i++)
  {
    // Check for new-style device block
    if(strcmp(this->GetSectionType(i), "interface") == 0)
    {
      if(!this->ParseInterface(i))
        return false;
    }
  }
  return true;
}

// Parse an interface block, and update the interface systems accordingly
bool
ConfigFile::ParseInterface(int section)
{
  const char *pluginname;
  const char *interfacename;
  int interfacecode;
  int replace;
  lt_dlhandle handle;
  playerxdr_function_t *flist;

  // Check if this interface is replacing an existing one's messages
  replace = this->ReadInt(section, "replace", 0);

  // Get the interface name
  interfacename = this->ReadString(section, "name", NULL);
  if (interfacename == NULL)
  {
    PLAYER_ERROR1("No interface name specified in section %d", section);
    return false;
  }

  // Get the interface code
  interfacecode = this->ReadInt(section, "code", -1);
  if (interfacecode == -1)
  {
    PLAYER_ERROR1("No interface code specified in section %d", section);
    return false;
  }

  // Find the plugin name
  pluginname = this->ReadString(section, "plugin", NULL);
  if (pluginname == NULL)
  {
    PLAYER_ERROR1("No plugin name specified for plugin interface in section %d", section);
    return false;
  }

  // Load the plugin
  if((handle = LoadPlugin(pluginname,this->filename)) == NULL)
  {
    PLAYER_ERROR1("failed to load plugin: %s", pluginname);
    return false;
  }

  // Get the function table list
  if((flist = InitInterfacePlugin(handle)) == NULL)
  {
    PLAYER_ERROR1("failed to initialise plugin: %s", pluginname);
    return false;
  }

  // Add the function table list
  if(playerxdr_ftable_add_multi(flist, replace) < 0)
  {
    PLAYER_ERROR1("Failed to add interface functions for plugin interface %s", interfacename);
    return false;
  }

  // Add the interface name to the interface names table
  if(itable_add (interfacename, interfacecode, replace) < 0)
  {
    PLAYER_ERROR2("Failed to add interface name/code: %s/%d", interfacename, interfacecode);
    return false;
  }

  return true;
}
