<?php

	{
		require 'common/private/start.inc.php';
		
		$page_info['title'] = 'Reports';
	}
	
?>

	<?php
		$exps = exp_list();
		$reports = array(
			
			5 => array(
				'test_report1.php' => 'Sample 1',
				'test_report2.php' => 'Sample 2',
				'test_report3.php' => 'Sample 3',
				'test_report4.php' => 'Sample 4',
			)
			
		);
		
		foreach ( $exps as $exp_id => $exp_name )
		{
			if ( isset( $reports[ $exp_id ] ) )
			{
				echo '<div class="section">';
					echo '<div class="title">';
						echo htmlentities( $exp_name );
					echo '</div>';
					echo '<div class="body">';
						echo '<ul>';
							foreach ( $reports[ $exp_id ] as $file_name => $report_name )
							{
								echo '<li><a href="' . htmlentities( $file_name ) . '">' . htmlentities( $report_name ) . '</a></li>';
							}
						echo '</ul>';
					echo '</div>';
				echo '</div>';
			}
		}
	?>

<?php
	
	{
		require 'common/private/end.inc.php';
	}

?>
