package soar2d.visuals;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.Random;

import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.Composite;

import soar2d.Direction;
import soar2d.Names;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.map.CellObject;
import soar2d.map.GridMap;
import soar2d.player.Player;

public class TankSoarVisualWorld extends VisualWorld {
	Image[][] background = null;

	public TankSoarVisualWorld(Composite parent, int style, int cellSize) {
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
		
		if (background == null) {
			generateBackground();
		}
		
		// Draw world
		ArrayList<DrawMissile> drawMissiles = new ArrayList<DrawMissile>();
		java.awt.Point location = new java.awt.Point();
		for(location.x = 0; location.x < map.getSize(); ++location.x){
			for(location.y = 0; location.y < map.getSize(); ++location.y){
				if ((this.map.removeObject(location, Names.kRedraw) == null) && painted) {
					continue;
				}
				
				ArrayList<CellObject> drawList;
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
			}
		}
		
		// actually draw the missiles now (so they appear on top of everything)
		Iterator<DrawMissile> drawMissileIter = drawMissiles.iterator();
		while (drawMissileIter.hasNext()) {
			drawMissileIter.next().draw();
		}
		
		painted = true;
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
	
	public void setMap(GridMap map) {
		super.setMap(map);
		
		background = null;
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
			// Do not use the simulation's random number generator because it will change
			// headless run behavior
			Random myRandom = new Random();
			int pick = myRandom.nextInt(max - min + 1);
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
