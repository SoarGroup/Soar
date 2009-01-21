package soar2d.visuals;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.Canvas;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;

import soar2d.Direction;
import soar2d.Names;
import soar2d.Soar2D;
import soar2d.map.CellObject;
import soar2d.map.CellObjectManager;
import soar2d.map.GridMap;
import soar2d.players.Player;
import soar2d.players.RadarCell;
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

		switch(Soar2D.config.game()) {
		case TANKSOAR:
			tanks.put(new Integer(Direction.kSouthInt), new Image(display, Soar2D.class.getResourceAsStream("/images/tanksoar/tank_down.gif")));
			tanks.put(new Integer(Direction.kNorthInt), new Image(display, Soar2D.class.getResourceAsStream("/images/tanksoar/tank_up.gif")));
			tanks.put(new Integer(Direction.kEastInt), new Image(display, Soar2D.class.getResourceAsStream("/images/tanksoar/tank_right.gif")));
			tanks.put(new Integer(Direction.kWestInt), new Image(display, Soar2D.class.getResourceAsStream("/images/tanksoar/tank_left.gif")));
			break;
			
		case EATERS:
			break;
			
		case ROOM:
			break;
			
		case KITCHEN:
			break;
			
		case TAXI:
			break;
		}
		
		font = new Font(parent.getDisplay(), "Helvetica", 9, SWT.NONE);
		
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

	int [] getCellAtPixel(int [] loc) {
		int [] pixelLoc = Arrays.copyOf(loc, loc.length);
		pixelLoc[0] /= cellSize;
		pixelLoc[1] /= cellSize;
		if (map.isInBounds(pixelLoc)) {
			return pixelLoc;
		}
		return null;
	}
	
	Player getPlayerAtPixel(int [] loc) {
		return this.map.getPlayer(getCellAtPixel(loc));
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
		
		int [] center = new int [] { xBase + halfCell, yBase + halfCell };
		int [] north = new int [] { center[0], yBase };
		int [] east = new int [] { xBase + cellSize, center[1] };
		int [] south = new int [] { center[0], yBase + cellSize };
		int [] west = new int [] { xBase, center[1] };
		
		gc.fillPolygon(new int[] {center[0], center[1], north[0], north[1], center[0] + offCenter, center[1]});
		gc.fillPolygon(new int[] {center[0], center[1], east[0],  east[1],  center[0], center[1] + offCenter});
		gc.fillPolygon(new int[] {center[0], center[1], south[0], south[1], center[0] - offCenter, center[1]});
		gc.fillPolygon(new int[] {center[0], center[1], west[0],  west[1],  center[0], center[1] - offCenter});
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
