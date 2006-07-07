/*
    This file is part of Sorts, an interface between Soar and ORTS.
    (c) 2006 James Irizarry, Sam Wintermute, and Joseph Xu

    Sorts is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Sorts is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Sorts; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA    
*/
#ifndef FSM_H
#define FSM_H

#include<map>
#include<string>
#include<list>
#include<vector>

#include "general.h"
#include "GameObj.H"
#include "SoarAction.h"

class Sorts;

class FSM{
public:
  FSM(GameObj *);
  virtual ~FSM();

  virtual int update()=0;

  GameObj* getGob(){return gob;}
  virtual void init(std::vector<sint4>);
  virtual ObjectActionType getName();
  virtual void panic();
  virtual void stop() { } // default stop behavior does nothing

protected:
  ObjectActionType name;
  GameObj *gob;
  Vector<sint4> params;
};


#define FSM_RUNNING 0
#define FSM_SUCCESS 1
#define FSM_FAILURE 2
#define FSM_STUCK 3
#define FSM_UNREACHABLE 4

#endif
