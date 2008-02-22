<?php
/*
 * Created on Feb 20, 2008
 *
 * To change the template for this generated file go to
 * Window - Preferences - PHPeclipse - PHP - Code Templates
 */
 
 	require_once 'lib.php';
 	
    session_start();

	if ( !isset( $_SESSION['login'] ) || !is_admin($_SESSION['email']) )
    {
    	header ('Location: index.php');
    	exit;
    }
    
    if ( isset( $_POST['delete'] ) )
    {
    	if ( $_POST['sure'] )
    	{
	    	//$_SESSION['msg'] = 'Would have deleted ' . $_POST['delete'] . '!';
			remove_by_id($_POST['delete']);  	
    	}
    	else 
    	{
    		$_SESSION['msg'] = 'You need to click "I\'m sure" to delete a record!';
    	}
    }
?>
<?php
	include 'header.php';
?>

<h3>Admin page</h3>

<p><table border='1'>
<tr>
<th>Name</th>
<th>Affiliation</th>
<th>Phone</th>
<th>Email</th>
<th>Time 1</th>
<th>Title 1</th>
<th bgcolor="silver">Time 2</th>
<th bgcolor="silver">Title 2</th>
<th>Time 3</th>
<th>Title 3</th>
<th>Other</th>
<th>Actions</th>
</tr>
<?php
	$all_emails = '';
	$total_time = 0;
	db_connect();
	$result = mysql_query("SELECT * FROM workshop28");
	$total_registered = mysql_num_rows($result);
	while ($reg = mysql_fetch_assoc($result)) {
		print '<tr style="font-size: x-small; font-family: sans-serif;">';
		print '<td>' . htmlentities($reg['name']) . '</td>';		
		print '<td>' . htmlentities($reg['affiliation']) . '</td>';		
		print '<td>' . htmlentities($reg['phone']) . '</td>';		
		print '<td><a href="mailto:' . htmlentities($reg['email']) . '">' . htmlentities($reg['email']) . '</a></td>';		
		print '<td>' . htmlentities($reg['pres1time']) . '</td>';		
		print '<td>' . htmlentities($reg['pres1title']) . '</td>';		
		print '<td bgcolor="silver">' . htmlentities($reg['pres2time']) . '</td>';		
		print '<td bgcolor="silver">' . htmlentities($reg['pres2title']) . '</td>';		
		print '<td>' . htmlentities($reg['pres3time']) . '</td>';		
		print '<td>' . htmlentities($reg['pres3title']) . '</td>';		
		print '<td>' . htmlentities($reg['other']) . '</td>';		
		print '<td><form action="admin.php" method="POST">' .
				'<input type="submit" value="Delete" /><br />' .
				'<input type="checkbox" name="sure" /> I\'m sure' .
				'<input type="hidden" name="delete" value="' . $reg['id'] . '"' .
				'</form></td>';
		print '</tr>';
		
		$all_emails .= htmlentities($reg['email']) . ', ';
		$total_time += intval($reg['pres1time']);
		$total_time += intval($reg['pres2time']);
		$total_time += intval($reg['pres3time']);
	}
	
?>
</table></p>

<p><table>
<tr>
<td><p>Total participants registered: <?php print $total_registered; ?></p>
</td>
<td>
</td>
</tr>
<tr>
<td><p>Total presentation time: <?php print $total_time; ?> minutes</p>
</td>
<td>
</td>
</tr>
</table></p>

<h3>All email addresses for cut-and-paste:</h3>

<?php
	print $all_emails;
	include 'footer.php';
?>
