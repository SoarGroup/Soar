<?php

	{
		require 'common/private/start.inc.php';
		
		$page_info['title'] = 'Sample 1';
		$page_info['head'] = jquery_tabs( 'tabs' );
	}
	
?>

	<?php
		
		// get data
		$sql = '';
		$data = array( 'schema'=>array( 'conn'=>EXP_TYPE_STRING, 'dcs'=>EXP_TYPE_INT, 'avg_time'=>EXP_TYPE_DOUBLE, 'avg_rss'=>EXP_TYPE_DOUBLE ), 'table'=>array() );
		$exp_id = 5;
		{
			$sql = ( 'SELECT' . 
						' ' . exp_field_name( 'connection' ) . ' AS conn' . 
						', ' . exp_field_name( 'decisions' ) . ' AS dcs' . 
						', AVG(' . exp_field_name( 'time' ) . ') AS avg_time' . 
						', ( AVG(' . exp_field_name( 'rss' ) . ') / 1024 ) AS avg_rss' . 
			         ' FROM' . 
			         	' ' . exp_table_name( $exp_id ) .
			         ' WHERE' . 
			         	' ' . exp_field_name( 'machine' ) . '=' . db_quote_smart( 'macsoar', $db ) .
			         	' AND ' . exp_field_name( 'branch' ) . '=' . db_quote_smart( 'nlderbin-epmem-smem', $db ) .
			         	' AND ' . exp_field_name( 'revision' ) . '=' . db_quote_smart( 11570, $db ) .
			         ' GROUP BY' .
			         	' ' . exp_field_name( 'connection' ) .
			         	', ' . exp_field_name( 'decisions' ) .
			         ' ORDER BY' .
			         	' ' . exp_field_name( 'connection' ) . ' DESC' .
			         	', ' . exp_field_name( 'decisions' ) . ' ASC' );
			
			$res = mysql_query( $sql, $db );
			while ( $row = mysql_fetch_assoc( $res ) )
			{
				$data['table'][] = $row;
			}			         	
		}
		
		// specialize
		$cpp_cpu = array();
		$java_rss = array();
		$compare_cpu = array( 'labels'=>array(), 'data'=>array() );
		$compare_rss = array( 'labels'=>array(), 'data'=>array() );
		{
			// single line
			{
				foreach ( $data['table'] as $row )
				{
					if ( $row['conn'] == 'cpp' )
					{
						$cpp_cpu[ ( intval( $row['dcs'] ) / 1000 ) . 'k' ] = doubleval( $row['avg_time'] );
					}
				}
			}
			
			// single bar
			{
				foreach ( $data['table'] as $row )
				{
					if ( $row['conn'] == 'java' )
					{
						$java_rss[ ( intval( $row['dcs'] ) / 1000 ) . 'k' ] = doubleval( $row['avg_rss'] );
					}
				}
			}
			
			// multi line
			{
				foreach ( $data['table'] as $row )
				{
					// label
					if ( !isset( $compare_cpu['labels'][ $row['dcs'] ] ) )
					{
						$compare_cpu['labels'][ $row['dcs'] ] = ( ( intval( $row['dcs'] ) / 1000 ) . 'k' );
					}
					
					// data
					if ( !isset( $compare_cpu['data'][ $row['conn'] ] ) )
					{
						$compare_cpu['data'][ $row['conn'] ] = array();
					}
					$compare_cpu['data'][ $row['conn'] ][] = doubleval( $row['avg_time'] );
				}
				$compare_cpu['labels'] = array_values( $compare_cpu['labels'] );
			}
			
			// multi bar
			{
				foreach ( $data['table'] as $row )
				{
					if ( ( $row['dcs'] % 20000 ) == 0 )
					{
						// label
						if ( !isset( $compare_rss['labels'][ $row['dcs'] ] ) )
						{
							$compare_rss['labels'][ $row['dcs'] ] = ( ( intval( $row['dcs'] ) / 1000 ) . 'k' );
						}
						
						// data
						if ( !isset( $compare_rss['data'][ $row['conn'] ] ) )
						{
							$compare_rss['data'][ $row['conn'] ] = array();
						}
						$compare_rss['data'][ $row['conn'] ][] = doubleval( $row['avg_rss'] );
					}
				}
				$compare_rss['labels'] = array_values( $compare_rss['labels'] );
			}
		}
		
		if ( isset( $_GET['format'] ) && ( $_GET['format'] == 'csv' ) )
		{
			$page_info['type'] = 'blank';
			
			echo tables_csv( $data['schema'], $data['table'] );
		}
		else
		{
			echo '<div id="tabs">';
					
				echo '<ul>';
					echo '<li><a href="#tabs-1">CPP CPU Time (s)</a></li>';
					echo '<li><a href="#tabs-2">Java Memory Usage (MB)</a></li>';
					echo '<li><a href="#tabs-3">Compare CPU Time (s)</a></li>';
					echo '<li><a href="#tabs-4">Compare Memory Usage (MB)</a></li>';
					echo '<li><a href="#tabs-5">Table</a></li>';
					echo '<li><a href="#tabs-6">Query</a></li>';
				echo '</ul>';
				
				echo '<div id="tabs-1">';
					echo '<div class="section">';
						echo '<div class="body">';
							
							echo ( '<img src="' . htmlentities( graphs_line_chart_url( $cpp_cpu, 600, 400, 0, 'default', true ) ) . '" />' );
														
						echo '</div>';
					echo '</div>';
				echo '</div>';
				
				echo '<div id="tabs-2">';
					echo '<div class="section">';
						echo '<div class="body">';
							
							echo ( '<img src="' . htmlentities( graphs_bar_chart_url( $java_rss, 600, 400, 0, 'default', true ) ) . '" />' );
														
						echo '</div>';
					echo '</div>';
				echo '</div>';
				
				echo '<div id="tabs-3">';
					echo '<div class="section">';
						echo '<div class="body">';
							
							echo ( '<img src="' . htmlentities( graphs_line_chart_multi_url( $compare_cpu['labels'], $compare_cpu['data'], 600, 400, 0, 'default' ) ) . '" />' );
														
						echo '</div>';
					echo '</div>';
				echo '</div>';
				
				echo '<div id="tabs-4">';
					echo '<div class="section">';
						echo '<div class="body">';
							
							echo ( '<img src="' . htmlentities( graphs_bar_chart_multi_url( $compare_rss['labels'], $compare_rss['data'], 600, 400, 0, 'default' ) ) . '" />' );
														
						echo '</div>';
					echo '</div>';
				echo '</div>';
				
				echo '<div id="tabs-5">';
					echo '<div class="section">';
						echo '<div class="body">';
							echo tables_make_perty( $data['schema'], $data['table'] );
							echo ( '<a href="?format=csv">download as csv</a>' );
						echo '</div>';
					echo '</div>';
				echo '</div>';
				
				echo '<div id="tabs-6">';
					echo '<div class="section">';
						echo '<div class="body">';
							
							echo htmlentities( $sql );
							
						echo '</div>';
					echo '</div>';
				echo '</div>';
				
			echo '</div>';
		}
				
				
	?>

<?php
	
	{
		require 'common/private/end.inc.php';
	}

?>
