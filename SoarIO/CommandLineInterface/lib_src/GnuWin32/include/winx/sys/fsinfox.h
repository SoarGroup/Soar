/* Constants from kernel header for various FSes.
   Copyright (C) 1998, 1999, 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _WINX_SYS_FSINFO_H_
#define _WINX_SYS_FSINFO_H_   1

/* These definitions come from the kernel headers.  But we cannot
   include the headers here because of type clashes.  If new
   filesystem types will become available we have to add the
   appropriate definitions here.*/


#define EXT_SUPER_MAGIC       0x137D

#define EXT2_OLD_SUPER_MAGIC  0xEF51

#define _XIAFS_SUPER_MAGIC    0x012FD16D


/* Constants that identify the `MS-Windows' filesystem.  */                
                                                                                   
#define CDFS_SUPER_MAGIC        0x4000
#define CDRFS_SUPER_MAGIC            0x4004
#define FAT_SUPER_MAGIC         0x4006
#define FAT32_SUPER_MAGIC            0x4008

#endif    /* _WINX_SYS_FSINFO_H_ */
