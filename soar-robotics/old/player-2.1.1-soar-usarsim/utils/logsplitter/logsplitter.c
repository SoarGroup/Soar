/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2006 Radu Bogdan Rusu (rusu@cs.tum.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * CVS: $Id: logsplitter.c 6566 2008-06-14 01:00:19Z thjc $
 */
/*
 Desc  : Splits log files into smaller chunks, based on the difference between 
         consecutive timestamp entries. A bit ugly, but it works. There's room 
	 for optimization, so feel free.
 Author: Radu Bogdan Rusu
 Date  : 20th of September, 2006
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>

// Splits a logfile and returns the new file handle
FILE 
*copySplitData (FILE *input, double t2, long before)
{
    char data[1024];
    char fileName[80];
    double t1;
    FILE *output;
    FILE *rest;
    struct stat fbuf;
    long lastdatalen;

    // Save current position in file
    long currentPos = ftell (input);
    
    // Seek to the beginning of the file
    fseek (input, 0L, SEEK_SET);
    
    // Read the first entry
    while (!feof (input))
    {
	fgets (data, 1024, input);
	if (strncmp (data, "##", 2) != 0)
	{
	    sscanf (data, "%lf", &t1);
	    break;
	}
    }
    memset (data, 0, 1024);

    sprintf (fileName, "%lf-split.log", t1);

    // Seek to the beginning of the file
    fseek (input, 0L, SEEK_SET);

    // Create output file

    if (stat (fileName, &fbuf) != 0)
    {
        output = fopen (fileName, "w+");
        printf ("I: Creating... %s\n", fileName);
	
	// Copy the relevant data
	while (!feof (input))
	{
	    fgets (data, 1024, input);
	    if (ftell (input) >= currentPos)
		break;
	    fputs (data, output);
	}
        
	// Seek back in the source file
	fseek (input, currentPos - strlen (data), SEEK_SET);
    
	// Close output file
	fclose (output);
    }
    else
    {
        printf ("W: %s already exists!\n", fileName);
	// Seek back in the source file
	fseek (input, currentPos - before, SEEK_SET);
    }
    
    // Create a new file and copy the remainings there
    sprintf (fileName, "%lf-split.log", t2);
    if (stat (fileName, &fbuf) != 0)
    {
	rest = fopen (fileName, "w+");
	printf ("I: Creating... %s\n", fileName);	
	while (1)
	{
	    fgets (data, 1024, input);
	    lastdatalen = strlen (data);
	    if (feof (input)) 
		break;
    	    fputs (data, rest);
	}
	//fseek (rest, 0L, SEEK_SET);
	fflush (rest);

	// Truncate the remainings from the source file
        ftruncate (fileno (input), currentPos - before);
	fclose (input);
	
	return rest;
    }
    else
    {
	fseek (input, 0L, SEEK_SET);
        printf ("W: %s already exists!\n", fileName);
	return input;
    }
}

int
main (int argc, const char **argv)
{
    char buf[1024];
    double t1, t2;
    FILE *fd, *tempfd;
    long before, after;
    char btime[26];
    const char *base_filename;
    float min_timedifference;
    struct stat fbuf;
    struct stat ftempbuf;
    
    // We need 2 parameters
    if (argc != 3)
    {
	printf ("\n"
                " logsplitter - split a log file into smaller chunks, based on the consecutive timestamp difference\n\n"
                "USAGE:  logsplitter [min_time_difference_in_seconds] [FILE]\n\n");
	return -1;
    }
    
    // Get the minimum time difference between two consecutive timestamps
    min_timedifference = atof (argv[1]);
    base_filename = argv[2];
    
    printf ("I: Minimum time difference is: %f seconds.\n", min_timedifference);
    
    // Open file for reading
    fd = fopen (base_filename, "r+");
    if (!fd)
    {
        printf ("E: Opening %s ...[failed]\n", base_filename);
	return -1;
    }
    else
        printf ("I: Opening %s ...[success]\n", base_filename);

    stat (base_filename, &fbuf);

    // Create a temporary file
    if ((tempfd = tmpfile ()) == NULL)
    {
	printf ("E: Cannot create a temporary file! Aborting...\n");
	return -1;
    }
    
    // Copy the content of our logfile to that temporary file
    while (1)
    {
	fgets (buf, 1024, fd);
        if (feof (fd)) 
    	    break;
    	fputs (buf, tempfd);
    }
    fflush (tempfd);
    
    // Close the original logfile
    fclose (fd);

    fstat (fileno (tempfd), &ftempbuf);

    if ((fbuf.st_size - ftempbuf.st_size) != 0)
    {
	printf ("E: The temporary file differs than the original log file by %ld bytes! Aborting...",
	    fbuf.st_size - ftempbuf.st_size);
    }
    
    // Get an initial reading
    rewind (tempfd);
    while (!feof (tempfd))
    {
	fgets (buf, 1024, tempfd);
	if (strncmp (buf, "##", 2) == 0)
	    printf ("> ignoring: %s", buf);
	else
	{
	    sscanf (buf, "%lf", &t1);
	    break;
	}
    }

    // Stop when end of file is reached
    while (!feof (tempfd))
    {
	before = ftell (tempfd);

	fgets (buf, 1024, tempfd);
	if (strncmp (buf, "##", 2) == 0)
	    printf ("> ignoring: %s", buf);
	else
	    sscanf (buf, "%lf", &t2);

	after  = ftell (tempfd);
	
	// Verify if we need a break
	if ((fabs (t2 - t1) > min_timedifference) && (after - before > 0))
	{
	    time_t t = (time_t)t1;
    	    ctime_r (&t, btime);
	    printf ("I: Break (%f) needed after T = %f -> %s", t2 - t1, t1, btime);

	    tempfd = copySplitData (tempfd, t2, after - before);
	    if (ftell (tempfd) == 0)
		break;
	    rewind (tempfd);
	}
	t1 = t2;
    }

    // Close file
    fclose (tempfd);
    
    return 0;
}
