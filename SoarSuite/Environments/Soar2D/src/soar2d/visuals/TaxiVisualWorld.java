package soar2d.visuals;

import java.util.ArrayList;
import java.util.Iterator;

import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.widgets.Composite;

import soar2d.Names;
import soar2d.Soar2D;
import soar2d.map.CellObject;
import soar2d.player.Player;

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
			if (Soar2D.config.getHide()) {
				painted = true;
				return;
			}
			
		} else {
			if (lastX != e.x || lastY != e.y || internalRepaint) {
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
		int fill1, fill2;
		java.awt.Point location = new java.awt.Point();
		for(location.x = 0; location.x < map.getSize(); ++location.x){			
			for(location.y = 0; location.y < map.getSize(); ++location.y){				
				if ((this.map.removeObject(location, Names.kRedraw) == null) && painted) {
					continue;
				}

				boolean empty = true;
				
				// destination
				ArrayList<CellObject> destinationList;
				destinationList = this.map.getAllWithProperty(location, "destination");
				if (destinationList.size() > 0) {
					empty = false;
					CellObject destination = destinationList.get(0);
					Color color = WindowManager.getColor(destination.getProperty(Names.kPropertyColor));
				    gc.fillRectangle(cellSize*location.x + 1, cellSize*location.y + 1, cellSize - 2, cellSize - 2);
				}
				
				if (this.map.hasObject(location, "fuel")) {
					empty = false;
					Color color = WindowManager.getColor("orange");
				    gc.fillRectangle(cellSize*location.x + 1, cellSize*location.y + 1, cellSize - 2, cellSize - 2);
				}
				
				Player taxi = this.map.getPlayer(location);
				
				if (taxi != null) {
					empty = false;
					
					// BUGBUG if multiple players are supported, this needs to be changed
					//gc.setBackground(playerColors.get(taxi));
					gc.setBackground(WindowManager.getColor("white"));
					gc.fillOval(cellSize*location.x, cellSize*location.y, cellSize, cellSize);
					gc.setBackground(WindowManager.widget_background);
				}
				
				if (this.map.hasObject(location, "passenger")) {
					empty = false;

					fill1 = (int)(cellSize/2.8);
					fill2 = cellSize - fill1 + 1;
					gc.setBackground(WindowManager.getColor("black"));
					gc.fillRectangle(cellSize*location.x + fill1, cellSize*location.y + fill1, cellSize - fill2, cellSize - fill2);
					gc.drawRectangle(cellSize*location.x + fill1, cellSize*location.y + fill1, cellSize - fill2, cellSize - fill2);
					gc.setBackground(WindowManager.widget_background);
				}
				
				if (empty) {
					gc.setBackground(WindowManager.widget_background);
					gc.fillRectangle(cellSize*location.x, cellSize*location.y, cellSize, cellSize);
				}
			}
		}
	}
}
