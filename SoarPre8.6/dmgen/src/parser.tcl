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
# Soar production "parser".
#
# Proc exports:
#  parse_production

proc parse_production { buf } {
   global file_buffer
   set file_buffer $buf
   global current_char
   set current_char " "
   global productionName lexeme_type lexeme_value
   get_lexeme
   if {($lexeme_type != "sym_constant") || ($lexeme_value != "sp")} {
      parseError "Production must start with sp"
      return 0
   }
   get_lexeme
   if {$lexeme_type != "l_brace"} {
      parseError "Production definition must begin with \{"
      return 0
   }
   get_lexeme
   if {$lexeme_type != "sym_constant"} {
      parseError "Expected symbol for production name"
      return 0
   }
   set productionName $lexeme_value
   get_lexeme
   if {$lexeme_type == "quoted_string"} {
      get_lexeme
   }
   while 1 {
      if {$lexeme_type != "sym_constant"} {
         break
      }
      if {($lexeme_value == ":o-support") || \
          ($lexeme_value == ":i-support") || ($lexeme_value == ":chunk") || \
          ($lexeme_value == ":default") || ($lexeme_value == ":justification")} {
         get_lexeme
         continue
      }
      break
   }
   set lhs [parse_lhs]
   if {$lhs == 0} {
      puts "parse_lhs failed"
      return 0
   }
   if {$lexeme_type != "right_arrow"} {
      parseError "Expected --> in production"
      return 0
   }
   get_lexeme
   set rhs [parse_rhs]
   if {$rhs == 0} {
      puts "parse_lhs failed"
      return 0
   }
   if {$lexeme_type != "r_brace"} {
      parseError "Expected \} to end production"
      return 0
   }
   get_lexeme
   if {$lexeme_type != "EOF"} {
      parseError "Found text after production end?"
      return 0
   }
   return [concat $lhs $rhs]
}

proc parse_lhs {} {
   return [parse_cond_plus]
}

proc parse_cond_plus {} {
   global lexeme_type
   set c ""
   for {set nc [parse_cond]} \
       {($lexeme_type == "minus") || ($lexeme_type == "l_paren")} \
       {set nc [parse_cond]} {
      if {$nc == 0} {
         return 0
      }
      set c [concat $c $nc]
   }
   if {$nc == 0} {
      return 0
   }
   set c [concat $c $nc]
   return $c
}

proc parse_cond {} {
   global lexeme_type
   if {$lexeme_type == "minus"} {
      get_lexeme
   }
   if {$lexeme_type == "l_brace"} {
      get_lexeme
      set c [parse_cond_plus]
      if {$c == 0} {
         return 0
      }
      if {$lexeme_type != "r_brace"} {
         parseError "Expected \} to end conjunctive condition"
         return 0
      }
      get_lexeme
   } else {
      set c [parse_single_cond]
      if {$c == 0} {
         return 0
      }
   }
   return $c
}

proc parse_single_cond {} {
   global lexeme_type lexeme_value
   if {$lexeme_type != "l_paren"} {
      parseError "Expected ( to begin condition"
      return 0
   }
   get_lexeme
   set headElement ""
   if {$lexeme_type == "sym_constant"} {
      if {($lexeme_value == "state") || ($lexeme_value == "impasse")} {
         set headElement $lexeme_value
         get_lexeme
      }
   }
   if {$lexeme_type != "variable"} {
      parseError "Expected variable for condition id test"
      return 0
   }
   set id_test $lexeme_value
   get_lexeme
   if {$lexeme_type == "minus"} {
      get_lexeme
   }
   if {$lexeme_type != "up_arrow"} {
      parseError "Expected ^ to begin attribute test"
      return 0
   }

   get_lexeme

   global relational_test_types
   
   set attr_test [parse_test]
   if {$attr_test == {} } {
      return 0
   }
   set attr_test_rels $relational_test_types
   set value_test [parse_test]
   if {$value_test == {} } {
      return 0
   }
   set value_test_rels $relational_test_types

   parse_preference ;# Make sure we get LHS operator preference
                    ;# Added by Dave Ray 05/02/2002

   if {$lexeme_type != "r_paren"} {
      parseError "Expected ) to end condition"
      return 0
   }
   get_lexeme
   set c ""
   if {$headElement != ""} {
      lappend c [list L "TOP" $headElement $id_test \
                     $attr_test_rels $value_test_rels]
   }
   foreach i $attr_test {
      foreach j $value_test {
         lappend c [list L \
                     $id_test \
                     $i $j \
                     $attr_test_rels $value_test_rels]
      }
   }
   return $c
}

proc parse_test {} {
   global lexeme_type relational_test_types
   set relational_test_types {}
   if {$lexeme_type != "l_brace"} {
       return [parse_simple_test]
   }
   get_lexeme
   set c ""
   for {set r [parse_simple_test]} {$lexeme_type != "r_brace"} \
       {set r [parse_simple_test]} {
      if {$r == {} } {
         return $r
      }
      set c [concat $c $r]
   }
   if {$r == {} } {
      return $r
   }
   set c [concat $c $r]
   get_lexeme
   return $c
}

proc parse_simple_test {} {
   global lexeme_type 
   if {$lexeme_type == "less_less"} {
      return [parse_disjunction_test]
   } else {
      return [parse_relational_test]
   }
}

proc parse_disjunction_test {} {
   global lexeme_type lexeme_value
   if {$lexeme_type != "less_less"} {
      parseError "Expected << to begin disjunction test"
      return {}
   }
   set c ""
   for {get_lexeme} {$lexeme_type != "greater_greater"} {get_lexeme} {
      if {($lexeme_type != "sym_constant") && \
          ($lexeme_type != "int_constant") && \
          ($lexeme_type != "float_constant")} {
         parseError "Expected constant or >> while reading disjunction test"
         return {}
      }
      lappend c $lexeme_value
   }
   get_lexeme
   return $c
}

proc parse_relational_test {} {
   global lexeme_type lexeme_value relational_test_types
   if {($lexeme_type == "equal") || ($lexeme_type == "not_equal") || \
       ($lexeme_type == "less") || ($lexeme_type == "greater") || \
       ($lexeme_type == "less_equal") || ($lexeme_type == "greater_equal") || \
       ($lexeme_type == "less_equal_greater")} {
      lappend relational_test_types $lexeme_type

      get_lexeme
   }
   if {($lexeme_type == "sym_constant") || ($lexeme_type == "int_constant") || \
       ($lexeme_type == "float_constant") || ($lexeme_type == "variable")} {
      set c $lexeme_value
      get_lexeme
      return $c
   # This is a special weird case that occurs when a production tests an
   # attribute for the empty string ||. In this case, print -internal prints
   # the value, but since it's the empty string it doesn't appear as anything.
   # Annoying...
   } elseif { $lexeme_type == "r_paren" } {
      return "____DMGEN_EMPTYSTRING____" ;
      # Can't just return "" because that's the error return code :(
      # Stupid tcl.
   } else {
      parseError "Expected variable or constant for test"
      return {}
      #return 0
   }
}

proc parse_rhs {} {
   global lexeme_type
   set c ""
   while {$lexeme_type != "r_brace"} {
      set nc [parse_rhs_action]
      if {$nc == 0} {
         return 0
      }
      set c [concat $c $nc]
   }
   return $c
}

proc parse_rhs_action {} {
   global lexeme_type lexeme_value
   if {$lexeme_type != "l_paren"} {
      parseError "Expected ( to begin RHS action"
      return 0
   }
   get_lexeme
   if {$lexeme_type != "variable"} {
      if {[parse_function_call_after_lparen] == {} } {
         return 0
      }
      return ""
   }
   set id_make $lexeme_value
   get_lexeme
   if {$lexeme_type != "up_arrow"} {
      parseError "Expected ^ in RHS action attribute"
      return 0
   }
   get_lexeme
   set attr_make [parse_rhs_value]
   if {$attr_make == {} } {
      return 0
   }
   set value_make [parse_rhs_value]
   set alt_value_make [parse_preference]
   if {$lexeme_type != "r_paren"} {
      parseError "Expected ) to end RHS action, got $lexeme_type"
      return 0
   }
   get_lexeme
   if {$alt_value_make == ""} {
      return [list [list R $id_make $attr_make $value_make]]
   } else {
      return [list [list R $id_make $attr_make $value_make] \
                   [list R $id_make $attr_make $alt_value_make]]
   }
}

# these are here just so we can store the text of a function call
set parsing_function 0  ;# current parse_function... call depth
set parsed_function ""  ;# buffer where function call text is stored

proc parse_function_call_after_lparen {} {
   global lexeme_type lexeme_value

   global parsing_function parsed_function 
   incr parsing_function ;# increment call depth
   if { $parsing_function == 1 } {
      # Initialize function buffer. The left paren and operator have
      # already been parsed
      set parsed_function "( $lexeme_value "
   }

   get_lexeme
   while {$lexeme_type != "r_paren"} {
      if { [parse_rhs_value] == {} } {
         set parsing_function 0 ;# clean up call depth
         set parsed_function ""
         #return 0
         return {}
      }
   }
   incr parsing_function -1 ;# decrement call depth
   get_lexeme
   if { $parsing_function == 0 } { ;# done parsing function
      return [list [list "FUNCTION" $parsed_function]]
   }
   return "FUNCTION_CALL?"
}

proc parse_rhs_value {} {
   global lexeme_type lexeme_value
   if {$lexeme_type == "l_paren"} {
      get_lexeme
      return [parse_function_call_after_lparen]
   }
   if {($lexeme_type == "sym_constant") || ($lexeme_type == "int_constant") || \
       ($lexeme_type == "float_constant") || ($lexeme_type == "variable")} {
      set c $lexeme_value
      get_lexeme
      return $c
   }
   parseError "Illegal value for RHS value"
   #return 0
   return {}
}

proc parse_preference {} {
   global lexeme_type
   set x [parse_preference_specifier_without_referent]
   if {$x == "binary"} {
      set c [parse_rhs_value]
      if {$c == {} } {
         return 0
      }
      return $c
   }
   return ""
}

proc parse_preference_specifier_without_referent {} {
   global lexeme_type
   if {($lexeme_type == "plus") || ($lexeme_type == "minus") || \
       ($lexeme_type == "exclamation_point") || ($lexeme_type == "tilde") || \
       ($lexeme_type == "at")} {
      get_lexeme
      return unary
   }
   if {($lexeme_type == "greater") || ($lexeme_type == "equal") || \
       ($lexeme_type == "less") || ($lexeme_type == "ampersand")} {
      get_lexeme
      if {$lexeme_type != "r_paren"} {
         return binary
      }
      return unary
   }
   return special
}

proc parseError {x} {
   global file_buffer lexeme_type lexeme_value
   LogError "Parse Error: $x, \
lexeme_type = $lexeme_type, lexeme_value = $lexeme_value \
### Remaining buffer:\n$file_buffer\n###"

}
