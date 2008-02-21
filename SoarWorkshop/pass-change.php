<?php
/*
 * Created on Feb 20, 2008
 *
 * To change the template for this generated file go to
 * Window - Preferences - PHPeclipse - PHP - Code Templates
 */
	include 'protected.php';
?>

<?php
	include 'header.php';
?>

<p>Use the following form to reset your password</p>

<form action="update-registration.php" method="POST">

<p><TABLE id="Table1" cellSpacing="1" cellPadding="1" width="600" bgColor="silver" border="0">
	<TR>
		<TD>
			<P align="right">Old password:</P>
		</TD>
		<TD><input type="password" maxLength="50" size="50" name="oldpass">&nbsp;<FONT color="#ff0000">(required)</FONT></TD>
	</TR>
<?
	include 'pass-form.php';
?>
</TABLE></p>

<input type="submit" value="Submit" />

</form>

<?php
	include 'footer.php';
?>
