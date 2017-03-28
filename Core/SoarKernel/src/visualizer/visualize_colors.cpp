#include "visualize.h"

#include "dprint.h"

#include <string>

#define m_num_colors 139
const char* m_X11_Colors[m_num_colors] =
{
    "LightPink",
    "Pink",
    "Crimson",
    "PaleVioletRed",
    "HotPink",
    "DeepPink",
    "MediumVioletRed",
    "Thistle",
    "Plum",
    "Violet",
    "Fuchsia",
    "Purple",
    "MediumOrchid",
    "DarkViolet",
    "MediumPurple",
    "MediumSlateBlue",
    "DarkSlateBlue",
    "Lavender",
    "RoyalBlue",
    "CornflowerBlue",
    "LightSteelBlue",
    "DodgerBlue",
    "SteelBlue",
    "LightSkyBlue",
    "DeepSkyBlue",
    "LightBlue",
    "PowderBlue",
    "CadetBlue",
    "LightCyan",
    "PaleTurquoise",
    "Aqua",
    "Teal",
    "MediumTurquoise",
    "LightSeaGreen",
    "Turquoise",
    "Aquamarine",
    "MediumAquamarine",
    "MediumSpringGreen",
    "SpringGreen",
    "MediumSeaGreen",
    "SeaGreen",
    "LightGreen",
    "PaleGreen",
    "DarkSeaGreen",
    "LimeGreen",
    "Lime",
    "Green",
    "DarkGreen",
    "LawnGreen",
    "GreenYellow",
    "DarkOliveGreen",
    "YellowGreen",
    "OliveDrab",
    "Beige",
    "Yellow",
    "Olive",
    "DarkKhaki",
    "LemonChiffon",
    "PaleGoldenrod",
    "Khaki",
    "Gold",
    "Goldenrod",
    "DarkGoldenrod",
    "Wheat",
    "Moccasin",
    "Orange",
    "NavajoWhite",
    "AntiqueWhite",
    "Tan",
    "Peru",
    "PeachPuff",
    "Chocolate",
    "SaddleBrown",
    "Sienna",
    "LightSalmon",
    "Coral",
    "OrangeRed",
    "DarkSalmon",
    "Tomato",
    "MistyRose",
    "Salmon",
    "LightCoral",
    "RosyBrown",
    "IndianRed",
    "Red",
    "Brown",
    "Maroon",
    "Gainsboro",
    "LightSlateGray",
    "SlateGray",
    "LightGrey",
    "Silver",
    "DarkGray",
    "DimGray",
    "gray",
    "gray100",
    "gray11",
    "gray12",
    "gray13",
    "gray14",
    "gray15",
    "gray16",
    "gray17",
    "gray18",
    "gray19",
    "gray20",
    "gray21",
    "gray22",
    "gray23",
    "gray24",
    "gray25",
    "gray26",
    "gray27",
    "gray28",
    "gray29",
    "gray30",
    "gray31",
    "gray32",
    "gray33",
    "gray34",
    "gray35",
    "gray36",
    "gray37",
    "gray38",
    "gray39",
    "gray40",
    "gray41",
    "gray42",
    "gray43",
    "gray44",
    "gray45",
    "gray46",
    "gray47",
    "gray48",
    "gray49",
    "gray5",
    "snow2",
    "snow3",
    "snow4"
};

std::string GraphViz_Visualizer::get_color_for_id(uint64_t pID)
{
    std::string returnStr;

    if (pID)
    {
        returnStr = " BGCOLOR=\"";
        auto iter = m_identity_colors.find(pID);
        if (iter != m_identity_colors.end())
        {
            uint64_t lIDColor = iter->second;
            assert(lIDColor < m_num_colors);
            //dprint(DT_DEBUG, "Returning color %s for %u.\n", m_X11_Colors[iter->second], pID);
            returnStr += m_X11_Colors[iter->second];
        } else {
            m_identity_colors[pID] = m_next_color;
            if (++m_next_color == m_num_colors) m_next_color = 1;
            //dprint(DT_DEBUG, "Returning new color %s for %u.\n", m_X11_Colors[m_next_color-1], pID);
            returnStr += m_X11_Colors[m_next_color-1];
        }
        returnStr += "\" ";
    }
    else returnStr = " ";

    return returnStr;
}
