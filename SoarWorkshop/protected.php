<?php
/*
 * Created on Feb 21, 2008
 *
 * To change the template for this generated file go to
 * Window - Preferences - PHPeclipse - PHP - Code Templates
 */
	if (strstr($_SERVER['PHP_SELF'], "protected.php") != false)
	{
		header ('Location: index.php');
		exit;	
	}

    session_start();
    if ( !isset( $_SESSION['login'] ) )
    {
	    $_SESSION['msg'] = 'You must login to view this page!';
	    header('Location: returning.php');
	    exit;
    }
?>
