package soar2d.visuals;

import java.awt.geom.Point2D;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;

import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Path;
import org.eclipse.swt.widgets.Composite;

import soar2d.Direction;
import soar2d.Names;
import soar2d.Soar2D;
import soar2d.map.CellObject;
import soar2d.player.Player;
import soar2d.world.PlayersManager;

public class BookVisualWorld extends VisualWorld {
	
	public BookVisualWorld(Composite parent, int style, int cellSize) {
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
		ArrayList<java.awt.Point> playerLocs = new ArrayList<java.awt.Point>();
		java.awt.Point location = new java.awt.Point();
		HashSet<Integer> roomIds = new HashSet<Integer>();
		for(location.x = 0; location.x < map.getSize(); ++location.x){
			for(location.y = 0; location.y < map.getSize(); ++location.y){
				if ((this.map.removeObject(location, Names.kRedraw) == null) && painted) {
					continue;
				}
				
				if (!this.map.enterable(location)) {
				    gc.setBackground(WindowManager.black);
				    gc.fillRectangle(cellSize*location.x, cellSize*location.y, cellSize, cellSize);
					
				} else {
					
					if (map.getAllWithProperty(location, Names.kPropertyGatewayRender).size() == 0) {

						if (!Soar2D.bConfig.getColoredRooms()) {
							// normal:
							gc.setBackground(WindowManager.widget_background);
						} else {
							// colored rooms:
							CellObject roomObject = map.getObject(location, Names.kRoomID);
							if (roomObject == null)  {
								gc.setBackground(WindowManager.widget_background);
							} else {
								int roomID = roomObject.getIntProperty(Names.kPropertyNumber);
								roomID %= Soar2D.simulation.kColors.length - 1; // the one off eliminates black
								gc.setBackground(WindowManager.getColor(Soar2D.simulation.kColors[roomID]));
							}
						}
						ArrayList<CellObject> blocks = map.getAllWithProperty(location, "mblock");
						if (blocks.size() > 0) {
							gc.setBackground(new Color(e.display, 191, 123, 79));
						}
					} else {
						gc.setBackground(WindowManager.white);
					}
					gc.fillRectangle(cellSize*location.x, cellSize*location.y, cellSize, cellSize);

					if (this.map.getPlayer(location) != null) {
						playerLocs.add(new java.awt.Point(location));
					}
				}

				ArrayList<CellObject> objectIds = map.getAllWithProperty(location, "object-id");
				if (objectIds.size() > 0) {
					gc.setForeground(WindowManager.red);
					gc.drawString(objectIds.get(0).getProperty("object-id"), cellSize*location.x, cellSize*location.y);
				} else  {
					ArrayList<CellObject> numbers = map.getAllWithProperty(location, "number");
					if (numbers.size() > 0) {
						if (!roomIds.contains(numbers.get(0).getIntProperty("number"))) {
							gc.setForeground(WindowManager.green);
							gc.drawString(numbers.get(0).getProperty("number"), cellSize*location.x, cellSize*location.y);
							roomIds.add(numbers.get(0).getIntProperty("number"));
						}
					}
				}
			}
		}
		
		// draw entities now so they appear on top
		Iterator<java.awt.Point> playerLocIter = playerLocs.iterator();
		while (playerLocIter.hasNext()) {
			Player player = this.map.getPlayer(playerLocIter.next());
			//FIXME:
			if (player == null) {
				continue;
			}
			assert player != null;

			PlayersManager players = Soar2D.simulation.world.getPlayers();
			Point2D.Double center = new Point2D.Double(players.getFloatLocation(player).x, players.getFloatLocation(player).y);
			Point2D.Double offset = new Point2D.Double(0,0);
			
			Path path = new Path(gc.getDevice());
			double heading = player.getHeadingRadians();
			if (Soar2D.bConfig.getContinuous() == false) {
				heading = Direction.radiansOf[player.getFacingInt()];
			}

			// first, move to the point representing the tip of the chevron
			offset.y = (float)kDotSize * (float)Math.sin(heading);
			offset.x = (float)kDotSize * (float)Math.cos(heading);
			Point2D.Double original = new Point2D.Double(offset.x, offset.y);
			path.moveTo((float)(center.x + offset.x), (float)(center.y + offset.y));
			//System.out.println("First: " + offset);

			// next draw a line to the corner
			offset.y = kDotSize/2.0f * (float)Math.sin(heading + (3*Math.PI)/4);
			offset.x = kDotSize/2.0f * (float)Math.cos(heading + (3*Math.PI)/4);
			path.lineTo((float)(center.x + offset.x), (float)(center.y + offset.y));
			//System.out.println("Second: " + offset);

			// next draw a line to the other corner
			offset.y = kDotSize/2.0f * (float)Math.sin(heading - (3*Math.PI)/4);
			offset.x = kDotSize/2.0f * (float)Math.cos(heading - (3*Math.PI)/4);
			path.lineTo((float)(center.x + offset.x), (float)(center.y + offset.y));				
			//System.out.println("Third: " + offset);

			// finally a line back to the original
			path.lineTo((float)(center.x + original.x), (float)(center.y + original.y));
			
			gc.setForeground(WindowManager.getColor(player.getColor()));
			gc.drawPath(path);
		}
	}
}
