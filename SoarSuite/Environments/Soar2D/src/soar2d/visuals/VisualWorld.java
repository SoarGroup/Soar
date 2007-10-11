package soar2d.visuals;

import java.util.*;

import org.eclipse.swt.*;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.*;

import soar2d.*;
import soar2d.map.*;
import soar2d.player.*;
import soar2d.world.PlayersManager;

public abstract class VisualWorld extends Canvas implements PaintListener {
	
	public static HashMap<Player, Color> playerColors = new HashMap<Player, Color>();
	
	public static void remapPlayerColors() {
		PlayersManager players = Soar2D.simulation.world.getPlayers();
		playerColors.clear();
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			String color = player.getColor();
			playerColors.put(player, WindowManager.getColor(color));
		}
	}
	
	protected static HashMap<String, Image> images = new HashMap<String, Image>();
	protected static HashMap<Integer, Image> tanks = new HashMap<Integer, Image>();

	public static boolean internalRepaint = false;
	
	protected Display display;
	protected int cellSize;
	protected static final int kDotSize = 7;
	protected boolean disabled = false;
	protected boolean painted = false;
	protected int lastX = 0;
	protected int lastY = 0;
	
	protected GridMap map;
	
	protected Font font;
	
	public VisualWorld(Composite parent, int style, int cellSize) {
		super(parent, style | SWT.NO_BACKGROUND);
		display = parent.getDisplay();
		this.cellSize = cellSize;

		switch(Soar2D.config.getType()) {
		case kTankSoar:
			tanks.put(new Integer(Direction.kSouthInt), new Image(display, Soar2D.class.getResourceAsStream("/images/tanksoar/tank_down.gif")));
			tanks.put(new Integer(Direction.kNorthInt), new Image(display, Soar2D.class.getResourceAsStream("/images/tanksoar/tank_up.gif")));
			tanks.put(new Integer(Direction.kEastInt), new Image(display, Soar2D.class.getResourceAsStream("/images/tanksoar/tank_right.gif")));
			tanks.put(new Integer(Direction.kWestInt), new Image(display, Soar2D.class.getResourceAsStream("/images/tanksoar/tank_left.gif")));
			break;
			
		case kEaters:
			break;
			
		case kBook:
			break;
			
		case kKitchen:
			break;
			
		case kTaxi:
			break;
		}
		
		font = new Font(parent.getDisplay(), "Helvetica", 7, SWT.NONE);
		
		addPaintListener(this);		
	}
	
	public void setMap(GridMap map) {
		this.map = map;
		assert this.map != null;
		
		painted = false;
		
		CellObjectManager manager = this.map.getObjectManager();
		Iterator<CellObject> iter = manager.getTemplatesWithProperty(Names.kPropertyMiniImage).iterator();
		while (iter.hasNext()) {
			CellObject obj = iter.next();
			Image image = new Image(WindowManager.display, Soar2D.class.getResourceAsStream("/images/tanksoar/" + obj.getProperty(Names.kPropertyMiniImage)));
			assert image != null;
			if (obj.getName().equals(Names.kEnergy)) {
				RadarCell.energyImage = image;
			} else if (obj.getName().equals(Names.kHealth)) {
				RadarCell.healthImage = image;
			} else if (obj.getName().equals(Names.kMissiles)) {
				RadarCell.missilesImage = image;
			} else if (obj.hasProperty(Names.kPropertyBlock)) {
				RadarCell.obstacleImage = image;
			} else if (obj.getName().equals(Names.kGround)) {
				RadarCell.openImage = image;
			}
		}
	}

	java.awt.Point getCellAtPixel(int x, int y) {
		x /= cellSize;
		y /= cellSize;
		if (map.isInBounds(x, y)) {
			return new java.awt.Point(x, y);
		}
		return null;
	}
	
	Player getPlayerAtPixel(int x, int y) {
		return this.map.getPlayer(getCellAtPixel(x, y));
	}
	
	class DrawMissile {
		private GC gc;
		private Image image;
		private int x;
		private int y;
		private Color color;
		
		public DrawMissile(GC gc, Image image, int x, int y, Color color) {
			this.gc = gc;
			this.image = image;
			this.x = x;
			this.y = y;
			this.color = color;
		}
		
		public void draw() {
			gc.drawImage(image, x, y);
			if (color != null) {
				gc.setBackground(color);
				gc.fillOval(x+3, y+3, kDotSize, kDotSize);
			}
		}
	}
	
	public void paintControl(PaintEvent e) {
		assert false;
	}
	
	Image bootstrapImage(String imageName) {
		Image image = new Image(WindowManager.display, Soar2D.class.getResourceAsStream("/images/tanksoar/" + imageName));
		assert image != null;
		images.put(imageName, image);
		return image;
	}
	
	void drawExplosion(GC gc, int x, int y) {
		gc.setBackground(WindowManager.red);
		int offCenter = cellSize/4;
		int xBase = cellSize*x;
		int yBase = cellSize*y;
		int halfCell = cellSize/2;
		
		java.awt.Point center = new java.awt.Point(xBase + halfCell, yBase + halfCell);
		java.awt.Point north = new java.awt.Point(center.x, yBase);
		java.awt.Point east = new java.awt.Point(xBase + cellSize, center.y);
		java.awt.Point south = new java.awt.Point(center.x, yBase + cellSize);
		java.awt.Point west = new java.awt.Point(xBase, center.y);
		
		gc.fillPolygon(new int[] {center.x, center.y, north.x, north.y, center.x + offCenter, center.y});
		gc.fillPolygon(new int[] {center.x, center.y, east.x,  east.y,  center.x, center.y + offCenter});
		gc.fillPolygon(new int[] {center.x, center.y, south.x, south.y, center.x - offCenter, center.y});
		gc.fillPolygon(new int[] {center.x, center.y, west.x,  west.y,  center.x, center.y - offCenter});
	}
	
	public void setRepaint() {
		painted = false;
	}
	
	public void disable() {
		disabled = true;
	}
	
	public void enable() {
		disabled = false;
	}
	
	public int getWidth() {
		return cellSize * this.map.getSize();
	}
	
	public int getHeight() {
		return cellSize * this.map.getSize();
	}
	
}
