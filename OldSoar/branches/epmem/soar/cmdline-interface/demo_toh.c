#include "demo_toh.h"

glb_info glbInfo;
FILE *moves;

#define PAUSE_CYCLE 10

void update( glb_info *g );
void printState( glb_info *g );
io_wme *get_output_wme( char *attribute, io_wme *head );

void toh_wait_cb(  agent *a, soar_callback_data data,
				   soar_call_data call_data ) {

  print( "*** Wait Callback Invoked. ***\n" );
  print( "    forcing environment to update\n" );
  glbInfo.forceUpdate = 1;


}

void toh_input_fn( agent *a, soar_callback_data data,
				   soar_call_data call_data ) {

  int pause = (int)data;

  switch ( (int) call_data ) {

  case TOP_STATE_JUST_CREATED:
    print( "\nTop state just created\n" );
	toh_initialize( 3 );
	moves = fopen( "toh-moves.mov", "w" );
    break;


  case NORMAL_INPUT_CYCLE:
    print( "\nNormal Input Cycle\n" );	
	if ( !pause || (glbInfo.cycle % PAUSE_CYCLE) == 0 || glbInfo.forceUpdate )
	  update( &glbInfo );

	printState( &glbInfo );
	glbInfo.cycle++;

	break;
  }
  
}




void toh_output_fn( agent *a, soar_callback_data data,
					soar_call_data call_data ) {

  io_wme *o_wme;
  char buff[10];
  glb_info *g;  
  int i;

  
  g = &glbInfo;



  switch( ((output_call_info *)call_data)->mode ) {
    
  case MODIFIED_OUTPUT_COMMAND:
  case ADDED_OUTPUT_COMMAND:
	
    /* 
     *  Just to show how the output-function is being invoked...
     */
    if ( ((output_call_info *)call_data)->mode == MODIFIED_OUTPUT_COMMAND ) {
      print ("\nA command on the output-link has been modified!\n" );
    }
    else if (((output_call_info *)call_data)->mode == ADDED_OUTPUT_COMMAND) {
      print ("\nA new command has been added to the output-link!\n" );
    }

	if ( !get_output_wme( "status",((output_call_info *)call_data)->outputs )){
	  
	  o_wme = get_output_wme( "move-disk", 
							  ((output_call_info *)call_data)->outputs );
	  
	  if ( o_wme == NULL )  return;
	  
	  soar_cGetWmeValue( o_wme, g->moveId, 5 );
	  
	  o_wme = get_output_wme( "size", 
							  ((output_call_info *)call_data)->outputs );
	  soar_cGetWmeValue( o_wme, buff, 20 );
	  g->cMoveDisk = atoi( buff );
	  
	  
	  o_wme = get_output_wme( "to", 
							  ((output_call_info *)call_data)->outputs );
	  soar_cGetWmeValue( o_wme, buff, 20 );

	
	  if ( buff[0] == '|' && buff[2] == '|' ) {
		buff[0] = buff[1];
		buff[1] = '\0';
	  }
	  fprintf( moves, "%ld : move-disk ( size = %d, to = %s )\n", 
			   current_agent(d_cycle_count), g->cMoveDisk, buff );
	  
	  g->cMovePeg = -1;
	  for ( i = 0; i < 3; i++ ) {
		if ( !strcmp( g->pegs[i], buff ) ) { g->cMovePeg = i; }
	  }
	
	  printf( "Got a move --> %d to peg # %d\n", g->cMoveDisk, g->cMovePeg );
	  //	  performMove( &glbInfo );
	}
  }
}

io_wme *get_output_wme( char *attribute, io_wme *head ) {
  
  char *buff;
    /*
     *  Cycle through all wmes which were added to the output-link
     */
  while ( head != NULL ) {

	buff = soar_cGetWmeAttr( head, NULL, 0 );
	if ( !strcmp( buff, attribute ) ) {
	  free( buff );
	  return head;
	}

	free( buff );
	head = head->next;
  } 
  return NULL;
}



int performMove ( glb_info *g ) {

  int udSz;


  if ( g->upper[g->disks[g->cMoveDisk].cOn] != &g->disks[g->cMoveDisk] ) {
	printf("Disk %d does not appear to be the upper disk\n", g->cMoveDisk);
	return -2;
  }


  if ( g->upper[g->cMovePeg] != NULL ) udSz = g->upper[g->cMovePeg]->cSz;
  else udSz = g->nDisks + 10;
  
  if ( udSz < g->cMoveDisk ) {
	printf( "The disk on the destination is too small (%d:%d)\n", udSz, g->cMoveDisk);
	return -1;
  }
  
  // make the disk below, the upper disk.
  g->upper[g->disks[g->cMoveDisk].cOn] = g->disks[g->cMoveDisk].cAbove;

  // make the disk to move the upper disk on the new peg
  g->upper[g->cMovePeg] = &g->disks[g->cMoveDisk];
  g->upper[g->cMovePeg]->cOn = g->cMovePeg;
  g->upper[g->cMovePeg]->on = g->pegs[g->cMovePeg];
  
  if ( udSz <= g->nDisks ) {
	g->upper[g->cMovePeg]->cAbove = &g->disks[udSz];
	sprintf( g->upper[g->cMovePeg]->above, "%d", udSz );
	return udSz;
  }
  else {
	g->upper[g->cMovePeg]->cAbove = NULL;
	sprintf( g->upper[g->cMovePeg]->above, "%d", 0 );
	return 0;
  }
  
  
}

void printState( glb_info *g ) {

  int h;
  int maxHeight;
  int i, j;
  disk *d;
  int heights[3];

  maxHeight = 0;
  for( i = 0; i < 3; i++ ) {

	d = g->upper[i];
	h = 0;
	while ( d != NULL ) { 
	  d = d->cAbove;
	  h++;
	}
	heights[i] = h;
	if ( h > maxHeight ) maxHeight = h;
  }
  printf( "\n" );
  for ( h = maxHeight; h > 0; h-- ) {
	
	for ( i = 0; i < 3; i++ ) {
	  printf( "        " );
	  if ( h <= heights[i] ) {
		j = heights[i] - h;
		d = g->upper[i];
		while( j > 0 ) {
		  d = d->cAbove;
		  j--;
		}
		printf( "%d", d->cSz );
	  }
	  else printf( " " );
	}
	printf( "\n" );
  }
  printf( "------------------------------------\n" );
}
  



void update( glb_info *g ) {
  
  int above;
  psoar_wme wme;

  printf( "Updating input link" );
  g->forceUpdate = 0;

  if ( g->cMoveDisk ) {
	above = performMove( g );
	
	if ( above == -1 ) {

	  printf( "Error you can't do such a move\n" );
	  return;
	}
	printf( "You are moving the disk above %d\n", above );
	soar_cRemoveWmeUsingTimetag( g->disks[g->cMoveDisk].onTT );
	soar_cRemoveWmeUsingTimetag( g->disks[g->cMoveDisk].aboveTT );
	
	g->disks[g->cMoveDisk].onTT = 
	  soar_cAddWme( g->disks[g->cMoveDisk].wmeID, "^on",
					g->disks[g->cMoveDisk].on, FALSE,  &wme );
	

	g->disks[g->cMoveDisk].aboveTT =
	  soar_cAddWme( g->disks[g->cMoveDisk].wmeID, "^above",
					g->disks[g->cMoveDisk].above, FALSE, &wme );

	soar_cAddWme( g->moveId, "^status", "complete", FALSE, &wme );

	/* Move complete */
	g->cMoveDisk = 0;

	
  }
}
void toh_initialize( int n ) {
  
  int i;
  psoar_wme wme;
  glb_info *g = &glbInfo;

  memset( g, sizeof( glb_info ), 0 );

  g->nDisks = n;
  strcpy( g->pegs[0], "A" );
  strcpy( g->pegs[1], "B" );
  strcpy( g->pegs[2], "C" );
  
  for ( i = 1; i <= n; i++ ) {
    

    g->disks[i].cOn = 0;
    g->disks[i].cSz = i;
    if ( g->disks[i].cSz < n ) g->disks[i].cAbove = &g->disks[i+1];
    else g->disks[i].cAbove = NULL;

    g->disks[i].on = g->pegs[g->disks[i].cOn];
    sprintf( g->disks[i].size, "%d", g->disks[i].cSz );
    
    if ( g->disks[i].cAbove ) 
      sprintf( g->disks[i].above, "%d",  i+1 );
    else sprintf( g->disks[i].above, "%d", 0 );

    soar_cAddWme( "I2", "^disk", "*", FALSE, &wme );
    soar_cGetWmeValue( wme, g->disks[i].wmeID, 6 );
    
    g->disks[i].onTT = 
      soar_cAddWme( g->disks[i].wmeID, "^on", g->disks[i].on, FALSE, &wme);
    
    g->disks[i].aboveTT =
      soar_cAddWme( g->disks[i].wmeID, "^above", g->disks[i].above, FALSE, &wme );

    soar_cAddWme( g->disks[i].wmeID, "^size", g->disks[i].size, FALSE, &wme );

  }

  
  for( i = 0; i < 3; i++ ) {
	soar_cAddWme( "I2", "^peg", g->pegs[i], FALSE, &wme );
	g->upper[i] = NULL;
  }
  g->upper[0] = &g->disks[1];
  soar_cAddWme( "I2", "^goal-peg", g->pegs[2], FALSE, &wme );


}

    
  
