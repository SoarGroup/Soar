/* File: EaterDrawer.java
 * Jul 21, 2004
 */

package edu.umich.eaters;

import org.eclipse.swt.graphics.GC;

/**
 * Class that implements a few static methods to draw an Eater in a graphics context
 * given that eater and its x and y locations in the GC.
 * @author jduchi
 */
public class EaterDrawer {

	/** Convenience <code>int</code> to keep track of the size of squares we draw
	 * (in pixels). */
    private static int SquareSize;
    
    /**
     * Method to draw an Eater at specified location in the graphics context.
     * @param gc The GC in which to draw the Eater, specified by the caller. Method returns if
     * this argument is null.
     * @param x The x location (in simulation coordinates, not in GC coordinates) at which to
     * draw the Eater
     * @param y The y location, in simulation coordinates, at which to draw the Eater
     * @param eater The eater to be drawn
     * @param squareSize The size, in pixels, at which squares are drawn in the simulation
     * @param worldCount The count the simulation is on (necessary for drawing Eater's mouth)
     */
    public static void drawEater(GC gc, int x, int y, Eater eater, int squareSize, int worldCount){
    	if(gc == null) return;
    	SquareSize = squareSize;
		String cName = eater.getColorName();
		gc.setBackground(EaterSWindowManager.getNamedColor(cName));
		gc.fillOval(SquareSize*x, SquareSize*y, SquareSize, SquareSize);
		gc.setBackground(EaterSWindowManager.widget_background);
		int decision = eater.getRecentDecision();
		switch(decision){
		case(Eater.JumpEast):
		    drawEaterMouth(x + 1, y, 0, 1, -1, 1, gc, worldCount);
			break;
		case(Eater.MoveEast):
		    drawEaterMouth(x + 1, y, 0, 1, -1, 1, gc, worldCount);
		    break;
		case(Eater.JumpWest):
		    drawEaterMouth(x, y, 0, 1, 1, 1, gc, worldCount);
			break;
		case(Eater.MoveWest):
		    drawEaterMouth(x, y, 0, 1, 1, 1, gc, worldCount);
		    break;
		case(Eater.JumpNorth):
		    drawEaterMouth(x, y, 1, 0, 1, 1, gc, worldCount);
			break;
		case(Eater.MoveNorth):
		    drawEaterMouth(x, y, 1, 0, 1, 1, gc, worldCount);
			break;
		case(Eater.JumpSouth):
		    drawEaterMouth(x, y+1, 1, 0, 1, -1, gc, worldCount);
			break;
		case(Eater.MoveSouth):
		    drawEaterMouth(x, y + 1, 1, 0, 1, -1, gc, worldCount);
		    break;
		default:
			drawEaterMouth(x, y, 1, 0, 1, 1, gc, worldCount);
		    break;
		}
		
	}
	
    /**
     * Method called to draw something from Eaters at the specified location. This
     * method will draw a wall as the default behavior.
     * @param gc The graphics context on which to draw
     * @param x The x location (in simulation coordinates, not in GC coordinates) at which to
     * draw
     * @param y The y location, in simulation coordinates, at which to draw
     * @param squareSize The size, in pixels, at which squares are drawn in the simulation
     * @param toDraw The <code>Object</code> to draw.
     */
    public static void drawContents(GC gc, int x, int y, Object toDraw, int squareSize){
        if(gc == null) return;
        SquareSize = squareSize;
        gc.setForeground(EaterSWindowManager.black);
		gc.setLineWidth(1);
		
		if(toDraw instanceof Eater){
		    drawEater(gc, x, y, (Eater)toDraw, squareSize, ((Eater)toDraw).getWorldCount());
		} else if(toDraw instanceof NormalFood){
			gc.setBackground(EaterSWindowManager.blue);
			int fill1 = (int)(SquareSize/2.8);
			int fill2 = SquareSize - fill1 + 1;
			gc.fillOval(SquareSize*x + fill1, SquareSize*y + fill1, SquareSize - fill2, SquareSize - fill2);
			gc.drawOval(SquareSize*x + fill1, SquareSize*y + fill1, SquareSize - fill2 - 1, SquareSize - fill2 - 1);
		}
		else if(toDraw instanceof BonusFood){
			gc.setBackground(EaterSWindowManager.red);
			int fill1 = (int)(SquareSize/2.8);
			int fill2 = SquareSize - fill1 + 1;
			gc.fillRectangle(SquareSize*x + fill1, SquareSize*y + fill1, SquareSize - fill2, SquareSize - fill2);
			gc.drawRectangle(SquareSize*x + fill1, SquareSize*y + fill1, SquareSize - fill2, SquareSize - fill2);
		}
		else if(toDraw instanceof EatersEmpty){
			gc.setBackground(EaterSWindowManager.widget_background);
			gc.fillRectangle(SquareSize*x, SquareSize*y, SquareSize, SquareSize);
		} else {
		    /* Default is Wall behavior */
		    gc.setBackground(EaterSWindowManager.black);
		    gc.fillRectangle(SquareSize*x + 1, SquareSize*y + 1, SquareSize - 2, SquareSize - 2);
		}
    }
    
    /**
     * Static function to draw the mouth of an <code>Eater</code> based on the world count (lets
     * them seem to chew).
     * @param x The x-coordinate (simulation coordinates) at which to draw the <code>Eater</code>.
     * @param y The y-coordinate (simulation coordinates) at which to draw the <code>Eater</code>.
     * @param x_mult -1, 0, or 1 depending on whether the x-coordinate on the other side
     * of the square from that specified by x is to the left (-1), right (1), or center (0).
     * @param y_mult -1, 0, or 1 depending on whether the y-coordinate on the other side
     * of the square from that specified by y is up (-1), the same (0), or below (1).
     * @param cx_mult -1, 0, or 1 depending on whether the center x-coordinate of the <code>
     * Eater</code> being drawn is to the left (-1), right (1), or center (0) the coordinate
     * specified by <code>x</code>.
     * @param cy_mult -1, 0, or 1 depending on whether the center y-coordinate of the <code>
     * Eater</code> being drawn is to above (-1), the same (0), or below (1) the coordinate
     * specified by <code>y</code>.
     * @param gc The graphics context on which to draw.
     * @param worldCount The world count of the eaters simulation.
     */
	private static void drawEaterMouth(int x, int y, int x_mult, int y_mult, int cx_mult,
	        int cy_mult, GC gc, int worldCount){
	    switch(worldCount%8){
		case(0):{
		    gc.fillPolygon(new int[]{SquareSize*x, SquareSize*y,
		            SquareSize*x + x_mult*SquareSize, SquareSize * y + y_mult*SquareSize,
		            SquareSize*x + cx_mult*SquareSize/2, SquareSize*y + cy_mult*SquareSize/2});
		    break;
		}
		case(1):{
		    gc.fillPolygon(new int[]{SquareSize*x + x_mult*SquareSize/8, SquareSize*y + y_mult*SquareSize/8,
		            SquareSize*x + x_mult*(SquareSize - SquareSize/8), SquareSize * y + y_mult*(SquareSize - SquareSize/8),
		            SquareSize*x + cx_mult*SquareSize/2, SquareSize*y + cy_mult*SquareSize/2});
		    break;
		}
		case(2):{
		    gc.fillPolygon(new int[]{SquareSize*x + x_mult*SquareSize/4, SquareSize*y + y_mult*SquareSize/4,
		            SquareSize*x + x_mult*(SquareSize - SquareSize/4), SquareSize * y + y_mult*(SquareSize - SquareSize/4),
		            SquareSize*x + cx_mult * SquareSize/2, SquareSize*y + cy_mult * SquareSize/2});
		    break;
		}
		case(3):{
		    gc.fillPolygon(new int[]{SquareSize*x + x_mult * 3*SquareSize/8, SquareSize*y + y_mult * 3*SquareSize/8,
		            SquareSize*x + x_mult*(SquareSize - 3*SquareSize/8), SquareSize * y + y_mult*(SquareSize - 3*SquareSize/8),
		            SquareSize*x + cx_mult*SquareSize/2, SquareSize*y + cy_mult*SquareSize/2});
		    break;
		}
		case(4): break;
		case(5):{
		    gc.fillPolygon(new int[]{SquareSize*x + x_mult * 3*SquareSize/8, SquareSize*y + y_mult * 3*SquareSize/8,
		            SquareSize*x + x_mult*(SquareSize - 3*SquareSize/8), SquareSize * y + y_mult*(SquareSize - 3*SquareSize/8),
		            SquareSize*x + cx_mult*SquareSize/2, SquareSize*y + cy_mult*SquareSize/2});
		    break;
		}
		case(6):{
		    gc.fillPolygon(new int[]{SquareSize*x + x_mult*SquareSize/4, SquareSize*y + y_mult*SquareSize/4,
		            SquareSize*x + x_mult*(SquareSize - SquareSize/4), SquareSize * y + y_mult*(SquareSize - SquareSize/4),
		            SquareSize*x + cx_mult * SquareSize/2, SquareSize*y + cy_mult * SquareSize/2});
		    break;
		}
		case(7):{
		    gc.fillPolygon(new int[]{SquareSize*x + x_mult*SquareSize/8, SquareSize*y + y_mult*SquareSize/8,
		            SquareSize*x + x_mult*(SquareSize - SquareSize/8), SquareSize * y + y_mult*(SquareSize - SquareSize/8),
		            SquareSize*x + cx_mult*SquareSize/2, SquareSize*y + cy_mult*SquareSize/2});
		    break;	
		}
		default: break;
		}
	}
    
}
