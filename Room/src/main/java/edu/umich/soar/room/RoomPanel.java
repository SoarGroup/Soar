package edu.umich.soar.room;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Polygon;
import java.awt.Stroke;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.commsen.stopwatch.Stopwatch;

import lcmtypes.pose_t;

import edu.umich.soar.room.core.Names;
import edu.umich.soar.room.core.Simulation;
import edu.umich.soar.room.core.events.ResetEvent;
import edu.umich.soar.room.events.SimEvent;
import edu.umich.soar.room.events.SimEventListener;
import edu.umich.soar.room.map.Cell;
import edu.umich.soar.room.map.CellObject;
import edu.umich.soar.room.map.Robot;
import edu.umich.soar.room.map.RoomMap;
import edu.umich.soar.room.map.RoomObject;
import edu.umich.soar.room.map.RoomWorld;

public class RoomPanel extends GridMapPanel implements SimEventListener {
	
	private static final long serialVersionUID = -8083633808532173643L;

	private static final double SCALE = 1.5;
	private static final int CELL_SIZE = (int)(RoomWorld.CELL_SIZE * SCALE);
	private static final float DOT_SCALE = 0.6f;
	private static final float DOT_SIZE = CELL_SIZE * DOT_SCALE;
	private static final float PYRAMID_SIZE = DOT_SIZE * 0.65f;
	private final Simulation sim;
	private static final Polygon TRIANGLE = new Polygon();
	private static final Polygon OBJECT_PYRAMID = new Polygon();
	private static final Polygon OBJECT_CUBE = new Polygon();
	
	static {
		{
			float x = DOT_SIZE;
			float y = 0;
			TRIANGLE.addPoint(Math.round(x), Math.round(y));
			
			// next draw a line to the corner
			x = DOT_SIZE/2.0f * (float)Math.cos((3*Math.PI)/4);
			y = DOT_SIZE/2.0f * (float)Math.sin((3*Math.PI)/4);
			TRIANGLE.addPoint(Math.round(x), Math.round(y));
	
			// next draw a line to the other corner
			x = DOT_SIZE/2.0f * (float)Math.cos((-3*Math.PI)/4);
			y = DOT_SIZE/2.0f * (float)Math.sin((-3*Math.PI)/4);
			TRIANGLE.addPoint(Math.round(x), Math.round(y));
		}
		{
			float x = PYRAMID_SIZE;
			float y = 0;
			OBJECT_PYRAMID.addPoint(Math.round(x), Math.round(y));
			
			x = PYRAMID_SIZE * (float)Math.cos((2*Math.PI)/3);
			y = PYRAMID_SIZE * (float)Math.sin((2*Math.PI)/3);
			OBJECT_PYRAMID.addPoint(Math.round(x), Math.round(y));
	
			// next draw a line to the other corner
			x = PYRAMID_SIZE * (float)Math.cos((-2*Math.PI)/3);
			y = PYRAMID_SIZE * (float)Math.sin((-2*Math.PI)/3);
			OBJECT_PYRAMID.addPoint(Math.round(x), Math.round(y));
		}
		{
			int half = Math.round(DOT_SIZE / 2.0f);
			OBJECT_CUBE.addPoint(half, half);
			OBJECT_CUBE.addPoint(-half, half);
			OBJECT_CUBE.addPoint(-half, -half);
			OBJECT_CUBE.addPoint(half, -half);
		}
	}
	
	public RoomPanel(Adaptable app) {
		sim = Adaptables.adapt(app, Simulation.class);
		
		this.setDoubleBuffered(true);
		
		// FIXME this needs to be called on map event
		setMap((RoomMap)sim.getMap());
		
		sim.getEvents().addListener(ResetEvent.class, this);
	}
	
	private static class IdLabel {
		int [] loc;
		String label;
	}
	
	private final Map<Integer, IdLabel> rmids = new HashMap<Integer, IdLabel>();
	private final Map<Robot, Breadcrumbs> breadcrumbs = new HashMap<Robot, Breadcrumbs>();
	private boolean breadcrumbsEnabled = true;
	
	private static class Location {
		private final int MAP_SIZE;
		private final int [] loc = new int [2];
		private final int [] dloc = new int [2];
		private boolean first;
		
		Location(int mapSize) {
			this.MAP_SIZE = mapSize;
			reset();
		}
		
		void reset() {
			loc[0] = 0;
			loc[1] = 0;
			dloc[0] = 0;
			dloc[1] = CELL_SIZE * (MAP_SIZE - 1);
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
	
	void setMap(RoomMap map) {
		rmids.clear();
		loc = new Location(map.size());
		
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
			CellObject number = map.getCell(loc.getLoc()).getFirstObjectWithProperty(Names.kPropertyNumber);
			if (number != null) {
				if (!rmids.containsKey(number.getProperty(Names.kPropertyNumber, -1, Integer.class))) {
					IdLabel label = new IdLabel();
					label.label = number.getProperty(Names.kPropertyNumber);
					label.loc = new int [] { loc.getDraw()[0] + 1, loc.getDraw()[1] };
					rmids.put(number.getProperty(Names.kPropertyNumber, -1, Integer.class), label);
				}
			}
			
			walls[loc.getLoc()[0]][loc.getLoc()[1]] = rmap.getCell(loc.getLoc()).hasObjectWithProperty(Names.kPropertyBlock);
			gateways[loc.getLoc()[0]][loc.getLoc()[1]] = rmap.getCell(loc.getLoc()).hasObjectWithProperty(Names.kPropertyGatewayRender);
		}
	}
	
	@Override
	int getCellSize() {
		return CELL_SIZE;
	}
	
	double[] worldToScreenPos(double[] world) {
		double[] screen = new double[world.length];
		System.arraycopy(world, 0, screen, 0, world.length);
		screen[0] *= SCALE;
		screen[1] *= SCALE;
		RoomMap map = (RoomMap)sim.getMap();
		screen[1] = CELL_SIZE * map.size() - (int)screen[1];
		return screen;
	}
	
	@Override
	int [] getCellAtPixel(int x, int y) {
		RoomMap map = (RoomMap)sim.getMap();
		y = map.size() * CELL_SIZE - y;
		return super.getCellAtPixel(x, y);
	}

	@Override
	protected void paintComponent(Graphics gr) {
		long id = Stopwatch.start("RoomPanel", "paintComponent");
		try {
			super.paintComponent(gr);
			RoomMap map = (RoomMap)sim.getMap();
			RoomWorld world = (RoomWorld)sim.getWorld();
			Graphics2D g2d = (Graphics2D)gr;
			
			loc.reset();
			while (loc.next()) {
				Cell cell = map.getCell(loc.getLoc());
				if (sim.isRunning()) {
					if (!cell.isModified()) {
						//continue;
					}
					cell.setModified(false);
				}
				
				if (walls[loc.getLoc()[0]][loc.getLoc()[1]]) {
					g2d.setColor(Color.BLACK);
				} 
				else if (gateways[loc.getLoc()[0]][loc.getLoc()[1]]) {
					//g2d.setColor(SystemColor.control);
					g2d.setColor(Color.decode("0xd3d3d3"));
				} 
				else {
					g2d.setColor(Color.WHITE);
				}
				
				g2d.fillRect(loc.getDraw()[0], loc.getDraw()[1], CELL_SIZE, CELL_SIZE);
			}
			
			// draw objects
			for (RoomObject ro : map.getRoomObjects()) {
				pose_t pose = ro.getPose();
				if (pose == null) {
					continue;
				}
				g2d.setColor(ro.getColor());
				double[] screen = worldToScreenPos(pose.pos);
				String shape = ro.getCellObject().getProperty("shape");
				if (shape != null && shape.equals("pyramid")) {
					g2d.translate(screen[0], screen[1]);
					g2d.fillPolygon(OBJECT_PYRAMID);
					g2d.translate(-screen[0], -screen[1]);
				} else if (shape != null && shape.equals("cube")) {
					g2d.translate(screen[0], screen[1]);
					g2d.fillPolygon(OBJECT_CUBE);
					g2d.translate(-screen[0], -screen[1]);
				} else {
					g2d.fillOval((int)screen[0] - (int)(DOT_SIZE/2), (int)screen[1] - (int)(DOT_SIZE/2), (int)DOT_SIZE, (int)DOT_SIZE);
				}
			}
	
			// draw id labels on top of map
			for (IdLabel label : rmids.values()) {
				g2d.setColor(Color.BLACK);
				g2d.drawString(label.label, label.loc[0], label.loc[1] + CELL_SIZE - 2);
			}
			
			// draw entities now so they appear on top
			for (Robot p : world.getPlayers()) {
				Robot player = (Robot)p;
				pose_t pose = player.getState().getPose();
				g2d.setColor(Colors.getColor(player.getColor()));
				
				double[] screen = worldToScreenPos(pose.pos);

				int[] bcX;
				int[] bcY;
				int bcS;
				synchronized(breadcrumbs) {
					if (!breadcrumbs.containsKey(player)) {
						breadcrumbs.put(player, new Breadcrumbs());
					}
					Breadcrumbs bc = breadcrumbs.get(player);
					bcX = bc.getX();
					bcY = bc.getY();
					bcS = bc.getSteps();

					// add new
					bc.addBreadcrumb(screen[0], screen[1]);
				}
				
				// draw breadcrumbs
				if (breadcrumbsEnabled) {
					g2d.setStroke(DOTTED_STROKE);
					g2d.drawPolyline(bcX, bcY, bcS);
					g2d.setStroke(new BasicStroke());
				}
				
				// draw player
				g2d.translate(screen[0], screen[1]);
				g2d.rotate(-player.getState().getYaw());
				g2d.fillPolygon(TRIANGLE);
				g2d.rotate(player.getState().getYaw());
				g2d.translate(-screen[0], -screen[1]);
				
				// draw waypoints on top of that
				List<double[]> waypoints = world.getWaypointList(player);
				for (double[] wp : waypoints) {
					double[] wpScreen = worldToScreenPos(wp);
					g2d.drawOval((int)wpScreen[0] - 2, (int)wpScreen[1] - 2, 4, 4);
				}
			}
			
		} finally {
			Stopwatch.stop(id);
		}
	}
	
	private static final Stroke DOTTED_STROKE = new BasicStroke(1.0f, BasicStroke.CAP_SQUARE, BasicStroke.JOIN_MITER, 10.0f, new float[] {3}, 0);
	
	private static class Breadcrumbs {
		private int[] x;
		private int[] y;
		private int size;
		private int steps;
		private final int CLUMP = 1000;
		
		Breadcrumbs() {
			clear();
		}
		
		private void addBreadcrumb(double x, double y) {
			int ix = (int)Math.round(x);
			int iy = (int)Math.round(y);
			if (steps > 0 && this.x[steps-1] == ix && this.y[steps-1] == iy) {
				return;
			}
			if (steps == size) {
				grow();
			}
			this.x[steps] = ix;
			this.y[steps] = iy;
			steps += 1;
		}
		
		int[] getX() {
			return x;
		}
		
		int[] getY() {
			return y;
		}
		
		int getSteps() {
			return steps;
		}
		
		void clear() {
			size = CLUMP;
			steps = 0;
			x = new int[size];
			y = new int[size];
		}

		private void grow() {
			int stemp = size;
			int[] temp = x;
			size += CLUMP;
			x = new int[size];
			System.arraycopy(temp, 0, x, 0, stemp);
			temp = y;
			y = new int[size];
			System.arraycopy(temp, 0, y, 0, stemp);
		}
		
	}
	
	@Override
	public void onEvent(SimEvent event) {
		synchronized(breadcrumbs) {
			breadcrumbs.clear();
		}
	}

	@Override
	public boolean areBreadcrumbsEnabled() {
		return breadcrumbsEnabled;
	}

	@Override
	public void clearBreadcrumbs() {
		synchronized(breadcrumbs) {
			breadcrumbs.clear();
		}
	}

	@Override
	public void setBreadcrumbsEnabled(boolean setting) {
		breadcrumbsEnabled = setting;
	}
}
