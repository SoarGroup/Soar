# Define the following aliases for Soar 6 compatibility.  Since
# these are only applicable in agent interpreters, we make sure
# that these are only defined for those interpreter types.

if [string match $interp_type agent] {
    alias create-agent          create-interp -agent
    alias destroy-agent         destroy-interp
    alias excise-all            excise -all
    alias excise-chunks         excise -chunks
    alias excise-task           excise -task
    alias list-agents           list-interps -agent
    alias list-chunks           list-productions -chunk
    alias list-help-topics      help -all
    alias list-justifications   list-productions -justification
    alias load                  source
    alias memory-stats          stats -memory
    alias object-trace-format   format-watch -object
    alias p                     print
    alias pgs                   print -stack
    alias print-stats           stats
    alias rete-stats            stats -rete
    alias r                     run
    alias select-agent          select-interp
    alias spr                   print
    alias stack-trace-format    format-watch -stack
}
