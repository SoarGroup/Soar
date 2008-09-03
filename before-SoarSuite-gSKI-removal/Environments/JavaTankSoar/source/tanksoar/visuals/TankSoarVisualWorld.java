package tanksoar.visuals;

import java.util.*;
import java.util.logging.*;

import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

import simulation.*;
import simulation.visuals.*;
import tanksoar.*;
import utilities.*;

public class TankSoarVisualWorld extends VisualWorld implements PaintListener {
	private static Logger logger = Logger.getLogger("tanksoar.visuals");
	
	private static HashMap kTanks = new HashMap(4);
	private static Image[] kRocks = new Image[3];
	private static Image[] kTrees = new Image[3];
	private static Image[] kGrass = new Image[3];
	private static Image kRecharger;
	private static Image kHealth;
	private static Image kMissiles;
	private static Image kMissile;
	private static Image kWTF;
	private static Image kRedRecharge;
	private static Image kBlueRecharge;
	private static Image kExplosion;
	private static final int kDotSize = 7;
	private static final int kCellSize = 32;
	
	private TankSoarSimulation m_Simulation;
	private TankSoarWorld m_World;
	private Point m_AgentLocation;
	private Image[][] m_Background;
	
	public TankSoarVisualWorld(Composite parent, int style, TankSoarSimulation simulation) {
		super(parent, style, simulation, kCellSize);
		
		m_Simulation = simulation;
		m_World = m_Simulation.getTankSoarWorld();
		
		loadImages(parent.getDisplay());
		addPaintListener(this);		
	}
	
	private void loadImages(Display display) {
		kTanks.put(new Integer(Direction.kSouthInt), new Image(display, TankSoar.class.getResourceAsStream("/images/tank_down.gif")));
		kTanks.put(new Integer(Direction.kNorthInt), new Image(display, TankSoar.class.getResourceAsStream("/images/tank_up.gif")));
		kTanks.put(new Integer(Direction.kEastInt), new Image(display, TankSoar.class.getResourceAsStream("/images/tank_right.gif")));
		kTanks.put(new Integer(Direction.kWestInt), new Image(display, TankSoar.class.getResourceAsStream("/images/tank_left.gif")));
		
		kRecharger = new Image(display, TankSoar.class.getResourceAsStream("/images/battery.gif"));
		kHealth = new Image(display, TankSoar.class.getResourceAsStream("/images/health.gif"));
		kMissiles = new Image(display, TankSoar.class.getResourceAsStream("/images/missile.gif"));
		kMissile = new Image(display, TankSoar.class.getResourceAsStream("/images/fire2.gif"));
		kWTF = new Image(display, TankSoar.class.getResourceAsStream("/images/wtf.gif"));
		kRedRecharge = new Image(display, TankSoar.class.getResourceAsStream("/images/recharge.gif"));
		kBlueRecharge = new Image(display, TankSoar.class.getResourceAsStream("/images/battrecharge.gif"));
		kExplosion = new Image(display, TankSoar.class.getResourceAsStream("/images/explosion.gif"));
		
		loadNumberedImage(display, kRocks, "rock");
		loadNumberedImage(display, kTrees, "tree");
		loadNumberedImage(display, kGrass, "ground");
	}
	
	private void loadNumberedImage(Display display, Image[] images, String name){
		for(int i = 0; i < images.length; i++){
			images[i] = new Image(display, TankSoar.class.getResourceAsStream("/images/" + name + (i+1) + ".gif"));
		}
	}
	
	void generateBackground() {
		m_Background = new Image[m_World.getSize()][m_World.getSize()];
		for(int x = 0; x < m_World.getSize(); ++x){
			for(int y = 0; y < m_World.getSize(); ++y){
				TankSoarCell cell = m_World.getCell(x, y);
				if (cell.isWall()) {
					if ((x == 0) || (x == m_World.getSize() - 1) || (y == 0) || (y == m_World.getSize() - 1)) {
						// Rocks on outer edge
						m_Background[x][y] = kRocks[Simulation.random.nextInt(kRocks.length)];
					} else {
						m_Background[x][y] = kTrees[Simulation.random.nextInt(kTrees.length)];
					}
				} else if (cell.isOpen()) {
					m_Background[x][y] = kGrass[Simulation.random.nextInt(kGrass.length)];
				} else if (cell.isEnergyRecharger()) {
					m_Background[x][y] = kRecharger;
				} else if (cell.isHealthRecharger()) {
					m_Background[x][y] = kHealth;
				} else {
					logger.warning("Unknown cell at " + x + "," + y);
					m_Background[x][y] = kWTF;
				}
			}
		}
	}
	
	Tank getTankAtPixel(int xIn, int yIn) {
		int x = xIn / m_CellSize;
		int y = yIn / m_CellSize;
		TankSoarCell cell = m_World.getCell(x, y);
		if (cell.containsTank()) {
			return cell.getTank();
		}
		return null;
	}

	public void paintControl(PaintEvent e){
		if (m_AgentLocation != null || m_LastX != e.x || m_LastY != e.y || internalRepaint) {
			m_LastX = e.x;
			m_LastY = e.y;
			setRepaint();
		}
		
		GC gc = e.gc;		
        gc.setForeground(WindowManager.black);
		gc.setLineWidth(1);

		if (m_Disabled || !m_Painted) {
			gc.setBackground(WindowManager.widget_background);
			gc.fillRectangle(0,0, this.getWidth(), this.getHeight());
			if (m_Disabled) {
				m_Painted = true;
				return;
			}
		}
		
		// Draw world
		for(int x = 0; x < m_World.getSize(); ++x){
			for(int y = 0; y < m_World.getSize(); ++y){
				TankSoarCell cell = m_World.getCell(x, y);
				if (!cell.needsRedraw() && m_Painted) {
					continue;
				}
				
				gc.drawImage(m_Background[x][y], x*m_CellSize, y*m_CellSize);
				if (cell.isExplosion()) {
					gc.drawImage(kExplosion, x*m_CellSize, y*m_CellSize);
					
				} else if (cell.containsTank()) {
					if (cell.isEnergyRecharger()) {
						gc.drawImage(kBlueRecharge, x*m_CellSize, y*m_CellSize);
						
					} else if (cell.isHealthRecharger()) {
						gc.drawImage(kRedRecharge, x*m_CellSize, y*m_CellSize);
						
					} else {
						Tank tank = cell.getTank();
						Image tankImage = (Image)kTanks.get(new Integer(tank.getFacingInt()));
						if (tankImage == null) {
							tankImage = kWTF;
						}
						gc.drawImage(tankImage, x*m_CellSize, y*m_CellSize);
						gc.setBackground((Color)m_EntityColors.get(tank));
						gc.fillOval(m_CellSize*x + m_CellSize/2 - kDotSize/2, m_CellSize*y + m_CellSize/2 - kDotSize/2, kDotSize, kDotSize);
												
						if (tank.getShieldStatus()) {
							
					        gc.setForeground((Color)m_EntityColors.get(tank));
							gc.setLineWidth(3);
							gc.drawOval(m_CellSize*x+2, m_CellSize*y+2, m_CellSize-5, m_CellSize-5);
					        gc.setForeground(WindowManager.black);
							gc.setLineWidth(1);
						}
						
						gc.setBackground(WindowManager.widget_background);
					}
						
				} else if (cell.containsMissilePack()) {
					gc.drawImage(kMissiles, x*m_CellSize, y*m_CellSize);
					
				}
				
				// Draw flying missiles regardless
				LinkedList missiles = m_World.getMissiles();
		  		ListIterator iter = missiles.listIterator();
		   		while (iter.hasNext()) {
		   			Missile missile = (Missile)iter.next();
					boolean plus = false;
					int dir = 0;
					switch (missile.getFlightPhase()) {
					case 0:
						dir = Direction.backwardOf[missile.getDirection()];
						break;
					case 1:
						dir = missile.getDirection();
						break;
					case 2:
						dir = missile.getDirection();
						plus = true;
						break;
					default:
						assert false;
						break;
					}
					int mX = 0;
					int mY = 0;
					switch (dir) {
					case Direction.kNorthInt:
						mX = 10;
						mY = plus ? -6 : 5;
						break;
					case Direction.kEastInt:
						mX = plus ? 26 : 15;
						mY = 10;
						break;
					case Direction.kSouthInt:
						mX = 10;
						mY = plus ? 26 : 15;
						break;
					case Direction.kWestInt:
						mX = plus ? -6 : 5;
						mY = 10;
						break;
					default:
						assert false;
						break;
					}
					gc.drawImage(kMissile, (missile.getLocation().x * m_CellSize) + mX, (missile.getLocation().y * m_CellSize) + mY);
		   		}
				
				// Draw radar trails
				Tank[] tanks = m_World.getTanks();
				if (tanks != null) {
					for (int i = 0; i < tanks.length; ++i) {
						if (tanks[i].getRadarStatus()) {
							int setting = tanks[i].getRadarDistance();
							java.awt.Point point = new java.awt.Point(tanks[i].getLocation());
							setting -= 1;
					        gc.setForeground(WindowManager.white);
							//gc.setLineWidth(2);
							int width = 0;
							int height = 0;
							int start = 0;
							switch (tanks[i].getFacingInt()) {
							case Direction.kNorthInt:
								width = m_CellSize*3;
								height = m_CellSize;
								start = 0;
								point = Direction.translate(point, tanks[i].getFacingInt());
								point = Direction.translate(point, Direction.leftOf[tanks[i].getFacingInt()]);
								break;
							case Direction.kSouthInt:
								width = m_CellSize*3;
								height = m_CellSize;
								start = 180;
								point = Direction.translate(point, tanks[i].getFacingInt());
								point = Direction.translate(point, Direction.rightOf[tanks[i].getFacingInt()]);
								break;
							case Direction.kEastInt:
								width = m_CellSize;
								height = m_CellSize*3;
								start = -90;
								point = Direction.translate(point, tanks[i].getFacingInt());
								point = Direction.translate(point, Direction.leftOf[tanks[i].getFacingInt()]);
								break;
							case Direction.kWestInt:
								width = m_CellSize;
								height = m_CellSize*3;
								start = 90;
								point = Direction.translate(point, tanks[i].getFacingInt());
								point = Direction.translate(point, Direction.rightOf[tanks[i].getFacingInt()]);
								break;
							default:
								assert false;
								break;
							}
							for (int j = 0; j < setting; ++j) {
								gc.drawArc(point.x*m_CellSize,point.y*m_CellSize,width - 2,height - 2, start,180);
								point = Direction.translate(point, tanks[i].getFacingInt());
							}
						}
					}
			        gc.setForeground(WindowManager.black);
				}
			}
		}
		
		m_Painted = true;
	}
}
