package soar2d.visuals;

import java.util.*;

import org.eclipse.swt.*;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.*;

import soar2d.*;
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
	
	public VisualWorld(Composite parent, int style, int cellSize) {
		super(parent, style | SWT.NO_BACKGROUND);
		display = parent.getDisplay();
		this.cellSize = cellSize;

		tanks.put(new Integer(Direction.kSouthInt), new Image(display, Soar2D.class.getResourceAsStream("/images/tank_down.gif")));
		tanks.put(new Integer(Direction.kNorthInt), new Image(display, Soar2D.class.getResourceAsStream("/images/tank_up.gif")));
		tanks.put(new Integer(Direction.kEastInt), new Image(display, Soar2D.class.getResourceAsStream("/images/tank_right.gif")));
		tanks.put(new Integer(Direction.kWestInt), new Image(display, Soar2D.class.getResourceAsStream("/images/tank_left.gif")));
		
		CellObjectManager manager = Soar2D.simulation.world.map.getObjectManager();
		Iterator<CellObject> iter = manager.getTemplatesWithProperty(Names.kPropertyMiniImage).iterator();
		while (iter.hasNext()) {
			CellObject obj = iter.next();
			Image image = new Image(WindowManager.display, Soar2D.class.getResourceAsStream("/images/" + obj.getProperty(Names.kPropertyMiniImage)));
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
		
		addPaintListener(this);		
	}

	public int getMiniWidth() {
		return cellSize * ((Soar2D.config.kEaterVision * 2) + 1);
	}
	
	public int getMiniHeight() {
		return cellSize * ((Soar2D.config.kEaterVision * 2) + 1);
	}
	
	public void setAgentLocation(java.awt.Point location) {
		agentLocation = location;
	}
	
	Player getPlayerAtPixel(int x, int y) {
		x /= cellSize;
		y /= cellSize;
		return Soar2D.simulation.world.map.getPlayer(new java.awt.Point(x, y));
	}
	
	void resetBackground() {
		background = null;
	}
	
	private void generateBackground(World world) {
		background = new Image[world.getSize()][world.getSize()];
		java.awt.Point location = new java.awt.Point();
		for(location.x = 0; location.x < world.getSize(); ++location.x){
			for(location.y = 0; location.y < world.getSize(); ++location.y){
				ArrayList<CellObject> drawList = world.map.getAllWithProperty(location, Names.kPropertyImage);
				
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
		}
	}

	public void paintControl(PaintEvent e){
		if (agentLocation != null || lastX != e.x || lastY != e.y || internalRepaint) {
			lastX = e.x;
			lastY = e.y;
			setRepaint();
		}
		
		GC gc = e.gc;		
        gc.setForeground(WindowManager.black);
		gc.setLineWidth(1);

		if (disabled || !painted) {
			gc.setBackground(WindowManager.widget_background);
			gc.fillRectangle(0,0, this.getWidth(), this.getHeight());
			if (disabled) {
				painted = true;
				return;
			}
		}
		
		World world = Soar2D.simulation.world;
		
		if (Soar2D.config.tanksoar) {
			if (background == null) {
				generateBackground(world);
			}
		}
		
		// Draw world
		int fill1, fill2, xDraw, yDraw;
		java.awt.Point location = new java.awt.Point();
		for(location.x = 0; location.x < world.getSize(); ++location.x){
			if (agentLocation != null) {
				if ((location.x < agentLocation.x - Soar2D.config.kEaterVision) || (location.x > agentLocation.x + Soar2D.config.kEaterVision)) {
					continue;
				} 
				xDraw = location.x + Soar2D.config.kEaterVision - agentLocation.x;
			} else {
				xDraw = location.x;
			}
			
			for(location.y = 0; location.y < world.getSize(); ++location.y){
				if (agentLocation != null) {
					if ((location.y < agentLocation.y - Soar2D.config.kEaterVision) || (location.y > agentLocation.y + Soar2D.config.kEaterVision)) {
						continue;
					} 
					yDraw = location.y + Soar2D.config.kEaterVision - agentLocation.y;
				} else {
					yDraw = location.y;
				}
				
				if (agentLocation == null) {
					if ((world.map.removeObject(location, Names.kRedraw) == null) && painted) {
						continue;
					}
				} else {
					if (!world.map.hasObject(location, Names.kRedraw) && painted) {
						continue;
					}
				}
				
				if (Soar2D.config.eaters) {
					ArrayList<CellObject> drawList = world.map.getAllWithProperty(location, Names.kPropertyShape);
					
					if (!world.map.enterable(location)) {
					    gc.setBackground(WindowManager.black);
					    gc.fillRectangle(cellSize*xDraw + 1, cellSize*yDraw + 1, cellSize - 2, cellSize - 2);
						
					} else {
						boolean empty = true;
						
						Player eater = world.map.getPlayer(location);
						
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
					
					if (world.map.hasObject(location, Names.kExplosion)) {
						drawExplosion(gc, xDraw, yDraw);
					}
				} else if (Soar2D.config.tanksoar) {
					
					ArrayList<CellObject> drawList = world.map.getAllWithProperty(location, Names.kPropertyImage);
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
					
					Player player = world.map.getPlayer(location);
					
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
					} else if (player != null) {
						Image image = tanks.get(new Integer(player.getFacingInt()));
						assert image != null;

						gc.drawImage(image, location.x*cellSize, location.y*cellSize);

						if (player.shieldsUp()) {
					        gc.setForeground(WindowManager.getColor(player.getColor()));
							gc.setLineWidth(3);
							gc.drawOval(cellSize*location.x+2, cellSize*location.y+2, cellSize-5, cellSize-5);
					        gc.setForeground(WindowManager.black);
							gc.setLineWidth(1);
						}

						// draw the player color
						gc.setBackground(WindowManager.getColor(player.getColor()));
						gc.fillOval(cellSize*location.x + cellSize/2 - kDotSize/2, 
								cellSize*location.y + cellSize/2 - kDotSize/2, 
								kDotSize, kDotSize);
					}

					// draw the missiles
					iter = missiles.iterator();
					while (iter.hasNext()) {
						CellObject missile = iter.next();
						
						String imageName = missile.getProperty(Names.kPropertyImage);
						Image image = images.get(imageName);
						if (image == null) {
							image = bootstrapImage(imageName);
						}
						
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
						
						gc.drawImage(image, (location.x * cellSize) + mX, (location.y * cellSize) + mY);						
					}
					
				} else {
					Soar2D.control.severeError("I don't know how to draw the world!");
				}
			}
		}
		painted = true;
	}
	
	Image bootstrapImage(String imageName) {
		Image image = new Image(WindowManager.display, Soar2D.class.getResourceAsStream("/images/" + imageName));
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
		return cellSize * Soar2D.simulation.world.getSize();
	}
	
	public int getHeight() {
		return cellSize * Soar2D.simulation.world.getSize();
	}
	
}
