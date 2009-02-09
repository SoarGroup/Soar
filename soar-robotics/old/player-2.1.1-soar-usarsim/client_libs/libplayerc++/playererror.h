/*
 *  Player - One Hell of a Robot Server
 *  Copyright (C) 2000-2003
 *     Brian Gerkey, Kasper Stoy, Richard Vaughan, & Andrew Howard
 *
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
 */
/********************************************************************
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ********************************************************************/

#ifndef PLAYERERROR_H
#define PLAYERERROR_H

#include <string>
#include <iostream>

namespace PlayerCc
{

/** @brief The C++ exception class
 *
 * When @em libplayerc++ receives an error from @em libplayerc
 * it throws a PlayerError exception.
 */
class PlayerError
{
  private:

    // a string describing the error
    std::string mStr;
    // a string describing the location of the error in the source
    std::string mFun;
    // error code returned by playerc
    int mCode;

  public:
    /// the error string
    std::string GetErrorStr() const { return(mStr); };
    /// the function that threw the error
    std::string GetErrorFun() const { return(mFun); };
    /// a numerical error code
    int GetErrorCode() const { return(mCode); };

    /// default constructor
    PlayerError(const std::string aFun="",
                const std::string aStr="",
                const int aCode=-1);
    /// default destructor
    ~PlayerError();
};

}

namespace std
{
std::ostream& operator << (std::ostream& os, const PlayerCc::PlayerError& e);
}

#endif
