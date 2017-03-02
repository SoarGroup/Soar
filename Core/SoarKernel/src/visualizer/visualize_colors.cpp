#include "visualize.h"

#include <string>

const char* m_X11_Colors[140] =
{ "Alice Blue", "Aqua", "Aquamarine", "Azure", "Beige", "Bisque", "Blue", "Brown",
  "Burlywood", "Cadet Blue", "Chartreuse", "Chocolate", "Coral", "Cornflower", "Cornsilk", "Crimson", "Cyan", "Dark Blue", "Dark Cyan", "Dark Goldenrod",
  "Dark Gray", "Dark Green", "Dark Khaki", "Dark Magenta", "Dark Olive Green", "Dark Orange", "Dark Orchid", "Dark Red", "Dark Salmon", "Dark Sea Green",
  "Dark Slate Blue", "Dark Slate Gray", "Dark Turquoise", "Dark Violet", "Deep Pink", "Deep Sky Blue", "Dim Gray", "Dodger Blue", "Firebrick", "Floral White",
  "Forest Green", "Fuchsia", "Gainsboro", "Ghost White", "Gold", "Goldenrod", "Gray", "Web Gray", "Green", "Web Green", "Green Yellow", "Honeydew", "Hot Pink",
  "Indian Red", "Indigo", "Ivory", "Khaki", "Lavender", "Lavender Blush", "Lawn Green", "Lemon Chiffon", "Light Blue", "Light Coral", "Light Cyan", "Light Goldenrod",
  "Light Gray", "Light Green", "Light Pink", "Light Salmon", "Light Sea Green", "Light Sky Blue", "Light Slate Gray", "Light Steel Blue", "Light Yellow", "Lime",
  "Lime Green", "Linen", "Magenta", "Maroon", "Web Maroon", "Medium Aquamarine", "Medium Blue", "Medium Orchid", "Medium Purple", "Medium Sea Green",
  "Medium Slate Blue", "Medium Spring Green", "Medium Turquoise", "Medium Violet Red", "Midnight Blue", "Mint Cream", "Misty Rose", "Moccasin", "Navajo White",
  "Navy Blue", "Old Lace", "Olive", "Olive Drab", "Orange", "Orange Red", "Orchid", "Pale Goldenrod", "Pale Green", "Pale Turquoise", "Pale Violet Red", "Papaya Whip",
  "Peach Puff", "Peru", "Pink", "Plum", "Powder Blue", "Purple", "Web Purple", "Rebecca Purple", "Red", "Rosy Brown", "Royal Blue", "Saddle Brown", "Salmon",
  "Sandy Brown", "Sea Green", "Seashell", "Sienna", "Silver", "Sky Blue", "Slate Blue", "Slate Gray", "Snow", "Spring Green", "Steel Blue", "Tan", "Teal", "Thistle",
  "Tomato", "Turquoise", "Violet", "Wheat", "Yellow", "Yellow Green"
};

std::string GraphViz_Visualizer::get_color_for_id(uint64_t pID)
{
    auto iter = m_identity_colors.find(pID);
    if (iter != m_identity_colors.end())
    {
        uint64_t lIDColor = iter->second;
        assert(lIDColor < 140);
        return m_X11_Colors[iter->second];
    } else {
        m_identity_colors[pID] = ++m_last_color;
        return m_X11_Colors[m_last_color];
    }
}
