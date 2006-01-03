/* File: DrawTankMap.java
 * Aug 24, 2004
 */
package edu.umich.tanksoar;

import edu.umich.JavaBaseEnvironment.SimulationControlListener;
import edu.umich.JavaBaseEnvironment.Location;
import edu.umich.JavaBaseEnvironment.SoarAgent;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;

/**
 * The visibile representation of TankSoar, this component can be placed in a <code>Shell</code> or other
 * <code>Composite</code> to give users a picture of what the world contains.
 * @author John Duchi
 */
public class DrawTankMap extends Canvas implements TankListener, SimulationControlListener,
										PaintListener, DisposeListener{

	/** The size (in pixels) of one location being drawn in the simulation. */
	public static int SquareSize = 32;
	/** The <code>TankSoarJControl</code> running the simulation. */
	private TankSoarJControl myTC;

	/** The three rock images. */
	private Image[] rocks = new Image[3];
	/** The three tree images. */
	private Image[] trees = new Image[3];
	/** The three grass images. */
	private Image[] grasses = new Image[3];
	/** The four tank images, pointing in different directions (<code>tanks[Tank.North]</code> being the image
	 * of the <code>Tank</code> pointing north). */
	private Image[] tanks = new Image[4];
	
	private Image recharger;
	private Image health;
	private Image missiles;
	private Image flyMissile;
	/** The <code>Image</code> displayed when a <code>Tank</code> is on an <code>EnergySquare</code>. */
	private Image blueRecharge;
	/** The <code>Image</code> displayed when a <code>Tank</code> is on a <code>HealthSquare</code>. */
	private Image redRecharge;
	private Image explosion;
	
	private Location[][] radarLocations = new Location[8][0];
	private Location[][] oldRadarLocations = new Location[8][Tank.MaxRadar];
	private int tankCount = 0;
	private int worldCountRadarCalculated = -1;
	private SoarAgent[] tankAgents;
	
	private Color white;
	
	private Image wtf;
	
	private Image[][] allImages = null;
	
	private Display myDisplay;
	
	private static String sep = System.getProperty("file.separator");
		
	public DrawTankMap(TankSoarJControl TC, Composite parent){
		super(parent, SWT.NONE);
		myTC = TC;
		myDisplay = parent.getDisplay();
		white = myDisplay.getSystemColor(SWT.COLOR_WHITE);
		myTC.addSimulationControlListener(this);
		addSelfToTankListeners();
		loadImages();
		fillMapImages();
		addPaintListener(this);
		addDisposeListener(this);
	}
	
	private void addSelfToTankListeners(){
		SoarAgent[] s = myTC.getAllAgents();
		for(int i = 0; i < s.length; ++i){
			((Tank)s[i]).addTankListener(this);
		}
	}
	
	public Point computeSize(int wHint, int hHint, boolean changed){
		return(new Point(SquareSize * myTC.getMapWidth(), SquareSize * myTC.getMapHeight()));
	}
	
	/**
	 * Message sent when this <code>Widget</code> is disposed.
	 * @param e The <code>DisposeEvent</code> containing information about the disposal.
	 */
	public void widgetDisposed(DisposeEvent e){
		SoarAgent[] tanks = myTC.getAllAgents();
		for(int i = 0; i < tanks.length; ++i){
			((Tank)tanks[i]).removeTankListener(this);
		}
		myTC.removeSimulationControlListener(this);
	}
	
	public void paintControl(PaintEvent e){
		GC gc = e.gc;
		if(allImages != null){
			repaintControl(gc);
			return;
		}
		fillMapImages();
		repaintControl(gc);
	}
	
	/**
	 * Fills the map internal to this <code>DrawTankMap</code> with the images permanently
	 * in it--grasses, the health, recharger, and obstacles.
	 */
	private void fillMapImages(){
		allImages = new Image[myTC.getMapWidth()][myTC.getMapHeight()];
		for(int x = 0; x < myTC.getMapWidth(); x++){
			for(int y = 0; y < myTC.getMapHeight(); y++){
				int xCo = x*SquareSize, yCo = y*SquareSize;
				Object o = myTC.getLocationContents(x, y);
				if(o instanceof EnterableSquare){
					if(o instanceof TSEmpty){
						allImages[x][y] = grasses[(int)(Math.random()*grasses.length)];
					}else if(o instanceof HealthSquare){
						allImages[x][y] = health;
					} else if(o instanceof EnergySquare){
						allImages[x][y] = recharger;
					}
				} else if(o instanceof TSWall){
					if(x == 0 || y == 0 || x == myTC.getMapWidth() - 1 || y == myTC.getMapHeight() - 1){
						allImages[x][y] = rocks[(int)(Math.random()*rocks.length)];
					} else {
						allImages[x][y] = trees[(int)(Math.random()*trees.length)];
					}
				} else {
					allImages[x][y] = wtf;
				}
			}
		}
	}
	
	private void repaintControl(GC gc){
		for(int x = 0; x < allImages.length; ++x){
			for(int y = 0; y < allImages[x].length; ++y){
				Object o = myTC.getLocationContents(x, y);
				int xCo = x*SquareSize, yCo = y*SquareSize;
				if(o instanceof EnterableSquare){
					if(((EnterableSquare)o).containsAgent()){
						if(o instanceof TSEmpty){
							Tank t = (Tank)((EnterableSquare)o).getAgent();
							gc.drawImage(tanks[t.getDirection()], xCo, yCo);
							if(t.getShieldsOn()){
								gc.setForeground(white);
								gc.drawOval(xCo, yCo, SquareSize, SquareSize);
							} else if(t.wasHitWithMissile()){
								gc.drawImage(explosion, xCo, yCo);
							}
						} else if(o instanceof HealthSquare){
							gc.drawImage(redRecharge, xCo, yCo);
						} else if(o instanceof EnergySquare){
							gc.drawImage(blueRecharge, xCo, yCo);
						}
						paintColorDiamond(gc, xCo, yCo, ((EnterableSquare)o).getAgent().getColorName());
					} else if(((EnterableSquare)o).containsMissiles()){
						gc.drawImage(missiles, xCo, yCo);
					} else {
						gc.drawImage(allImages[x][y], xCo, yCo);
					}
				} else {
					gc.drawImage(allImages[x][y], xCo, yCo);
				}
			}
		}
		FlyingMissile[] fms = myTC.getFlyingMissiles();
		for(int i = 0; i < fms.length; ++i){
			gc.drawImage(flyMissile, fms[i].getLocation().getX()*SquareSize + SquareSize/8, fms[i].getLocation().getY()*SquareSize + SquareSize/8);
		}
		paintAllRadar(gc);
	}
	
	/**
	 * Paints a colored diamond to represent the color that a given <code>Tank</code> is.
	 * @param gc The <code>GC</code> object that is being drawn to.
	 * @param xCo The x-coordinate (in pixels) at which to draw.
	 * @param yCo The y-coordinate (in pixels) at which to draw.
	 * @param colorName The <code>String</code> name of the color being used.
	 */
	private void paintColorDiamond(GC gc, int xCo, int yCo, String colorName){
		Color c = null;
		try{
			c = TankSWindowManager.getNamedColor(colorName);
		} catch(SWTException ignored){}
		if(c == null) return;
		gc.setForeground(c);
		gc.setLineWidth(2);
		gc.drawPolygon(new int[]{xCo + SquareSize/2, yCo,
				xCo + SquareSize, yCo + SquareSize/2,
				xCo + SquareSize/2, yCo + SquareSize,
				xCo, yCo + SquareSize/2});
	}
	
	/**
	 * Paints all the radars in the simulation using calls to <code>paintRadarSquare(GC, int, int, int)</code>.
	 * @param gc The graphics context in which to draw the radar.
	 */
	private void paintAllRadar(GC gc){
		gc.setForeground(white);
		gc.setLineWidth(1);
		for(int i = 0; i < tankCount; ++i){
			for(int j = 0; j < radarLocations[i].length && radarLocations[i][j] != null; ++j){
				paintRadarSquare(gc, radarLocations[i][j].getX()*SquareSize, radarLocations[i][j].getY()*SquareSize, ((Tank)tankAgents[i]).getDirection());
			}
		}
	}
	
	/**
	 * Paints radar travelling through a square over the pixels in the square to the lower right
	 * of (xCo, yCo) in the direction specified, using the specified graphics context.
	 * @param gc The <code>GC</code> in which to draw the radar.
	 * @param xCo The x-coordinate (in pixels) at the upper left corner of the square in which
	 * the radar is to be drawn.
	 * @param yCo The y-coordinate (in pixels) at the upper left corner of the square in which
	 * the radar is to be drawn.
	 * @param direction The integer value of the direction in which the radar is traveling (as
	 * defined in class <code>Tank</code>.
	 */
	private void paintRadarSquare(GC gc, int xCo, int yCo, int direction){
		switch(direction){
		case(Tank.NORTH):{
			gc.drawArc(xCo-SquareSize/4, yCo, 3*SquareSize/2, SquareSize, 130, -80);
			gc.drawArc(xCo-SquareSize/4, yCo+SquareSize/2, 3*SquareSize/2, SquareSize, 130, -80);
			break;
		}
		case(Tank.EAST):{
			gc.drawArc(xCo, yCo - SquareSize/4, SquareSize-1, 3*SquareSize/2, 40, -80);
			gc.drawArc(xCo - SquareSize/2, yCo - SquareSize/4, SquareSize-1, 3*SquareSize/2, 40, -80);
			break;
		}
		case(Tank.SOUTH):{
			gc.drawArc(xCo - SquareSize/4, yCo, 3*SquareSize/2, SquareSize-1, 310, -80);
			gc.drawArc(xCo - SquareSize/4, yCo - SquareSize/2, 3*SquareSize/2, SquareSize-1, 310, -80);
			break;
		}
		case(Tank.WEST):{
			gc.drawArc(xCo, yCo - SquareSize/4, SquareSize, 3*SquareSize/2, 140, 80);
			gc.drawArc(xCo + SquareSize/2, yCo - SquareSize/4, SquareSize, 3*SquareSize/2, 140, 80);
			break;
		}
		}
	}
	
	/**
	 * Loads all the images necessary for drawing the map. These, are:
	 * <ul><li>rock1.gif, rock2.gif, rock3.gif</li><li>tree1.gif, tree2.gif, tree3.gif</li>
	 * <li>ground1.gif, ground2.gif, ground3.gif</li><li>tank_down.gif, tank_down.gif,
	 * tank_down.gif, tank_down.gif </li><li>battery.gif</li><li>health.gif</li>
	 * <li>missile.gif</li><li>fire4.gif</li><li>wtf.gif</li><li>recharge.gif</li>
	 * <li>battrecharge.gif</li><li>explosion.gif</li></ul>
	 */
	private void loadImages(){
		tanks[Tank.SOUTH] = new Image(myDisplay, 
				TanksoarJ.class.getResourceAsStream("/images/tank_down.gif"));
		tanks[Tank.NORTH] = new Image(myDisplay, 
				TanksoarJ.class.getResourceAsStream("/images/tank_up.gif"));
		tanks[Tank.EAST] = new Image(myDisplay, 
				TanksoarJ.class.getResourceAsStream("/images/tank_right.gif"));
		tanks[Tank.WEST] = new Image(myDisplay, 
				TanksoarJ.class.getResourceAsStream("/images/tank_left.gif"));
		recharger = new Image(myDisplay,
				TanksoarJ.class.getResourceAsStream("/images/battery.gif"));
		health = new Image(myDisplay, 
				TanksoarJ.class.getResourceAsStream("/images/health.gif"));
		missiles = new Image(myDisplay, 
				TanksoarJ.class.getResourceAsStream("/images/missile.gif"));
		flyMissile = new Image(myDisplay, 
				TanksoarJ.class.getResourceAsStream("/images/fire4.gif"));
		wtf = new Image(myDisplay,
				TanksoarJ.class.getResourceAsStream("/images/wtf.gif"));
		redRecharge = new Image(myDisplay,
				TanksoarJ.class.getResourceAsStream("/images/recharge.gif"));
		blueRecharge = new Image(myDisplay,
				TanksoarJ.class.getResourceAsStream("/images/battrecharge.gif"));
		explosion = new Image(myDisplay, 
				TanksoarJ.class.getResourceAsStream("/images/explosion.gif"));
		loadNumberedImage(rocks, "rock");
		loadNumberedImage(trees, "tree");
		loadNumberedImage(grasses, "ground");
	}
	
	/**
	 * Loads the images with numbers after their names--trees, rocks, and grounds.
	 * @param arr The array in which to store the images loaded.
	 * @param name The name (sans "#.gif") of the image to be loaded.
	 */
	private void loadNumberedImage(Image[] arr, String name){
		for(int i = 0; i < rocks.length; i++){
			arr[i] = new Image(myDisplay,
					TanksoarJ.class.getResourceAsStream("/images/" + name + (i+1) + ".gif"));
		}
	}
	
	/**
	 * Calculates the positions of all the radar output by the <code>Tank</code>s,
	 * as well as cacheing their previous radar positions, and messages the
	 * <code>Display</code> (via <code>locationChanged(Location)</code>) to redraw
	 * those squares.
	 * @param agentCreated If there was <code>SoarAgent</code> created, or for some reason
	 * the radar <code>Location</code>s need to be recalculated, this is <code>true</code>.
	 * Otherwise, it should be <code>false</code>.
	 */
	private void calculateDrawRadars(boolean agentCreated){
		int i, j;
		if(worldCountRadarCalculated != myTC.getWorldCount() || agentCreated){
			//System.out.println("Recalculating radar drawing ");
			tankAgents = myTC.getAllAgents();
			tankCount = tankAgents.length;
			for(i = 0; i < radarLocations.length; ++i){
				for(j = 0; j < radarLocations[i].length; ++j){
					oldRadarLocations[i][j] = radarLocations[i][j];
				}
			}
			for(i = 0; i < tankCount; ++i){
				radarLocations[i] = ((Tank)tankAgents[i]).getRadarLocations();
			}
			worldCountRadarCalculated = myTC.getWorldCount();
		}
		for(i = 0; i < oldRadarLocations.length && oldRadarLocations[i] != null; ++i){
			for(j = 0; j < oldRadarLocations[i].length && oldRadarLocations[i][j] != null; ++j){
				locationChanged(oldRadarLocations[i][j]);
			}
		}
		for(i = 0; i < tankCount; ++i){
			for(j = 0; j < radarLocations[i].length && radarLocations[i][j] != null; ++j){
				//System.out.println("Will be drawing radar at " + radarLocations[i][j]);
				locationChanged(radarLocations[i][j]);
			}
		}
	}
	
	/* ============ Listeners ============ */

	public void decisionMade(Tank t, TankOutputInfo decision){
		if(decision == null) return;
		if(decision.rotate != null || decision.shields != null){
			locationChanged(t.getLocation());
		}
	}
	
	public void allDecisionsMade(Tank t, TankInputInfo sensors){
		
	}
	
	public void radarSwitch(Tank t){
		
	}

	public void rotationChanged(Tank t){
		/* Ignored because decisionMade takes care of it */
	}
	
	public void radarSettingChanged(Tank t){
		
	}
	
	public void scoreChanged(Tank t){
		
	}
	
	public void energyChanged(Tank t){
		
	}
	
	public void healthChanged(Tank t){
		
	}
	
	public void missileCountChanged(Tank t){
		
	}
	
	public void missileHit(Tank t){
		locationChanged(t.getLocation());
	}

	public void locationChanged(Tank t){
		
	}
	
	public void resurrected(Tank t){
		calculateDrawRadars(true);
	}

	
	/* -------- TankSoarJControl -------- */
	//NOT LISTENING FOR NOW//
	public void tankWon(Tank winner){
		
	}
	
	/* -------- SimulationControl -------- */
	public void newMap(String message){
		allImages = null;
		worldCountRadarCalculated = -1;
		fillMapImages();
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				redraw();
			}
		});
		calculateDrawRadars(true);
	}
	
	public void agentCreated(SoarAgent created){
		Location loc = created.getLocation();
		locationChanged(loc);
		calculateDrawRadars(true);
		((Tank)created).addTankListener(this);
	}
	
	public void agentDestroyed(SoarAgent destroyed){
		Location loc = destroyed.getLocation();
		locationChanged(loc);
		calculateDrawRadars(true);
		((Tank)destroyed).removeTankListener(this);
	}
	
	public void locationChanged(Location loc){
		final int x = loc.getX(), y = loc.getY();
		if(myDisplay.isDisposed()) return;
		/* NOTE: This is the only sync exec. It is because sometimes the Control behind
		 * this view gets ahead and we can run into null pointer problems if there
		 * aren't agents in certain squares any longer. */
    
		myDisplay.syncExec(new Runnable(){
			public void run(){
				if(isDisposed()) return;
				redraw(x*SquareSize, y*SquareSize, SquareSize, SquareSize, false);
			}
		});
	}
    
	public void simEnded(String message){
		
	}
	
	public void simQuit(){
		
	}
	
	public void simStarted(){
		
	}
	
	public void worldCountChanged(int newCount){
		calculateDrawRadars(false);
	}
}
