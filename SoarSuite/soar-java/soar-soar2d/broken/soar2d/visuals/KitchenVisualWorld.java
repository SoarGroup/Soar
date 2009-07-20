package broken.soar2d.visuals;

import java.util.ArrayList;

import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.widgets.Composite;

import soar2d.Names;
import soar2d.Soar2D;
import soar2d.map.CellObject;
import soar2d.players.Player;

public class KitchenVisualWorld extends VisualWorld {
	public KitchenVisualWorld(Composite parent, int style, int cellSize) {
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
		int fill1, fill2;
		int [] location = new int [2];
		for(location[0] = 0; location[0] < map.getSize(); ++location[0]){			
			for(location[1] = 0; location[1] < map.getSize(); ++location[1]){				
				if (!this.map.resetRedraw(location) && painted) {
					continue;
				}

				List<CellObject> drawList;
				drawList = this.map.getAllWithProperty(location, Names.kPropertyColor);
				
				if (this.map.hasAnyWithProperty(location, Names.kPropertyBlock)) {
				    gc.setBackground(WindowManager.black);
				    gc.fillRectangle(cellSize*location[0] + 1, cellSize*location[1] + 1, cellSize - 2, cellSize - 2);
					
				} else {
					boolean empty = true;
					
					Player cook = this.map.getPlayer(location);
					
					if (cook != null) {
						empty = false;
						
						gc.setBackground(playerColors.get(cook));
						gc.fillOval(cellSize*location[0], cellSize*location[1], cellSize, cellSize);
						gc.setBackground(WindowManager.widget_background);
					}
					
					if (drawList != null) {
						for (CellObject object : drawList) {
							if (empty) {
								gc.setBackground(WindowManager.widget_background);
								gc.fillRectangle(cellSize*location[0], cellSize*location[1], cellSize, cellSize);
							}
							empty = false;
						    
						    Color color = WindowManager.getColor(object.getProperty(Names.kPropertyColor));
						    assert color != null;
							gc.setBackground(color);
							
							Shape shape = Shape.getShape(object.getProperty(Names.kPropertyShape));
							if (shape != null) {
								if (shape.equals(Shape.ROUND)) {
									fill1 = (int)(cellSize/2.8);
									fill2 = cellSize - fill1 + 1;
									gc.fillOval(cellSize*location[0] + fill1, cellSize*location[1] + fill1, cellSize - fill2, cellSize - fill2);
									gc.drawOval(cellSize*location[0] + fill1, cellSize*location[1] + fill1, cellSize - fill2 - 1, cellSize - fill2 - 1);
									
								} else if (shape.equals(Shape.SQUARE)) {
									fill1 = (int)(cellSize/2.8);
									fill2 = cellSize - fill1 + 1;
									gc.fillRectangle(cellSize*location[0] + fill1, cellSize*location[1] + fill1, cellSize - fill2, cellSize - fill2);
									gc.drawRectangle(cellSize*location[0] + fill1, cellSize*location[1] + fill1, cellSize - fill2, cellSize - fill2);
									
								} else if (shape.equals(Shape.TRIANGLE)) {
									fill1 = (int)(cellSize/2.8);
									fill2 = cellSize - fill1 + 1;
									
									int [] verts = new int[6];
	
									verts [0] = cellSize*location[0] + cellSize/2;
									verts [1] = cellSize*location[1] + fill1;
									
									verts [2] = cellSize*location[0] + fill1;
									verts [3] = cellSize*location[1] + fill2;
									
									verts [4] = cellSize*location[0] + fill2;
									verts [5] = cellSize*location[1] + fill2;
									
									gc.fillPolygon(verts);
									gc.drawPolygon(verts);
								}
							} else {
								gc.setForeground(color);
								gc.drawRectangle(cellSize*location[0], cellSize*location[1], cellSize-1, cellSize-1);
								gc.setForeground(WindowManager.black);
							}
						}
					}
					
					// BUGBUG redundant, remove
					if (empty) {
						gc.setBackground(WindowManager.widget_background);
						gc.fillRectangle(cellSize*location[0], cellSize*location[1], cellSize, cellSize);
					}
				}
			}
		}
	}
}
