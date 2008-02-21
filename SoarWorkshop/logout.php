<?php

    session_start();
    
    unset( $_SESSION['login'] );
    
    $_SESSION['msg'] = 'You have been logged out!';
    header( 'Location: index.php');
?>