<?php
/*
 * Created on Feb 19, 2008
 *
 * To change the template for this generated file go to
 * Window - Preferences - PHPeclipse - PHP - Code Templates
 */
    require_once 'lib.php';

	include 'protected.php';
    
    unset($_SESSION['oldpost']);
    $registration = load_registration( $_SESSION['email'] );
    if ($registration == 0)
    {
		$_SESSION['msg'] = 'Error loading registration data, please re-register.';
    	unset( $_SESSION['login'] );
		header('Location: register.php');
		exit;
    }
?>
<?php
	include 'header.php';
?>

<p><a href="pass-change.php">Click here</a> change your password using a separate form.</p>

<?php
	if (is_admin($_SESSION['email']))
	{
		print '<p><strong>Admin:</strong> <a href="admin.php">Click here</a> for registration reports.</p>';
	}
?>

<p>Update any information that you need to below, and then click submit to send us the data. 
Your registration was last updated on <?php print $registration['updated']; ?>.</p>

<?php
	include 'reg-form.php';
	include 'footer.php';
?>
