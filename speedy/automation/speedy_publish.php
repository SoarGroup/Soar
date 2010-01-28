<?php
	
	// CONFIG: assumes final slash
	define( 'SPEEDY_BASE_URL', 'http://domain/path/to/speedy/' );
	$add_url = ( SPEEDY_BASE_URL . 'experiments.php' );
	
	//
	
	function usage()
	{
		global $argv;
		
		echo ( $argv[0] . ' <experiment number> [any value to debug]' . "\n" );
	}
	
	if ( ( $argc < 2 ) || ( $argc > 3 ) )
	{
		usage();
		exit( 0 );
	}
	$exp_id = intval( $argv[1] );
	$debug_mode = ( $argc == 3 );
	
	// data comes from standard in
	$in = file( 'php://stdin' );
	
	//
	
	foreach ( $in as $ct => $datum )
	{
		// kill extra white
		$datum = trim( $datum );
		
		// split on white
		$datum = preg_split( '/\s+/' , $datum );
		
		// break into key/value
		$vars = array( 'cmd'=>'data', 'exp_id'=>$exp_id );
		foreach ( $datum as $key => $val )
		{
			$temp = explode( '=', $val );
			$vars[ $temp[0] ] = $temp[1];
		}
		$datum = $vars;
		
		// produce url
		$datum = ( $add_url . '?' . http_build_query( $datum ) );
		
		// status
		echo ( 'Datum ' . ( $ct + 1 ) . '... ' );
		if ( $debug_mode )
		{
			echo $datum;
		}
		else
		{
			$curl_session = curl_init();
	
			curl_setopt( $curl_session, CURLOPT_URL, $datum );
			curl_setopt( $curl_session, CURLOPT_HEADER, false );
			curl_setopt( $curl_session, CURLOPT_RETURNTRANSFER, true );
	
			$success = ( strstr( curl_exec( $curl_session ), 'ADDED DATUM' ) !== false );
	 
			curl_close( $curl_session );
			
			echo ( ( $success )?('success'):('failure') );
		}		
		echo "\n";
	}
?>
