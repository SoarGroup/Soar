<?php
    require_once 'lib.php';
    
    session_start();

    $_SESSION['msg'] = '';
    $_SESSION['oldpost'] = $_POST;
    
    if (isset($_POST['oldpass']))
    {
    	// using password change form
    	if ( !auth($_SESSION['email'], $_POST['oldpass']) )
    	{
	    	$_SESSION['msg'] .= 'Old password incorrect.';
    		
    	}
    	else if ( !verify_pass( $_POST['pass1'], $_POST['pass2'] ) )
	    {
	    	$_SESSION['msg'] .= 'New passwords missing or do not match.';
	    }
	    else
	    {
	    	// success
	    	change_password( $_SESSION['email'], $_POST['pass1'] );
			reg_change_email($_SESSION['email']);

			header('Location: verify.php');
	    	exit;
	    }
	    
	    header('Location: pass-change.php');
	    exit;
    }
    
    $_POST['name'] = trim( $_POST['name'] );
    if ( !verify_name( $_POST['name'] ) )
    {
    	$_SESSION['msg'] .= 'Name is required.<br>';
    }

    $_POST['affiliation'] = trim( $_POST['affiliation'] );
    $_POST['phone'] = trim( $_POST['phone'] );

    $_POST['email'] = trim( $_POST['email'] );
    $emailResult = verify_email( $_POST['email'] );
    if ( is_string( $emailResult ) )
    {
    	$_SESSION['msg'] .= $emailResult . '<br>';
    }
    
    if ( !isset($_SESSION['login']))
    {
	    if ( !is_email_unique( $_POST['email'] ) )
	    {
	    	$_SESSION['msg'] .= 'Email address already exists in database.<br>';
	    }
	    if ( !verify_pass( $_POST['pass1'], $_POST['pass2'] ) )
	    {
	    	$_SESSION['msg'] .= 'Passwords missing or do not match.<br>';
	    }
    }
        
    if ( $_POST['paymethod'] == '' )
    {
    	$_SESSION['msg'] .= 'Please select a pay method.<br>';
    }
    
    $_POST['pres1title'] = trim( $_POST['pres1title'] );
    $_POST['pres2title'] = trim( $_POST['pres2title'] );
    $_POST['pres3title'] = trim( $_POST['pres3title'] );
    
    if ( !verify_pres( $_POST['pres1title'], $_POST['pres1time'] ) )
    {
    	$_SESSION['msg'] .= 'Invalid presentation 1 information.<br>';
    }
    if ( !verify_pres( $_POST['pres2title'], $_POST['pres2time'] ) )
    {
    	$_SESSION['msg'] .= 'Invalid presentation 2 information.<br>';
    }
    if ( !verify_pres( $_POST['pres3title'], $_POST['pres3time'] ) )
    {
    	$_SESSION['msg'] .= 'Invalid presentation 3 information.<br>';
    }
    
    if ( strlen( $_SESSION['msg'] ) ) {
    	
    	if ( isset( $_SESSION['login'] ) )
    	{
    		header('Location: verify.php');
    	}
    	else 
    	{
	   		header('Location: register.php');
    	}
   		exit;
    }
    
	// success, update
	
    if ( !isset( $_SESSION['login'] ) )
    {
    	// insert new row
		if (!insert_registration( $_POST )) 
		{
			$_SESSION['msg'] = 'Error saving registration data.';
		   	header('Location: register.php');
		   	exit;
		}

	    $_SESSION['login'] = true;
    	$_SESSION['email'] = $_POST['email'];
    	$_SESSION['just_registered'] = true;
    	
		reg_add_email($_POST['email']);

		header('Location: confirmation.php');
		exit;
    }
    else 
    {
    	// update existing row
		if (!update_registration( $_POST ))
		{
			$_SESSION['msg'] = 'Error saving registration data: ' . mysql_error();
		   	include 'register.php';
		   	exit;
		}

		reg_change_email($_POST['email']);
		
    	$_SESSION['msg'] = 'Your workshop registration has been updated!';
    	header('Location: verify.php');
    	exit;
    }
?>