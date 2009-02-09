/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000  Brian Gerkey   &  Kasper Stoy
 *                      gerkey@usc.edu    kaspers@robotics.usc.edu
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

#if HAVE_CONFIG_H
  #include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// Shouldn't really need this; <libplayercore/playercommon.h> includes
// it...
#include <limits.h> // for PATH_MAX

#include <libplayercore/drivertable.h>
#include <libplayercore/error.h>
#include <libplayercore/globals.h>
#include <libplayercore/playercommon.h>

#include <replace/replace.h>

#include "plugins.h"

// Try to load a given plugin, using a particular search algorithm.
// Returns true on success and false on failure.
lt_dlhandle
LoadPlugin(const char* pluginname, const char* cfgfile)
{
#if HAVE_LIBLTDL
  static int init_done = 0;

  if( !init_done )
  {
    int errors = 0;
    if((errors = lt_dlinit()))
    {
      PLAYER_ERROR2( "Error(s) initializing dynamic loader (%d, %s)",
                     errors, lt_dlerror() );
      return NULL;
    }
    else
      init_done = 1;
  }

  lt_dlhandle handle=NULL;
  char fullpath[PATH_MAX] = {0};
  char* playerpath;
  char* tmp;
  char* cfgdir;
  unsigned int i,j;

  // see if we got an absolute path
  if(pluginname[0] == '/' || pluginname[0] == '~')
  {
    strcpy(fullpath,pluginname);
    PLAYER_MSG1(1, "trying to load %s...", fullpath);
    fflush(stdout);
    if((handle = lt_dlopenext(fullpath)))
      PLAYER_MSG0(1, "success");
    else
    {
      PLAYER_MSG1(2, "failed (%s)\n", lt_dlerror() );
      return NULL;
    }
  }

  // we got a relative path, so search for the module

  // did the user set PLAYERPATH?
  if(!handle && (playerpath = getenv("PLAYERPATH")))
  {
    PLAYER_MSG1(1,"PLAYERPATH: %s\n", playerpath);

    // yep, now parse it, as a colon-separated list of directories
    i=0;
    while(i<strlen(playerpath))
    {
      j=i;
      while(j<strlen(playerpath))
      {
        if(playerpath[j] == ':')
        {
          break;
        }
        j++;
      }
      memset(fullpath,0,PATH_MAX);
      strncpy(fullpath,playerpath+i,j-i);
      strcat(fullpath,"/");
      strcat(fullpath,pluginname);

      PLAYER_MSG1(1, "trying to load %s...", fullpath);

      if((handle = lt_dlopenext(fullpath)))
      {
        PLAYER_MSG0(1, "success");
        break;
      }
      else
      {
        PLAYER_MSG2(2, "failed to load %s (error %s)\n", fullpath,  lt_dlerror());
      }
      i=j+1;
    }
  }

  // try to load it from the directory where the config file is
  if(!handle && cfgfile)
  {
    // Note that dirname() modifies the contents, so
    // we need to make a copy of the filename.
    tmp = strdup(cfgfile);
    assert(tmp);
    memset(fullpath,0,PATH_MAX);
    cfgdir = dirname(tmp);
    if(cfgdir[0] != '/' && cfgdir[0] != '~')
    {
      getcwd(fullpath, PATH_MAX);
      strncat(fullpath,"/", PATH_MAX);
    }
    strncat(fullpath,cfgdir, PATH_MAX);
    strncat(fullpath,"/", PATH_MAX);
    strncat(fullpath,pluginname, PATH_MAX);
    PLAYER_MSG1(1, "trying to load %s...", fullpath);
    if((handle = lt_dlopenext(fullpath)))//, RTLD_NOW)))
      PLAYER_MSG0(1, "success");
    else
      PLAYER_MSG1(1, "failed (%s)\n", lt_dlerror());
    free(tmp); // this should ha
  }

  // try to load it from prefix/lib
  if(!handle)
  {
    memset(fullpath,0,PATH_MAX);
    strncpy(fullpath,PLAYER_INSTALL_PREFIX, PATH_MAX);
    strncat(fullpath,"/lib/", PATH_MAX);
    strncat(fullpath,pluginname, PATH_MAX);
    PLAYER_MSG1(1, "trying to load %s...", fullpath);
    if((handle = lt_dlopenext(fullpath)))
      PLAYER_MSG0(1, "success");
    else
      PLAYER_MSG1(1, "failed (%s)\n", lt_dlerror() );
  }

  // just pass the libname to lt_dlopenext, to see if it can handle it
  // (this may work when the plugin is installed in a default system
  // location).
  if(!handle)
  {
    PLAYER_MSG1(1, "trying to load %s...", pluginname);
    if((handle = lt_dlopenext(pluginname)))
      PLAYER_MSG0(1, "success");
    else
      PLAYER_MSG1(1, "failed (%s)\n", lt_dlerror());
  }

  if (handle == NULL)
  {
    PLAYER_ERROR1("error loading plugin: %s", pluginname);
    return NULL;
  }

  return handle;

#else
  PLAYER_ERROR("Sorry, no support for shared libraries, so can't load plugins.");
  PLAYER_ERROR("You should install libltdl, which is part of GNU libtool, then re-compile player.");
  return 0;
#endif
}

// Initialise a driver plugin
bool InitDriverPlugin(lt_dlhandle handle)
{
#if HAVE_LIBLTDL
  DriverPluginInitFn initfunc;
  // Invoke the initialization function
  if(handle)
  {
    PLAYER_MSG0(1, "invoking player_driver_init()...");

    initfunc = (DriverPluginInitFn)lt_dlsym(handle,"player_driver_init");
    if( !initfunc )
    {
      PLAYER_ERROR1("failed to resolve player_driver_init: %s\n", lt_dlerror());
      return(false);
    }

    int initfunc_result = 0;
    if( (initfunc_result = (*initfunc)(driverTable)) != 0)
    {
      PLAYER_ERROR1("error returned by player_driver_init: %d", initfunc_result);
      return(false);
    }

    PLAYER_MSG0(1, "success");

    return(true);
  }
  else
    return(false);
#else
  PLAYER_ERROR("Sorry, no support for shared libraries, so can't load plugins.");
  PLAYER_ERROR("You should install libltdl, which is part of GNU libtool, then re-compile player.");
  return(false);
#endif
}

// Initialise an interface plugin
playerxdr_function_t* InitInterfacePlugin(lt_dlhandle handle)
{
#if HAVE_LIBLTDL
  InterfPluginInitFn initfunc;
  playerxdr_function_t *flist;

  // Invoke the initialization function
  if(handle)
  {
    PLAYER_MSG0(1, "invoking player_plugininterf_gettable()...");

    initfunc = (InterfPluginInitFn)lt_dlsym(handle,"player_plugininterf_gettable");
    if( !initfunc )
    {
      PLAYER_ERROR1("failed to resolve player_plugininterf_gettable: %s\n", lt_dlerror());
      return(NULL);
    }

    if( (flist = (*initfunc)()) == NULL)
    {
      PLAYER_ERROR("player_plugininterf_gettable returned NULL");
      return(NULL);
    }

    PLAYER_MSG0(1, "success");

    return(flist);
  }
  else
    return(NULL);
#else
  PLAYER_ERROR("Sorry, no support for shared libraries, so can't load plugins.");
  PLAYER_ERROR("You should install libltdl, which is part of GNU libtool, then re-compile player.");
  return(NULL);
#endif
}
