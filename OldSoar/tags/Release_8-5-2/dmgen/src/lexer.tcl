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
# Soar lexer.
#
# Proc exports:
#  get_lexeme
#
# Globals:
#  lexeme_value
#  lexeme_type


proc initialize_file_scan {fd} {
   global lexeme_value lexeme_type current_char file_buffer
   set lexeme_value {}
   set lexeme_type {}
   set current_char { }
   # global file_buffer must be set by caller before any calls
   # to this module!
}

proc get_lexeme {} {
   global lexeme_value lexeme_type current_char whitespace lexer_routine
   global constituent_chars
   set lexeme_value {}
   set lexeme_type {}
   while 1 {
      if ![string compare $current_char {EOF}] break;
      if [regexp \[$whitespace\] $current_char] {
         get_next_char
         continue
      }
      if ![string compare $current_char {;}] {
         get_next_char
         continue
      }
      if ![string compare $current_char {#}] {
         while {[string compare $current_char "\n"] && \
                [string compare $current_char {EOF}]} {
            get_next_char
         }
         if [string compare $current_char {EOF}] {
            get_next_char
         }
         continue
      }
      ## no whitespace found
      break
   }
   if ![string compare $current_char {EOF}] {
      lex_eof
   } elseif [info exists lexer_routine("$current_char")] {
      eval $lexer_routine("$current_char")
   } elseif [regexp {[0-9]} $current_char] {
      lex_digit
   } elseif [regexp \[$constituent_chars\] $current_char] {
      lex_constituent_string
   } else {
      lex_unknown
   }
   if 0 {
   if ![string compare $current_char {EOF}] {
      lex_eof
   } elseif [info exists lexer_routine("$current_char")] {
      eval $lexer_routine("$current_char")
   } elseif [regexp \[$constituent_chars\] $current_char] {
      lex_constituent_string
   } elseif [regexp {[0-9]} $current_char] {
      lex_digit
   } else {
      lex_unknown
   }
   }
}

proc get_next_char {} {
   global current_char file_buffer
   if {$file_buffer == ""} {
      set current_char "EOF"
   } else {
      set current_char [string index $file_buffer 0]
      set file_buffer [string range $file_buffer 1 end]

      global parsing_function parsed_function
      if { $parsing_function > 0 } {
         set parsed_function "$parsed_function$current_char"
      }
   }
}

proc lex_eof {} {
   global lexeme_type
   store_and_advance
   set lexeme_type "EOF"
}

proc lex_at {} {
   global lexeme_type
   store_and_advance
   set lexeme_type at
}

proc lex_tilde {} {
   global lexeme_type
   store_and_advance
   set lexeme_type tilde
}

proc lex_up_arrow {} {
   global lexeme_type
   store_and_advance
   set lexeme_type up_arrow
}

proc lex_lbrace {} {
   global lexeme_type
   store_and_advance
   set lexeme_type l_brace
}

proc lex_rbrace {} {
   global lexeme_type
   store_and_advance
   set lexeme_type r_brace
}

proc lex_exclamation_point {} {
   global lexeme_type
   store_and_advance
   set lexeme_type exclamation_point
}

proc lex_comma {} {
   global lexeme_type
   store_and_advance
   set lexeme_type comma
}

proc lex_equal {} {
   global lexeme_type lexeme_value
   read_constituent_string
   if {[string length $lexeme_value] == 1} {
      set lexeme_type equal
      return
   }
   determine_type_of_constituent_string
}

proc lex_ampersand {} {
   global lexeme_type lexeme_value
   read_constituent_string
   if {[string length $lexeme_value] == 1} {
      set lexeme_type ampersand
      return
   }
   determine_type_of_constituent_string
}

proc lex_lparen {} {
   global lexeme_type
   store_and_advance
   set lexeme_type l_paren
}

proc lex_rparen {} {
   global lexeme_type
   store_and_advance
   set lexeme_type r_paren
}

proc lex_greater {} {
   global lexeme_type lexeme_value
   read_constituent_string
   if {[string length $lexeme_value] == 1} {
      set lexeme_type greater
      return
   }
   if {$lexeme_value == ">>"} {
      set lexeme_type greater_greater
      return
   }
   if {$lexeme_value == ">="} {
      set lexeme_type greater_equal
      return
   }
   determine_type_of_constituent_string
}

proc lex_less {} {
   global lexeme_type lexeme_value
   read_constituent_string
   if {[string length $lexeme_value] == 1} {
      set lexeme_type less
      return
   }
   if {$lexeme_value == "<>"} {
      set lexeme_type not_equal
      return
   }
   if {$lexeme_value == "<="} {
      set lexeme_type less_equal
      return
   }
   if {$lexeme_value == "<<"} {
      set lexeme_type less_less
      return
   }
   if {$lexeme_value == "<=>"} {
      set lexeme_type less_equal_greater
      return
   }
   determine_type_of_constituent_string
}

proc lex_period {} {
   global lexeme_type lexeme_value current_char
   store_and_advance
   if [regexp {[0-9]} $current_char] {
      read_rest_of_floating_point_number
   }
   if {[string length $lexeme_value] == 1} {
      set lexeme_type period
      return
   }
   determine_type_of_constituent_string
}

proc lex_plus {} {
   global current_char lexeme_value lexeme_type
   read_constituent_string
   if {$current_char == "."} {
      set could_be_floating_point 1
      foreach i [split [string range $lexeme_value 1 end]] {
         if ![regexp {[0-9]} $i] {
            set could_be_floating_point 0
         }
      }
      if $could_be_floating_point {
         read_rest_of_floating_point_number
      }
   }
   if {[string length $lexeme_value] == 1} {
      set lexeme_type plus
      return
   }
   determine_type_of_constituent_string
}

proc lex_minus {} {
   global current_char lexeme_value lexeme_type
   read_constituent_string
   if {$current_char == "."} {
      set could_be_floating_point 1
      foreach i [split [string range $lexeme_value 1 end]] {
         if ![regexp {[0-9]} $i] {
            set could_be_floating_point 0
         }
      }
      if $could_be_floating_point {
         read_rest_of_floating_point_number
      }
   }
   if {[string length $lexeme_value] == 1} {
      set lexeme_type minus
      return
   }
   if {$lexeme_value == "-->"} {
      set lexeme_type right_arrow
      return
   }
   determine_type_of_constituent_string
}

proc lex_digit {} {
   global current_char lexeme_value lexeme_type
   read_constituent_string
   if {$current_char == "."} {
      set could_be_floating_point 1
      foreach i [split [string range $lexeme_value 1 end]] {
         if ![regexp {[0-9]} $i] {
            set could_be_floating_point 0
         }
      }
      if $could_be_floating_point {
         read_rest_of_floating_point_number
      }
   }
   determine_type_of_constituent_string
}

proc lex_unknown {} {
   global current_char
   parseError "Unknown character encountered by lexer '$current_char'"
   get_next_char
   get_lexeme
}

proc lex_constituent_string {} {
   read_constituent_string
   determine_type_of_constituent_string
}

proc lex_vbar {} {
   global lexeme_type lexeme_value current_char
   set lexeme_type sym_constant
   get_next_char
   while 1 {
      if {$current_char == "EOF"} {
         parseError "Opening '|' without closing '|'"
         set lexeme_type "EOF"
         return
      }
      if {$current_char == "\\"} {
         get_next_char
         set lexeme_value [format "%s%s" $lexeme_value $current_char]
         get_next_char
      } elseif {$current_char == "|"} {
         get_next_char
         break
      } else {
         set lexeme_value [format "%s%s" $lexeme_value $current_char]
         get_next_char
      }
   }
}

proc lex_quote {} {
   global lexeme_type lexeme_value current_char
   set lexeme_type quoted_string
   get_next_char
   while 1 {
      if {$current_char == "EOF"} {
         parseError "Opening '\"' without closing '\"'"
         set lexeme_type "EOF"
         return
      }
      if {$current_char == "\\"} {
         get_next_char
         set lexeme_value [format "%s%s" $lexeme_value $current_char]
         get_next_char
      } elseif {$current_char == "\""} {
         get_next_char
         break
      } else {
         set lexeme_value [format "%s%s" $lexeme_value $current_char]
         get_next_char
      }
   }
}

proc lex_dollar {} {
   global lexeme_type lexeme_value current_char
   set lexeme_type dollar_string
   set lexeme_value "$"
   get_next_char
   while {($current_char != "\n") && ($current_char != "EOF")} {
      set lexeme_value [format "%s%s" $lexeme_value $current_char]
      get_next_char
   }
}

proc read_rest_of_floating_point_number {} {
   global current_char
   store_and_advance
   while {[regexp {[0-9]} $current_char]} {
      store_and_advance
   }
   if {($current_char == "e") || ($current_char == "E")} {
      store_and_advance
      if {($current_char == "+") || ($current_char == "-")} {
         store_and_advance
      }
      while {[regexp {[0-9]} $current_char]} {
         store_and_advance
      }
   }
}

proc read_constituent_string {} {
   global constituent_chars current_char
   while {($current_char != "EOF") && \
          [regexp \[$constituent_chars\] $current_char]} {
      store_and_advance
   }
}
      
proc store_and_advance {} {
   global lexeme_value current_char
   set lexeme_value [format "%s%s" $lexeme_value [string tolower $current_char]]
   get_next_char
}

proc determine_type_of_constituent_string {} {
   global lexeme_value lexeme_type
   set s [determine_possible_symbol_types_for_string [split $lexeme_value ""]]
   if {[lsearch $s possible_var] >= 0} {
      set lexeme_type variable
      return
   }
   if {[lsearch $s possible_ic] >= 0} {
      set lexeme_type int_constant
      return
   }
   if {[lsearch $s possible_fc] >= 0} {
      set lexeme_type float_constant
      return
   }
   # For now we will always have this as 0, because we are just interested
   # in productions (which do not contain identifiers), but if we want
   # to care about other commands someday, we can use this code
   set allow_ids 0
   if {$allow_ids && ([lsearch $s possible_id] >= 0)} {
      set lexeme_type identifier
      return
   }
   if {[lsearch $s possible_sc] >= 0} {
      set lexeme_type sym_constant
      if {[string index $lexeme_value 0] == "<"} {
         if {[string index $lexeme_value 1] == "<"} {
            parseError "Warning: Possible disjunct intended but interpreted as symbolic constant"
         } else {
            parseError "Warning: Possible variable intended but interpreted as symbolic constant"
         }
      } else {
         if {[string index $lexeme_value [expr [string length $lexeme_value] - 1]] == ">"} {
            if {[string index $lexeme_value [expr [string length $lexeme_value] - 2]] == ">"} {
               parseError "Warning: Possible disjunct intended but interpreted as symbolic constant"
            } else {
               parseError "Warning: Possible variable intended but interpreted as symbolic constant"
            }
         }
      }
      return
   }
   set lexeme_type quoted_string
}

proc determine_possible_symbol_types_for_string {s} {
   global number_starters constituent_chars
   set retval ""
   if [regexp \[$number_starters\] [lindex $s 0]] {
      set i 0
      if {([lindex $s 0] == "+") || ([lindex $s 0] == "-")} {
         incr i
      }
      while {[regexp {[0-9]} [lindex $s $i]]} {
         incr i
      }
      if {([lindex $s $i] == "") && \
          [regexp {[0-9]} [lindex $s [expr $i - 1]]]} {
         lappend retval possible_ic
      }
      if {[lindex $s $i] == "."} {
         incr i
         while {[regexp {[0-9]} [lindex $s $i]]} {
            incr i
         }
         if {([lindex $s $i] == "e") || ([lindex $s $i] == "E")} {
            incr i
            if {([lindex $s $i] == "+") || ([lindex $s $i] == "-")} {
               incr i
            }
            while {[regexp {[0-9]} [lindex $s $i]]} {
               incr i
            }
         }
         if {[lindex $s $i] == ""} {
            lappend retval possible_fc
         }
      }
   }
   for {set i 0} {[lindex $s $i] != ""} {incr i} {
      if ![regexp \[$constituent_chars\] [lindex $s $i]] {
         return $retval
      }
   }
   lappend retval possible_sc
   if {([lindex $s 0] == "<") && \
       ([lindex $s [expr [llength $s] - 1]] == ">")} {
      lappend retval possible_var
   }
   if [regexp {[A-Za-z]} [lindex $s 0]] {
      set i 1
      while {[regexp {[0-9]} [lindex $s $i]]} {
         incr i
      }
      if {([lindex $s $i] == "") && \
          [regexp {[0-9]} [lindex $s [expr $i - 1]]]} {
         lappend retval possible_id
      }
   }
   return $retval
}

proc discard_line {} {
   global file_buffer
   set file_buffer ""
}

#############################

initialize_file_scan ""

global constituent_chars whitespace number_starters
set constituent_chars "A-Za-z0-9$%&*+/:<=>?_-"
set whitespace " \f\n\r\t\v"
set number_starters "0-9+\-."

global lexer_routine
set lexer_routine("@") lex_at
set lexer_routine("(") lex_lparen
set lexer_routine(")") lex_rparen
set lexer_routine("+") lex_plus
set lexer_routine("-") lex_minus
set lexer_routine("~") lex_tilde
set lexer_routine("^") lex_up_arrow
set lexer_routine("{") lex_lbrace
set lexer_routine("}") lex_rbrace
set lexer_routine("!") lex_exclamation_point
set lexer_routine(">") lex_greater
set lexer_routine("<") lex_less
set lexer_routine("=") lex_equal
set lexer_routine("&") lex_ampersand
set lexer_routine("|") lex_vbar
set lexer_routine(",") lex_comma
set lexer_routine(".") lex_period
set lexer_routine("\"") lex_quote
set lexer_routine("$") lex_dollar

# just a function for debugging the lexer
proc testLex { s } {
   global file_buffer current_char lexeme_type lexeme_value
   set file_buffer $s
   set current_char { }
   set lexeme_type ""
   while { $lexeme_type != "EOF" } {
      get_lexeme
      puts "Lexeme: $lexeme_type, $lexeme_value"
   }
}
