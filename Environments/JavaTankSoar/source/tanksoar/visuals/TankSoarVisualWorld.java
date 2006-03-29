package tanksoar.visuals;

import java.util.*;

import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

import simulation.*;
import simulation.visuals.*;
import tanksoar.*;
import utilities.*;

public class TankSoarVisualWorld extends VisualWorld implements PaintListener {
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
	private MapPoint m_AgentLocation;
	private Random m_Random;
	private Image[][] m_Background;
	RelativeDirections m_RD = new RelativeDirections();
	
	public TankSoarVisualWorld(Composite parent, int style, TankSoarSimulation simulation) {
		super(parent, style, simulation, kCellSize);
		
		m_Simulation = simulation;
		m_World = m_Simulation.getTankSoarWorld();
		m_Random = new Random();

		loadImages(parent.getDisplay());
		addPaintListener(this);		
	}
	
	private void loadImages(Display display) {
		kTanks.put(new Integer(WorldEntity.kSouthInt), new Image(display, TankSoar.class.getResourceAsStream("/images/tank_down.gif")));
		kTanks.put(new Integer(WorldEntity.kNorthInt), new Image(display, TankSoar.class.getResourceAsStream("/images/tank_up.gif")));
		kTanks.put(new Integer(WorldEntity.kEastInt), new Image(display, TankSoar.class.getResourceAsStream("/images/tank_right.gif")));
		kTanks.put(new Integer(WorldEntity.kWestInt), new Image(display, TankSoar.class.getResourceAsStream("/images/tank_left.gif")));
		
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
		m_Background = new Image[m_World.getWidth()][m_World.getHeight()];
		for(int x = 0; x < m_World.getWidth(); ++x){
			for(int y = 0; y < m_World.getHeight(); ++y){
				TankSoarCell cell = m_World.getCell(x, y);
				if (cell.isWall()) {
					if ((x == 0) || (x == m_World.getWidth() - 1) || (y == 0) || (y == m_World.getHeight() - 1)) {
						// Rocks on outer edge
						m_Background[x][y] = kRocks[m_Random.nextInt(kRocks.length)];
					} else {
						m_Background[x][y] = kTrees[m_Random.nextInt(kTrees.length)];
					}
				} else if (cell.isOpen()) {
					m_Background[x][y] = kGrass[m_Random.nextInt(kGrass.length)];
				} else if (cell.isEnergyRecharger()) {
					m_Background[x][y] = kRecharger;
				} else if (cell.isHealthRecharger()) {
					m_Background[x][y] = kHealth;
				} else {
					m_Logger.log("Unknown cell at " + x + "," + y);
					m_Background[x][y] = kWTF;
				}
			}
		}
	}
	
	Tank getTankAtPixel(int x, int y) {
		x /= m_CellSize;
		y /= m_CellSize;
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
		for(int x = 0; x < m_World.getWidth(); ++x){
			for(int y = 0; y < m_World.getHeight(); ++y){
				TankSoarCell cell = m_World.getCell(x, y);
				if (!cell.isModified() && m_Painted) {
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
				Missile[] missiles = m_World.getMissiles();
				if (missiles != null) {
					for (int i = 0; i < missiles.length; ++i) {
						gc.drawImage(kMissile, (missiles[i].getCurrentLocation().x * m_CellSize) + 9, (missiles[i].getCurrentLocation().y * m_CellSize) + 9);
					}
				}
				
				// Draw radar trails
				Tank[] tanks = m_World.getTanks();
				if (tanks != null) {
					for (int i = 0; i < tanks.length; ++i) {
						if (tanks[i].getRadarStatus()) {
							int setting = tanks[i].getRadarDistance();
							m_RD.calculate(tanks[i].getFacingInt());
							MapPoint point = new MapPoint(tanks[i].getLocation());
							point.travel(m_RD.forward);
							point.travel(m_RD.left);
							setting -= 1;
							//gc.setLineWidth(2);
							int width = 0;
							int height = 0;
							int start = 0;
							switch (m_RD.forward) {
							case WorldEntity.kNorthInt:
								width = m_CellSize*3;
								height = m_CellSize;
								start = 0;
								break;
							case WorldEntity.kSouthInt:
								width = m_CellSize*3;
								height = m_CellSize;
								start = 180;
								break;
							case WorldEntity.kEastInt:
								width = m_CellSize;
								height = m_CellSize*3;
								start = -90;
								break;
							case WorldEntity.kWestInt:
								width = m_CellSize;
								height = m_CellSize*3;
								start = 90;
								break;
							}
							for (int j = 0; j < setting; ++j) {
								//gc.drawArc(point.x*m_CellSize,point.y*m_CellSize,width,height, -90,180);
								point.travel(m_RD.forward);
							}
						}
					}
				}
			}
		}
		
		m_Painted = true;
	}
}
