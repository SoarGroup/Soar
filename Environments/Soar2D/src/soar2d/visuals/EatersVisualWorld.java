package soar2d.visuals;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;

import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.widgets.Composite;

import soar2d.Direction;
import soar2d.Names;
import soar2d.Soar2D;
import soar2d.map.CellObject;
import soar2d.players.Player;

public class EatersVisualWorld extends VisualWorld {
	public EatersVisualWorld(Composite parent, int style, int cellSize) {
		super(parent, style, cellSize);
	}

	int [] agentLocation;
	
	public void setAgentLocation(int [] location) {
		agentLocation = Arrays.copyOf(location, location.length);
	}
	
	public int getMiniWidth() {
		return cellSize * ((Soar2D.config.eatersConfig().vision * 2) + 1);
	}
	
	public int getMiniHeight() {
		return cellSize * ((Soar2D.config.eatersConfig().vision * 2) + 1);
	}
	
	public void paintControl(PaintEvent e) {
		GC gc = e.gc;		
		gc.setFont(font);
        gc.setForeground(WindowManager.black);
		gc.setLineWidth(1);

		if (Soar2D.control.isRunning()) {
			if (agentLocation != null) {
				painted = false;
			}
			
			if (Soar2D.config.generalConfig().hidemap) {
				painted = true;
				return;
			}
			
		} else {
			if (agentLocation != null || lastX != e.x || lastY != e.y || internalRepaint) {
				lastX = e.x;
				lastY = e.y;
				painted = false;
			}

			if (Soar2D.config.generalConfig().hidemap || disabled || !painted) {
				gc.setBackground(WindowManager.widget_background);
				gc.fillRectangle(0,0, this.getWidth(), this.getHeight());
				if (disabled || Soar2D.config.generalConfig().hidemap) {
					painted = true;
					return;
				}
			}
		}
		
		// Draw world
		int fill1, fill2, xDraw, yDraw;
		int [] location = new int [2];
		for(location[0] = 0; location[0] < map.getSize(); ++location[0]){
			if (agentLocation != null) {
				if ((location[0] < agentLocation[0] - Soar2D.config.eatersConfig().vision) 
						|| (location[0] > agentLocation[0] + Soar2D.config.eatersConfig().vision)) {
					continue;
				} 
				xDraw = location[0] + Soar2D.config.eatersConfig().vision - agentLocation[0];
			} else {
				xDraw = location[0];
			}
			
			for(location[1] = 0; location[1] < map.getSize(); ++location[1]){
				if (agentLocation != null) {
					if ((location[1] < agentLocation[1] - Soar2D.config.eatersConfig().vision) 
							|| (location[1] > agentLocation[1] + Soar2D.config.eatersConfig().vision)) {
						continue;
					} 
					yDraw = location[1] + Soar2D.config.eatersConfig().vision - agentLocation[1];
				} else {
					yDraw = location[1];
				}
				
				if (agentLocation == null) {
					if ((this.map.removeObject(location, Names.kRedraw) == null) && painted) {
						continue;
					}
				} else {
					if (!this.map.hasObject(location, Names.kRedraw) && painted) {
						continue;
					}
				}
				
				ArrayList<CellObject> drawList;
				drawList = this.map.getAllWithProperty(location, Names.kPropertyShape);
				
				if (this.map.hasAnyWithProperty(location, Names.kPropertyBlock)) {
				    gc.setBackground(WindowManager.black);
				    gc.fillRectangle(cellSize*xDraw + 1, cellSize*yDraw + 1, cellSize - 2, cellSize - 2);
					
				} else {
					boolean empty = true;
					
					Player eater = this.map.getPlayer(location);
					
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
					
					if (drawList != null) {
						for (CellObject object : drawList) {
							
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
					}
					
					if (empty) {
						gc.setBackground(WindowManager.widget_background);
						gc.fillRectangle(cellSize*xDraw, cellSize*yDraw, cellSize, cellSize);
					}
				}
				
				if (this.map.hasObject(location, Names.kExplosion)) {
					drawExplosion(gc, xDraw, yDraw);
				}
			}
		}
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
	
}
