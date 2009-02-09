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
#ifndef map_HEADER
#define map_HEADER

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <sys/stat.h>
class MapInfo
{
public :
	int height;
	int width;
	double pixel_size;
};
typedef struct mapgrid
{
	double prob_occ,prob_free;
	bool added;
} mapgrid_t;
class MAP
{
	private:
    		double pixel_size;
    		int negate;
  			GdkPixbuf * pixbuf;
	public:
	   		int size_x, size_y;
			mapgrid * * occ_grid;
    		char * Mapname;
			MapInfo    GetMapInfo();
			void SavePixelBufferToFile();
			void DrawPixel(int,int,int,int,int);
			void ClearData();
			void ResetProb();
			int  SavePgm();
			GdkPixbuf * CreateMap();
			~MAP();
			MAP(char*, double,double);
};
#endif
