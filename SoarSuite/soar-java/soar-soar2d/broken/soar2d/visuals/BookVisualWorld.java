package broken.soar2d.visuals;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Path;
import org.eclipse.swt.widgets.Composite;

import soar2d.Names;
import soar2d.Soar2D;
import soar2d.map.CellObject;
import soar2d.players.Player;
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
		List<int []> playerLocs = new ArrayList<int []>();
		int [] location = new int [2];
		Set<Integer> roomIds = new HashSet<Integer>();
		for(location[0] = 0; location[0] < map.getSize(); ++location[0]){
			for(location[1] = 0; location[1] < map.getSize(); ++location[1]){
				if (!this.map.resetRedraw(location) && painted) {
					continue;
				}
				
				if (this.map.hasAnyWithProperty(location, Names.kPropertyBlock)) {
				    gc.setBackground(WindowManager.black);
				    gc.fillRectangle(cellSize*location[0], cellSize*location[1], cellSize, cellSize);
					
				} else {
					
					if (!map.hasAnyWithProperty(location, Names.kPropertyGatewayRender)) {

						if (!Soar2D.config.roomConfig().colored_rooms) {
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
						if (map.hasAnyWithProperty(location, Names.kBookObjectName)) {
							gc.setBackground(WindowManager.darkGray);
						}
					} else {
						gc.setBackground(WindowManager.white);
					}
					gc.fillRectangle(cellSize*location[0], cellSize*location[1], cellSize, cellSize);

					if (this.map.getPlayer(location) != null) {
						playerLocs.add(Arrays.copyOf(location, location.length));
					}
				}

				List<CellObject> objectIds = map.getAllWithProperty(location, "object-id");
				if (objectIds != null) {
					gc.setForeground(WindowManager.white);
					gc.drawString(objectIds.get(0).getProperty("object-id"), cellSize*location[0], cellSize*location[1]);
				} else  {
					List<CellObject> numbers = map.getAllWithProperty(location, "number");
					if (numbers!= null) {
						if (!roomIds.contains(numbers.get(0).getIntProperty("number"))) {
							gc.setForeground(WindowManager.black);
							gc.drawString(numbers.get(0).getProperty("number"), cellSize*location[0], cellSize*location[1]);
							roomIds.add(numbers.get(0).getIntProperty("number"));
						}
					}
				}
			}
		}
		
		// draw entities now so they appear on top
		for (int [] playerLoc : playerLocs) {
			Player player = this.map.getPlayer(playerLoc);
			//FIXME:
			if (player == null) {
				continue;
			}
			assert player != null;

			PlayersManager players = Soar2D.simulation.world.getPlayers();
			double [] center = new double [] { players.getFloatLocation(player)[0], players.getFloatLocation(player)[1] };
			double [] offset = new double [] { 0, 0 };
			
			Path path = new Path(gc.getDevice());
			double heading = player.getHeadingRadians();

			// first, move to the point representing the tip of the chevron
			offset[1] = (float)kDotSize * (float)Math.sin(heading);
			offset[0] = (float)kDotSize * (float)Math.cos(heading);
			double [] original = new double [] { offset[0], offset[1] };
			path.moveTo((float)(center[0] + offset[0]), (float)(center[1] + offset[1]));
			//System.out.println("First: " + offset);

			// next draw a line to the corner
			offset[1] = kDotSize/2.0f * (float)Math.sin(heading + (3*Math.PI)/4);
			offset[0] = kDotSize/2.0f * (float)Math.cos(heading + (3*Math.PI)/4);
			path.lineTo((float)(center[0] + offset[0]), (float)(center[1] + offset[1]));
			//System.out.println("Second: " + offset);

			// next draw a line to the other corner
			offset[1] = kDotSize/2.0f * (float)Math.sin(heading - (3*Math.PI)/4);
			offset[0] = kDotSize/2.0f * (float)Math.cos(heading - (3*Math.PI)/4);
			path.lineTo((float)(center[0] + offset[0]), (float)(center[1] + offset[1]));				
			//System.out.println("Third: " + offset);

			// finally a line back to the original
			path.lineTo((float)(center[0] + original[0]), (float)(center[1] + original[1]));
			
			gc.setForeground(WindowManager.getColor(player.getColor()));
			gc.drawPath(path);
		}
	}
}
