<?php
	if (strstr($_SERVER['PHP_SELF'], "lib.php") != false)
	{
		header ('Location: index.php');
		exit;	
	}
    
    function verify_name ( $name )
    {
    	return ( strlen( $name ) > 0 );
    }

    function verify_email ( $email )
    {
    	if ( strlen( $email ) == 0 ) 
    	{
    		return "Email address not provided.";
    	}
    	
    	if ( eregi ( "[A-Z0-9._%+-]+@[A-Z0-9.-]+\.[A-Z]{2,4}", $email ) )
    	{
    		return true;
    	}
    	return "Invalid email address.";
    }

    function is_email_unique ( $email )
    {
    	if (!db_connect())
    	{
    		return false;
    	}
    	$query = sprintf("SELECT email FROM workshop28 WHERE email=%s",
    		safe($email));
		$result = mysql_query($query);
		if ( mysql_num_rows($result) == 0 ) 
		{
    		return true;
		}
    	return false;
    }

    function verify_pass ( $pass1, $pass2 )
    {
    	if (strlen( $pass1 ) == 0 || strlen( $pass2 ) == 0) 
    	{
			return false;
    	}
    	
    	return $pass1 == $pass2;
    }
    
    function verify_pres ( $title, $duration )
    {
    	if (strlen( $title ) == 0 && strlen( $duration ) == 0)
    	{
    		return true;
    	}
    	if (strlen( $title ) == 0 || strlen( $duration ) == 0) 
    	{
			return false;
    	}
    	if ( ereg ( "[a-zA-Z]", $duration ) )
    	{
    		return false;
    	}
    	$duration = intval($duration);
    	if ( $duration < 5 )
   		{
   			return false;
   		}
    	if ( $duration > 60 )
   		{
   			return false;
   		}
   		return true;
    }
    
    function db_connect ()
    {
    	$username = 'soar';
    	$password = '9QuDL7zsdGwDQ2G6';
    	if (!mysql_connect('localhost', $username, $password))
    	{
    		return false;
    	}
    	return mysql_select_db('workshop');
    }
    
	function safe( $string ) {
		if (!strlen($string))
		{
			return "''";
		}
		
	   	return "'" . mysql_real_escape_string( $string ) . "'";
	}
    
    function auth ( $email, $pass )
    {
    	if (!db_connect())
    	{
    		return false;
    	}
    	$query = sprintf("SELECT password FROM workshop28 WHERE email=%s",
    		safe($email));
    	$result = mysql_query($query);
    	if (!$result) {
    		return false;
    	}
    	$row = mysql_fetch_assoc($result);
   		
    	return md5($pass) == $row['password']; 
    }
    
    function is_admin ( $email )
    {
    	if (!db_connect())
    	{
    		return false;
    	}
    	$query = sprintf("SELECT admin FROM workshop28 WHERE email=%s",
    		safe($email));
    	$result = mysql_query($query);
    	if (!$result) {
    		return false;
    	}
    	$row = mysql_fetch_assoc($result);
   		
    	return $row['admin'] == '1'; 
    }
    
    function load_registration ( $email )
    {
    	if (!db_connect())
    	{
    		return 0;
    	}
    	$query = sprintf("SELECT * FROM workshop28 WHERE email=%s",
    		safe($email));
    	$result = mysql_query($query);
    	if (!$result) {
	    	$query = sprintf("DELETE FROM workshop28 WHERE email=%s LIMIT 1", safe($email));
	    	mysql_query($query);
    		return 0;
    	}
    	$reg = mysql_fetch_assoc($result);
    	$reg['name'] = stripslashes($reg['name']);
    	$reg['affiliation'] = stripslashes($reg['affiliation']);
    	$reg['phone'] = stripslashes($reg['phone']);
    	$reg['pres1title'] = stripslashes($reg['pres1title']);
    	$reg['pres2title'] = stripslashes($reg['pres2title']);
    	$reg['pres3title'] = stripslashes($reg['pres3title']);
    	$reg['other'] = stripslashes($reg['other']);
    	return $reg;
    }
    
    function remove_by_id ( $id )
    {
		$query = sprintf("DELETE FROM workshop28 WHERE id=%s LIMIT 1", safe($id));
    	mysql_query($query);
    }
    
    function insert_registration ( $reg )
    {
    	if (!db_connect())
    	{
    		return false;
    	}
    	$query = sprintf("INSERT INTO workshop28 " .
    			"(name, affiliation, phone, email, password, paymethod, pres1time, pres1title, pres2time, pres2title, pres3time, pres3title, other, admin) " .
    			"VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, 0)",
	    		safe($reg['name']), 
	    		safe($reg['affiliation']), 
	    		safe($reg['phone']), 
	    		safe($reg['email']), 
	    		safe(md5($reg['pass1'])), 
	    		safe($reg['paymethod']), 
	    		safe($reg['pres1time']), 
	    		safe($reg['pres1title']), 
	    		safe($reg['pres2time']), 
	    		safe($reg['pres2title']), 
	    		safe($reg['pres3time']), 
	    		safe($reg['pres3title']), 
	    		safe($reg['other']) 
	    		);
    	$result = mysql_query($query);
    	if (!$result) {
    		return false;
    	}
    	return true;
    }
    
    function update_registration ( $reg )
    {
    	if (!db_connect())
    	{
    		return false;
    	}
    	$query = sprintf("UPDATE workshop28 " .
    			"SET name = %s, affiliation = %s, phone = %s, email = %s, paymethod = %s, pres1time = %s, pres1title = %s, pres2time = %s, pres2title = %s, pres3time = %s, pres3title = %s, other = %s " .
    			"WHERE email = %s",
	    		safe($reg['name']), 
	    		safe($reg['affiliation']), 
	    		safe($reg['phone']), 
	    		safe($reg['email']), 
	    		safe($reg['paymethod']), 
	    		safe($reg['pres1time']), 
	    		safe($reg['pres1title']), 
	    		safe($reg['pres2time']), 
	    		safe($reg['pres2title']), 
	    		safe($reg['pres3time']), 
	    		safe($reg['pres3title']), 
	    		safe($reg['other']), 
	    		safe($reg['email']) 
	    		);
    	$result = mysql_query($query);
    	if (!$result) {
    		return false;
    	}
    	return true;
    }

	function reg_change_email ( $email )
	{
		$body = "You are receiving this message because your registration information\n" .
				"was recently updated. You can check on your registration by logging\n" .
				"in to the workshop website here:\n" .
				"   http://winter.eecs.umich.edu/workshop/returning.php\n" .
				"\n" .
				"Soar Workshop 28 home:\n" .
				"   http://winter.eecs.umich.edu/workshop\n";
		$headers = "From: voigtjr@gmail.com";
		mail($email, "Soar Workshop 28 Registration Updated", $body, $headers);
	}
	
	function reg_add_email ( $email )
	{
    	$body = "This message confirms your registration for Soar Workshop 28.\n" .
			"If you need to change your registration information, you can at\n" .
			"the following website:\n" .
			"   http://winter.eecs.umich.edu/workshop/returning.php\n" .
			"\n" .
			"Soar Workshop 28 home:\n" .
			"   http://winter.eecs.umich.edu/workshop\n";
		$headers = "From: voigtjr@gmail.com";
		mail($email, "Soar Workshop 28 Registration Confirmation", $body, $headers);
	}
    
    function change_password ( $email, $pass )
    {
    	if (!db_connect())
    	{
    		return false;
    	}
    	$query = sprintf("UPDATE workshop28 " .
    			"SET password = %s WHERE email = %s",
	    		safe(md5($pass)), 
	    		safe($email) 
	    		);
    	$result = mysql_query($query);
    	if (!$result) {
    		return false;
    	}

    	return true;
    }
?>