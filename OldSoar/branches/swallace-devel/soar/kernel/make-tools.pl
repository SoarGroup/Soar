

sub extract_used_flags_from_files() {

    local($path) = shift();
    local(@fileTypes) = @_;
    local(@allFiles);
    local($fileType);
    local(@files);
    local($file);

    opendir( DIR, $path );
    @allFiles = readdir( DIR );
    closedir( DIR );
    
    foreach $fileType (@FileTypes) {
	
	@files = grep (/$fileType/, @allFiles );
	foreach $file (@files) {

	    open( README, $path.'/'.$file) or die "Can't open $file";
	    
	    while( <README> ) {
				
		if ( /\#ifdef ([\w\d]+)/ ) {
		    $Flags{$1} = 1;
		    
		} elsif ( /\#ifndef ([\w\d]+)/ ) {
		    $Flags{$1} = 1;
		} 
				
	    }
	    close( README );
	    
	}
    }
}

