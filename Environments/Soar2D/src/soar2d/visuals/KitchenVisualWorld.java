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

				ArrayList<CellObject> drawList;
				drawList = this.map.getAllWithProperty(location, Names.kPropertyColor);
				
				if (!this.map.enterable(location)) {
				    gc.setBackground(WindowManager.black);
				    gc.fillRectangle(cellSize*location.x + 1, cellSize*location.y + 1, cellSize - 2, cellSize - 2);
					
				} else {
					boolean empty = true;
					
					Player cook = this.map.getPlayer(location);
					
					if (cook != null) {
						empty = false;
						
						gc.setBackground(playerColors.get(cook));
						gc.fillOval(cellSize*location.x, cellSize*location.y, cellSize, cellSize);
						gc.setBackground(WindowManager.widget_background);
					}
					
					Iterator<CellObject> iter = drawList.iterator();
					while (iter.hasNext()) {
						CellObject object = iter.next();
						
						if (empty) {
							gc.setBackground(WindowManager.widget_background);
							gc.fillRectangle(cellSize*location.x, cellSize*location.y, cellSize, cellSize);
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
								gc.fillOval(cellSize*location.x + fill1, cellSize*location.y + fill1, cellSize - fill2, cellSize - fill2);
								gc.drawOval(cellSize*location.x + fill1, cellSize*location.y + fill1, cellSize - fill2 - 1, cellSize - fill2 - 1);
								
							} else if (shape.equals(Shape.SQUARE)) {
								fill1 = (int)(cellSize/2.8);
								fill2 = cellSize - fill1 + 1;
								gc.fillRectangle(cellSize*location.x + fill1, cellSize*location.y + fill1, cellSize - fill2, cellSize - fill2);
								gc.drawRectangle(cellSize*location.x + fill1, cellSize*location.y + fill1, cellSize - fill2, cellSize - fill2);
								
							} else if (shape.equals(Shape.TRIANGLE)) {
								fill1 = (int)(cellSize/2.8);
								fill2 = cellSize - fill1 + 1;
								
								int [] verts = new int[6];

								verts [0] = cellSize*location.x + cellSize/2;
								verts [1] = cellSize*location.y + fill1;
								
								verts [2] = cellSize*location.x + fill1;
								verts [3] = cellSize*location.y + fill2;
								
								verts [4] = cellSize*location.x + fill2;
								verts [5] = cellSize*location.y + fill2;
								
								gc.fillPolygon(verts);
								gc.drawPolygon(verts);
							}
						} else {
							gc.setForeground(color);
							gc.drawRectangle(cellSize*location.x, cellSize*location.y, cellSize-1, cellSize-1);
							gc.setForeground(WindowManager.black);
						}
					}
					
					// BUGBUG redundant, remove
					if (empty) {
						gc.setBackground(WindowManager.widget_background);
						gc.fillRectangle(cellSize*location.x, cellSize*location.y, cellSize, cellSize);
					}
				}
			}
		}
	}
}
