if {$tcl_platform(platform) == "windows"} {
	set directory "."
} else {
	set directory "../ClientSMLSWIG/Tcl/.libs"
}

pkg_mkIndex -verbose $directory *Tcl_sml_ClientInterface[info sharedlibextension]