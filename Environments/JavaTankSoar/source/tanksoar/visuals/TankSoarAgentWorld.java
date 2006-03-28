package tanksoar.visuals;

import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

import tanksoar.*;
import simulation.visuals.*;

public class TankSoarAgentWorld extends VisualWorld implements PaintListener {

	private static Image kMiniWTF;
	private static Image kMiniOpen;
	private static Image kMiniObstacle;
	private static Image kMiniMissiles;
	private static Image kMiniEnergy;
	private static Image kMiniHealth;
	private static Image kMiniTank;
	
	private static final int kCellSize = 20;
	private Image[][] m_Radar = new Image[Tank.kRadarWidth][Tank.kRadarHeight];

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
		for(int x = 0; x < Tank.kRadarWidth; ++x){
			for(int y = 0; y < Tank.kRadarHeight; ++y){
				if (x == 1 && y == 0) {
					m_Radar[x][y] = kMiniTank;
					continue;
				}
				
				String id = tank.getRadarID(x, y);
				if (id == null) {
					m_Radar[x][y] = kMiniWTF;
					           
				} else if (id.equalsIgnoreCase(InputLinkManager.kObstacleID)) {
					m_Radar[x][y] = kMiniObstacle;
					
				} else if (id.equalsIgnoreCase(InputLinkManager.kOpenID)) {
					m_Radar[x][y] = kMiniOpen;
					
				} else if (id.equalsIgnoreCase(InputLinkManager.kMissilesID)) {
					m_Radar[x][y] = kMiniMissiles;
					
				} else if (id.equalsIgnoreCase(InputLinkManager.kEnergyID)) {
					m_Radar[x][y] = kMiniEnergy;
					
				} else if (id.equalsIgnoreCase(InputLinkManager.kHealthID)) {
					m_Radar[x][y] = kMiniHealth;
					
				} else if (id.equalsIgnoreCase(InputLinkManager.kTankID)) {
					m_Radar[x][y] = kMiniTank;
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
				gc.drawImage(m_Radar[x][y], x*m_CellSize, (Tank.kRadarHeight - y - 1)*m_CellSize);
			}
		}
		m_Painted = true;
	}

	public int getWidth() {
		return m_CellSize * Tank.kRadarWidth;
	}
	
	public int getHeight() {
		return m_CellSize * Tank.kRadarHeight;
	}
	
}
