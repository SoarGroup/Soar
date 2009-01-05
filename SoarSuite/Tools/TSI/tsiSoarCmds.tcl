
proc add-wme {args} {return [soar_agent ExecuteCommandLine "add-wme $args"]}
proc alias-soar {args} {return [soar_agent ExecuteCommandLine "alias $args"]}
proc alias {args} {return [soar_agent ExecuteCommandLine "alias $args"]}
proc attribute-preferences-mode {args} {
    return [soar_agent ExecuteCommandLine "attribute-preferences-mode $args"]}
proc chunk-name-format {args} {return [soar_agent ExecuteCommandLine "chunk-name-format $args"]}
proc default-wme-depth {args} {return [soar_agent ExecuteCommandLine "default-wme-depth $args"]}
proc echo {args} {return [soar_agent ExecuteCommandLine "echo $args"]}
proc excise {args} {return [soar_agent ExecuteCommandLine "excise $args"]}
proc explain-backtraces {args} {return [soar_agent ExecuteCommandLine "explain-backtraces $args"]}
proc firing-counts {args} {return [soar_agent ExecuteCommandLine "firing-counts $args"]}
proc gds-print {args} {return [soar_agent ExecuteCommandLine "gds-print $args"]}
proc help  {args} {return [soar_agent ExecuteCommandLine "help  $args"]}
proc indifferent-selection {args} {return [soar_agent ExecuteCommandLine "indifferent-selection $args"]}
proc init-soar  {args} {return [soar_agent ExecuteCommandLine "init-soar  $args"]}
proc input-period {args} {return [soar_agent ExecuteCommandLine "input-period $args"]}
proc internal-symbols {args} {return [soar_agent ExecuteCommandLine "internal-symbols $args"]}
proc learn {args} {return [soar_agent ExecuteCommandLine "learn $args"]}
proc log {args} {return [soar_agent ExecuteCommandLine "log $args"]}
proc matches {args} {return [soar_agent ExecuteCommandLine "matches $args"]}
proc max-chunks {args} {return [soar_agent ExecuteCommandLine "max-chunks $args"]}
proc max-elaborations {args} {return [soar_agent ExecuteCommandLine "max-elaborations $args"]}
proc max-nil-output-cycles {args} {return [soar_agent ExecuteCommandLine "max-nil-output-cycles $args"]}
proc memories {args} {return [soar_agent ExecuteCommandLine "memories $args"]}
proc multi-attributes {args} {return [soar_agent ExecuteCommandLine "multi-attributes $args"]}
proc numeric-indifferent-mode {args} {return [soar_agent ExecuteCommandLine "numeric-indifferent-mode $args"]}
proc o-support-mode {args} {return [soar_agent ExecuteCommandLine "o-support-mode $args"]}
proc preferences {args} {return [soar_agent ExecuteCommandLine "preferences $args"]}
proc print {args} {return [soar_agent ExecuteCommandLine "print $args"]}
proc production-find {args} {return [soar_agent ExecuteCommandLine "production-find $args"]}
proc pwatch {args} {return [soar_agent ExecuteCommandLine "pwatch $args"]}
proc soar-quit {args} {return [soar_agent ExecuteCommandLine "quit $args"]}
proc remove-wme {args} {return [soar_agent ExecuteCommandLine "remove-wme $args"]}
proc rete-net {args} {return [soar_agent ExecuteCommandLine "rete-net $args"]}
proc save-backtraces {args} {return [soar_agent ExecuteCommandLine "save-backtraces $args"]}
proc set-library-location {args} {return [soar_agent ExecuteCommandLine "set-library-location $args"]}
proc soar8 {args} {return [soar_agent ExecuteCommandLine "soar8 $args"]}
proc soar-news {args} {return [soar_agent ExecuteCommandLine "soar-news $args"]}
proc sp {args} {return [soar_agent ExecuteCommandLine "sp $args"]}
proc stats {args} {return [soar_agent ExecuteCommandLine "stats $args"]}
proc stop-soar {args} {return [soar_agent ExecuteCommandLine "stop-soar $args"]}
proc time {args} {return [soar_agent ExecuteCommandLine "time $args"]}
proc timers {args} {return [soar_agent ExecuteCommandLine "timers $args"]}
proc verbose {args} {return [soar_agent ExecuteCommandLine "verbose $args"]}
proc soar-version {args} {return [soar_agent ExecuteCommandLine "version $args"]}
proc wait-snc {args} {return [soar_agent ExecuteCommandLine "wait-snc $args"]}
proc watch {args} {return [soar_agent ExecuteCommandLine "watch $args"]}
proc watch-wmes {args} {return [soar_agent ExecuteCommandLine "watch-wmes $args"]}

## soar's predefined aliases.  

proc ?  {args} {return [soar_agent ExecuteCommandLine "help  $args"]}
proc a {args} {return [soar_agent ExecuteCommandLine "alias $args"]}
proc aw {args} {return [soar_agent ExecuteCommandLine "add-wme $args"]}
proc d {args} {return [soar_agent ExecuteCommandLine "run 1 -d $args"]}
proc e {args} {return [soar_agent ExecuteCommandLine "run 1 -e $args"]}
proc eb {args} {return [soar_agent ExecuteCommandLine "explain-backtraces $args"]}
proc ex {args} {return [soar_agent ExecuteCommandLine "excise $args"]}
proc exit {args} {return [soar_agent ExecuteCommandLine "quit $args"]}
proc fc {args} {return [soar_agent ExecuteCommandLine "firing-counts $args"]}
proc gds_print {args} {return [soar_agent ExecuteCommandLine "gds-print $args"]}
proc inds {args} {return [soar_agent ExecuteCommandLine "indifferent-selection $args"]}
proc init  {args} {return [soar_agent ExecuteCommandLine "init-soar  $args"]}
proc interrupt  {args} {return [soar_agent ExecuteCommandLine "stop-soar  $args"]}
proc is  {args} {return [soar_agent ExecuteCommandLine "init-soar  $args"]}
proc l {args} {return [soar_agent ExecuteCommandLine "learn $args"]}
proc log {args} {return [soar_agent ExecuteCommandLine "log $args"]}
proc man {args} {return [soar_agent ExecuteCommandLine "help  $args"]}

proc p {args} {return [soar_agent ExecuteCommandLine "print $args"]}
proc pc {args} {return [soar_agent ExecuteCommandLine "print --chunks $args"]}
proc pr {args} {return [soar_agent ExecuteCommandLine "preferences $args"]}
proc pw {args} {return [soar_agent ExecuteCommandLine "pwatch $args"]}
proc rn {args} {return [soar_agent ExecuteCommandLine "rete-net $args"]}
proc rw {args} {return [soar_agent ExecuteCommandLine "remove-wme $args"]}
proc sb {args} {return [soar_agent ExecuteCommandLine "save-backtraces $args"]}
proc set-default-depth {args} {return [soar_agent ExecuteCommandLine "default-wme-depth $args"]}
proc sn {args} {return [soar_agent ExecuteCommandLine "soar-news $args"]}
proc ss {args} {return [soar_agent ExecuteCommandLine "stop-soar $args"]}
proc sp {args} {return [soar_agent ExecuteCommandLine "sp $args"]}
proc st {args} {return [soar_agent ExecuteCommandLine "stats $args"]}
proc step {args} {return [soar_agent ExecuteCommandLine "run 1 $args"]}
proc stop {args} {return [soar_agent ExecuteCommandLine "stop-soar $args"]}
proc unalias {args} {return [soar_agent ExecuteCommandLine "alias -d $args"]}
proc un {args} {return [soar_agent ExecuteCommandLine "alias -d $args"]}
proc w {args} {return [soar_agent ExecuteCommandLine "watch $args"]}
proc wmes {args} {return [soar_agent ExecuteCommandLine "print -i $args"]}


