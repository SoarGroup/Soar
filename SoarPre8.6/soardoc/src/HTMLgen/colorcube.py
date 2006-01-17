#!/usr/bin/env python

"""Utility to generate a table of browser safe colors.
"""
__version__ = '$Id$'
__author__ = "Robin Friedrich"
__date__ = "Feb. 18, 1998"

# colorcube.py
# COPYRIGHT (C) 1998  ROBIN FRIEDRICH  email:Robin.Friedrich@pdq.net
# Permission to  use, copy, modify, and distribute this software and its
# documentation  for  any  purpose  and  without fee  is hereby granted,
# provided that the above copyright notice appear in all copies and that
# both that copyright notice and this permission notice appear in
# supporting documentation.
# THE  AUTHOR  DISCLAIMS  ALL  WARRANTIES WITH  REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
# EVENT  SHALL THE  AUTHOR  BE  LIABLE  FOR  ANY  SPECIAL,   INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
# USE, DATA OR PROFITS,  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.

from HTMLgen import *

hexes = ('00','33','66','99','CC','FF')

def main(filename):
    # Create a fresh document
    doc = SimpleDocument(title='Web Safe Colors', bgcolor = '#FFFFFF')
    # Create an invisible table for layout
    bodytable = TableLite(cellpadding=5, border=0, html_escape="off")
    # Set up a table for the 216 color cube
    colortable = TableLite(cellpadding=0, border=0, cellspacing=1, html_escape="off")
    colortable.append(Caption("The 216 Web Color Cube"))
    for i in (0,1,2,3,4,5):
        for j in (0,1,2,3,4,5):
            tr = TR()
            for k in (0,1,2,3,4,5):
                tr.append(makecell("#%s%s%s" % (hexes[i], hexes[j], hexes[k])))
            colortable.append(tr)
    # append the table as a cell in the body table
    bodyrow = TR(TD(colortable))
    # Set up a table for the greyscale
    greytable = TableLite(cellpadding=0, border=0, cellspacing=1, html_escape="off")
    greytable.append(Caption("The 16 Grey Levels"))
    for a in '0123456789ABCDEF':
        greytable.append( TR( makecell("#%s%s%s%s%s%s" % ((a,)*6)) ) )
    bodyrow.append(TD(greytable))
    # Set up a table containing the pure colors
    puretable = TableLite(cellpadding=0, border=0, cellspacing=1, html_escape="off")
    puretable.append(Caption("The Pure Colors"))
    for a in '123456789ABCDEF':
        tr = TR()
        tr.append(makecell("#%s%s0000" % (a,a) ) )
        tr.append(makecell("#00%s%s00" % (a,a) ) )
        tr.append(makecell("#0000%s%s" % (a,a) ) )
        puretable.append(tr)
    bodyrow.append(TD(puretable))
    # Now attach the body row to the bodytable
    bodytable.append(bodyrow)
    # Attach the bodytable to the document and write it all out
    doc.append(bodytable)
    doc.write(filename)

def makecell(color):
    """Return a table cell object (TD) of the given color tag
    """
    cell = TD(bgcolor=color, align='center', height=26, width=60)
    cell.append(Font(color, color = contrast(color), size = -2))
    return cell

def contrast(color):
    """Compute luminous efficiency of given color and return either
    white or black based on which would contrast more.
    """
    # I know this is not exact; just my guess
    R = eval('0x'+color[1:3])
    G = eval('0x'+color[3:5])
    B = eval('0x'+color[5:7])
    lumen = 0.6*R + G + 0.3*B
    if lumen > 250:
        return "#000000"
    else:
        return "#FFFFFF"

if __name__ == '__main__':
    main('colorcube.html')
