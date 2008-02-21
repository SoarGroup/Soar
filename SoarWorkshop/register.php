<?php
/*
 * Created on Feb 19, 2008
 *
 * To change the template for this generated file go to
 * Window - Preferences - PHPeclipse - PHP - Code Templates
 */
    session_start();
	if ( isset( $_SESSION['login'] ) )
    {
   		header('Location: verify.php');
    	exit;
    }
    
    $registration = $_SESSION['oldpost'];
    unset($_SESSION['oldpost']);
?>
<?php
	include 'header.php';
?>
<p> 
<a href="returning.php">Click here to log in</a> if you already registered and would like to view or update your registration information.</p>

<?php
	include 'reg-form.php';
	include 'footer.php';
?>