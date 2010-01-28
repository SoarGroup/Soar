<?php
	session_start();
	
	// required libraries
	require_once 'db.inc.php';
	require_once 'experiment.inc.php';
	require_once 'tables.inc.php';
	require_once 'graphs.inc.php';
	require_once 'misc.inc.php';
	
	// jquery
	function jquery_tabs( $id )
	{
		return '<script type="text/javascript">$(function() {$("#' . $id . '").tabs(); });</script>';
	}
	
	// constants
	define( 'SYSTEM_URL', 'http://domain/path/to/speedy/' );
	
	$page_info = array();
	$page_info['system'] = SYSTEM_URL;
	
	$page_info['title'] = '';
	$page_info['align'] = 'left';
	$page_info['head'] = '';
	
	$page_info['nav'] = 'hola - <a href="experiments.php">experiments</a>';
		
	// currently supported: full, blank
	$page_info['type'] = 'full';
	
	ob_start();
?>
