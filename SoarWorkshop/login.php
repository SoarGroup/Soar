<?php
/*
 * Created on Feb 21, 2008
 *
 * To change the template for this generated file go to
 * Window - Preferences - PHPeclipse - PHP - Code Templates
 */
    require_once 'lib.php';

    session_start();

    if ( !isset( $_SESSION['login'] ) )
    {
		if ( isset( $_POST['email'] ) &&
		     isset( $_POST['pass'] ))
		{
			
		    $_SESSION['prev_email'] = $_POST['email'];
		    
			if ( auth( $_POST['email'], $_POST['pass'] ) )
			{
				$_SESSION['login'] = true;
				$_SESSION['email'] = $_POST['email'];
			}
			else 
			{
			    $_SESSION['msg'] = 'Login incorrect.';
			    header('Location: returning.php');
			    exit;
			}
		}
				 
		if ( !isset( $_SESSION['login'] ) )
		{
			header('Location: returning.php');
			exit;
		}
    }
    
	header('Location: verify.php');
	exit;
?>
