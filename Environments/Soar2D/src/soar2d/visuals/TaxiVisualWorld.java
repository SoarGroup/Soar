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
import soar2d.map.TaxiMap;
import soar2d.players.Player;

public class TaxiVisualWorld extends VisualWorld {
	public TaxiVisualWorld(Composite parent, int style, int cellSize) {
		super(parent, style, cellSize);
	}

	public void paintControl(PaintEvent e) {
		GC gc = e.gc;		
		gc.setFont(font);
        gc.setForeground(WindowManager.black);
		gc.setLineWidth(1);

		if (Soar2D.control.isRunning()) {
			if (Soar2D.config.generalConfig().hidemap) {
				painted = true;
				return;
			}
			
		} else {
			if (lastX != e.x || lastY != e.y || internalRepaint) {
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
		int fill;
		int [] location = new int [2];
		for(location[0] = 0; location[0] < map.getSize(); ++location[0]){			
			for(location[1] = 0; location[1] < map.getSize(); ++location[1]){				
				if ((this.map.removeObject(location, Names.kRedraw) == null) && painted) {
					//continue;
				}

				gc.setBackground(WindowManager.widget_background);
				gc.fillRectangle(cellSize*location[0]+1, cellSize*location[1]+1, cellSize-2, cellSize-2);
				
				// destination
				ArrayList<CellObject> destinationList;
				destinationList = this.map.getAllWithProperty(location, "destination");
				if (destinationList != null) {
					CellObject destination = destinationList.get(0);
					Color color = WindowManager.getColor(destination.getProperty(Names.kPropertyColor));
					gc.setBackground(color);
				    gc.fillRectangle(cellSize*location[0] + 1, cellSize*location[1] + 1, cellSize - 2, cellSize - 2);
				}
				
				if (this.map.hasObject(location, "fuel")) {
					int size = 14;
					fill = cellSize/2 - size/2;

					gc.setForeground(WindowManager.orange);
					gc.setBackground(WindowManager.widget_background);
					
					gc.fillRectangle(cellSize*location[0] + fill, cellSize*location[1] + fill, size, size);
					gc.drawRectangle(cellSize*location[0] + fill, cellSize*location[1] + fill, size-1, size-1);
			        
					gc.setForeground(WindowManager.black);
				}
				
				Player taxi = this.map.getPlayer(location);
				
				if (taxi != null) {
					// BUGBUG if multiple players are supported, this needs to be changed
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
				
				if (this.map.hasObject(location, "passenger")) {

					int size = 8;
					fill = cellSize/2 - size/2;
					gc.setBackground(WindowManager.getColor("black"));
					gc.fillOval(cellSize*location[0] + fill, cellSize*location[1] + fill, size, size);
					gc.drawOval(cellSize*location[0] + fill, cellSize*location[1] + fill, size - 1, size - 1);
				}

				// walls
				ArrayList<CellObject> wallList;
				wallList = this.map.getAllWithProperty(location, "block");
				if (wallList != null) {
					for (CellObject wall : wallList ) {
						switch(Direction.getInt(wall.getProperty("direction"))) {
						case Direction.kNorthInt:
							gc.drawLine(cellSize*location[0], cellSize*location[1], cellSize*location[0] + cellSize-1, cellSize*location[1]);
							break; 
						case Direction.kSouthInt:
							gc.drawLine(cellSize*location[0], cellSize*location[1] + cellSize-1, cellSize*location[0] + cellSize-1, cellSize*location[1] + cellSize-1);
							break;
						case Direction.kEastInt:
							gc.drawLine(cellSize*location[0] + cellSize-1, cellSize*location[1], cellSize*location[0] + cellSize-1, cellSize*location[1] + cellSize-1);
							break;
						case Direction.kWestInt:
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
