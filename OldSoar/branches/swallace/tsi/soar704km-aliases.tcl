### -*- Mode: SDE -*-
###############################################################################
###
### File              : soar704km-aliases.tcl
### Original author(s): Frank E. Ritter (Frank.Ritter@nottingham.ac.uk)
### Organization      : University of Nottingham
### Created on        : 24 Nov 1996, 18:29:33
### Last Modified By  : Frank E. Ritter <ritter@vpsyc.psychology.nottingham.ac.uk>
### Last Modified On  : 01 Feb 1997, 14:51:33
### Soar Version      : 7.0.4
###
### Description : Aliases for Soar704.  Derived from commonly used and 
### psychologically supported rules, computed to save users time and errors.
### Based on the keystroke model and designed to be learned by a command
### name rule.
###
###	TABLE OF CONTENTS
###
###	i.	Notes
###	ii.	Man page for aliases
###	I.	The aliases from the rule
###
###############################################################################


###
###	i.	Notes
###
	
# These aliases were computed using a dismal spreadsheet, and modified
# to take account of command completion already within Soar.  It also
# includes a few aliases common in the community, such as "m". 
#
# This set of aliases, using the assumptions in Nichols and Ritter
# nominally reduces typing to about 45% of the original
# commands (but not for the arguments, which I've found even harder to
# predict) using both a flat and a weighted command distribution.  The
# Ifor set, which is simpler, reduces typing to about 75% of the 
# original (25% savings).  My weighting and the flat weighting are 
# both probably too flat and overestimate the savings.  I only included
# "source" from commands that are solely Tcl & Unix commands. 
# 
# This file is now designed to be included directly in the Soar
# distribution.  It could/should be merged with soar-ifor.tcl, but
# does not have to be.
# 
# The alias generation rule (used by the program and usable by users):
#  If there are multiple words, use their first letters 
#      (29 commands with 6 exceptions)
#  If there is a single long (6 letters or more) word, use the first two
#      (8 commands, 0 exceptions)
#  If there is a short word, use the first letter
#      (14 commands with 8 exceptions)
# (Some of this explanation should go in the help file.)
#
#
##  If there are multiple words, use their first letters (29 with 6 exceptions)
##                              (a)  (b)   (c) (d)  (e)     (f)     (g)  (h)
##  9 add-wme                    1  0.36    1   2  4.86    1.75    8.84   aw
## 58 stop-soar                  3  1.08    1   2  5.40    5.85    9.82   ss
## 12 create-interpreter         2  0.72    1   2  7.83    5.65   14.24   ci
## 11 command-to-file            1  0.36    1   2  8.37    3.02   15.22   ctf
## 14 destroy-interpreter        1  0.36    1   2  8.10    2.92   14.73   di
## 18 excise -all                8  2.89    1   2  7.29   21.05   13.25   ea
## 19 excise -chunks            10  3.61    1   2  8.10   29.24   14.73   ec
## 20 excise -task               2  0.72    1   2  7.56    5.46   13.75   et
## 21 explain-backtraces         2  0.72    1   2  7.83    5.65   14.24   eb
## 22 firing-counts              2  0.72    1   2  6.48    4.68   11.78   fc
## 23 format-watch               1  0.36    1   2  6.21    2.24   11.29   fw
## 25 help -all                  3  1.08    1   2  6.75    7.31   12.27   ha
## 26 indifferent-selection      2  0.72    1   2  8.64    6.24   15.71  *inds
## 27 init-interpreter           2  0.72    1   2  7.29    5.26   13.25   ii
## 28 init-soar                 30 10.83    1   2  5.40   58.48    9.82   is
## 29 input-period               1  0.36    1   2  6.21    2.24   11.29   ip
## 30 internal-symbols           1  0.36    1   2  7.29    2.63   13.25  *
## 33 print -chunks              2  0.72    1   2  7.83    5.65   14.24   pc
## 34 list-interpreters          1  0.36    1   2  7.56    2.73   13.75   li
## 39 multi-attributes                                                   *
## 40 output-strings-destinatio  1  0.36    1   2 11.34    4.09   20.62   osd
## 41 print -stack              10  3.61    1   2  7.56   27.29   13.75   ps
## 44 production-find            1  0.36    1   2  7.02    2.53   12.76   pf
## 47 run 1 e                   30 10.83    1   2  6.21   67.26   11.29  *r
## 49 remove-wme                 1  0.36    1   2  5.67    2.05   10.31   rw
## 50 rete-net                   1  0.36    1   2  5.13    1.85    9.33   rn
## 51 schedule-interpreter       1  0.36    1   2  8.37    3.02   15.22  *sci
## 52 select-interpreter         5  1.81    1   2  7.83   14.18   14.24  *sei
## 54 send-to-interpreters       1  0.36    1   2  9.72    3.51   17.67   sti
## 55 soar-news                  1  0.36    1   2  5.40    1.95    9.82   sn   
## * indicates an exception
## 
##  If there is a single long (6 letters or more) word, use the first two
## (8, no exceptions, one duplication, source)
## 
## 17 excise                     1  0.36    1   2  3.24    1.17    5.89   ex   
## 36 matches                   10  3.61    1   2  3.51   12.67    6.38   ma   
## 37 memories                   1  0.36    1   2  3.78    1.36    6.87   me   
## 38 monitor                    2  0.72    1   2  3.51    2.53    6.38   mo   
## 42 preferences               15  5.42    1   2  4.59   24.86    8.35   pr   
## 45 pwatch                     3  1.08    1   2  3.24    3.51    5.89   pw   
## 59 unalias                    1  0.36    1   2  3.51    1.27    6.38   un   
## 60 version                    1  0.36    1   2  3.51    1.27    6.38   ve   
## 56 source                     5  1.81    1   2  3.24    5.85    5.89  *s
## 
##  If there is a short word, use the first letter (14 with 6 exceptions)
## 
## 53 send                       1  0.36    1   2  2.70    0.97    4.91  *se   
## 48 run                        1  0.36    1   2  2.43    0.88    4.42  r
## 46 quit                       2  0.72    1   2  2.70    1.95    4.91  *quit 
## 43 print                     20  7.22    1   2  2.97   21.44    5.40   p    
## 10 alias                      2  0.72    1   2  2.97    2.14    5.40   a    
## 35 log                        2  0.72    1   2  2.43    1.75    4.42  *log  
## 16 echo                       2  0.72    1   2  2.70    1.95    4.91  *echo 
## 24 help                       5  1.81    1   2  2.70    4.87    4.91   h    
## 31 io                         1  0.36    1   2  2.16    0.78    3.93  *io   
## 57 stats                      2  0.72    1   2  2.97    2.14    5.40  *st   
## 62 watch                      5  1.81    1   2  2.97    5.36    5.40   w    
## 32 learn                      5  1.81    1   2  2.97    5.36    5.40   l    
##    * indicates an exception
##    (a) Adjusted expert distribution
##    (b) Normalized expert weighting
##    (c) Flat distribution
##    (d) Normalized flat weighting 
##    (e) Original command execution time
##    (f) Command execution time weighted by expert distribution 
##    (g) Command execution time weighted by flat distribution
##    (h) New alias


###
###	ii.	Man page for aliases
###
##

##  
##  Most Soar commands have had one or more aliases created for them.  You can 
##  see the complete list by typing 'alias <CR>'.
##  
##  Most of the aliases are generated with a rule:
##    If there are multiple words, use their first letters 
##        (29 commands with 6 exceptions, the most common being 
##             indifferent-selection:inds,
##             schedule-interpreter:sci, and select-interpreter:sei)
##    If there is a single long word (6 letters or more), 
##        use the first two letters
##        (8 commands, 0 exceptions)
##    If there is a short word, use the first letter
##        (14 commands with 6 exceptions, none common)
##  
##  Aliases should NEVER appear in files.  They may be changed or not 
##  loaded by other users, and will not be as clear as the full command names.
##  
##
##  You can also use command completion to enter commands more quickly, and
##  you also define your own.
##
##  A listing of the aliases and further notes on how they were generated are
##  available in $soar_library/soar704km-aliases.tcl


##
##	I.	The aliases from the rule
##
## no watch on aliases as they load, too many aliases
## could print rule, but users will eventually complain
alias aw       add-wme
alias a        alias
alias ctf      command-to-file
alias ci       create-interpreter
alias di       destroy-interpreter
## ex also stops ed from starting up, which is probably not a bad thing.
alias ex       excise
# defined before, 1-Feb-97 -FER  alias ea       excise -all
# ec was unique for 'echo', but excise -chunks is far more useful
# defined before, 1-Feb-97 -FER  alias ec       excise -chunks
# defined before, 1-Feb-97 -FER  alias et       excise -task
alias eb       explain-backtraces
# defined before, 1-Feb-97 -FER  alias fc       firing-counts
alias fw       format-watch
alias h        help
alias ha       help -all
alias inds     indifferent-selection
alias ii       init-interpreter
alias is       init-soar
alias ip       input-period
# no alias, not a common command and init-soar is more common
# alias is       internal-symbols
alias l        learn
alias pc       print -chunks
alias li       list-interpreters
# left out as explicet aliases, for ma, me, and mo will be 
#    caught by command completion
# alias ma       matches
# alias me       memories
# alias mo       monitor
## multi-attributes left without an alias, for it should be ma, which 
##   would cause problems for matches, there is no obvious or necessary
##   other alias, and it must be done before productions are loaded.
# alias multi-attributes
alias osd      output-strings-destination
# defined before, 1-Feb-97 -FER  alias ps       print -stack
alias pr       preferences
# defined before, 1-Feb-97 -FER  alias p        print
# defined before, 1-Feb-97 -FER  alias pf       production-find
# not required, command completion will pick it up
# alias q        quit
alias pw       pwatch
alias rw       remove-wme
alias rn       rete-net
## This is defined somewhere else in Soar, so don't need to do it here.
## alias run      r
alias sci      schedule-interpreter
alias sei      select-interpreter
alias se       send
alias sti      send-to-interpreters
alias sn       soarnews
alias st       stats
alias ss       stop-soar
alias un       unalias
# left out, for will be caught by command completion
# alias ve       version
# note: will not be caught by command completion as 'w'
# defined before, 1-Feb-97 -FER  alias w        watch


##
##	II.	Some further common typos
##
alias help-all help -all


##
##	III.	Aliases missing from alias-soar6.tcl
##

alias user-select puts {Use the new command 'indifferent-selection' instead.}
# alias ms       matches
## more for typos but also catches old command
alias soar-news soarnews

# Some of the aliases done in soar-ifor, so not included here.
#alias m        matches
#alias exit     quit
#alias s	source
#alias ?	help      
# make sure run/r gets included as well.

