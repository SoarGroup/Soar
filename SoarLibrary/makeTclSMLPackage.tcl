if {$tcl_platform(platform) == "windows"} {
	set directory "."
} else {
	set directory ".libs"
}

pkg_mkIndex -verbose $directory *Tcl_sml_ClientInterface[info sharedlibextension]
