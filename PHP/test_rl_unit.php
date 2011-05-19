<?php
	
	/**
	 * Author: Nate Derbinsky
	 *
	 * Automatic RL unit testing (via the rl-unit demo agent, see README)
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
        $demo_agent = realpath( THIS_DIR . '/../share/soar/Demos/rl-unit.soar' );
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
    
    $gold = array();
    $gold[1] = array(
    'rl*value*function*2' => array( 'u'=>1, 'v'=>10 ),
    'rl*value*function*3' => array( 'u'=>1, 'v'=>10 ),
    'rl*value*function*4' => array( 'u'=>1, 'v'=>15 ),
    'rl*value*function*5' => array( 'u'=>1, 'v'=>10 ),
    'rl*value*function*6' => array( 'u'=>1, 'v'=>15 ),
    'rl*value*function*7' => array( 'u'=>1, 'v'=>10 ),
    'rl*value*function*8' => array( 'u'=>1, 'v'=>18.75 ),
    'rl*value*function*9' => array( 'u'=>1, 'v'=>10 ),
    );
    
    $gold[2] = array(
    'rl*value*function*2' => array( 'u'=>2, 'v'=>19.5 ),
    'rl*value*function*3' => array( 'u'=>2, 'v'=>19 ),
    'rl*value*function*4' => array( 'u'=>2, 'v'=>28.75 ),
    'rl*value*function*5' => array( 'u'=>2, 'v'=>19 ),
    'rl*value*function*6' => array( 'u'=>2, 'v'=>28.75 ),
    'rl*value*function*7' => array( 'u'=>2, 'v'=>19 ),
    'rl*value*function*8' => array( 'u'=>2, 'v'=>35.6875 ),
    'rl*value*function*9' => array( 'u'=>2, 'v'=>19 ),
    );
    
    $gold[3] = array(
    'rl*value*function*2' => array( 'u'=>3, 'v'=>28.5 ),
    'rl*value*function*3' => array( 'u'=>3, 'v'=>27.1 ),
    'rl*value*function*4' => array( 'u'=>3, 'v'=>41.35 ),
    'rl*value*function*5' => array( 'u'=>3, 'v'=>27.1 ),
    'rl*value*function*6' => array( 'u'=>3, 'v'=>41.35 ),
    'rl*value*function*7' => array( 'u'=>3, 'v'=>27.1 ),
    'rl*value*function*8' => array( 'u'=>3, 'v'=>50.9875 ),
    'rl*value*function*9' => array( 'u'=>3, 'v'=>27.1 ),
    );
    
    $gold[4] = array(
    'rl*value*function*2' => array( 'u'=>4, 'v'=>37.005 ),
    'rl*value*function*3' => array( 'u'=>4, 'v'=>34.39 ),
    'rl*value*function*4' => array( 'u'=>4, 'v'=>52.8925 ),
    'rl*value*function*5' => array( 'u'=>4, 'v'=>34.39 ),
    'rl*value*function*6' => array( 'u'=>4, 'v'=>52.8925 ),
    'rl*value*function*7' => array( 'u'=>4, 'v'=>34.39 ),
    'rl*value*function*8' => array( 'u'=>4, 'v'=>64.808125 ),
    'rl*value*function*9' => array( 'u'=>4, 'v'=>34.39 ),
    );
    
    $gold[5] = array(
    'rl*value*function*2' => array( 'u'=>5, 'v'=>45.024 ),
    'rl*value*function*3' => array( 'u'=>5, 'v'=>40.951 ),
    'rl*value*function*4' => array( 'u'=>5, 'v'=>63.463 ),
    'rl*value*function*5' => array( 'u'=>5, 'v'=>40.951 ),
    'rl*value*function*6' => array( 'u'=>5, 'v'=>63.463 ),
    'rl*value*function*7' => array( 'u'=>5, 'v'=>40.951 ),
    'rl*value*function*8' => array( 'u'=>5, 'v'=>77.29225 ),
    'rl*value*function*9' => array( 'u'=>5, 'v'=>40.951 ),
    );
    
    
    $large = 176; // happens to be the lower bound for eps=0.00001
    $gold[ $large ] = array(
    'rl*value*function*2' => array( 'u'=>$large, 'v'=>150 ),
    'rl*value*function*3' => array( 'u'=>$large, 'v'=>100 ),
    'rl*value*function*4' => array( 'u'=>$large, 'v'=>175 ),
    'rl*value*function*5' => array( 'u'=>$large, 'v'=>100 ),
    'rl*value*function*6' => array( 'u'=>$large, 'v'=>175 ),
    'rl*value*function*7' => array( 'u'=>$large, 'v'=>100 ),
    'rl*value*function*8' => array( 'u'=>$large, 'v'=>193.75 ),
    'rl*value*function*9' => array( 'u'=>$large, 'v'=>100 ),
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
        $result = compare_rules( $info, $agent_rules, true );
        
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
