### -*- Mode: SDE -*-
###############################################################################
###
### File              : soar7km-aliases.tcl
### Original author(s): Frank E. Ritter (Frank.Ritter@nottingham.ac.uk)
### Organization      : University of Nottingham
### Created on        : 24 Nov 1996, 18:29:33
### Last Modified By  : K Coulter (kcoulter@umich.edu)
### Last Modified On  : 11 June 1998
### Soar Version      : 7.2
###
### Description : Aliases for Soar72.  Derived from commonly used and 
### psychologically supported rules, computed to save users time and errors.
### Based on the keystroke model and designed to be learned by a command
### name rule.
###
### This file updated to Soar 7.2 by K Coulter, June 1998.
### Removed all multi-agent commands.
###
###	TABLE OF CONTENTS
###
###	i.	Notes
###	ii.	Man page for aliases
###	I.	The aliases from the rule
###     II.     Some further common typo's
###     III.    Aliases related to other alias files
###     IV.     Documentation for aliases
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
# distribution.
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
## 11 command-to-file            1  0.36    1   2  8.37    3.02   15.22   ctf
## 18 excise -all                8  2.89    1   2  7.29   21.05   13.25   ea
## 19 excise -chunks            10  3.61    1   2  8.10   29.24   14.73   ec
## 20 excise -task               2  0.72    1   2  7.56    5.46   13.75   et
## 21 explain-backtraces         2  0.72    1   2  7.83    5.65   14.24   eb
## 22 firing-counts              2  0.72    1   2  6.48    4.68   11.78   fc
## 23 format-watch               1  0.36    1   2  6.21    2.24   11.29   fw
## 25 help -all                  3  1.08    1   2  6.75    7.31   12.27   ha
## 26 indifferent-selection      2  0.72    1   2  8.64    6.24   15.71  *inds
## 28 init-soar                 30 10.83    1   2  5.40   58.48    9.82   is
## 29 input-period               1  0.36    1   2  6.21    2.24   11.29   ip
## 30 internal-symbols           1  0.36    1   2  7.29    2.63   13.25  *
## 33 print -chunks              2  0.72    1   2  7.83    5.65   14.24   pc
## 39 multi-attributes                                                   *
## 40 output-strings-destinatio  1  0.36    1   2 11.34    4.09   20.62   osd
## 41 print -stack              10  3.61    1   2  7.56   27.29   13.75   ps
## 44 production-find            1  0.36    1   2  7.02    2.53   12.76   pf
## 47 run 1 e                   30 10.83    1   2  6.21   67.26   11.29  *r
## 49 remove-wme                 1  0.36    1   2  5.67    2.05   10.31   rw
## 50 rete-net                   1  0.36    1   2  5.13    1.85    9.33   rn

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
##  available in $soar_library/soar7km-aliases.tcl


##
##	I.	The aliases from the rule
##
## no watch on aliases as they load, too many aliases
## could print rule, but users will eventually complain

alias a        alias
alias aw       add-wme

# Command added in Soar 7.0.5, so added here but not yet in comments above
alias cnf      chunk-name-format

alias ctf      command-to-file
alias ea       excise -all
alias eb       explain-backtraces
# ec was unique for 'echo', but excise -chunks is far more useful
alias ec       excise -chunks
alias et       excise -task
## ex also stops ed from starting up, which is probably not a bad thing.
alias ex       excise
alias fc       firing-counts
alias fw       format-watch
alias h        help
alias ha       help -all
alias inds     indifferent-selection
alias ip       input-period
alias is       init-soar
# no alias, not a common command and init-soar is more common
# alias is       internal-symbols
alias l        learn
# left out as explicet aliases, for ma, me, and mo will be 
#    caught by command completion
# alias ma       matches
# alias me       memories
# alias mo       monitor
## multi-attributes left without an alias, for it should be ma, which 
##   would cause problems for matches, there is no obvious or necessary
##   other alias, and it must be done before productions are loaded.
##   mu does work due to command completion.
# alias multi-attributes
alias osd      output-strings-destination
alias p        print
alias pc       print -chunks
alias pf       production-find
alias pr       preferences
alias ps       print -stack
alias pw       pwatch
# not required, command completion will pick it up
# alias q        quit
alias r        run
alias rn       rete-net
alias rw       remove-wme
alias se       send
alias sn       soarnews
alias ss       stop-soar
alias st       stats
alias un       unalias
# left out, for will be caught by command completion
# alias ve       version
# note: will not be caught by command completion as 'w'
alias w        watch


##
##	II.	Some further common typos
##
alias help-all help -all


##
##	III.	Aliases related to other alias files
##

alias user-select echo "Use the new command 'indifferent-selection' instead."
## more for typos but also catches old command
alias soar-news soarnews

# Some of the aliases done in soar.tcl, so not included here.
#alias ?	help      
#alias exit     quit
alias m        matches
alias s        source
#alias wmes     print -depth 0 -internal

# make sure run/r gets included as well.


##
##    IV.     Documentation for aliases
##

## When help is invoked, a predefined aliases man page will
## come up for any aliases listed in the global Tcl variable
## documented_aliases. The ones listed below come from either
## this file or $soar_library/soar.tcl aliases.

global documented_aliases, soar7km_aliases

set soar7km_aliases "a aw ci cnf ctf di ea eb ec et ex fc fw h ha help-all \
ii inds ip is l li m osd p pc pf pr ps pw r rn rw s sci se sei sn soar-news \
ss st sti un w"

set documented_aliases [lsort [concat $documented_aliases $soar7km_aliases]]

