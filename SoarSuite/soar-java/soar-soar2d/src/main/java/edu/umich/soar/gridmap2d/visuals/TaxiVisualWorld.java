package edu.umich.soar.gridmap2d.visuals;

import java.util.List;

import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.widgets.Composite;

import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.map.TaxiMap;
import edu.umich.soar.gridmap2d.players.Taxi;

public class TaxiVisualWorld extends VisualWorld {
	public TaxiVisualWorld(Composite parent, int style, int cellSize) {
		super(parent, style, cellSize);
	}

	public void paintControl(PaintEvent e) {
		GC gc = e.gc;		
		gc.setFont(font);
        gc.setForeground(WindowManager.black);
		gc.setLineWidth(1);

		if (!Gridmap2D.control.isRunning()) {
			if (lastX != e.x || lastY != e.y || internalRepaint) {
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
		int fill;
		int [] location = new int [2];
		for(location[0] = 0; location[0] < map.size(); ++location[0]){			
			for(location[1] = 0; location[1] < map.size(); ++location[1]){				
				if (!this.map.getCell(location).checkAndResetRedraw() && painted) {
					//continue;
				}

				gc.setBackground(WindowManager.widget_background);
				gc.fillRectangle(cellSize*location[0]+1, cellSize*location[1]+1, cellSize-2, cellSize-2);
				
				// destination
				List<CellObject> destinationList;
				destinationList = this.map.getCell(location).getAllWithProperty("destination");
				if (destinationList != null) {
					CellObject destination = destinationList.get(0);
					String colorString = destination.getProperty(Names.kPropertyColor);
					Color color = WindowManager.getColor(colorString);
					gc.setBackground(color);
				    gc.fillRectangle(cellSize*location[0] + 1, cellSize*location[1] + 1, cellSize - 2, cellSize - 2);
				    
				    if (colorString.equals("blue")) {
				    	gc.setForeground(WindowManager.white);
				    } else {
				    	gc.setForeground(WindowManager.black);
				    }
				    gc.drawString(colorString.substring(0, 1), cellSize*location[0] + 1, cellSize*location[1] + 1);
			    	gc.setForeground(WindowManager.black);
				}
				
				if (this.map.getCell(location).hasObject("fuel")) {
					int size = 14;
					fill = cellSize/2 - size/2;

					gc.setForeground(WindowManager.orange);
					gc.setBackground(WindowManager.widget_background);
					
					gc.fillRectangle(cellSize*location[0] + fill, cellSize*location[1] + fill, size, size);
					gc.drawRectangle(cellSize*location[0] + fill, cellSize*location[1] + fill, size-1, size-1);
			        
					gc.setForeground(WindowManager.black);
				    gc.drawString("f", cellSize*location[0] + 8, cellSize*location[1] + 3);

				}
				
				Taxi taxi = (Taxi)this.map.getCell(location).getPlayer();
				
				if (taxi != null) {
					gc.setBackground(WindowManager.getColor("white"));

					int size = 12;
					fill = cellSize/2 - size/2;
					gc.fillRectangle(cellSize*location[0] + fill, cellSize*location[1] + fill, size, size);
					gc.drawRectangle(cellSize*location[0] + fill, cellSize*location[1] + fill, size-1, size-1);
					
					TaxiMap xMap = (TaxiMap)map;
					if (xMap.isPassengerCarried()) {
						size = 4;
						fill = cellSize/2 - size/2;
						gc.drawOval(cellSize*location[0] + fill, cellSize*location[1] + fill, size - 1, size - 1);
					}
				}
				
				if (this.map.getCell(location).hasObject("passenger")) {

					int size = 8;
					fill = cellSize/2 - size/2;
					gc.setBackground(WindowManager.getColor("black"));
					gc.fillOval(cellSize*location[0] + fill, cellSize*location[1] + fill, size, size);
					gc.drawOval(cellSize*location[0] + fill, cellSize*location[1] + fill, size - 1, size - 1);
				}

				// walls
				List<CellObject> wallList;
				wallList = this.map.getCell(location).getAllWithProperty("block");
				if (wallList != null) {
					for (CellObject wall : wallList ) {
						switch(Direction.parse(wall.getProperty("direction"))) {
						case NORTH:
							gc.drawLine(cellSize*location[0], cellSize*location[1], cellSize*location[0] + cellSize-1, cellSize*location[1]);
							break; 
						case SOUTH:
							gc.drawLine(cellSize*location[0], cellSize*location[1] + cellSize-1, cellSize*location[0] + cellSize-1, cellSize*location[1] + cellSize-1);
							break;
						case EAST:
							gc.drawLine(cellSize*location[0] + cellSize-1, cellSize*location[1], cellSize*location[0] + cellSize-1, cellSize*location[1] + cellSize-1);
							break;
						case WEST:
							gc.drawLine(cellSize*location[0], cellSize*location[1], cellSize*location[0], cellSize*location[1] + cellSize-1);
							break;
						default:
							assert false;
							break;	
						}
					}
				}
				
			}
		}
	}
}
