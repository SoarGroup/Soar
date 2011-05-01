<?php
	
	/**
	 * Author: Nate Derbinsky
	 *
	 * If run via PHP-CLI, serves as a minimalist Soar CLI (with proof-of-concept events).
	 * If accessed from the web, just converts request variables to input-link structures.
	 */
	
	require '../lib/PHP_sml_ClientInterface.php';
	
	$kernel = Kernel::CreateKernelInCurrentThread( Kernel::GetDefaultLibraryName(), true, 0 );
	$agent = $kernel->CreateAgent('php');
	
	// terminal vs. web
	if ( isset( $argc ) )
	{
		function readline( $prompt = '' ) 
		{
			echo $prompt;
			
			$o = '';
			$c = '';
			while ( ( $c != "\r" ) && ( $c != "\n" ) ) 
			{
				$o .= $c;
				$c = fread( STDIN, 1 );
			}
			
			return $o;
		}
		
		function my_print_handler( $event_id, $user_data, $agent_name, $msg )
		{
			global $kernel;
			$agent = $kernel->GetAgent( $agent_name );
			
			echo ( $msg . "\n" );
		}
		
		function my_run_handler( $event_id, $user_data, $agent_name, $sml_phase )
		{
			global $kernel;
			$agent = $kernel->GetAgent( $agent_name );
			
			echo ( 'event: ' . $event_id . ', phase: ' . $sml_phase . "\n" );
		}
		
		function my_prod_handler( $event_id, $user_data, $agent_name, $prod_name, $inst_name )
		{
			global $kernel;
			$agent = $kernel->GetAgent( $agent_name );
			
			echo ( 'event: ' . $event_id . ', prod: ' . $prod_name . ', inst: ' . $inst_name . "\n" );
		}
		
		$print_event_id = $agent->RegisterForPrintEvent( smlEVENT_PRINT, 'my_print_handler', '' );
		//$run_event_id = $agent->RegisterForRunEvent( smlEVENT_AFTER_HALTED, 'my_run_handler', '' );
		//$prod_event_id = $agent->RegisterForProductionEvent( smlEVENT_AFTER_PRODUCTION_ADDED, 'my_prod_handler', '' );
		
		//
		
		$cmd = '';
		do
		{
			$cmd = readline('> ');
			
			if ( $cmd == 'silence' )
			{
				if ( !is_null( $print_event_id ) )
				{
					$agent->UnregisterForPrintEvent( $print_event_id );
					$print_event_id = null;
				}
			}
			else if ( $cmd == 'loud' )
			{
				if ( is_null( $print_event_id ) )
				{
					$print_event_id = $agent->RegisterForPrintEvent( smlEVENT_PRINT, 'my_print_handler', '' );
				}
			}
			else if ( $cmd != 'quit' )
			{
				echo ( $agent->ExecuteCommandLine( $cmd ) . "\n" );
			}
			
		} while ( $cmd != 'quit' );
	}
	else
	{
		//error_reporting( -1 );
		//ini_set( 'display_errors', 'On' );
		//ini_set( 'display_startup_errors', 'On' );
		
		$get = $agent->GetInputLink()->CreateIdWME( 'get' );
		$post = $agent->GetInputLink()->CreateIdWME( 'post' );
		
		foreach ( $_GET as $k => $v )
		{
			$get->CreateStringWME( $k, $v );
		}
		
		foreach ( $_POST as $k => $v )
		{
			$post->CreateStringWME( $k, $v );
		}
		
		$agent->ExecuteCommandLine( 'step' );
		echo nl2br( $agent->ExecuteCommandLine( 'p -d 10 <s>' ) );
	}
	
	$kernel->Shutdown();
?>
