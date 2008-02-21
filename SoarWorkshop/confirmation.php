<?php
/*
 * Created on Feb 20, 2008
 *
 * To change the template for this generated file go to
 * Window - Preferences - PHPeclipse - PHP - Code Templates
 */
    require_once 'lib.php';

    session_start();
	if ( !isset( $_SESSION['just_registered'] ) )
	{
	    header( 'Location: register.php' );
	    exit;
	}
	unset($_SESSION['just_registered']);
?>
<?php
	include 'header.php';
?>


<p>Thank you for registering for the Soar Workshop. A confirmation email has been sent to you.
If you do not receive the email, please let us know by sending a message to Jonathan 
(<a href="mailto:voigtjr@gmail.com">voigtjr@gmail.com</a>). You can return to this site 
at any time to review or change your registration information using
your email address and password.</p>

<p>If you are presenting, please be prepared to send a copy of your presentation(s) via email
before the workshop as we will be generating and providing proceedings for all workshop
attendees as well as archiving presentations on the web. Powerpoint and Adobe PDF formats 
are preferred.</p>

<p>If this is not possible, please bring a hardcopy of your slides to the dinner on the
evening of Sunday, May 4 printed single-sided with multiple slides per page (2, 4, or 6
depending on readability) to conserve space in the readings.</p>

<p><a href="logout.php">Click here to log out, </a> or 
<a href="verify.php">click here to review your registration infromation.</a></p>

<p><a href="index.php">Soar Workshop 28</a>.</p>
<p><a href="http://sitemaker.umich.edu/soar">Soar home page</a>.</p>

<?php
	include 'footer.php';
?>
