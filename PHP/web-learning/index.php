<?php
	
	require ( 'PHP_sml_ClientInterface.php' );
	
	define( 'RL_FILE', 'rl.soar' );
	
	$params = array( 'jug1'=>5, 'jug2'=>3, 'target'=>4, 'watch'=>0, 'rseed'=>0 );
	foreach ( $params as $param => $default )
	{
		$$param = $default;
		
		if ( isset( $_GET[ $param ] ) )
		{
			$$param = intval( $_GET[ $param ] );
		}
	}
?>

<html>
	<head>
		<title>Server-side Water-Jug</title>
	</head>
	<body>
		<h1>Server-side Water-Jug</h1>
		<h2>Problem Instance</h2>
		<p>
			Enter details of your problem instance here:
			<form method="GET" action="<?php echo htmlentities($_SERVER['PHP_SELF']); ?>">
				Jug #1 Volume: <input name="jug1" type="text" size="2" maxlength="2" value="<?php echo htmlentities( $jug1 ); ?>" />
				<br />
				Jug #2 Volume: <input name="jug2" type="text" size="2" maxlength="2" value="<?php echo htmlentities( $jug2 ); ?>" />
				<br />
				Target Volume (jug #1): <input name="target" type="text" size="2" maxlength="2" value="<?php echo htmlentities( $target ); ?>" />
				<br /><br />
				Watch level: <select name="watch"><option value="0" <?php echo (($watch==0)?('selected="selected"'):('')); ?>>0</option><option value="1" <?php echo (($watch==1)?('selected="selected"'):('')); ?>>1</option><option value="3" <?php echo (($watch==3)?('selected="selected"'):('')); ?>>3</option><option value="5" <?php echo (($watch==5)?('selected="selected"'):('')); ?>>5</option></select>
				<br />
				Random seed (0=none): <input name="rseed" type="text" value="<?php echo htmlentities( $rseed ); ?>" />
				<br />
				Reset RL: <input name="reset_rl" type="checkbox" value="Y" />
				<br />
				<input type="submit" />
			</form>
		</p>

		<h2>Result</h2>
		<p>

<?php
	if ( ( $jug1>0 ) && ( $jug2>0 ) && ( $target>=0 ) && ( $target<=$jug1 ) )
	{
	
		// create kernel/agent
		$kernel = Kernel::CreateKernelInCurrentThread( Kernel::GetDefaultLibraryName(), true, 0 );
		$agent = $kernel->CreateAgent('water-jug');
		
		// capture trace
		$print_event_id = $agent->RegisterForPrintEvent( smlEVENT_PRINT, 'my_print_handler', '' );
		$agent->ExecuteCommandLine( 'w ' . $watch );
		if ( $rseed != 0 )
		{
			$agent->ExecuteCommandLine( 'srand ' . $rseed );
		}
							   
		if ( isset( $_GET['reset_rl'] ) && ( $_GET['reset_rl'] == 'Y' ) )
		{
			unlink( RL_FILE );
		}
		
		// source rules
		$agent->LoadProductions('agent.soar');
		$agent->LoadProductions( RL_FILE );
		
		$trace = '';
		function my_print_handler( $event_id, $user_data, $agent_name, $msg )
		{
			global $kernel;
			global $trace;
			
			$trace .= $msg;
		}
		
		
		$get = $agent->GetInputLink()->CreateIdWME( 'get' );
		$post = $agent->GetInputLink()->CreateIdWME( 'post' );
		
		$get->CreateIntWME( 'jug1', $jug1 );
		$get->CreateIntWME( 'jug2', $jug2 );
		$get->CreateIntWME( 'target', $target );
		
		$agent->ExecuteCommandLine( 'run' );
		$dcs = $agent->GetDecisionCycleCounter();
							   
		$agent->ExecuteCommandLine( 'command-to-file ' . RL_FILE . ' print --rl --full' );
		
		$kernel->Shutdown();
?>
			<span style="font-weight: bold">Decisions:</span> <?php echo htmlentities( $dcs ); ?>
			<br /><br />

			<span style="font-weight: bold">Trace:</span>
			<br />
			<textarea rows="10" cols="60" readonly="readonly"><?php echo htmlentities( $trace ); ?></textarea>
		</p>
<?php
	}
	else
	{
		echo 'Invalid problem instance';
	}
?>
		</p>
	</body>
</html>
