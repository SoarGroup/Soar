package edu.umich.soar.gridmap2d.visuals;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import lcmtypes.pose_t;

import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Path;
import org.eclipse.swt.widgets.Composite;

import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.players.RoomPlayer;
import edu.umich.soar.gridmap2d.world.RoomWorld;
import edu.umich.soar.gridmap2d.world.World;

public class RoomVisualWorld extends VisualWorld {

	private boolean colored_rooms = false;
	RoomWorld world;
	
	public RoomVisualWorld(Composite parent, int style, int cellSize, World world) {
		super(parent, style, cellSize);
		this.world = (RoomWorld)world;
	}
	
	private static class IdLabel {
		int [] loc;
		boolean object;
		boolean gateway;
		String label;
	}
	
	List<IdLabel> ids = new ArrayList<IdLabel>();

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
		ids.clear();
		int [] location = new int [2];
		HashSet<Integer> roomIds = new HashSet<Integer>();
		for(location[0] = 0; location[0] < map.size(); ++location[0]){
			for(location[1] = 0; location[1] < map.size(); ++location[1]){
				int [] drawLocation = new int [] { cellSize*location[0], cellSize*(map.size() - location[1] - 1) };
				
				boolean gateway = false;
				if (!this.map.getCell(location).checkAndResetRedraw() && painted) {
					continue;
				}
				
				if (this.map.getCell(location).hasAnyWithProperty(Names.kPropertyBlock)) {
				    gc.setBackground(WindowManager.black);
				    gc.fillRectangle(drawLocation[0], drawLocation[1], cellSize, cellSize);
					
				} else {
					
					gateway = map.getCell(location).hasAnyWithProperty(Names.kPropertyGatewayRender); 
					if (!gateway) {

						if (!colored_rooms) {
							// normal:
							gc.setBackground(WindowManager.widget_background);
						} else {
							// colored rooms:
							CellObject roomObject = map.getCell(location).getObject(Names.kRoomID);
							if (roomObject == null)  {
								gc.setBackground(WindowManager.widget_background);
							} else {
								int roomID = roomObject.getIntProperty(Names.kPropertyNumber, -1);
								roomID %= Gridmap2D.simulation.kColors.length - 1; // the one off eliminates black
								gc.setBackground(WindowManager.getColor(Gridmap2D.simulation.kColors[roomID]));
							}
						}
						if (map.getCell(location).hasAnyWithProperty(Names.kRoomObjectName)) {
							gc.setBackground(WindowManager.darkGray);
						}
					} else {
						gc.setBackground(WindowManager.white);
					}
					gc.fillRectangle(drawLocation[0], drawLocation[1], cellSize, cellSize);
				}

				List<CellObject> objectIds = map.getCell(location).getAllWithProperty("object-id");
				if (objectIds != null) {
					IdLabel label = new IdLabel();
					label.object = true;
					label.label = objectIds.get(0).getProperty("object-id");
					label.loc = new int [] { drawLocation[0] + 1, drawLocation[1] };
					ids.add(label);
				} else  {
					List<CellObject> numbers = map.getCell(location).getAllWithProperty("number");
					if (numbers!= null) {
						if (!roomIds.contains(numbers.get(0).getIntProperty("number", -1))) {
							roomIds.add(numbers.get(0).getIntProperty("number", -1));
							IdLabel label = new IdLabel();
							label.object = false;
							label.gateway = gateway;
							label.label = numbers.get(0).getProperty("number");
							label.loc = new int [] { drawLocation[0] + 1, drawLocation[1] };
							ids.add(label);
						}
					}
				}
			}
		}
		
		// draw id labels on top of map
		for (IdLabel label : ids) {
			if (label.object) {
				gc.setBackground(WindowManager.black);
				gc.setForeground(WindowManager.white);
				gc.drawString(label.label, label.loc[0], label.loc[1]);
			} else {
				if (label.gateway) {
					gc.setBackground(WindowManager.white);
				} else {
					gc.setBackground(WindowManager.widget_background);
				}
				gc.setForeground(WindowManager.black);
				gc.drawString(label.label, label.loc[0], label.loc[1]);
			}
		}
		
		// draw entities now so they appear on top
		for (Player p : world.getPlayers()) {
			RoomPlayer player = (RoomPlayer)p;
			
			pose_t pose = player.getState().getPose();
			float [] center = new float [] { (float)pose.pos[0], (float)pose.pos[1] };
			float [] offset = new float [] { 0, 0 };
			
			Path path = new Path(gc.getDevice());
			float heading = (float)player.getState().getYaw();

			// first, move to the point representing the tip of the chevron
			offset[1] = kDotSize * (float)Math.sin(heading);
			offset[0] = kDotSize * (float)Math.cos(heading);
			float [] original = new float [] { offset[0], offset[1] };
			path.moveTo((center[0] + offset[0]), map.size() * cellSize - (center[1] + offset[1]));
			//System.out.println("First: " + offset);

			// next draw a line to the corner
			offset[1] = kDotSize/2.0f * (float)Math.sin(heading + (3*Math.PI)/4);
			offset[0] = kDotSize/2.0f * (float)Math.cos(heading + (3*Math.PI)/4);
			path.lineTo((center[0] + offset[0]), map.size() * cellSize - (center[1] + offset[1]));
			//System.out.println("Second: " + offset);

			// next draw a line to the other corner
			offset[1] = kDotSize/2.0f * (float)Math.sin(heading - (3*Math.PI)/4);
			offset[0] = kDotSize/2.0f * (float)Math.cos(heading - (3*Math.PI)/4);
			path.lineTo((center[0] + offset[0]), map.size() * cellSize - (center[1] + offset[1]));				
			//System.out.println("Third: " + offset);

			// finally a line back to the original
			path.lineTo((center[0] + original[0]), map.size() * cellSize - (center[1] + original[1]));
			
			gc.setForeground(WindowManager.getColor(player.getColor()));
			gc.drawPath(path);
		}
	}

	@Override
	Player getPlayerAtPixel(int [] loc) {
		loc[1] = map.size() * cellSize - loc[1];
		int[] xy = getCellAtPixel(loc);
		if (xy == null) {
			return null;
		}
		return this.map.getCell(xy).getPlayer();
	}
	
}
