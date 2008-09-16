package soar2d.visuals;

import java.util.ArrayList;
import java.util.Iterator;

import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.widgets.Composite;

import soar2d.Direction;
import soar2d.Names;
import soar2d.Soar2D;
import soar2d.map.CellObject;
import soar2d.player.Player;

public class EatersVisualWorld extends VisualWorld {
	public EatersVisualWorld(Composite parent, int style, int cellSize) {
		super(parent, style, cellSize);
	}

	java.awt.Point agentLocation;
	
	public void setAgentLocation(java.awt.Point location) {
		agentLocation = new java.awt.Point(location);
	}
	
	public int getMiniWidth() {
		return cellSize * ((Soar2D.config.eConfig.getEaterVision() * 2) + 1);
	}
	
	public int getMiniHeight() {
		return cellSize * ((Soar2D.config.eConfig.getEaterVision() * 2) + 1);
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
			
			if (Soar2D.config.getHide()) {
				painted = true;
				return;
			}
			
		} else {
			if (agentLocation != null || lastX != e.x || lastY != e.y || internalRepaint) {
				lastX = e.x;
				lastY = e.y;
				painted = false;
			}

			if (Soar2D.config.getHide() || disabled || !painted) {
				gc.setBackground(WindowManager.widget_background);
				gc.fillRectangle(0,0, this.getWidth(), this.getHeight());
				if (disabled || Soar2D.config.getHide()) {
					painted = true;
					return;
				}
			}
		}
		
		// Draw world
		int fill1, fill2, xDraw, yDraw;
		java.awt.Point location = new java.awt.Point();
		for(location.x = 0; location.x < map.getSize(); ++location.x){
			if (agentLocation != null) {
				if ((location.x < agentLocation.x - Soar2D.config.eConfig.getEaterVision()) || (location.x > agentLocation.x + Soar2D.config.eConfig.getEaterVision())) {
					continue;
				} 
				xDraw = location.x + Soar2D.config.eConfig.getEaterVision() - agentLocation.x;
			} else {
				xDraw = location.x;
			}
			
			for(location.y = 0; location.y < map.getSize(); ++location.y){
				if (agentLocation != null) {
					if ((location.y < agentLocation.y - Soar2D.config.eConfig.getEaterVision()) || (location.y > agentLocation.y + Soar2D.config.eConfig.getEaterVision())) {
						continue;
					} 
					yDraw = location.y + Soar2D.config.eConfig.getEaterVision() - agentLocation.y;
				} else {
					yDraw = location.y;
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
				
				if (!this.map.enterable(location)) {
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
