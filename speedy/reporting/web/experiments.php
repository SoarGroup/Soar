<?php

	{
		require 'common/private/start.inc.php';
		
		$page_info['title'] = 'Experiments';
		
		{
			$add_field_js = array();
			$add_field_js[] = '<script type="text/javascript">';
			$add_field_js[] = 'num_fields = 1;';
			$add_field_js[] = 'function add_field() {';
			$add_field_js[] = '$("#schema > tbody:last").append("<tr><td><input type=\"text\" name=\"field_name[" + num_fields + "]\" value=\"\" style=\"width: 200px\" /></td><td><select name=\"field_type[" + num_fields + "]\" style=\"width: 100px\"><option value=\"' . htmlentities( EXP_TYPE_INT ) . '\">' . htmlentities( exp_type_2_english( EXP_TYPE_INT ) ) . '</option><option value=\"' . htmlentities( EXP_TYPE_DOUBLE ) . '\">' . htmlentities( exp_type_2_english( EXP_TYPE_DOUBLE ) ) . '</option><option value=\"' . htmlentities( EXP_TYPE_STRING ) . '\">' . htmlentities( exp_type_2_english( EXP_TYPE_STRING ) ) . '</option></select></td></tr>");';
			$add_field_js[] = 'num_fields++;';
			$add_field_js[] = '}';
			$add_field_js[] = '</script>';
			
			$page_info['head'] = ( jquery_tabs( 'tabs' ) . implode( "\n", $add_field_js ) );
		}
	}
	
?>
	
		<?php
			// respond to commands
			$show_forms = true;
			{
				if ( isset( $_GET['cmd'] ) )
				{
					if ( $_GET['cmd'] === 'drop' )
					{
						if ( isset( $_GET['exp'] ) )
						{
							if ( exp_valid( $_GET['exp'] ) )
							{
								exp_remove_experiment( $_GET['exp'] );
								
								echo '<div class="section">';
									echo '<div class="body">';
										echo ( 'REMOVED EXPERIMENT: ' . htmlentities( $_GET['exp'] ) );
									echo '</div>';
								echo '</div>';
							}
						}
					}
					else if ( $_GET['cmd'] === 'clear' )
					{
						if ( isset( $_GET['exp'] ) )
						{
							if ( exp_valid( $_GET['exp'] ) )
							{
								exp_clear_data( $_GET['exp'] );
								
								echo '<div class="section">';
									echo '<div class="body">';
										echo ( 'CLEARED EXPERIMENT: ' . htmlentities( $_GET['exp'] ) );
									echo '</div>';
								echo '</div>';
							}
						}
					}
					else if ( $_GET['cmd'] == 'add' )
					{
						if ( isset( $_GET['exp'] ) && isset( $_GET['field_name'] ) && isset( $_GET['field_type'] ) )
						{
							if ( is_array( $_GET['field_name'] ) && is_array( $_GET['field_type'] ) && ( count( $_GET['field_name'] ) == count( $_GET['field_type'] ) ) )
							{
								$exp_fields = array();
								{
									foreach ( $_GET['field_name'] as $key => $val )
									{
										if ( isset( $_GET['field_type'][ $key ] ) )
										{
											$exp_fields[ trim( strval( $_GET['field_name'][ $key ] ) ) ] = intval( $_GET['field_type'][ $key ] );
										}
									}
								}
								
								if ( count( $exp_fields ) == count( $_GET['field_name'] ) )
								{
									$exp_id = exp_add_experiment( strval( $_GET['exp'] ), $exp_fields );
									
									echo '<div class="section">';
										echo '<div class="body">';
									
											if ( is_null( $exp_id ) )
											{
												echo ( 'DUPLICATE EXPERIMENT NAME' );
											}
											else
											{
												echo ( 'ADDED EXPERIMENT: ' . htmlentities( $exp_id ) );
											}
										echo '</div>';
									echo '</div>';
								}
							}
						}
					}
					else if ( $_GET['cmd'] == 'data' )
					{
						if ( isset( $_GET['exp_id'] ) )
						{
							$exp_id = intval( $_GET['exp_id'] );
							
							if ( exp_valid( $exp_id ) )
							{
								$schema = exp_schema( $exp_id );
								$values = array();
								
								foreach ( $schema as $field_name => $field_type )
								{
									if ( isset( $_GET[ $field_name ] ) )
									{
										$val = $_GET[ $field_name ];
										
										switch ( $field_type )
										{
											case EXP_TYPE_INT: $val = intval( $val ); break;
											case EXP_TYPE_DOUBLE: $val = doubleval( $val ); break;
											case EXP_TYPE_STRING: $val = strval( $val ); break;
										}
										
										$values[ $field_name ] = $val;
									}
								}
								
								if ( count( $schema ) == count( $values ) )
								{
									$res = exp_add_datum( $exp_id, $values );
									
									echo '<div class="section">';
										echo '<div class="body">';
											if ( !is_null( $res ) )
											{
												echo ( 'ADDED DATUM ' . htmlentities( $res ) . ' TO EXPERIMENT ' . htmlentities( $exp_id ) );
											}
											else
											{
												echo 'INVALID DATA';
											}
										echo '</div>';
									echo '</div>';
								}
							}
						}
					}
					else if ( $_GET['cmd'] == 'view' )
					{
						if ( isset( $_GET['exp'] ) )
						{
							$exp_id = intval( $_GET['exp'] );
							
							if ( exp_valid( $exp_id ) )
							{
								$show_forms = false;								
								
								// format
								$format = 'html';
								if ( isset( $_GET['format'] ) )
								{
									if ( $_GET['format'] == 'csv' )
									{
										$format = 'csv';
									}
								}
								
								// data
								$exp_data = exp_data( $exp_id, ( ( isset( $_GET['qry'] ) )?( db_strip_slashes( strval( $_GET['qry'] ) ) ):( NULL ) ) );
								
								if ( $format == 'html' )
								{
									$exps = exp_list();
									$page_info['title'] = $exps[ $exp_id ];									
									$go_chart = NULL;
									
									echo '<div class="section">';
										echo '<div class="title">';
											echo ( 'Query' );
										echo '</div>';
										echo '<div class="body">';
										
											if ( !is_null( $exp_data['err'] ) )
											{
												echo '<p style="color: red">';
													echo htmlentities( $exp_data['err'] );
												echo '</p>';
											}
											else
											{
												if ( count( $exp_data['schema'] ) == 2 )
												{
													if ( ( isset( $exp_data['schema']['x'] ) ) && ( isset( $exp_data['schema']['y'] ) ) )
													{
														$go_chart = 'Line';
													}
													else if ( ( isset( $exp_data['schema']['bin'] ) ) && ( isset( $exp_data['schema']['y'] ) ) )
													{
														$go_chart = 'Bar';
													}
												}
											}
											
											echo '<ul>';
												echo '<li>' . htmlentities( '{table} is replaced with the experiment data table' ) . '</li>';
												echo '<li>' . htmlentities( '{primary} is replaced with the experiment data primary key' ) . '</li>';
												echo '<li>' . htmlentities( '{field_*} is replaced with a field name' ) . '</li>';
												echo '<li>' . htmlentities( 'two columns with names x,y => line chart' ) . '</li>';
												echo '<li>' . htmlentities( 'two columns with names bin,y => bar chart' ) . '</li>';
											echo '</ul>';
										
											echo '<form method="get" action="">';
												echo '<input type="hidden" name="cmd" value="view" />';
												echo ( '<input type="hidden" name="exp" value="' . htmlentities( $exp_id ) . '" />' );
												
												echo ( '<textarea name="qry" cols="60" rows="4">' . htmlentities( $exp_data['sql'] ) . '</textarea>' );
												echo '<br />';
												echo '<input type="submit" value="query" />';
											echo '</form>';
										echo '</div>';
									echo '</div>';
									
									echo '<div id="tabs">';
				
										echo '<ul>';
											echo '<li><a href="#tabs-1">Table</a></li>';
											echo ( ( is_null( $go_chart ) )?(''):( '<li><a href="#tabs-2">' . htmlentities( $go_chart ) . ' Chart</a></li>' ) );
										echo '</ul>';								
									
										echo '<div id="tabs-1">';
										
											echo '<div class="section">';
												echo '<div class="title">';
													echo ( 'Query Result (<a href="?' . htmlentities( $_SERVER['QUERY_STRING'] ) . '&amp;format=csv' . '">csv</a>)' );
												echo '</div>';
												echo '<div class="body">';									
													echo tables_make_perty( $exp_data['schema'], $exp_data['data'] );
												echo '</div>';
											echo '</div>';
											
										echo '</div>';
										
										if ( !is_null( $go_chart ) )
										{										
											echo '<div id="tabs-2">';
											
												if ( $go_chart == 'Bar' )
												{
													$chart_data = array();
													
													foreach ( $exp_data['data'] as $row )
													{
														$chart_data[ $row['bin'] ] = $row['y'];
													}
													
													echo '<div class="section">';
														echo '<div class="body">';
															echo '<img src="' . htmlentities( graphs_bar_chart_url( $chart_data, 600, 400, 0, 'default', true ) ) . '" />';
														echo '</div>';
													echo '</div>';
												}
												else if ( $go_chart == 'Line' )
												{
													$chart_data = array();
													
													foreach ( $exp_data['data'] as $row )
													{
														$chart_data[ $row['x'] ] = $row['y'];
													}
													
													echo '<div class="section">';
														echo '<div class="body">';
															echo '<img src="' . htmlentities( graphs_line_chart_url( $chart_data, 600, 400, 0, 'default', true ) ) . '" />';
														echo '</div>';
													echo '</div>';
												}
												
											echo '</div>';
										}
										
									echo '</div>';
								}
								else if ( $format == 'csv' )
								{
									$page_info['type'] = 'blank';
									
									echo tables_csv( $exp_data['schema'], $exp_data['data'] );
								}
							}
						}
					}
				}
			}
		?>
		
		<?php
			if ( $show_forms )
			{
				$exps = exp_list();
				
				echo '<div id="tabs">';
				
					echo '<ul>';
						echo '<li><a href="#tabs-1">Existing</a></li>';
						echo '<li><a href="#tabs-2">New</a></li>';
					echo '</ul>';
					
					echo '<div id="tabs-1">';
				
						if ( empty( $exps ) )
						{
							echo '<div class="section">';
								echo '<div class="body">';
									echo 'No experiments!';
								echo '</div>';
							echo '</div>';
						}
						else
						{
							foreach ( $exps as $exp_id => $exp_name )
							{
								echo '<div class="section">';
									echo '<div class="title">';
										echo ( htmlentities( $exp_id . ': ' . $exp_name ) . ( ', ' . htmlentities( exp_data_size( $exp_id ) ) ) . ' (<a href="?cmd=view&amp;exp=' . htmlentities( $exp_id ) . '">view</a>, <a href="?cmd=clear&amp;exp=' . htmlentities( $exp_id ) . '">clear</a>, <a href="?cmd=drop&amp;exp=' . htmlentities( $exp_id ) . '">drop</a>)' );
									echo '</div>';
									echo '<div class="body">';
										
										echo '<form method="get" action="">';
											echo ( '<input type="hidden" name="cmd" value="data" />' );
											echo ( '<input type="hidden" name="exp_id" value="' . htmlentities( $exp_id ) . '" />' );
										
											echo '<table class="perty">';
												echo '<thead>';
													echo '<tr>';
														echo '<td style="width: 200px">Field</td>';
														echo '<td style="width: 80px">Type</td>';
														echo '<td style="width: 80px"></td>';
													echo '</tr>';
												echo '</thead>';
											
												echo '<tbody>';
												
													$schema = exp_schema( $exp_id );
													foreach ( $schema as $field_name => $field_type )
													{
														echo '<tr>';
															echo ( '<td>' . htmlentities( $field_name ) . '</td>' );
															echo ( '<td>' . htmlentities( exp_type_2_english( $field_type ) ) . '</td>' );
															echo ( '<td><input type="text" name="' . htmlentities( $field_name ) . '" value="" style="width: 70px" /></td>' );
														echo '</tr>';
													}
													
													echo '<td colspan="2"></td>';
													echo '<td><input type="submit" value="add" /></td>';
													
												echo '</tbody>';
											
											echo '</table>';
										echo '</form>';
										
									echo '</div>';
								echo '</div>';
							}
						}
					echo '</div>';
					
					echo '<div id="tabs-2">';
						echo '<form method="get" action="">';
							
							echo '<div class="section">';
								echo '<div class="body">';
									echo ( 'Experiment name: &nbsp;&nbsp; <input type="text" name="exp" value="" style="width: 250px" />' );
								echo '</div>';
							echo '</div>';
							
							echo '<div class="section">';
								echo '<div class="title">';
									echo 'Schema';
								echo '</div>';
								echo '<div class="body">';
									echo '<table class="perty" id="schema">';
										echo '<thead>';
											echo '<tr>';
												echo '<td style="width: 250px">Field</td>';
												echo '<td style="width: 150px">Type</td>';
											echo '</tr>';
										echo '</thead>';
										echo '<tbody>';
											echo '<tr>';
												echo '<td><input type="text" name="field_name[0]" value="" style="width: 200px" /></td>';
												echo '<td><select name="field_type[0]" style="width: 100px">';
													echo '<option value="' . htmlentities( EXP_TYPE_INT ) . '">' . htmlentities( exp_type_2_english( EXP_TYPE_INT ) ) . '</option>';
													echo '<option value="' . htmlentities( EXP_TYPE_DOUBLE ) . '">' . htmlentities( exp_type_2_english( EXP_TYPE_DOUBLE ) ) . '</option>';
													echo '<option value="' . htmlentities( EXP_TYPE_STRING ) . '">' . htmlentities( exp_type_2_english( EXP_TYPE_STRING ) ) . '</option>';
												echo '</select></td>';
											echo '</tr>';
										echo '</tbody>';
									echo '</table>';
								echo '</div>';
							echo '</div>';
							
							echo '<div class="section">';
								echo '<div class="body">';
							
									echo '<input type="hidden" name="cmd" value="add" />';
									echo '<input type="button" value="add field" onclick="add_field(); return false;" /> &nbsp;&nbsp; <input type="submit" value="save" />';
									
								echo '</div>';
							echo '</div>';
							
						echo '</form>';
					echo '</div>';
					
				echo '</div>';
			}
		?>
		
<?php
	
	{
		require 'common/private/end.inc.php';
	}

?>
