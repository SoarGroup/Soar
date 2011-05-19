<?php
    
    /**
	 * Author: Nate Derbinsky
	 *
	 * Some common code to automate testing 
     * of RL agents
	 */
    
    
    // returns an array: rule_name => array( 'u'=>num_updates, 'v'=>value )
    function parse_rl_rules( &$agent )
    {
        $rules = array();
        
        $str = explode( "\n", trim( $agent->ExecuteCommandLine( 'p --rl' ) ) );
        foreach ( $str as $s )
        {
            $s = explode( ' ', $s );
            foreach ( $s as $k => $v )
            {
                if ( !strlen( $v ) )
                {
                    unset( $s[ $k ] );
                }
            }
            $s = array_values( $s );
            
            $rules[ $s[0] ] = array( 'u'=>intval( $s[1] ), 'v'=>doubleval( $s[2] ) );
        }
        
        return $rules;
    }
    
    //
    
    // returns true if values are within an episolon (defined within)
    function compare_values( $v1, $v2 )
    {
        $epsilon = 0.00001;
        
        return ( ( $v1 > ( $v2 - $epsilon ) ) && ( $v1 < ( $v2 + $epsilon ) ) );
    }
    
    // inputs two parsed rule sets: gold standard and test set
    // - only checks rules in gold (i.e. may be subset of test)
    // - checks for rule existence, value, and, optionally, update count
    //
    // returns empty array if all is well, otherwise rule_name => error_string
    function compare_rules( &$gold, &$test, $check_updates )
    {
        $return_val = array();
        
        foreach ( $gold as $rule => $info )
        {
            if ( isset( $test[ $rule ] ) )
            {
                if ( !$check_updates || ( $info['u'] == $test[ $rule ]['u'] ) )
                {
                    if ( !compare_values( $info['v'], $test[ $rule ]['v'] ) )
                    {
                        $return_val[] = ( 'Rule "' . $rule . '": value should be ' . $info['v'] . ', is ' . $test[ $rule ]['v'] );
                    }
                }
                else if ( $check_updates )
                {
                    $return_val[] = ( 'Rule "' . $rule . '": update should be ' . $info['u'] . ', is ' . $test[ $rule ]['u'] );
                }
            }
            else
            {
                $return_val[] = ( 'Does not contain rule "' . $rule . '"' );
            }
        }
        
        return $return_val;
    }
?>
