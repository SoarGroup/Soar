<?php
	
	define( 'CONNECTION', 'java' );
	define( 'NUM_TRIALS', 5 );
	define( 'SPEEDY_EXPERIMENT', 5 );
	define( 'EXTRA_PARAMS', ( 'machine=macsoar branch=nlderbin-epmem-smem revision=11570 connection=' . CONNECTION ) );
	
	for ( $i=0; $i<NUM_TRIALS; $i++ )
	{
		echo ( 'TRIAL #' . ( $i + 1 ) . "\n" );
		echo shell_exec( 'cd ' . CONNECTION . '; bash run.sh "' . EXTRA_PARAMS . '" | php ../../speedy_publish.php ' . SPEEDY_EXPERIMENT );
		echo "\n";
	}
	
?>
