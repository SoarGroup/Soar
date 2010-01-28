<?php

	function misc_shorten( $str, $len )
	{
		if ( strlen( $str ) > $len )
			$str = ( substr( $str, 0, ( $len - 3 ) ) . '...' );
			
		return $str;
	}

?>
