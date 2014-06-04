#
# pushd.tcl --
#
# C-shell style directory stack procs.
#
#------------------------------------------------------------------------------
# Copyright 1992-1994 Karl Lehenbauer and Mark Diekhans.
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appear in all copies.  Karl Lehenbauer and
# Mark Diekhans make no representations about the suitability of this
# software for any purpose.  It is provided "as is" without express or
# implied warranty.
#------------------------------------------------------------------------------
# $Id: pushd.tcl 2 2004-03-24 19:15:00Z wessling $
#------------------------------------------------------------------------------
#
# Modified by KBS, 2 Feb. 95, to use standard Tcl list ops. instead
# of TclX lvarpop command.  Also changed "dirs" to return list rather
# than printing it.
#
# Modified by KBS, 8 Mar. 95, to store cwd explicitly in dir stack
# as opposed in implicitly via pwd.  This works better in a multi-
# interpreter environment where different interpreters are doing
# cd's.
#
# Renamed from pushd to dir-stack to be more general since this file
# contains more than just the pushd function.

global DIR_STACK

if { ![info exists DIR_STACK] } {
  set DIR_STACK "\"[pwd]\""

  rename cd tcl-cd
  proc cd {{dir $env(HOME)}} {
    global env DIR_STACK
    if [catch "tcl-cd \"$dir\"" msg] {
      return $msg
    } else {
      set fulldir [pwd]
      set DIR_STACK \
        [linsert [lrange $DIR_STACK 1 end] \
        0 "$fulldir"]
      return $fulldir
    }
  }

  proc pushd {dir} {
    global DIR_STACK

    if [catch "tcl-cd \"$dir\"" msg] {
      return $msg
    } else {
      set dir [pwd]
      set DIR_STACK [linsert $DIR_STACK 0 "$dir"]
      return $dir
    }
  }

  proc popd {} {
    global DIR_STACK

    if {[llength $DIR_STACK] > 1} {
      set prev_dir [lindex $DIR_STACK 1]
      set DIR_STACK [lrange $DIR_STACK 1 end]
      if [catch "tcl-cd \"$prev_dir\"" msg] {
        return $msg
      } else {
        return $prev_dir
      }
    } else {
      error "directory stack cannot be empty"
    }
  }

  proc topd {} {
    global DIR_STACK
    return [lindex $DIR_STACK 0]
  }

  proc dirs {} {
    global DIR_STACK
    return $DIR_STACK
  }
}
