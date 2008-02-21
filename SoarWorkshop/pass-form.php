<?php
/*
 * Created on Feb 20, 2008
 *
 * To change the template for this generated file go to
 * Window - Preferences - PHPeclipse - PHP - Code Templates
 */

	if (strstr($_SERVER['PHP_SELF'], "pass-form.php") != false)
	{
		header ('Location: index.php');
		exit;	
	}
?>

	<TR>
		<TD>
			<P align="right">Password:</P>
		</TD>
		<TD><input type="password" maxLength="50" size="50" name="pass1">&nbsp;<FONT color="#ff0000">(required)</FONT></TD>
	</TR>
	<TR>
		<TD>
			<P align="right">Confirm password:</P>
		</TD>
		<TD><input type="password" maxLength="50" size="50" name="pass2">&nbsp;<FONT color="#ff0000">(required)</FONT></TD>
	</TR>
