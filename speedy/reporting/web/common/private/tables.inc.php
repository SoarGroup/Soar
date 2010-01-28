<?php

	function tables_make_perty( $schema, $data )
	{
		$return_val = '';
		
		//start
		$return_val .= '<table class="perty">';
		
			// head_start
			$return_val .= '<thead><tr>';
			
				// header
				foreach ( $schema as $field_name => $field_type )
				{
					$return_val .= '<td>' . htmlentities( $field_name ) . '</td>';
				}
			
			// head_end
			$return_val .= '</tr></thead>';
			
			//
			
			// body_start
			$return_val .= '<tbody>';
			
				// data
				foreach ( $data as $row )
				{
					$return_val .= '<tr>';
					
					foreach ( $schema as $field_name => $field_type )
					{
						$return_val .= '<td style="text-align:' . ( ( $field_type == EXP_TYPE_STRING )?('left'):('right') ) . '">' . htmlentities( ( $field_type == EXP_TYPE_STRING )?( $row[ $field_name ] ):( number_format( ( ( $field_type == EXP_TYPE_INT )?( intval( $row[ $field_name ] ) ):( doubleval( $row[ $field_name ] ) ) ), ( ( $field_type == EXP_TYPE_INT )?(0):(4) ) ) ) ) . '</td>';
					}					
					
					$return_val .= '</tr>';
				}
			
			// body_start
			$return_val .= '</tbody>';
		
		//end
		$return_val .= '</table>';		
		
		return $return_val;
	}
	
	function _tables_csv_quote( $val )
	{
		return ( '"' . str_replace( '"', '""', trim( $val ) ) . '"' );
	}
	
	function tables_csv( $schema, $data )
	{
		if ( !empty( $data ) )
		{
			$rows = array();
			
			// header
			{
				$header = array();
				
				foreach ( $schema as $field_name => $field_type )
				{
					$header[] = _tables_csv_quote( $field_name );
				}
				
				$rows[] = implode( ',', $header );
			}
			
			// data
			{
				foreach ( $data as $row )
				{
					$temp = array();
					
					foreach ( $schema as $field_name => $field_type )
					{
						$temp[] = _tables_csv_quote( $row[ $field_name ] );
					}
					
					$rows[] = implode( ',', $temp );
				}
			}
			
			$output = implode( "\n", $rows );
		}
		else
		{
			$output = '';
		}
		
		// IE Fixes
		header("Pragma: public");
		header("Expires: 0");
		header("Cache-Control: must-revalidate, post-check=0, pre-check=0"); 
		
		// Force download
		header("Content-Type: application/force-download");
		header('Content-Disposition: attachment; filename="report.csv"' );
		
		// Allows for download progress bar
		$size = strlen( $output );
		
		if ( $size )
		{
			header ("Accept-Ranges: bytes");
			header ("Content-Length: " . $size);
		}
		
		header("Content-Description: File Transfer");
		
		echo $output;
	}

?>
