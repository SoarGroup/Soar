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
?>
<?php
	include 'header.php';
?>

<form action="login.php" method="POST">
    
    Email address: <input type="text" name="email" value="<?php print $_SESSION['prev_email'] ?>"/><br />
    Password: <input type="password" name="pass" /><br />
    
    <input type="submit" value="Login" />
</form>

<p>Forgot your password? Email <a href="mailto:voigtjr@gmail.com">Jonathan</a> to reset it.</p>

<?php
	include 'footer.php';
?>
