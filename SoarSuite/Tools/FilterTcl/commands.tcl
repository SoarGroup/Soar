# This allows us to have Soar commands that are intercepted
# and handled as we wish.

# Required for pushd and friends
package require Tclx

#these are from tsiSoarCmds
proc add-wme {args} {return [soar_agent ExecuteCommandLine "add-wme $args"]}
proc alias-soar {args} {return [soar_agent ExecuteCommandLine "alias $args"]}
proc alias {args} {return [soar_agent ExecuteCommandLine "alias $args"]}
proc attribute-preferences-mode {args} {
    return [soar_agent ExecuteCommandLine "attribute-preferences-mode $args"]}
proc chunk-name-format {args} {return [soar_agent ExecuteCommandLine "chunk-name-format $args"]}
proc clog {args} {return [soar_agent ExecuteCommandLine "clog $args"]}
proc default-wme-depth {args} {return [soar_agent ExecuteCommandLine "default-wme-depth $args"]}
proc echo {args} {return [soar_agent ExecuteCommandLine "echo $args"]}
proc echo-commands {args} {return [soar_agent ExecuteCommandLine "echo-comands $args"]}
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
proc ls {args} {return [soar_agent ExecuteCommandLine "ls $args"]}
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
proc run {args} {return [soar_agent ExecuteCommandLine "run $args"]}
proc save-backtraces {args} {return [soar_agent ExecuteCommandLine "save-backtraces $args"]}
proc set-library-location {args} {return [soar_agent ExecuteCommandLine "set-library-location $args"]}
proc set-stop-phase {args} {return [soar_agent ExecuteCommandLine "set-stop-phase $args"]}
proc soar8 {args} {return [soar_agent ExecuteCommandLine "soar8 $args"]}
proc soar-news {args} {return [soar_agent ExecuteCommandLine "soar-news $args"]}
proc sp {args} {return [soar_agent ExecuteCommandLine "sp $args"]}
proc srand {args} {return [soar_agent ExecuteCommandLine "srand $args"]}
proc stats {args} {return [soar_agent ExecuteCommandLine "stats $args"]}
proc stop-soar {args} {return [soar_agent ExecuteCommandLine "stop-soar $args"]}
proc time {args} {return [soar_agent ExecuteCommandLine "time $args"]}
proc timers {args} {return [soar_agent ExecuteCommandLine "timers $args"]}
proc verbose {args} {return [soar_agent ExecuteCommandLine "verbose $args"]}
proc soar-version {args} {return [soar_agent ExecuteCommandLine "version $args"]}
proc wait-snc {args} {return [soar_agent ExecuteCommandLine "wait-snc $args"]}
proc warnings {args} {return [soar_agent ExecuteCommandLine "warnings $args"]}
proc watch {args} {return [soar_agent ExecuteCommandLine "watch $args"]}
proc watch-wmes {args} {return [soar_agent ExecuteCommandLine "watch-wmes $args"]}

# We can use this technique to intercept "puts" and send the output somewhere else
# of our choosing.  In this case we'll return the output string as the result of the
# call which means it'll be passed back as the result of the "puts" command and be
# printed in the debugger (if issued from there).
rename puts myPuts
proc puts {arg} {
	myPuts "Called internal puts"
	myPuts $arg
	return $arg
}

# The internal Soar source now includes an implicit "pushd"/"popd" logic which makes it easier
# to use the files.  So we'll include that here, although that may cause problems in systems that
# assume cwd isn't changed within the source call (in which case, remove this function).
rename source mySource
proc source {arg} {
	myPuts "Called internal source $arg"
	
	set folder [file dirname $arg]
	if ![string equal $folder .] {
		pushd $folder 
	}
	
	set result [mySource $arg]
	
	if ![string equal $folder .] {
		popd
	}
	
	myPuts "Result of source $arg is $result"
	return $result
}
