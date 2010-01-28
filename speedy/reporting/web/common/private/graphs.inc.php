<?php
	
	define( 'GOOGLE_CHART_URL', 'http://chart.apis.google.com/chart' );
	define( 'SCALING_CONST', 1.1 );
	define( 'FONT_SIZE', 10 );
	define( 'POINT_SIZE', 5 );
	
	$chart_colors = array(
		'default' => array( '02243C', '005DB3', '5195CE', '57B57D', 'D15C5C', '45453F', '450035', 'BA7836', 'FFE099', 'font'=>'000000' ),		
	);
	
	function graphs_gen_url( $base, $params )
	{
		return ( $base . '?' . http_build_query( $params ) );
	}
	
	// input: array( 'x-label'=>y )
	function graphs_line_chart_url( $data, $width, $height, $min = null, $color_scheme = 'default', $data_labels = true )
	{
		global $chart_colors;
		$params = array();
		
		// chart type
		$params['cht'] = 'lxy';
		
		// chart size
		$params['chs'] = ( $width . 'x' . $height );
		
		// colors
		$params['chco'] = $chart_colors[ $color_scheme ][0];
		
		// axis type
		$params['chxt'] = 'x,y';
		
		// chart data
		{			
			$labels = array_keys( $data );
			
			$min_data = ( ( is_null( $min ) )?( floor( min( array_values( $data ) ) ) ):( $min ) );
			$max_data = ceil( max( array_values( $data ) ) );
			
			// data and scaling
			{
				$params['chd'] = ( 't:' . implode( ',', array_keys( $labels ) ) . '|' . implode( ',', $data ) );
				$params['chds'] = ( '-1,' . ( count( $data ) ) . ',' . $min_data . ',' . ceil( $max_data * SCALING_CONST ) );
			}			
			
			// y-axis range
			$params['chxr'] = ( '1,' . $min_data . ',' . ceil( $max_data * SCALING_CONST ) );
			
			// x-axis labels
			$params['chxl'] = ( '0:||' . implode( '|', array_keys( $data ) ) . '|' );
			
			// label data points
			{
				$chm = array();
				
				// points as squares
				$chm[] = ( 's,' . $chart_colors[ $color_scheme ][1] . ',0,-1,' . POINT_SIZE . ',-1' );
				
				if ( $data_labels )
				{
					foreach ( $labels as $key => $val )
					{
						$chm[] = ( 'N*f2*,' . $chart_colors[ $color_scheme ]['font'] . ',0,' . $key . ',' . FONT_SIZE );
					}
				}
				
				$params['chm'] = implode( '|', $chm );
			}
		}
		
		return graphs_gen_url( GOOGLE_CHART_URL, $params );
	}
	
	// labels: array( label_name )
	// data: array( data_set_name => array( value ) )
	function graphs_line_chart_multi_url( $labels, $data, $width, $height, $min = null, $color_scheme = 'default' )
	{
		global $chart_colors;
		$params = array();
		
		// chart type
		$params['cht'] = 'lxy';
		
		// chart size
		$params['chs'] = ( $width . 'x' . $height );
		
		// colors
		$params['chco'] = implode( ',', array_slice( $chart_colors[ $color_scheme ], 0, count( $data ) ) );
		
		// legend
		$params['chdl'] = implode( '|', array_keys( $data ) );
		
		// axis type
		$params['chxt'] = 'x,y';
		
		// chart data
		{			
			$minmin = null;
			$maxmax = null;
			
			$data_sets = array();
			$data_scaling = array();
			
			// global data scaling info
			foreach ( $data as $val )
			{			
				$min_data = ( ( is_null( $min ) )?( floor( min( array_values( $val ) ) ) ):( $min ) );
				$max_data = ceil( max( array_values( $val ) ) );			
				
				$minmin = ( ( is_null( $minmin ) || ( $min_data < $minmin ) )?( $min_data ):( $minmin ) );
				$maxmax = ( ( is_null( $maxmax ) || ( $max_data > $maxmax ) )?( $max_data ):( $maxmax ) );
			}
			
			// data and scaling
			foreach ( $data as $val )
			{
				$data_sets[] = ( implode( ',', array_keys( $labels ) ) . '|' . implode( ',', $val ) );
				$data_scaling[] = ( '-1,' . ( count( $labels ) ) . ',' . $minmin . ',' . ceil( $maxmax * SCALING_CONST ) );
			}

			$params['chd'] = ( 't:' . implode( '|', $data_sets ) );
			$params['chds'] = ( implode( ',', $data_scaling ) );
			
			
			// x-axis labels
			$params['chxl'] = ( '0:||' . implode( '|', $labels ) . '|' );
			
			// y-axis range
			$params['chxr'] = ( '1,' . $minmin . ',' . ceil( $maxmax * SCALING_CONST ) );
			
			
			// label data points
			{
				$chm = array();
				
				// points as squares
				foreach ( array_values( $data ) as $key => $val )
				{
					$chm[] = ( 's,' . $chart_colors[ $color_scheme ][ $key ] . ',' . $key . ',-1,' . POINT_SIZE . ',-1' );
				}
				
				$params['chm'] = implode( '|', $chm );
			}
		}
		
		return graphs_gen_url( GOOGLE_CHART_URL, $params );
	}
	
	/////////////////////////////////
	/////////////////////////////////
	
	// input: array( 'x-label'=>y )
	function graphs_bar_chart_url( $data, $width, $height, $min = null, $color_scheme = 'default', $data_labels = true )
	{
		global $chart_colors;
		$params = array();
		
		// chart type
		$params['cht'] = 'bvs';
		
		// chart size
		$params['chs'] = ( $width . 'x' . $height );
		
		// auto-size
		$params['chbh'] = 'a';
		
		// colors
		$params['chco'] = implode( ',', array_slice( $chart_colors[ $color_scheme ], 0, count( $data ) ) );
		
		// axis type
		$params['chxt'] = 'x,y';
		$params['chxl'] = ( '0:|' . implode( '|', array_keys( $data ) ) . '|' );
		
		// chart data
		{
			$min_data = ( ( is_null( $min ) )?( floor( min( array_values( $data ) ) ) ):( $min ) );
			$max_data = ceil( max( array_values( $data ) ) );
			
			$temp_data = array();
			$temp_scaling = array();
			$temp_labels = array();
			$counter = 0;
			foreach ( array_values( $data ) as $key => $val )
			{
				$temp2 = array();
				
				foreach ( array_values( $data ) as $key2 => $val2 )
				{
					if ( $key2 == $counter )
					{
						$temp2[] = $val2;		
					}
					else
					{
						$temp2[] = 0;
					}
				}
				
				$temp_data[] = implode( ',', $temp2 );
				$temp_scaling[] = ( $min_data . ',' . ceil( $max_data * SCALING_CONST ) );
				$temp_labels[] = ( 'N*f2*,' . $chart_colors[ $color_scheme ][ 'font' ] . ',' . $counter . ',' . $counter . ',' . FONT_SIZE );
				
				$counter++;
			}
			
			$params['chd'] = ( 't:' . implode( '|', $temp_data ) );
			$params['chds'] = implode( ',', $temp_scaling );
			
			$params['chxr'] = ( '1,' . $min_data . ',' . ceil( $max_data * SCALING_CONST ) );
			
			if ( $data_labels )
			{
				$params['chm'] = implode( '|', $temp_labels );
			}
		}
		
		return graphs_gen_url( GOOGLE_CHART_URL, $params );
	}
	
	function graphs_bar_chart_multi_url( $labels, $data, $width, $height, $min = null, $color_scheme = 'default' )
	{
		global $chart_colors;
		$params = array();
		
		// chart type
		$params['cht'] = 'bvg';
		
		// chart size
		$params['chs'] = ( $width . 'x' . $height );
		
		// auto-size
		$params['chbh'] = 'a';
		
		// colors
		$params['chco'] = implode( ',', array_slice( $chart_colors[ $color_scheme ], 0, count( $data ) ) );
		
		// legend
		$params['chdl'] = implode( '|', array_keys( $data ) );
		
		// axis type
		$params['chxt'] = 'x,y';
		$params['chxl'] = ( '0:|' . implode( '|', $labels ) . '|' );
		
		// chart data
		{
			$minmin = null;
			$maxmax = null;
			
			$data_sets = array();
			$data_scaling = array();
			
			foreach ( $data as $val )
			{				
				$data_sets[] = implode( ',', $val );
				
				$min_data = ( ( is_null( $min ) )?( floor( min( array_values( $val ) ) ) ):( $min ) );
				$max_data = ceil( max( array_values( $val ) ) );			
				
				$minmin = ( ( is_null( $minmin ) || ( $min_data < $minmin ) )?( $min_data ):( $minmin ) );
				$maxmax = ( ( is_null( $maxmax ) || ( $max_data > $maxmax ) )?( $max_data ):( $maxmax ) );
			}
			
			foreach ( $data as $val )
			{
				$data_scaling[] = ( $minmin . ',' . ceil( $maxmax * SCALING_CONST ) );
			}
			
			$params['chd'] = ( 't:' . implode( '|', $data_sets ) );
			$params['chds'] = implode( ',', $data_scaling );
			
			$params['chxr'] = ( '1,' . $minmin . ',' . ceil( $maxmax * SCALING_CONST ) );
		}
		
		return graphs_gen_url( GOOGLE_CHART_URL, $params );
	}
	
?>
