/*
 * ansi_colors.h
 *
 *  Created on: May 21, 2014
 *      Author: mazzin
 */

#ifndef ANSI_COLORS_H_
#define ANSI_COLORS_H_

namespace ANSI_Color_Constants
{

    // Reset
    static const char* Off = "\033[0m";               // Text Reset

    // Regular Colors
    static const char* Black = "\033[0;30m";                // Black
    static const char* Red = "\033[0;31m";                  // Red
    static const char* Green = "\033[0;32m";                // Green
    static const char* Yellow = "\033[0;33m";               // Yellow
    static const char* Blue = "\033[0;34m";                 // Blue
    static const char* Purple = "\033[0;35m";               // Purple
    static const char* Cyan = "\033[0;36m";                 // Cyan
    static const char* White = "\033[0;37m";                // White

    // Bold
    static const char* BBlack = "\033[1;30m";               // Black
    static const char* BRed = "\033[1;31m";                 // Red
    static const char* BGreen = "\033[1;32m";               // Green
    static const char* BYellow = "\033[1;33m";              // Yellow
    static const char* BBlue = "\033[1;34m";                // Blue
    static const char* BPurple = "\033[1;35m";              // Purple
    static const char* BCyan = "\033[1;36m";                // Cyan
    static const char* BWhite = "\033[1;37m";               // White

    // Underline
    static const char* UBlack = "\033[4;30m";               // Black
    static const char* URed = "\033[4;31m";                 // Red
    static const char* UGreen = "\033[4;32m";               // Green
    static const char* UYellow = "\033[4;33m";              // Yellow
    static const char* UBlue = "\033[4;34m";                // Blue
    static const char* UPurple = "\033[4;35m";              // Purple
    static const char* UCyan = "\033[4;36m";                // Cyan
    static const char* UWhite = "\033[4;37m";               // White

    // Background
    static const char* On_Black = "\033[40m";               // Black
    static const char* On_Red = "\033[41m";                 // Red
    static const char* On_Green = "\033[42m";               // Green
    static const char* On_Yellow = "\033[43m";              // Yellow
    static const char* On_Blue = "\033[44m";                // Blue
    static const char* On_Purple = "\033[45m";              // Purple
    static const char* On_Cyan = "\033[46m";                // Cyan
    static const char* On_White = "\033[47m";               // White

    // High Intensity
    static const char* IBlack = "\033[0;90m";               // Black
    static const char* IRed = "\033[0;91m";                 // Red
    static const char* IGreen = "\033[0;92m";               // Green
    static const char* IYellow = "\033[0;93m";              // Yellow
    static const char* IBlue = "\033[0;94m";                // Blue
    static const char* IPurple = "\033[0;95m";              // Purple
    static const char* ICyan = "\033[0;96m";                // Cyan
    static const char* IWhite = "\033[0;97m";               // White

    // Bold High Intensity
    static const char* BIBlack = "\033[1;90m";              // Black
    static const char* BIRed = "\033[1;91m";                // Red
    static const char* BIGreen = "\033[1;92m";              // Green
    static const char* BIYellow = "\033[1;93m";             // Yellow
    static const char* BIBlue = "\033[1;94m";               // Blue
    static const char* BIPurple = "\033[1;95m";             // Purple
    static const char* BICyan = "\033[1;96m";               // Cyan
    static const char* BIWhite = "\033[1;97m";              // White

    // High Intensity backgrounds
    static const char* On_IBlack = "\033[0;100m";           // Black
    static const char* On_IRed = "\033[0;101m";             // Red
    static const char* On_IGreen = "\033[0;102m";           // Green
    static const char* On_IYellow = "\033[0;103m";          // Yellow
    static const char* On_IBlue = "\033[0;104m";            // Blue
    static const char* On_IPurple = "\033[0;105m";          // Purple
    static const char* On_ICyan = "\033[0;106m";            // Cyan
    static const char* On_IWhite = "\033[0;107m";           // White

//    static const char* Off = "";
//
//    // Regular Colors
//    static const char* Black = "";
//    static const char* Red = "";
//    static const char* Green = "";
//    static const char* Yellow = "";
//    static const char* Blue = "";
//    static const char* Purple = "";
//    static const char* Cyan = "";
//    static const char* White = "";
//
//    // Bold
//    static const char* BBlack = "";
//    static const char* BRed = "";
//    static const char* BGreen = "";
//    static const char* BYellow = "";
//    static const char* BBlue = "";
//    static const char* BPurple = "";
//    static const char* BCyan = "";
//    static const char* BWhite = "";
//
//    // Underline
//    static const char* UBlack = "";
//    static const char* URed = "";
//    static const char* UGreen = "";
//    static const char* UYellow = "";
//    static const char* UBlue = "";
//    static const char* UPurple = "";
//    static const char* UCyan = "";
//    static const char* UWhite = "";
//
//    // Background
//    static const char* On_Black = "";
//    static const char* On_Red = "";
//    static const char* On_Green = "";
//    static const char* On_Yellow = "";
//    static const char* On_Blue = "";
//    static const char* On_Purple = "";
//    static const char* On_Cyan = "";
//    static const char* On_White = "";
//
//    // High Intensity
//    static const char* IBlack = "";
//    static const char* IRed = "";
//    static const char* IGreen = "";
//    static const char* IYellow = "";
//    static const char* IBlue = "";
//    static const char* IPurple = "";
//    static const char* ICyan = "";
//    static const char* IWhite = "";
//
//    // Bold High Intensity
//    static const char* BIBlack = "";
//    static const char* BIRed = "";
//    static const char* BIGreen = "";
//    static const char* BIYellow = "";
//    static const char* BIBlue = "";
//    static const char* BIPurple = "";
//    static const char* BICyan = "";
//    static const char* BIWhite = "";
//
//    // High Intensity backgrounds
//    static const char* On_IBlack = "";
//    static const char* On_IRed = "";
//    static const char* On_IGreen = "";
//    static const char* On_IYellow = "";
//    static const char* On_IBlue = "";
//    static const char* On_IPurple = "";
//    static const char* On_ICyan = "";
//    static const char* On_IWhite = "";
}
#endif /* ANSI_COLORS_H_ */
