#-----------------------------------------------------------------------
#
# Random number generator.
#
# Taken from "Practical Programming in Tcl/Tk" by Brent Welch (pg 44).
#

proc randomInit {seed} {
  global rand

  set rand(ia)    9301 ;# Multiplier
  set rand(ic)   49297 ;# Constant
  set rand(im)  233280 ;# Divisor
  set rand(seed) $seed ;# Last result
}

proc random {} {
  global rand

  set rand(seed) \
      [expr ($rand(seed)*$rand(ia) + $rand(ic)) % $rand(im)]
  return [expr $rand(seed)/double($rand(im))]
}

proc randomRange {range} {
  expr int([random]*$range)
}


#-----------------------------------------------------------------------
