package soar2d.visuals;

import java.util.*;
import java.awt.geom.Point2D;

import org.eclipse.swt.*;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.*;

import soar2d.*;
import soar2d.Configuration.SimType;
import soar2d.player.*;
import soar2d.world.*;

public class VisualWorld extends Canvas implements PaintListener {
	
	public static HashMap<Player, Color> playerColors = new HashMap<Player, Color>();
	
	public static void remapPlayerColors() {
		ArrayList<Player> players = Soar2D.simulation.world.getPlayers();
		playerColors.clear();
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			String color = player.getColor();
			playerColors.put(player, WindowManager.getColor(color));
		}
	}
	
	private static HashMap<String, Image> images = new HashMap<String, Image>();
	private static HashMap<Integer, Image> tanks = new HashMap<Integer, Image>();
	private static Image dog;
	private static Image cat;
	private static Image mouse;

	public static boolean internalRepaint = false;
	
	java.awt.Point agentLocation;
	
	Image[][] background = null;
	
	protected Display display;
	protected int cellSize;
	private static final int kDotSize = 7;
	protected boolean disabled = false;
	protected boolean painted = false;
	protected int lastX = 0;
	protected int lastY = 0;
	
	private GridMap map;
	
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
			dog = new Image(display, Soar2D.class.getResourceAsStream("/images/book/dog.gif"));
			cat = new Image(display, Soar2D.class.getResourceAsStream("/images/book/cat.gif"));
			mouse = new Image(display, Soar2D.class.getResourceAsStream("/images/book/mouse.gif"));
			break;
		}
		
		addPaintListener(this);		
	}
	
	public void setMap(GridMap map) {
		this.map = map;
		assert this.map != null;
		
		background = null;
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

	public int getMiniWidth() {
		return cellSize * ((Soar2D.config.getEaterVision() * 2) + 1);
	}
	
	public int getMiniHeight() {
		return cellSize * ((Soar2D.config.getEaterVision() * 2) + 1);
	}
	
	public void setAgentLocation(java.awt.Point location) {
		agentLocation = location;
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
	
	private void generateBackground() {
		background = new Image[map.getSize()][map.getSize()];
		java.awt.Point location = new java.awt.Point();
		for(location.x = 0; location.x < map.getSize(); ++location.x){
			for(location.y = 0; location.y < map.getSize(); ++location.y){
				updateBackground(location);
			}
		}
	}
	
	public void updateBackground(java.awt.Point location) {
		ArrayList<CellObject> drawList = this.map.getAllWithProperty(location, Names.kPropertyImage);
		
		Iterator<CellObject> iter = drawList.iterator();
		CellObject backgroundObject = null;
		while (iter.hasNext()) {
			CellObject cellObject = iter.next();
			if (cellObject.hasProperty(Names.kPropertyBlock)) {
				backgroundObject = cellObject;
			} else if (cellObject.getName().equals(Names.kGround)) {
				backgroundObject = cellObject;
			} else if (cellObject.hasProperty(Names.kPropertyCharger)) {
				backgroundObject = cellObject;
				break;
			}
		}
		// FIXME: handle gracefully
		assert backgroundObject != null;
		
		String imageName = backgroundObject.getProperty(Names.kPropertyImage);
		if (backgroundObject.hasProperty(Names.kPropertyImageMin)) {
			int min = backgroundObject.getIntProperty(Names.kPropertyImageMin);
			int max = backgroundObject.getIntProperty(Names.kPropertyImageMax);
			int pick = Simulation.random.nextInt(max - min + 1);
			pick += min;
			imageName = imageName.substring(0, imageName.indexOf(".")) + pick + imageName.substring(imageName.indexOf("."));
		}

		Image image = images.get(imageName);
		if (image == null) {
			image = bootstrapImage(imageName);
		}
		background[location.x][location.y] = image;
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
	
	public void paintControl(PaintEvent e){
		GC gc = e.gc;		
        gc.setForeground(WindowManager.black);
		gc.setLineWidth(1);

		if (Soar2D.control.isRunning()) {
			if (agentLocation != null) {
				painted = false;
			}
			
			if (Soar2D.config.getHide()) {
				painted = true;
				if (Soar2D.control.isRunning()) {
					if (agentLocation != null) {
						synchronized(Soar2D.wm) {
							Soar2D.wm.agentDisplayUpdated = true;
							Soar2D.wm.notify();
						}
					} else {
						synchronized(Soar2D.wm) {
							Soar2D.wm.worldDisplayUpdated = true;
							Soar2D.wm.notify();
						}
					}
				}
				return;
			}
			
		} else {
			if (agentLocation != null || lastX != e.x || lastY != e.y || internalRepaint) {
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
		
		if (Soar2D.config.getType() == SimType.kTankSoar) {
			if (background == null) {
				generateBackground();
			}
		}
		
		// Draw world
		int fill1, fill2, xDraw, yDraw;
		ArrayList<DrawMissile> drawMissiles = new ArrayList<DrawMissile>();
		java.awt.Point location = new java.awt.Point();
		for(location.x = 0; location.x < map.getSize(); ++location.x){
			if (agentLocation != null) {
				if ((location.x < agentLocation.x - Soar2D.config.getEaterVision()) || (location.x > agentLocation.x + Soar2D.config.getEaterVision())) {
					continue;
				} 
				xDraw = location.x + Soar2D.config.getEaterVision() - agentLocation.x;
			} else {
				xDraw = location.x;
			}
			
			for(location.y = 0; location.y < map.getSize(); ++location.y){
				if (agentLocation != null) {
					if ((location.y < agentLocation.y - Soar2D.config.getEaterVision()) || (location.y > agentLocation.y + Soar2D.config.getEaterVision())) {
						continue;
					} 
					yDraw = location.y + Soar2D.config.getEaterVision() - agentLocation.y;
				} else {
					yDraw = location.y;
				}
				
				if (agentLocation == null) {
					if ((this.map.removeObject(location, Names.kRedraw) == null) && painted) {
						continue;
					}
				} else {
					if (!this.map.hasObject(location, Names.kRedraw) && painted) {
						continue;
					}
				}
				
				ArrayList<CellObject> drawList;
				switch (Soar2D.config.getType()) {
				case kEaters:
					drawList = this.map.getAllWithProperty(location, Names.kPropertyShape);
					
					if (!this.map.enterable(location)) {
					    gc.setBackground(WindowManager.black);
					    gc.fillRectangle(cellSize*xDraw + 1, cellSize*yDraw + 1, cellSize - 2, cellSize - 2);
						
					} else {
						boolean empty = true;
						
						Player eater = this.map.getPlayer(location);
						
						if (eater != null) {
							empty = false;
							
							gc.setBackground(playerColors.get(eater));
							gc.fillOval(cellSize*xDraw, cellSize*yDraw, cellSize, cellSize);
							gc.setBackground(WindowManager.widget_background);
							
							
							switch (eater.getFacingInt()) {
							case Direction.kNorthInt:
								drawEaterMouth(xDraw, yDraw, 1, 0, 1, 1, gc);
								break;
							case Direction.kEastInt:
								drawEaterMouth(xDraw + 1, yDraw, 0, 1, -1, 1, gc);
								break;
							case Direction.kSouthInt:
								drawEaterMouth(xDraw, yDraw + 1, 1, 0, 1, -1, gc);
								break;
							case Direction.kWestInt:
								drawEaterMouth(xDraw, yDraw, 0, 1, 1, 1, gc);
								break;
							default:
								break;
							}
						}
						
						Iterator<CellObject> iter = drawList.iterator();
						while (iter.hasNext()) {
							CellObject object = iter.next();
							
							if (empty) {
								gc.setBackground(WindowManager.widget_background);
								gc.fillRectangle(cellSize*xDraw, cellSize*yDraw, cellSize, cellSize);
							}
							empty = false;
						    
						    Color color = WindowManager.getColor(object.getProperty(Names.kPropertyColor));
						    if (color == null) {
						    	//TODO: draw outline!
						    }
							gc.setBackground(color);
							
							Shape shape = Shape.getShape(object.getProperty(Names.kPropertyShape));
							if (shape != null) {
								if (shape.equals(Shape.ROUND)) {
									fill1 = (int)(cellSize/2.8);
									fill2 = cellSize - fill1 + 1;
									gc.fillOval(cellSize*xDraw + fill1, cellSize*yDraw + fill1, cellSize - fill2, cellSize - fill2);
									gc.drawOval(cellSize*xDraw + fill1, cellSize*yDraw + fill1, cellSize - fill2 - 1, cellSize - fill2 - 1);
									
								} else if (shape.equals(Shape.SQUARE)) {
									fill1 = (int)(cellSize/2.8);
									fill2 = cellSize - fill1 + 1;
									gc.fillRectangle(cellSize*xDraw + fill1, cellSize*yDraw + fill1, cellSize - fill2, cellSize - fill2);
									gc.drawRectangle(cellSize*xDraw + fill1, cellSize*yDraw + fill1, cellSize - fill2, cellSize - fill2);
								}
							}
						}
						
						if (empty) {
							gc.setBackground(WindowManager.widget_background);
							gc.fillRectangle(cellSize*xDraw, cellSize*yDraw, cellSize, cellSize);
						}
					}
					
					if (this.map.hasObject(location, Names.kExplosion)) {
						drawExplosion(gc, xDraw, yDraw);
					}
					break;
					
				case kTankSoar:
					drawList = this.map.getAllWithProperty(location, Names.kPropertyImage);
					CellObject explosion = null;
					CellObject object = null;
					ArrayList<CellObject> missiles = new ArrayList<CellObject>();
					
					Iterator<CellObject> iter = drawList.iterator();
					while (iter.hasNext()) {
						CellObject cellObject = iter.next();
						if (cellObject.getName().equals(Names.kExplosion)) {
							explosion = cellObject;
						} else if (cellObject.hasProperty(Names.kPropertyMissiles)) {
							object = cellObject;
						} else if (cellObject.hasProperty(Names.kPropertyMissile)) {
							missiles.add(cellObject);
						}
					}
					
					Player tank = this.map.getPlayer(location);
					
					// draw the wall or ground or energy charger or health charger
					gc.drawImage(background[location.x][location.y], location.x*cellSize, location.y*cellSize);
					
					// draw the explosion
					if (explosion != null) {
						String imageName = explosion.getProperty(Names.kPropertyImage);
						Image image = images.get(imageName);
						if (image == null) {
							image = bootstrapImage(imageName);
						}
						gc.drawImage(image, location.x*cellSize, location.y*cellSize);
					}
					
					// draw the missile packs or tanks
					if (object != null) {
						String imageName = object.getProperty(Names.kPropertyImage);
						Image image = images.get(imageName);
						if (image == null) {
							image = bootstrapImage(imageName);
						}
						gc.drawImage(image, location.x*cellSize, location.y*cellSize);
					} else if (tank != null) {
						Image image = tanks.get(new Integer(tank.getFacingInt()));
						assert image != null;

						gc.drawImage(image, location.x*cellSize, location.y*cellSize);

						if (tank.shieldsUp()) {
					        gc.setForeground(WindowManager.getColor(tank.getColor()));
							gc.setLineWidth(3);
							gc.drawOval(cellSize*location.x+2, cellSize*location.y+2, cellSize-5, cellSize-5);
					        gc.setForeground(WindowManager.black);
							gc.setLineWidth(1);
						}

						// draw the player color
						gc.setBackground(WindowManager.getColor(tank.getColor()));
						gc.fillOval(cellSize*location.x + cellSize/2 - kDotSize/2, 
								cellSize*location.y + cellSize/2 - kDotSize/2, 
								kDotSize, kDotSize);
					}

					// cache all the missiles
					iter = missiles.iterator();
					while (iter.hasNext()) {
						CellObject missile = iter.next();
						
						String imageName = missile.getProperty(Names.kPropertyImage);
						Image image = images.get(imageName);
						if (image == null) {
							image = bootstrapImage(imageName);
						}
						
						String colorName = missile.getProperty(Names.kPropertyColor);
						assert colorName !=  null;
						
						Color missileColor = WindowManager.getColor(colorName);
						
						int flightPhase = missile.getIntProperty(Names.kPropertyFlyPhase);
						int direction = missile.getIntProperty(Names.kPropertyDirection);

						if (flightPhase == 0) {
							direction = Direction.backwardOf[direction];
						}
						
						boolean thirdPhase = (flightPhase == 3);

						int mX = 0;
						int mY = 0;
						switch (direction) {
						case Direction.kNorthInt:
							mX = 10;
							mY = thirdPhase ? 26 : 5;
							break;
						case Direction.kEastInt:
							mX = thirdPhase ? -6 : 15;
							mY = 10;
							break;
						case Direction.kSouthInt:
							mX = 10;
							mY = thirdPhase ? -6 : 15;
							break;
						case Direction.kWestInt:
							mX = thirdPhase ? 26 : 5;
							mY = 10;
							break;
						default:
							assert false;
							break;
						}
						//gc.drawImage(image, (location.x * cellSize) + mX, (location.y * cellSize) + mY);						
						drawMissiles.add(new DrawMissile(gc, image, (location.x * cellSize) + mX, (location.y * cellSize) + mY, missileColor));
					}
					
					// Finally, draw the radar waves
					ArrayList<CellObject> radarWaves = this.map.getAllWithProperty(location, Names.kPropertyRadarWaves);
					iter = radarWaves.iterator();
					gc.setForeground(WindowManager.getColor("white"));
					while (iter.hasNext()) {
						CellObject cellObject = iter.next();
						int direction = cellObject.getIntProperty(Names.kPropertyDirection);
						int start = 0;
						int xMod = 0;
						int yMod = 0;
						switch (direction) {
						case Direction.kNorthInt:
							start = 0;
							yMod = cellSize / 4;
							break;
						case Direction.kSouthInt:
							start = -180;
							yMod = cellSize / -4;
							break;
						case Direction.kEastInt:
							start = -90;
							xMod = cellSize / -4;
							break;
						case Direction.kWestInt:
							start = 90;
							xMod = cellSize / 4;
							break;
						default:
							// TODO: warn
							assert false;
							break;
						}
						gc.drawArc((location.x * cellSize) + xMod, (location.y * cellSize) + yMod, cellSize - 1, cellSize - 1, start, 180);
					}
					break;
					
				case kBook:
					if (!this.map.enterable(location)) {
					    gc.setBackground(WindowManager.black);
					    gc.fillRectangle(cellSize*xDraw, cellSize*yDraw, cellSize, cellSize);
						
					} else {
						
						if (map.getAllWithProperty(location, Names.kPropertyDoor).size() == 0) {
							
							if (!Soar2D.config.getColoredRooms()) {
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
						} else {
							gc.setBackground(WindowManager.white);
						}
						gc.fillRectangle(cellSize*xDraw, cellSize*yDraw, cellSize, cellSize);
					}
					
					Player player = this.map.getPlayer(location);
					if (player != null) {
						boolean drawDot = false;
						Image image = null;
						if (player.getName().equalsIgnoreCase("dog")) {
							image = VisualWorld.dog;
						} else if (player.getName().equalsIgnoreCase("mouse")) {
							image = VisualWorld.mouse;
						} else {
							image = VisualWorld.cat;
							drawDot = true;
						}
						
						// draw the player color
						if (drawDot) {
							// image using float location then dot
							Point2D.Float floatLocation = new Point2D.Float(Soar2D.simulation.world.getFloatLocation(player).x, Soar2D.simulation.world.getFloatLocation(player).y);
							floatLocation.x -= cellSize/2;
							floatLocation.y -= cellSize/2;
							gc.drawImage(image, (int)floatLocation.x, (int)floatLocation.y);
							
							gc.setBackground(WindowManager.getColor(player.getColor()));
							gc.fillOval(cellSize*location.x + cellSize/2 - kDotSize/2, 
									cellSize*location.y + cellSize/2 - kDotSize/2, 
									kDotSize, kDotSize);
						} else {
							// only the image
							gc.drawImage(image, location.x*cellSize, location.y*cellSize);
						}
					}

					break;
				}
			}
		}
		
		// actually draw the missiles now (so they appear on top of everything)
		Iterator<DrawMissile> drawMissileIter = drawMissiles.iterator();
		while (drawMissileIter.hasNext()) {
			drawMissileIter.next().draw();
		}
		
		painted = true;
		if (Soar2D.control.isRunning()) {
			if (agentLocation != null) {
				synchronized(Soar2D.wm) {
					Soar2D.wm.agentDisplayUpdated = true;
					Soar2D.wm.notify();
				}
			} else {
				synchronized(Soar2D.wm) {
					Soar2D.wm.worldDisplayUpdated = true;
					Soar2D.wm.notify();
				}
			}
		}
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
	
	void drawEaterMouth(int x, int y, int x_mult, int y_mult, int cx_mult, int cy_mult, GC gc){		
	    switch(Soar2D.simulation.world.getWorldCount() % 8){
			case(0):{
			    gc.fillPolygon(new int[]{cellSize*x, cellSize*y,
			            cellSize*x + x_mult*cellSize, cellSize * y + y_mult*cellSize,
			            cellSize*x + cx_mult*cellSize/2, cellSize*y + cy_mult*cellSize/2});
			    break;
			}
			case(1):{
			    gc.fillPolygon(new int[]{cellSize*x + x_mult*cellSize/8, cellSize*y + y_mult*cellSize/8,
			            cellSize*x + x_mult*(cellSize - cellSize/8), cellSize * y + y_mult*(cellSize - cellSize/8),
			            cellSize*x + cx_mult*cellSize/2, cellSize*y + cy_mult*cellSize/2});
			    break;
			}
			case(2):{
			    gc.fillPolygon(new int[]{cellSize*x + x_mult*cellSize/4, cellSize*y + y_mult*cellSize/4,
			            cellSize*x + x_mult*(cellSize - cellSize/4), cellSize * y + y_mult*(cellSize - cellSize/4),
			            cellSize*x + cx_mult * cellSize/2, cellSize*y + cy_mult * cellSize/2});
			    break;
			}
			case(3):{
			    gc.fillPolygon(new int[]{cellSize*x + x_mult * 3*cellSize/8, cellSize*y + y_mult * 3*cellSize/8,
			            cellSize*x + x_mult*(cellSize - 3*cellSize/8), cellSize * y + y_mult*(cellSize - 3*cellSize/8),
			            cellSize*x + cx_mult*cellSize/2, cellSize*y + cy_mult*cellSize/2});
			    break;
			}
			case(4): break;
			case(5):{
			    gc.fillPolygon(new int[]{cellSize*x + x_mult * 3*cellSize/8, cellSize*y + y_mult * 3*cellSize/8,
			            cellSize*x + x_mult*(cellSize - 3*cellSize/8), cellSize * y + y_mult*(cellSize - 3*cellSize/8),
			            cellSize*x + cx_mult*cellSize/2, cellSize*y + cy_mult*cellSize/2});
			    break;
			}
			case(6):{
			    gc.fillPolygon(new int[]{cellSize*x + x_mult*cellSize/4, cellSize*y + y_mult*cellSize/4,
			            cellSize*x + x_mult*(cellSize - cellSize/4), cellSize * y + y_mult*(cellSize - cellSize/4),
			            cellSize*x + cx_mult * cellSize/2, cellSize*y + cy_mult * cellSize/2});
			    break;
			}
			case(7):{
			    gc.fillPolygon(new int[]{cellSize*x + x_mult*cellSize/8, cellSize*y + y_mult*cellSize/8,
			            cellSize*x + x_mult*(cellSize - cellSize/8), cellSize * y + y_mult*(cellSize - cellSize/8),
			            cellSize*x + cx_mult*cellSize/2, cellSize*y + cy_mult*cellSize/2});
			    break;	
			}
		}
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
