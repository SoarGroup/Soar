<?php
	
	// get pid
	$pid = intval( $argv[1] );

	// use ps to parse out memory (rss) and cpu time (time)	
	$ps = explode( ',', trim( shell_exec( 'ps -o rss,time -p ' . $pid . ' | tail -n 1 | awk \'{printf("%s,%s", $1, $2)}\'' ) ) );
	
	// process and output
	{
		$output = array();
		
		// rss
		{
			$output['rss'] = intval( $ps[0] );
		}
		
		// time
		{
			$nums = array();
			$left = $ps[1];
			while ( $pos = strrpos( $left, ':' ) )
			{
				$nums[] = doubleval( substr( $left, $pos+1 ) );
				$left = substr( $left, 0, $pos );
			}
			$nums[] = doubleval( $left );
			
			$sum = 0;
			foreach ( $nums as $exp => $mult )
			{
				$sum += ( $mult * pow( 60, $exp ) );
			}
			
			$output['time'] = $sum;
		}
		
		// param = value
		{
			foreach ( $output as $key => $val )
			{
				$output[ $key ] = ( $key . '=' . $val );
			}
		}
		
		echo implode( ' ', $output );
	}

?>
