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

#ifndef _WIN32
    // Reset
    static const char* Off = "\e[0m";               // Text Reset
    
    // Regular Colors
    static const char* Black = "\e[0;30m";                // Black
    static const char* Red = "\e[0;31m";                  // Red
    static const char* Green = "\e[0;32m";                // Green
    static const char* Yellow = "\e[0;33m";               // Yellow
    static const char* Blue = "\e[0;34m";                 // Blue
    static const char* Purple = "\e[0;35m";               // Purple
    static const char* Cyan = "\e[0;36m";                 // Cyan
    static const char* White = "\e[0;37m";                // White
    
    // Bold
    static const char* BBlack = "\e[1;30m";               // Black
    static const char* BRed = "\e[1;31m";                 // Red
    static const char* BGreen = "\e[1;32m";               // Green
    static const char* BYellow = "\e[1;33m";              // Yellow
    static const char* BBlue = "\e[1;34m";                // Blue
    static const char* BPurple = "\e[1;35m";              // Purple
    static const char* BCyan = "\e[1;36m";                // Cyan
    static const char* BWhite = "\e[1;37m";               // White
    
    // Underline
    static const char* UBlack = "\e[4;30m";               // Black
    static const char* URed = "\e[4;31m";                 // Red
    static const char* UGreen = "\e[4;32m";               // Green
    static const char* UYellow = "\e[4;33m";              // Yellow
    static const char* UBlue = "\e[4;34m";                // Blue
    static const char* UPurple = "\e[4;35m";              // Purple
    static const char* UCyan = "\e[4;36m";                // Cyan
    static const char* UWhite = "\e[4;37m";               // White
    
    // Background
    static const char* On_Black = "\e[40m";               // Black
    static const char* On_Red = "\e[41m";                 // Red
    static const char* On_Green = "\e[42m";               // Green
    static const char* On_Yellow = "\e[43m";              // Yellow
    static const char* On_Blue = "\e[44m";                // Blue
    static const char* On_Purple = "\e[45m";              // Purple
    static const char* On_Cyan = "\e[46m";                // Cyan
    static const char* On_White = "\e[47m";               // White
    
    // High Intensity
    static const char* IBlack = "\e[0;90m";               // Black
    static const char* IRed = "\e[0;91m";                 // Red
    static const char* IGreen = "\e[0;92m";               // Green
    static const char* IYellow = "\e[0;93m";              // Yellow
    static const char* IBlue = "\e[0;94m";                // Blue
    static const char* IPurple = "\e[0;95m";              // Purple
    static const char* ICyan = "\e[0;96m";                // Cyan
    static const char* IWhite = "\e[0;97m";               // White
    
    // Bold High Intensity
    static const char* BIBlack = "\e[1;90m";              // Black
    static const char* BIRed = "\e[1;91m";                // Red
    static const char* BIGreen = "\e[1;92m";              // Green
    static const char* BIYellow = "\e[1;93m";             // Yellow
    static const char* BIBlue = "\e[1;94m";               // Blue
    static const char* BIPurple = "\e[1;95m";             // Purple
    static const char* BICyan = "\e[1;96m";               // Cyan
    static const char* BIWhite = "\e[1;97m";              // White
    
    // High Intensity backgrounds
    static const char* On_IBlack = "\e[0;100m";           // Black
    static const char* On_IRed = "\e[0;101m";             // Red
    static const char* On_IGreen = "\e[0;102m";           // Green
    static const char* On_IYellow = "\e[0;103m";          // Yellow
    static const char* On_IBlue = "\e[0;104m";            // Blue
    static const char* On_IPurple = "\e[0;105m";          // Purple
    static const char* On_ICyan = "\e[0;106m";            // Cyan
    static const char* On_IWhite = "\e[0;107m";           // White
#else
    
    static const char* Off = "";
    
    // Regular Colors
    static const char* Black = "";
    static const char* Red = "";
    static const char* Green = "";
    static const char* Yellow = "";
    static const char* Blue = "";
    static const char* Purple = "";
    static const char* Cyan = "";
    static const char* White = "";
    
    // Bold
    static const char* BBlack = "";
    static const char* BRed = "";
    static const char* BGreen = "";
    static const char* BYellow = "";
    static const char* BBlue = "";
    static const char* BPurple = "";
    static const char* BCyan = "";
    static const char* BWhite = "";
    
    // Underline
    static const char* UBlack = "";
    static const char* URed = "";
    static const char* UGreen = "";
    static const char* UYellow = "";
    static const char* UBlue = "";
    static const char* UPurple = "";
    static const char* UCyan = "";
    static const char* UWhite = "";
    
    // Background
    static const char* On_Black = "";
    static const char* On_Red = "";
    static const char* On_Green = "";
    static const char* On_Yellow = "";
    static const char* On_Blue = "";
    static const char* On_Purple = "";
    static const char* On_Cyan = "";
    static const char* On_White = "";
    
    // High Intensity
    static const char* IBlack = "";
    static const char* IRed = "";
    static const char* IGreen = "";
    static const char* IYellow = "";
    static const char* IBlue = "";
    static const char* IPurple = "";
    static const char* ICyan = "";
    static const char* IWhite = "";
    
    // Bold High Intensity
    static const char* BIBlack = "";
    static const char* BIRed = "";
    static const char* BIGreen = "";
    static const char* BIYellow = "";
    static const char* BIBlue = "";
    static const char* BIPurple = "";
    static const char* BICyan = "";
    static const char* BIWhite = "";
    
    // High Intensity backgrounds
    static const char* On_IBlack = "";
    static const char* On_IRed = "";
    static const char* On_IGreen = "";
    static const char* On_IYellow = "";
    static const char* On_IBlue = "";
    static const char* On_IPurple = "";
    static const char* On_ICyan = "";
    static const char* On_IWhite = "";
#endif
}
#endif /* ANSI_COLORS_H_ */
