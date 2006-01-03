/* File: RadarView.java
 * Sep 8, 2004
 */
package edu.umich.tanksoar;


import org.eclipse.swt.widgets.*;
import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;


/**
 * Implements a canvas that draws what a <code>Tank</code> can see on its radar.
 * @author John Duchi
 */
public class RadarView extends Canvas implements PaintListener, TankListener, DisposeListener{

	/** The <code>Display</code> managing the GUI. */
	private Display myDisplay;
	/** The <code>Tank</code> to which this <code>RadarView</code> is listening. */
	private Tank myTank;
	/** A 2D array that is 3 * <code>Tank.MaxRadar</code> that contains what was last
	 * seen on the radar. */
	private Object[][] myLastRadar = null;
	/** The size of a side of a location (in pixels) drawn in the <code>RadarView</code>. */
	public static final int SquareSize = 20;
	/** The number of pixels the top of the actual radar views is offset for text. */
	private static final int TopOffset = 20;
	/** The preferred width of this <code>RadarView</code>, which is 3*<code>SquareSize</code>. */
	private static final int Width = 3*SquareSize;
	/** The preferred height of this <code>RadarView</code>, which is <code>(Tank.MaxRadar+1)*SquareSize
	 * + TopOffset</code>. */
	private static final int Height = (Tank.MaxRadar+1)*SquareSize + TopOffset;
	/** Small image used to draw <code>TSEmpty</code>s in the radar. */
	private Image groundSmall;
	/** Small image used to draw unseeable locations (<code>null</code>s) in the radar. */
	private Image question;
	/** Small image used to draw <code>TSWall</code>s in the radar. */
	private Image obstacleSmall;
	/** Small image used to draw <code>MissileBucket</code>s in the radar. */
	private Image missileSmall;
	/** Small image used to draw <code>EnergySquare</code>s in the radar. */
	private Image batterySmall;
	/** Small image used to draw other (or this) <code>Tank</code>s in the radar. */
	private Image tankSmall;
	/** Small image used to draw <code>HealthSquare</code>s in the radar. */
	private Image healthSmall;
	
	/**
	 * Constructs a new instance of RadarView, which can be placed in a component of users
	 * choosing. Useful, for instance, in a view of a Tank's input link.
	 * @param parent The parent Composite into which this RadarView will be placed.
	 * @param toListen The Tank whose radar information this RadarView is showing.
	 * @param imageDirectory The String name of the directory that contains the images
	 * that will be used to display the radar.
	 */
    public RadarView(Composite parent, Tank toListen){
		super(parent, SWT.NONE);
		myDisplay = parent.getDisplay();
		myTank = toListen;
		myTank.addTankListener(this);
		addPaintListener(this);
		addDisposeListener(this);
		loadImages();
    }
	
    /**
     * Loads the images used to display the RadarView into memory. Images loaded are
     * <code>ground_small.gif</code>, <code>question.gif</code>, <code>obstacle_small.gif</code>,
     * <code>missile_small.gif</code>, <code>battery_small.gif</code>, <code>tank_small.gif</code>,
     * and <code>health_small.gif</code>. To display properly, these images should all be 20 x 20
     * pixels, because paintControl will draw them as such.
     * @param imageDirectory The String name of the directory that contains the images,
     * expected to be ending with a backslash character, or whatever system dependent directory
     * change character is necessary.
     * @throws <code>NullPointerException</code> If imageDirectory is null.
     */
	public void loadImages(){
		question = new Image(myDisplay, 
				TanksoarJ.class.getResourceAsStream("/images/question.gif"));
		groundSmall = new Image(myDisplay, 
				TanksoarJ.class.getResourceAsStream("/images/ground_small.gif"));
		obstacleSmall = new Image(myDisplay, 
				TanksoarJ.class.getResourceAsStream("/images/obstacle_small.gif"));
		missileSmall = new Image(myDisplay, 
				TanksoarJ.class.getResourceAsStream("/images/missile_small.gif"));
		batterySmall = new Image(myDisplay, 
				TanksoarJ.class.getResourceAsStream("/images/battery_small.gif"));
		tankSmall = new Image(myDisplay, 
				TanksoarJ.class.getResourceAsStream("/images/tank_small.gif"));
		healthSmall = new Image(myDisplay, 
				TanksoarJ.class.getResourceAsStream("/images/health_small.gif"));
	}
	
	/**
	 * Returns the desired size of this <code>RadarView</code> as a <code>Point</code>.
	 * Ignores the parameters to pass back a (<code>Width</code>, <code>Height</code>)
	 * <code>Point</code>.
	 * @return The desired size of this <code>RadarView</code> as a <code>Point</code> whose
	 * x-coordinate is the desired width, y-coordinate the desired height. More specifically,
	 * returns <code>new Point(Width, Height)</code>.
	 */
	public Point computeSize(int wHint, int hHint, boolean changed){
		return(new Point(Width, Height));
	}
	
	/**
	 * Notification fired when this <code>Widget</code> is disposed, usually by closing
	 * the <code>Shell</code> holding it. Removes this <code>RadarView</code> from
	 * its <code>Tank</code>'s listeners.
	 */
	public void widgetDisposed(DisposeEvent e){
		myTank.removeTankListener(this);
	}
	
	/**
	 * Notification sent when a paint event occurs for this <code>RadarView</code>, causing
	 * it to paint itself.
	 * @param e An event containing information about the paint, including its graphics
	 * context.
	 */
	public void paintControl(PaintEvent e){
		GC gc = e.gc;
		if(myLastRadar == null){
			myLastRadar = myTank.getRadarSights();
		}
		repaintControl(gc);
	}
	
	/**
	 * Helper method that, given a <code>GC</code>, actually paints all the <code>RadarView
	 * </code> to its place.
	 * @param gc The graphics context in which to paint this <code>RadarView</code>.
	 */
	private void repaintControl(GC gc){
	    gc.setBackground(myDisplay.getSystemColor(SWT.COLOR_WHITE));
	    gc.fillRectangle(0, 0, Width, Height);
	    gc.setForeground(myDisplay.getSystemColor(SWT.COLOR_BLACK));
	    gc.drawText("Radar", Width/2 - gc.stringExtent("Radar").x/2, 2);
		Object[][] rSights = myLastRadar;
	    for(int x = 0; x < rSights.length; ++x){
			for(int y = rSights[x].length-1, yDraw = 0; y >= 0; --y, ++yDraw){
				Image toDraw = question;
				if(rSights[x][y] == null){
					
				} else if(rSights[x][y] instanceof EnterableSquare){
					if(((EnterableSquare)rSights[x][y]).containsAgent()){
						toDraw = tankSmall;
					} else if(((EnterableSquare)rSights[x][y]).containsMissiles()){
						toDraw = missileSmall;
					} else if(rSights[x][y] instanceof TSEmpty){
						toDraw = groundSmall;
					} else if(rSights[x][y] instanceof EnergySquare){
						toDraw = batterySmall;
					} else if(rSights[x][y] instanceof HealthSquare){
						toDraw = healthSmall;
					}
				} else if(rSights[x][y] instanceof TSWall){
					toDraw = obstacleSmall;
				}
				gc.drawImage(toDraw, x*SquareSize, TopOffset + yDraw*SquareSize);
				if(toDraw == tankSmall){
					Color c = null;
					try{
						c = TankSWindowManager.getNamedColor(((EnterableSquare)rSights[x][y]).getAgent().getColorName());
					} catch(SWTException noColor){
						TankSWindowManager.initColors(myDisplay);
						c = TankSWindowManager.getNamedColor(((EnterableSquare)rSights[x][y]).getAgent().getColorName());
					}
					gc.setForeground(c);
					gc.setLineWidth(2);
					gc.drawPolygon(new int[]{x*SquareSize + SquareSize/2, TopOffset + yDraw*SquareSize,
											(x+1)*SquareSize, TopOffset + yDraw*SquareSize + SquareSize/2,
											x*SquareSize + SquareSize/2, TopOffset + (yDraw+1)*SquareSize,
											x*SquareSize, TopOffset + yDraw*SquareSize + SquareSize/2});
				}
			}
		}
	}
	
	/* TankListener methods: Most of these we ignore because we only update
	 * the Tank is supposed to be calculating a decision.
	 */

	/**
	 * Ignored. {@inheritDoc}
	 */
	public void decisionMade(Tank t, TankOutputInfo decision){
		/* Ignored because all updating is done in allDecisionsMade. */
	}
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void energyChanged(Tank t){ /* IGNORED */ }
	
	/**
	 * Redraws this <code>RadarView</code> with the information provided by <code>sensors</code>.
	 * Takes the place of basically all other updates to this <code>RadarView</code> from its
	 * <code>Tank</code>.
	 * {@inheritDoc}
	 */
	public void allDecisionsMade(Tank t, TankInputInfo sensors){
		myLastRadar = sensors.radarSights;
		if(myDisplay.isDisposed()) return;
		myDisplay.syncExec(new Runnable(){
			public void run(){
				if(isDisposed() || myTank.getTankSimulationControl().hasQuit()) return;
				redraw(0, TopOffset, Width, Height - TopOffset, false);
			}
		});
	}
	
	/**
	 * Redraws this <code>RadarView</code> because the <code>Tank</code>, as it has
	 * been resurrected, has new information available to it.
	 * {@inheritDoc}
	 */
	public void resurrected(Tank t){
		if(myDisplay.isDisposed()) return;
		myLastRadar = t.getRadarSights();
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if(myTank.getTankSimulationControl().hasQuit()) return;
				redraw(0, TopOffset, Width, Height - TopOffset, false);
			}
		});
	}
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void healthChanged(Tank t){ /* IGNORED */ }
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void locationChanged(Tank t){ /* IGNORED */ }
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void missileCountChanged(Tank t){ /* IGNORED */ }
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void missileHit(Tank t){ /* IGNORED */ }
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void radarSettingChanged(Tank t){ /* IGNORED */ }
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void radarSwitch(Tank t){ /* IGNORED */ }
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void rotationChanged(Tank t){ /* IGNORED */ }
	
	/**
	 * Ignored. {@inheritDoc}
	 */
	public void scoreChanged(Tank t){ /* IGNORED */ }
	
}
