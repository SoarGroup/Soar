<?php
	
	/**
	 * Author: Nate Derbinsky
	 *
	 * Automatic RL agent testing (via the left-right demo agent)
	 */
    
    define( 'THIS_DIR', dirname( realpath( $_SERVER['PHP_SELF'] ) ) );
    
    
    //
	
	require ( THIS_DIR . '/../lib/PHP_sml_ClientInterface.php' );
    require ( THIS_DIR . '/../lib/test_rl.inc.php' );
    
    //
	
	$kernel = Kernel::CreateKernelInCurrentThread( Kernel::GetDefaultLibraryName(), true, 0 );
	$agent = $kernel->CreateAgent('php');
	
	// source demo agent
    {
        $demo_agent = realpath( THIS_DIR . '/../share/soar/Demos/left-right.soar' );
        echo ( 'Sourcing... ' . $demo_agent );
        
        if ( file_exists( $demo_agent ) && is_readable( $demo_agent ) )
        {
            if ( !$agent->LoadProductions( $demo_agent ) )
            {
                echo ( 'Error while loading productions' . "\n" );
                exit;
            }
        }
        else
        {
            echo ( 'Cannot read agent source.' . "\n" );
            exit;
        }
        
        echo "\n";
    }
    
    $agent->ExecuteCommandLine( 'timers -d' );    
    
    //    
    
    $large = 1000;
    $gold[ $large ] = array(
    'left-right*rl*right' => array( 'v'=>1 ),
    'left-right*rl*left' => array( 'v'=>-1 ),
    );
    
    //
    
    $current_run = 0;
    
    foreach ( $gold as $run => $info )
    {
        while ( $current_run != $run )
        {
            $current_run++;
            $agent->ExecuteCommandLine( 'run' );
            $agent->ExecuteCommandLine( 'init' );
        }
        
        $agent_rules = parse_rl_rules( $agent );
        $result = compare_rules( $info, $agent_rules, false );
        
        echo ( 'Run #' . $current_run . ': ' . ( ( empty( $result ) )?('success'):('failure') ) . "\n" );
        
        if ( !empty( $result ) )
        {
            foreach ( $result as $reason )
            {
                echo ( ' ' . $reason . "\n" );
            }
            
            break;
        }
    }
	
	$kernel->Shutdown();
?>
