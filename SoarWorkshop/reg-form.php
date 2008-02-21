<?php
/*
 * Created on Feb 20, 2008
 *
 * To change the template for this generated file go to
 * Window - Preferences - PHPeclipse - PHP - Code Templates
 */

	if (strstr($_SERVER['PHP_SELF'], "reg-form.php") != false)
	{
		header ('Location: index.php');
		exit;	
	}
?>

<form action="update-registration.php" method="POST">
    
<p><TABLE id="Table1" cellSpacing="1" cellPadding="1" width="600" bgColor="silver" border="0">
	<TR>
		<TD>
			<P align="right">Name:</P>

		</TD>
		<TD><input maxLength="50" size="50" name="name" value="<?php print htmlentities($registration['name']); ?>" > <FONT color="#ff0000">(required)</FONT></TD>
	</TR>
	<TR>
		<TD>
			<P align="right">Affiliation:</P>
		</TD>

		<TD><input maxLength="50" size="50" name="affiliation" value="<?php print htmlentities($registration['affiliation']); ?>" ></TD>
	</TR>
	<TR>
		<TD>
			<P align="right">Phone:</P>
		</TD>
		<TD><input maxLength="25" size="20" name="phone" value="<?php print htmlentities($registration['phone']); ?>" ></TD>
	</TR>

	<TR>
		<TD>
			<P align="right">E-Mail:</P>
		</TD>
		<TD><input maxLength="50" size="50" name="email" value="<?php print htmlentities($registration['email']); ?>" >&nbsp;<FONT color="#ff0000">(required)</FONT></TD>
	</TR>
<?php
	if ( !isset( $_SESSION['login'] ) )
	{
		include 'pass-form.php';
	}
?>
</TABLE></p>

<hr>

<p>To help defray costs of the workshop, we require a $100 
(USD) registration fee per person (except for invited speakers).</p>

<p><TABLE id="Table3" cellSpacing="1" cellPadding="1" width="600" bgColor="silver" border="0">
	<TR>
	<td colspan="2">Please select one: <FONT color="#ff0000">(required)</FONT>
	</td>
	</tr>
	<TR>
		<TD vAlign="top" style="WIDTH: 15px"><input type="radio" value="onarrival" name="paymethod" ID="Radio10" <?php print $registration['paymethod'] == "onarrival" ? "checked" : ""; ?> ></TD>
		<TD>I will pay the <STRONG>$100</STRONG> (USD) registration fee at&nbsp;the 
			workshop.</TD>
	</TR>

	<TR>
		<TD vAlign="top" style="WIDTH: 15px"><input type="radio" value="invited" name="paymethod" ID="Radio11" <?php print $registration['paymethod'] == "invited" ? "checked" : ""; ?>  ></TD>
		<TD>I am an invited speaker (registration is free).</TD>
	</TR>
</TABLE></p>

<hr>

<p>We will give as many members of the community as possible the opportunity to 
describe their research or discuss the Soar issues that are of concern to them.</p>

<p><TABLE id="Table4" cellSpacing="1" cellPadding="1" width="600" bgColor="silver" border="0">
	<TR>
		<TD>I would like to give a presentation at the Soar Workshop:<br>
			Desired length of talk (usually 5 or 15 minutes): 
			<input id="Text1" maxLength="3" size="3" name="pres1time"  value="<?php if ($registration['pres1time'] == 0) { print ""; } else { print htmlentities($registration['pres1time']); } ?>" > minutes.<br>
			Title:<input id="Text2" maxLength="80" size="80" name="pres1title" value="<?php print htmlentities($registration['pres1title']); ?>" ></TD>
	</TR>
</TABLE></p>
<p><TABLE id="Table6" cellSpacing="1" cellPadding="1" width="600" bgColor="silver" border="0">
	<TR>
		<TD>I would like to give a second presentation at the Soar Workshop:<br>
			Desired length of talk (usually 5 or 15 minutes): 
			<input id="Text1" maxLength="3" size="3" name="pres2time" value="<?php if ($registration['pres2time'] == 0) { print ""; } else { print htmlentities($registration['pres2time']); } ?>" > minutes.<br>
			Title:<input id="Text4" maxLength="80" size="80" name="pres2title" value="<?php print htmlentities($registration['pres2title']); ?>" ></TD>
	</TR>

</TABLE></p>
<p><TABLE id="Table7" cellSpacing="1" cellPadding="1" width="600" bgColor="silver" border="0">
	<TR>
		<TD>I would like to give a third presentation at the Soar Workshop!<br>
			Desired length of talk (usually 5 or 15 minutes): 
			<input id="Text1" maxLength="3" size="3" name="pres3time" value="<?php if ($registration['pres3time'] == 0) { print ""; } else { print htmlentities($registration['pres3time']); } ?>" > minutes.<br>
			Title:<input id="Text6" maxLength="80" size="80" name="pres3title" value="<?php print htmlentities($registration['pres3title']); ?>" ></TD>
	</TR>
</TABLE></p>

<P>If you would like to do more than&nbsp;three presentations, put the information 
in the text block below.</P>
			
<hr>

<p>If you have additional information you would like to provide the 
workshop organizers (such as requests for special dietary considerations or 
requests for more than three presentation slots, etc), please enter it here:</p>

<p><textarea name="other" rows="10" wrap="soft" cols="72">
<?php print htmlentities($registration['other']); ?></textarea></p>

<input type="submit" value="Submit" />

</form>

