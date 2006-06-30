package tanksoar.visuals;

import java.util.logging.*;

import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

import tanksoar.*;
import simulation.visuals.*;

public class TankSoarAgentWorld extends VisualWorld implements PaintListener {

	private static Logger logger = Logger.getLogger("tanksoar.visuals");
	
	private static Image kMiniWTF;
	private static Image kMiniOpen;
	private static Image kMiniObstacle;
	private static Image kMiniMissiles;
	private static Image kMiniEnergy;
	private static Image kMiniHealth;
	private static Image kMiniTank;
	private static final int kDotSize = 7;
	
	private static final int kCellSize = 20;
	private Image[][] m_Radar = new Image[Tank.kRadarWidth][Tank.kRadarHeight];
	private Color[][] m_Color = new Color[Tank.kRadarWidth][Tank.kRadarHeight];
	private boolean[][] m_Modified = new boolean[Tank.kRadarWidth][Tank.kRadarHeight];

	public TankSoarAgentWorld(Composite parent, int style, TankSoarSimulation simulation) {
		super(parent, style, simulation, kCellSize);
		
		loadImages(parent.getDisplay());
		addPaintListener(this);		
	}
	
	private void loadImages(Display display) {
		kMiniWTF = new Image(display, TankSoar.class.getResourceAsStream("/images/question.gif"));
		kMiniOpen = new Image(display, TankSoar.class.getResourceAsStream("/images/ground_small.gif"));
		kMiniObstacle = new Image(display, TankSoar.class.getResourceAsStream("/images/obstacle_small.gif"));
		kMiniMissiles = new Image(display, TankSoar.class.getResourceAsStream("/images/missile_small.gif"));
		kMiniEnergy = new Image(display, TankSoar.class.getResourceAsStream("/images/battery_small.gif"));
		kMiniHealth = new Image(display, TankSoar.class.getResourceAsStream("/images/health_small.gif"));
		kMiniTank = new Image(display, TankSoar.class.getResourceAsStream("/images/tank_small.gif"));
	}
	
	public void update(Tank tank) {
		// update radar using tank
		TankSoarCell[][] radarCells = tank.getRadarCells();
		for(int x = 0; x < Tank.kRadarWidth; ++x){
			for(int y = 0; y < Tank.kRadarHeight; ++y){
				m_Color[x][y] = null;
				TankSoarCell cell = radarCells[x][y];
				
				if (cell != null && !cell.isModified()) {
					m_Modified[x][y] = false;
				} else {
					m_Modified[x][y] = true;
				}
				
				if (cell == null) {
					m_Radar[x][y] = kMiniWTF;
					           
				} else if (cell.containsTank()) {
					// Used to draw tank but direction unknown, just draw open
					// unless self
					if (x == 1 && y == 0) {
						m_Radar[x][y] = kMiniTank;
					} else {
						m_Radar[x][y] = kMiniOpen;
					}
					m_Color[x][y] = (Color)m_EntityColors.get(cell.getTank());

				} else if (cell.containsMissilePack()) {
					m_Radar[x][y] = kMiniMissiles;
					
				} else if (cell.isWall()) {
					m_Radar[x][y] = kMiniObstacle;
					
				} else if (cell.isOpen()) {
					m_Radar[x][y] = kMiniOpen;
					
				} else if (cell.isEnergyRecharger()) {
					m_Radar[x][y] = kMiniEnergy;
					
				} else if (cell.isHealthRecharger()) {
					m_Radar[x][y] = kMiniHealth;					
				}
			}
		}
	}
	
	public void paintControl(PaintEvent e){
		m_LastX = e.x;
		m_LastY = e.y;
		setRepaint();
		
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
		for(int x = 0; x < Tank.kRadarWidth; ++x){
			for(int y = 0; y < Tank.kRadarHeight; ++y){
				gc.drawImage(m_Radar[x][y], y*m_CellSize, x*m_CellSize);
				if (m_Color[x][y] != null) {
					gc.setBackground(m_Color[x][y]);
					gc.fillOval(m_CellSize*y + m_CellSize/2 - kDotSize/2, m_CellSize*x + m_CellSize/2 - kDotSize/2, kDotSize, kDotSize);
					gc.setBackground(WindowManager.widget_background);
				}
				
//				if (m_Modified[x][y]) {
//					gc.setForeground(WindowManager.white);
//					gc.drawOval(m_CellSize*x + m_CellSize/2 - kDotSize/2, m_CellSize*(Tank.kRadarHeight - y - 1) + m_CellSize/2 - kDotSize/2, kDotSize, kDotSize);
//			        gc.setForeground(WindowManager.black);
//				}
			}
		}
		m_Painted = true;
	}

	public int getWidth() {
		return m_CellSize * Tank.kRadarHeight;
	}
	
	public int getHeight() {
		return m_CellSize * Tank.kRadarWidth;
	}
	
}
