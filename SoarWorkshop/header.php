<?php
/*
 * Created on Feb 19, 2008
 *
 * To change the template for this generated file go to
 * Window - Preferences - PHPeclipse - PHP - Code Templates
 */
	if (strstr($_SERVER['PHP_SELF'], "header.php") != false)
	{
		header ('Location: index.php');
		exit;	
	}

session_start();

?>
<html>
<head>
  <meta content="text/html; charset=ISO-8859-1"
 http-equiv="content-type">
  <title>Soar Workshop 28</title>
</head>
<body>
<a id="Top"></a>
<table style="text-align: left; width: 100%;" border="0"
 cellpadding="2" cellspacing="2">
  <tbody>
    <tr>
      <td style="width: 150px;"><a href="/workshop"><img
 style="width: 135px; height: 135px;" alt="Soar 2008"
 src="soar2008.jpg" border=0 ></a></td>
      <td><big style="font-weight: bold;"><big><big>
Soar Workshop 28</big></big></big>
      <br>
      <big><big>May 5-7, 2008
      <br>
Ann Arbor, MI</big></big>
	  <br>
	  <a href="/workshop">Home</a> &nbsp;&nbsp;
	  <a href="register.php">Registration</a> &nbsp;&nbsp;
<?php
	if (isset($_SESSION['login']))
	{
		print '<a href="logout.php">Logout</a>';
	}
?>
      </td>
    </tr>
  </tbody>
</table>

<div style="color: red">
    <?php
        if ( isset( $_SESSION['msg'] ) )
        {
            print ( $_SESSION['msg'] );
            unset( $_SESSION['msg'] );
        }
    ?>
</div>




