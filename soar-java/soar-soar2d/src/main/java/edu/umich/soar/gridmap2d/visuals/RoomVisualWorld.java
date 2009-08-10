package edu.umich.soar.gridmap2d.visuals;

import java.util.HashMap;
import java.util.List;

import lcmtypes.pose_t;

import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Path;
import org.eclipse.swt.widgets.Composite;

import com.commsen.stopwatch.Stopwatch;

import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.map.GridMap;
import edu.umich.soar.gridmap2d.map.RoomMap;
import edu.umich.soar.gridmap2d.map.RoomObject;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.players.Robot;
import edu.umich.soar.gridmap2d.world.RoomWorld;
import edu.umich.soar.gridmap2d.world.World;

public class RoomVisualWorld extends VisualWorld {

	RoomWorld world;
	
	public RoomVisualWorld(Composite parent, int style, int cellSize, World world) {
		super(parent, style, cellSize);
		this.world = (RoomWorld)world;
	}
	
	private static class IdLabel {
		int [] loc;
		boolean gateway;
		String label;
	}
	
	private final HashMap<Integer, IdLabel> rmids = new HashMap<Integer, IdLabel>();
	
	private static class Location {
		private final int CELL_SIZE;
		private final int MAP_SIZE;
		private final int [] loc = new int [2];
		private final int [] dloc = new int [2];
		private boolean first;
		
		Location(int cellSize, int mapSize) {
			this.CELL_SIZE = cellSize;
			this.MAP_SIZE = mapSize;
			reset();
		}
		
		void reset() {
			loc[0] = 0;
			loc[1] = 0;
			dloc[0] = 0;
			dloc[1] = CELL_SIZE*(MAP_SIZE - 1);
			first = true;
		}
		
		boolean next() {
			if (first) {
				first = false;
				return true;
			}
			
			if (++loc[0] < MAP_SIZE) {
				dloc[0] += CELL_SIZE;
				return true;
			}
			loc[0] = 0;
			dloc[0] = 0;
			if (++loc[1] < MAP_SIZE) {
				dloc[1] -= CELL_SIZE;
				return true;
			}
			return false;
		}
		
		int[] getLoc() {
			return loc;
		}
		
		int[] getDraw() {
			return dloc;
		}
	}
	
	private Location loc;
	private boolean walls[][];
	private boolean gateways[][];
	
	@Override
	public void setMap(GridMap map) {
		super.setMap(map);
		
		rmids.clear();
		loc = new Location(cellSize, map.size());
		
		walls = new boolean[map.size()][];
		for (int i = 0; i < walls.length; ++i) {
			walls[i] = new boolean[map.size()];
		}

		gateways = new boolean[map.size()][];
		for (int i = 0; i < gateways.length; ++i) {
			gateways[i] = new boolean[map.size()];
		}
		
		RoomMap rmap = (RoomMap)map;
		while (loc.next()) {
			CellObject number = map.getFirstObjectWithProperty(loc.getLoc(), Names.kPropertyNumber);
			if (number != null) {
				if (!rmids.containsKey(number.getIntProperty(Names.kPropertyNumber, -1))) {
					IdLabel label = new IdLabel();
					label.gateway = rmap.hasAnyObjectWithProperty(loc.getLoc(), Names.kPropertyGatewayRender);
					label.label = number.getProperty(Names.kPropertyNumber);
					label.loc = new int [] { loc.getDraw()[0] + 1, loc.getDraw()[1] };
					rmids.put(number.getIntProperty(Names.kPropertyNumber, -1), label);
				}
			}
			
			walls[loc.getLoc()[0]][loc.getLoc()[1]] = rmap.hasAnyObjectWithProperty(loc.getLoc(), Names.kPropertyBlock);
			gateways[loc.getLoc()[0]][loc.getLoc()[1]] = rmap.hasAnyObjectWithProperty(loc.getLoc(), Names.kPropertyGatewayRender);
		}
	}
	
	@Override
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
		}
		
		if (System.getProperty("os.name").contains("Mac OS X"))
			painted = false;

		// Draw world
		long id = Stopwatch.start("paintControl", "draw world");
		RoomMap rmap = (RoomMap)map;
		loc.reset();
		while (loc.next()) {
			if (!rmap.checkAndResetRedraw(loc.getLoc()) && painted) {
				continue;
			}
			
			if (walls[loc.getLoc()[0]][loc.getLoc()[1]]) {
			    gc.setBackground(WindowManager.black);
			} 
			else if (gateways[loc.getLoc()[0]][loc.getLoc()[1]]) {
				gc.setBackground(WindowManager.white);
			} 
			else {
				gc.setBackground(WindowManager.widget_background);
			}
			
			gc.fillRectangle(loc.getDraw()[0], loc.getDraw()[1], cellSize, cellSize);
		}
		
		// draw objects
		for (RoomObject ro : rmap.getRoomObjects()) {
			pose_t pose = ro.getPose();
			if (pose == null) {
				continue;
			}
			gc.setBackground(ro.getColor());
			gc.fillOval((int)pose.pos[0] - 2, cellSize*map.size() - (int)pose.pos[1] - 2, 4, 4);
		}
		
		// draw id labels on top of map
		for (IdLabel label : rmids.values()) {
			if (label.gateway) {
				gc.setBackground(WindowManager.white);
			} else {
				gc.setBackground(WindowManager.widget_background);
			}
			gc.setForeground(WindowManager.black);
			gc.drawString(label.label, label.loc[0], label.loc[1]);
		}
		
		// draw entities now so they appear on top
		for (Player p : world.getPlayers()) {
			Robot player = (Robot)p;
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
			
			// draw waypoints
			List<double[]> waypoints = world.getWaypointList(player);
			for (double[] wp : waypoints) {
				int left = (int)wp[0] - 2;
				int top = cellSize*map.size() - (int)wp[1] - 2;
				int right = left + 4;
				int bottom = top + 4;
				
				gc.setForeground(WindowManager.getColor(player.getColor()));
				gc.drawOval(left, top, 4, 4);
				
				// set redraw on all four points for next cycle
				map.forceRedraw(new int[] {left / cellSize, top / cellSize});
				map.forceRedraw(new int[] {left / cellSize, bottom / cellSize});
				map.forceRedraw(new int[] {right / cellSize, top / cellSize});
				map.forceRedraw(new int[] {right / cellSize, bottom / cellSize});
			}
		}
		Stopwatch.stop(id);	
		
		painted = true;
	}

	@Override
	Player getPlayerAtPixel(int [] loc) {
		loc[1] = map.size() * cellSize - loc[1];
		int[] xy = getCellAtPixel(loc);
		if (xy == null) {
			return null;
		}
		return this.map.getFirstPlayer(xy);
	}
	
}
