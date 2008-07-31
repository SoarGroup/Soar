/***************************************************************************
 *   Copyright (C) 2005 by Tarek Taha                                      *
 *   tataha@eng.uts.edu.au                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "map.h"
#include <math.h>
#include <cassert>
#include <iostream>
using namespace std;


MAP::MAP(char * Mapname, double pixel_size, double map_size)
{
  	this->Mapname = Mapname;
  	this->pixel_size = pixel_size;
  	this->negate   = TRUE;
  	this->occ_grid = NULL;
	this->pixbuf = NULL;
  	this->size_x =  (int)ceil(map_size/pixel_size); // Square MAP
  	this->size_y = this->size_x + 1;
}
void MAP::ResetProb()
{
	if (!this->occ_grid)
	{
		cout<<"\n Occupancy Grid not Allocated Yet!!!";
		return;
	}
	for (int i=0; i < this->size_x; i++)
    	for(int j =0; j < this->size_y;j++)
    	{
    		occ_grid[i][j].prob_occ = occ_grid[i][j].prob_free = double(1)/3;
    		occ_grid[i][j].added =false;
    	}
}
GdkPixbuf * MAP::CreateMap()
{
  	g_type_init();// Initialize glib
	if (this->occ_grid)
	{
		for (int i=0; i < this->size_x; i++)
    		delete  [] this->occ_grid[i];
		delete [] this->occ_grid;
	}
	if (!this->pixbuf)
	{
  		cout<<"\n	--->>> Creating Image Pixel Buffer:"<<this->Mapname;
  		fflush(stdout);	
  		if(!(this->pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, this->size_x,this->size_y))) 	// Read the image
  		{
    			printf("\nfailed to Create Map Buffer %s", this->Mapname);
    			return(NULL);
  		}
  		gdk_pixbuf_fill (pixbuf, 0x7f7f7f00);
	}
  	assert(this->size_x == gdk_pixbuf_get_width(this->pixbuf));
	assert(this->size_y == gdk_pixbuf_get_height(this->pixbuf));
	if (!(this->occ_grid= new mapgrid * [size_x]))
		perror("\nCould not allocated the requested memory space");
	for(int m=0;m<size_x;m++)
	{
		if(!(this->occ_grid[m] = new mapgrid [size_y]))
			perror("\nCould not allocated the requested memory space");
	}
	ClearData();
  	printf("\n	--->>> MAP Created with Height=%d Width=%d Resolution=%.3f <<<---",this->size_y, this->size_x, this->pixel_size);
  	fflush(stdout);
  	return this->pixbuf;
}
void MAP::ClearData()
{
	for (int i=0; i < this->size_x; i++)
    	for(int j =0; j < this->size_y;j++)
    	{
    		this->occ_grid[i][j].prob_occ = this->occ_grid[i][j].prob_free = 1/3;
    	}
    if(pixbuf)
   		gdk_pixbuf_fill (pixbuf, 0x7f7f7f00);
}
MAP::~ MAP()
{
	for (int i=0; i < this->size_x; i++)
        	delete  [] this->occ_grid[i];
    delete [] this->occ_grid;
    if(this->pixbuf)
    	gdk_pixbuf_unref(this->pixbuf);
    cout <<"\n	<<<--- IMAGE DATA FREED <<<---";
    fflush(stdout); 
};
MapInfo MAP::GetMapInfo()
{
	MapInfo mapinfo={0};
	if(this->occ_grid == NULL)
		{
		mapinfo.pixel_size=-1;
		return mapinfo;
		}
  	mapinfo.width = this->size_x;
  	mapinfo.height =this->size_y;
	mapinfo.pixel_size=this->pixel_size;
	return mapinfo;
};
void MAP::DrawPixel (int red,int green,int blue,int i,int j)
{
	int rowstride=0, n_channels, bps;
  	guchar *pixels;
  	guchar * p;
  	rowstride = gdk_pixbuf_get_rowstride(this->pixbuf);
  	bps = gdk_pixbuf_get_bits_per_sample(this->pixbuf)/8;
  	n_channels = gdk_pixbuf_get_n_channels(this->pixbuf);
  	pixels = gdk_pixbuf_get_pixels(this->pixbuf);
  	if(gdk_pixbuf_get_has_alpha(this->pixbuf))
	  	n_channels++;
  	p= pixels +j*rowstride + i*n_channels;
  	p[0]=red;
  	p[1]=green;
  	p[2]=blue;
  	//p[3]=;
	  return;
}
void MAP::SavePixelBufferToFile()
{
	char command[40],filename[40];
	struct stat stat_buf;
  	if(!this->pixbuf)	
	{
		cout<<"		--->>> Nothing To SAVE Buffer Empty !!! ";
		return;
	}
  	sprintf(filename,"%s%s",this->Mapname,".png");
	// Delete the file if it exists
 	if (stat(filename,&stat_buf) != 0 || (stat_buf.st_mode & S_IFMT) == S_IFREG)
	{
		sprintf(command,"%s%s","rm -f -r ",filename); 
		if(system(command)==-1)
		{
			perror("\nError Happened while trying to Delete Existing File");
			exit(1);
		}
		else
			cout<<"\n	--->>> Map already existed with the same name : Deleted Successfully";
	}
	cout<<"\n	--->>> Saving the map into: "<<filename; fflush(stdout);
	// Save the file
	gdk_pixbuf_save(this->pixbuf,filename,"png",NULL,NULL);
  	cout<<"\n	--->>> PIXEL BUFFER SAVED <<<---	"; fflush(stdout);
};
// Save as PGM format
int MAP::SavePgm()
{
  char filename[40];
  int i, j;
  signed char c;
  unsigned char d;
  FILE *file;
  sprintf(filename,"%s%s",this->Mapname,".pgm");
  file = fopen(filename, "w+");
  if (file == NULL)
  {
    fprintf(stderr, "error writing to %s ", filename);
    return -1;
  }

  fprintf(file, "P5 %d %d 255\n",this->size_x , this->size_y -1);

  for (j = 0; j < this->size_y-1; j++)
  {
    for (i = 0; i < this->size_x; i++)
    {
	    c = (int)(occ_grid[i][j].prob_occ * 255.0);
	    d = (unsigned char) (255 - c);
        fwrite(&d, 1, 1,  file);
    }
  }
  cout<<"\n		--->>> Pgm Map Saved <<<--- "; fflush(stdout);
  fclose(file);
  return 0;
};
