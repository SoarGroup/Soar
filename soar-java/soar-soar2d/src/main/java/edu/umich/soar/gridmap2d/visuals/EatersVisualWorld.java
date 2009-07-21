package edu.umich.soar.gridmap2d.visuals;

import java.util.List;

import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.widgets.Composite;

import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.players.Player;


public class EatersVisualWorld extends VisualWorld {
	//private static Logger logger = Logger.getLogger(EatersVisualWorld.class);

	public EatersVisualWorld(Composite parent, int style, int cellSize) {
		super(parent, style, cellSize);
	}

	int [] agentLocation;
	
	public void setAgentLocation(int [] location) {
		agentLocation = edu.umich.soar.gridmap2d.Arrays.copyOf(location, location.length);
	}
	
	public int getMiniWidth() {
		return cellSize * ((Gridmap2D.config.eatersConfig().vision * 2) + 1);
	}
	
	public int getMiniHeight() {
		return cellSize * ((Gridmap2D.config.eatersConfig().vision * 2) + 1);
	}
	
	public void paintControl(PaintEvent e) {
		GC gc = e.gc;		
		gc.setFont(font);
        gc.setForeground(WindowManager.black);
		gc.setLineWidth(1);

		if (Gridmap2D.control.isRunning()) {
			if (agentLocation != null) {
				painted = false;
			}
		} else {
			if (agentLocation != null || lastX != e.x || lastY != e.y || internalRepaint) {
				lastX = e.x;
				lastY = e.y;
				painted = false;
			}

			if (disabled || !painted) {
				gc.setBackground(WindowManager.widget_background);
				gc.fillRectangle(0,0, this.getWidth(), this.getHeight());
				if (disabled) {
					painted = true;
					return;
				}
			}
		}
		
		if (System.getProperty("os.name").contains("Mac OS X"))
			painted = false;
		
		// Draw world
		int fill1, fill2, xDraw, yDraw;
		int [] location = new int [2];
		for(location[0] = 0; location[0] < map.size(); ++location[0]){
			if (agentLocation != null) {
				if ((location[0] < agentLocation[0] - Gridmap2D.config.eatersConfig().vision) 
						|| (location[0] > agentLocation[0] + Gridmap2D.config.eatersConfig().vision)) {
					continue;
				} 
				xDraw = location[0] + Gridmap2D.config.eatersConfig().vision - agentLocation[0];
			} else {
				xDraw = location[0];
			}
			
			for(location[1] = 0; location[1] < map.size(); ++location[1]){
				if (agentLocation != null) {
					if ((location[1] < agentLocation[1] - Gridmap2D.config.eatersConfig().vision) 
							|| (location[1] > agentLocation[1] + Gridmap2D.config.eatersConfig().vision)) {
						continue;
					} 
					yDraw = location[1] + Gridmap2D.config.eatersConfig().vision - agentLocation[1];
				} else {
					yDraw = location[1];
				}
				
				if (agentLocation == null) {
					if (!this.map.getCell(location).checkAndResetRedraw() && painted) {
						continue;
					}
				} else {
					if (!this.map.getCell(location).checkRedraw() && painted) {
						continue;
					}
				}
				
				List<CellObject> drawList;
				drawList = this.map.getCell(location).getAllWithProperty(Names.kPropertyShape);
				
				if (this.map.getCell(location).hasAnyWithProperty(Names.kPropertyBlock)) {
				    gc.setBackground(WindowManager.black);
				    gc.fillRectangle(cellSize*xDraw + 1, cellSize*yDraw + 1, cellSize - 2, cellSize - 2);
					
				} else {
					boolean empty = true;
					
					Player eater = this.map.getCell(location).getPlayer();
					
					if (eater != null) {
						empty = false;
						
						gc.setBackground(playerColors.get(eater));
						gc.fillOval(cellSize*xDraw, cellSize*yDraw, cellSize, cellSize);
						gc.setBackground(WindowManager.widget_background);
						
						
						switch (eater.getFacing()) {
						case NORTH:
							drawEaterMouth(xDraw, yDraw, 1, 0, 1, 1, gc);
							break;
						case EAST:
							drawEaterMouth(xDraw + 1, yDraw, 0, 1, -1, 1, gc);
							break;
						case SOUTH:
							drawEaterMouth(xDraw, yDraw + 1, 1, 0, 1, -1, gc);
							break;
						case WEST:
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
				
				if (this.map.getCell(location).hasObject(Names.kExplosion)) {
					drawExplosion(gc, xDraw, yDraw);
				}
			}
		}
		
		painted = true;
	}
	
	int mouthCount = 0;
	void drawEaterMouth(int x, int y, int x_mult, int y_mult, int cx_mult, int cy_mult, GC gc){		
	    switch(++mouthCount % 8){
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
