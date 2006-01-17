###
# Copyright 1995-2004 Soar Technology, Inc., University of Michigan. All 
# rights reserved.
# 
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions are 
# met:
# 
#    1.	Redistributions of source code must retain the above copyright 
#       notice, this list of conditions and the following disclaimer. 
# 
#    2.	Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in 
#       the documentation and/or other materials provided with the 
#       distribution. 
# 
# THIS SOFTWARE IS PROVIDED BY THE SOAR CONSORTIUM ``AS IS'' AND 
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE SOAR 
# CONSORTIUM  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
# DAMAGE.
# 
# The views and conclusions contained in the software and documentation 
# are those of the authors and should not be interpreted as representing 
# official policies, either expressed or implied, of Soar Technology, Inc., 
# the University of Michigan, or the Soar consortium.
### 

##
# A simple set of logging functions. Sends output to stdout as well as a 
# file in the current directory (created at the time of the 
# first Log call)

set LogAutoFlush 0 ;# if true, then every log write is flushed to disk
set LogFileName "" ;# name of file to write to

proc LogSetFileName { fileName } {
   global LogFileName
   LogReset
   set LogFileName $fileName
}

proc doLog { prefix s log } {
   global LogAutoFlush
   set s "$prefix $s"
   puts $s
   if { $log != -1 } {
      puts $log $s
      if { $LogAutoFlush } {
         flush $log
      }
   }
}

##
# Log an error
proc LogError { s } {
   doLog "*** ERROR: " $s [getLogFileId]
}

##
# Log a warning
proc LogWarning { s } {
   doLog "--- WARNING: " $s [getLogFileId]
}

##
# Log info
proc LogInfo { s } {
   doLog "--- INFO: " $s [getLogFileId]
}

##
# Write an arbitrary string to the log
proc Log { s { newLine 1 } } {
   set log [getLogFileId]

   if { $newLine } {
      puts $s
      if { $log != -1 } {
         puts $log $s
      }
   } else {
      puts -nonewline $s
      if { $log != -1 } {
         puts -nonewline $log $s
      }
   }
   global LogAutoFlush
   if { $log != -1 && $LogAutoFlush } {
      flush $log
   }
}
##
# Flush the log to disk
proc LogFlush { } {
   global LogFileId
   if [info exists LogFileId] {
      flush $LogFileId
   }
}

##
# Reset the log file.
proc LogReset { } {
   global LogFileId

   if [info exists LogFileId] {
      close $LogFileId
      unset LogFileId
   }
}
##
# Set the value of the auto-flush parameter
proc LogSetAutoFlush { b } {
   global LogAutoFlush

   set LogAutoFlush $b
}

##
# internal function to open log file.
proc getLogFileId {} {
   global LogFileId LogFileName

   if ![info exists LogFileId] {
      if { $LogFileName == "" } {
         return -1
      }
      set LogFileId [open $LogFileName w]
      puts $LogFileId ""
      puts $LogFileId "#################### LOG OPENED ##########################"
   }
   return $LogFileId
}
