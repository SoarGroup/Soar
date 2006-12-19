package soar2d.visuals;

import java.util.*;
import org.eclipse.swt.*;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.widgets.*;

import soar2d.*;
import soar2d.player.Player;
import soar2d.world.*;

public class VisualWorld extends Canvas implements PaintListener {
	public static HashMap<Player, Color> playerColors = new HashMap<Player, Color>();
	
	public static void remapPlayerColors() {
		ArrayList<Player> players = Soar2D.simulation.world.getPlayers();
		playerColors.clear();
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			String color = player.getColor();
			playerColors.put(player, WindowManager.getColor(color));
		}
	}

	public static boolean internalRepaint = false;
	
	java.awt.Point agentLocation;
	
	protected Display display;
	protected int cellSize;
	protected boolean disabled = false;
	protected boolean painted = false;
	protected int lastX = 0;
	protected int lastY = 0;
	
	public VisualWorld(Composite parent, int style, int cellSize) {
		super(parent, style | SWT.NO_BACKGROUND);
		display = parent.getDisplay();
		this.cellSize = cellSize;

		addPaintListener(this);		
	}

	public int getMiniWidth() {
		return cellSize * ((Soar2D.config.kEaterVision * 2) + 1);
	}
	
	public int getMiniHeight() {
		return cellSize * ((Soar2D.config.kEaterVision * 2) + 1);
	}
	
	public void setAgentLocation(java.awt.Point location) {
		agentLocation = location;
	}
	
	Player getPlayerAtPixel(int x, int y) {
		x /= cellSize;
		y /= cellSize;
		return Soar2D.simulation.world.map.getPlayer(new java.awt.Point(x, y));
	}

	public void paintControl(PaintEvent e){
		if (agentLocation != null || lastX != e.x || lastY != e.y || internalRepaint) {
			lastX = e.x;
			lastY = e.y;
			setRepaint();
		}
		
		GC gc = e.gc;		
        gc.setForeground(WindowManager.black);
		gc.setLineWidth(1);

		if (disabled || !painted) {
			gc.setBackground(WindowManager.widget_background);
			gc.fillRectangle(0,0, this.getWidth(), this.getHeight());
			if (disabled) {
				painted = true;
				return;
			}
		}
		
		World world = Soar2D.simulation.world;
		
		// Draw world
		int fill1, fill2, xDraw, yDraw;
		java.awt.Point location = new java.awt.Point();
		for(location.x = 0; location.x < world.getSize(); ++location.x){
			if (agentLocation != null) {
				if ((location.x < agentLocation.x - Soar2D.config.kEaterVision) || (location.x > agentLocation.x + Soar2D.config.kEaterVision)) {
					continue;
				} 
				xDraw = location.x + Soar2D.config.kEaterVision - agentLocation.x;
			} else {
				xDraw = location.x;
			}
			
			for(location.y = 0; location.y < world.getSize(); ++location.y){
				if (agentLocation != null) {
					if ((location.y < agentLocation.y - Soar2D.config.kEaterVision) || (location.y > agentLocation.y + Soar2D.config.kEaterVision)) {
						continue;
					} 
					yDraw = location.y + Soar2D.config.kEaterVision - agentLocation.y;
				} else {
					yDraw = location.y;
				}
				
				if (agentLocation == null) {
					if ((world.map.removeObject(location, Names.kRedraw) == null) && painted) {
						continue;
					}
				}
				
				ArrayList<CellObject> drawList = world.map.getAllWithProperty(location, Names.kPropertyShape);
				
				if (!world.map.enterable(location)) {
				    gc.setBackground(WindowManager.black);
				    gc.fillRectangle(cellSize*xDraw + 1, cellSize*yDraw + 1, cellSize - 2, cellSize - 2);
					
				} else {
					boolean empty = true;
					
					Player eater = world.map.getPlayer(location);
					
					if (eater != null) {
						empty = false;
						
						gc.setBackground(playerColors.get(eater));
						gc.fillOval(cellSize*xDraw, cellSize*yDraw, cellSize, cellSize);
						gc.setBackground(WindowManager.widget_background);
						
						
						switch (eater.getFacingInt()) {
						case Direction.kNorthInt:
							drawEaterMouth(xDraw, yDraw, 1, 0, 1, 1, gc);
							break;
						case Direction.kEastInt:
							drawEaterMouth(xDraw + 1, yDraw, 0, 1, -1, 1, gc);
							break;
						case Direction.kSouthInt:
							drawEaterMouth(xDraw, yDraw + 1, 1, 0, 1, -1, gc);
							break;
						case Direction.kWestInt:
							drawEaterMouth(xDraw, yDraw, 0, 1, 1, 1, gc);
							break;
						default:
							break;
						}
					}
					
					Iterator<CellObject> iter = drawList.iterator();
					while (iter.hasNext()) {
						CellObject object = iter.next();
						
						if (empty) {
							gc.setBackground(WindowManager.widget_background);
							gc.fillRectangle(cellSize*xDraw, cellSize*yDraw, cellSize, cellSize);
						}
						empty = false;
					    
					    Color color = WindowManager.getColor(object.getProperty(Names.kPropertyColor));
					    if (color == null) {
					    	//TODO: draw outline!
					    }
						gc.setBackground(color);
						
						Shape shape = Shape.getShape(object.getProperty(Names.kPropertyShape));
						if (shape != null) {
							if (shape.equals(Shape.ROUND)) {
								fill1 = (int)(cellSize/2.8);
								fill2 = cellSize - fill1 + 1;
								gc.fillOval(cellSize*xDraw + fill1, cellSize*yDraw + fill1, cellSize - fill2, cellSize - fill2);
								gc.drawOval(cellSize*xDraw + fill1, cellSize*yDraw + fill1, cellSize - fill2 - 1, cellSize - fill2 - 1);
								
							} else if (shape.equals(Shape.SQUARE)) {
								fill1 = (int)(cellSize/2.8);
								fill2 = cellSize - fill1 + 1;
								gc.fillRectangle(cellSize*xDraw + fill1, cellSize*yDraw + fill1, cellSize - fill2, cellSize - fill2);
								gc.drawRectangle(cellSize*xDraw + fill1, cellSize*yDraw + fill1, cellSize - fill2, cellSize - fill2);
							}
						}
					}
					
					if (empty) {
						gc.setBackground(WindowManager.widget_background);
						gc.fillRectangle(cellSize*xDraw, cellSize*yDraw, cellSize, cellSize);
					}
				}
				
				if (world.map.removeObject(location, Names.kExplosion) != null) {
					drawExplosion(gc, xDraw, yDraw);
				}
			}
		}
		painted = true;
	}
	
	void drawExplosion(GC gc, int x, int y) {
		gc.setBackground(WindowManager.red);
		int offCenter = cellSize/4;
		int xBase = cellSize*x;
		int yBase = cellSize*y;
		int halfCell = cellSize/2;
		
		java.awt.Point center = new java.awt.Point(xBase + halfCell, yBase + halfCell);
		java.awt.Point north = new java.awt.Point(center.x, yBase);
		java.awt.Point east = new java.awt.Point(xBase + cellSize, center.y);
		java.awt.Point south = new java.awt.Point(center.x, yBase + cellSize);
		java.awt.Point west = new java.awt.Point(xBase, center.y);
		
		gc.fillPolygon(new int[] {center.x, center.y, north.x, north.y, center.x + offCenter, center.y});
		gc.fillPolygon(new int[] {center.x, center.y, east.x,  east.y,  center.x, center.y + offCenter});
		gc.fillPolygon(new int[] {center.x, center.y, south.x, south.y, center.x - offCenter, center.y});
		gc.fillPolygon(new int[] {center.x, center.y, west.x,  west.y,  center.x, center.y - offCenter});
	}
	
	void drawEaterMouth(int x, int y, int x_mult, int y_mult, int cx_mult, int cy_mult, GC gc){		
	    switch(Soar2D.simulation.world.getWorldCount() % 8){
			case(0):{
			    gc.fillPolygon(new int[]{cellSize*x, cellSize*y,
			            cellSize*x + x_mult*cellSize, cellSize * y + y_mult*cellSize,
			            cellSize*x + cx_mult*cellSize/2, cellSize*y + cy_mult*cellSize/2});
			    break;
			}
			case(1):{
			    gc.fillPolygon(new int[]{cellSize*x + x_mult*cellSize/8, cellSize*y + y_mult*cellSize/8,
			            cellSize*x + x_mult*(cellSize - cellSize/8), cellSize * y + y_mult*(cellSize - cellSize/8),
			            cellSize*x + cx_mult*cellSize/2, cellSize*y + cy_mult*cellSize/2});
			    break;
			}
			case(2):{
			    gc.fillPolygon(new int[]{cellSize*x + x_mult*cellSize/4, cellSize*y + y_mult*cellSize/4,
			            cellSize*x + x_mult*(cellSize - cellSize/4), cellSize * y + y_mult*(cellSize - cellSize/4),
			            cellSize*x + cx_mult * cellSize/2, cellSize*y + cy_mult * cellSize/2});
			    break;
			}
			case(3):{
			    gc.fillPolygon(new int[]{cellSize*x + x_mult * 3*cellSize/8, cellSize*y + y_mult * 3*cellSize/8,
			            cellSize*x + x_mult*(cellSize - 3*cellSize/8), cellSize * y + y_mult*(cellSize - 3*cellSize/8),
			            cellSize*x + cx_mult*cellSize/2, cellSize*y + cy_mult*cellSize/2});
			    break;
			}
			case(4): break;
			case(5):{
			    gc.fillPolygon(new int[]{cellSize*x + x_mult * 3*cellSize/8, cellSize*y + y_mult * 3*cellSize/8,
			            cellSize*x + x_mult*(cellSize - 3*cellSize/8), cellSize * y + y_mult*(cellSize - 3*cellSize/8),
			            cellSize*x + cx_mult*cellSize/2, cellSize*y + cy_mult*cellSize/2});
			    break;
			}
			case(6):{
			    gc.fillPolygon(new int[]{cellSize*x + x_mult*cellSize/4, cellSize*y + y_mult*cellSize/4,
			            cellSize*x + x_mult*(cellSize - cellSize/4), cellSize * y + y_mult*(cellSize - cellSize/4),
			            cellSize*x + cx_mult * cellSize/2, cellSize*y + cy_mult * cellSize/2});
			    break;
			}
			case(7):{
			    gc.fillPolygon(new int[]{cellSize*x + x_mult*cellSize/8, cellSize*y + y_mult*cellSize/8,
			            cellSize*x + x_mult*(cellSize - cellSize/8), cellSize * y + y_mult*(cellSize - cellSize/8),
			            cellSize*x + cx_mult*cellSize/2, cellSize*y + cy_mult*cellSize/2});
			    break;	
			}
		}
	}
	
	public void setRepaint() {
		painted = false;
	}
	
	public void disable() {
		disabled = true;
	}
	
	public void enable() {
		disabled = false;
	}
	
	public int getWidth() {
		return cellSize * Soar2D.simulation.world.getSize();
	}
	
	public int getHeight() {
		return cellSize * Soar2D.simulation.world.getSize();
	}
	
}
