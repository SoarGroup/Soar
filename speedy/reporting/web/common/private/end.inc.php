<?php
	
	// get page content
	$page_info['content'] = ob_get_contents();
	ob_clean();
	
	// get the template
	$template = '';
	
	if ( $page_info['type'] == 'full' )
	{
		require 'template.inc.php';
		$template = ob_get_clean();
	}
	else if ( $page_info['type'] == 'blank' )
	{
		$template = '{content}';
	}
	ob_end_clean();
	
	// replace values in the template
	$page_info['dash_title'] = ( ( strlen( $page_info['title'] ) )?( '- ' . $page_info['title'] ):( '' ) );
	foreach ( $page_info as $key => $val )
	{
		$template = str_replace( ( '{' . $key . '}' ), $val, $template );
	}
		
	// output the page
	echo trim( $template );
	
	mysql_close( $db );
?>
