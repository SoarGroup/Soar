package soar2d.visuals;

import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

import soar2d.*;
import soar2d.player.*;

public class TankSoarAgentWorld extends Canvas implements PaintListener {

	private static final int kDotSize = 7;
	private static final int kCellSize = 20;
	
	Image[][] radar = new Image[Soar2D.config.kRadarWidth][Soar2D.config.kRadarHeight];
	Image question;
	boolean blank = false;
	
	int lastX = 0;
	int lastY = 0;
	public static boolean internalRepaint = false;
	protected boolean disabled = false;
	protected boolean painted = false;

	public TankSoarAgentWorld(Composite parent, int style) {
		super(parent, style);
		question = new Image(WindowManager.display, Soar2D.class.getResourceAsStream("/images/question.gif"));
		addPaintListener(this);		
	}
	
	private void makeBlank() {
		for (int i = 0; i < radar.length; ++i) {
			for (int j = 0; j < radar[i].length; ++j) {
				radar[i][j] = null;
			}
		}
	}
	
	public void setRepaint() {
		painted = false;
	}
	public void update(Tank tank) {
		if (!tank.getRadarSwitch()) {
			if (blank) {
				return;
			}
			
			makeBlank();
			blank = true;
			return;
		}
		
//		// update radar using tank
//		TankSoarCell[][] radarCells = tank.getRadarCells();
//		for(int x = 0; x < Tank.kRadarWidth; ++x){
//			for(int y = 0; y < Tank.kRadarHeight; ++y){
//				m_Color[x][y] = null;
//				TankSoarCell cell = radarCells[x][y];
//				
//				if (cell != null && !cell.isModified()) {
//					m_Modified[x][y] = false;
//				} else {
//					m_Modified[x][y] = true;
//				}
//				
//				if (cell == null) {
//					m_Radar[x][y] = kMiniWTF;
//					           
//				} else if (cell.containsTank()) {
//					// Used to draw tank but direction unknown, just draw open
//					// unless self
//					if (x == 1 && y == 0) {
//						m_Radar[x][y] = kMiniTank;
//					} else {
//						m_Radar[x][y] = kMiniOpen;
//					}
//					m_Color[x][y] = (Color)m_EntityColors.get(cell.getTank());
//
//				} else if (cell.containsMissilePack()) {
//					m_Radar[x][y] = kMiniMissiles;
//					
//				} else if (cell.isWall()) {
//					m_Radar[x][y] = kMiniObstacle;
//					
//				} else if (cell.isOpen()) {
//					m_Radar[x][y] = kMiniOpen;
//					
//				} else if (cell.isEnergyRecharger()) {
//					m_Radar[x][y] = kMiniEnergy;
//					
//				} else if (cell.isHealthRecharger()) {
//					m_Radar[x][y] = kMiniHealth;					
//				}
//			}
//		}
	}
	
	public void paintControl(PaintEvent e){
		if (lastX != e.x || lastY != e.y || internalRepaint) {
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
		
		// Draw world
		for (int x = 0; x < radar.length; ++x) {
			for (int y = 0; y < radar[x].length; ++y) {
				if (radar[x][y] == null) {
					gc.drawImage(question, y*kCellSize, x*kCellSize);
				}
			}
		}
		painted = true;
	}

	public int getWidth() {
		return kCellSize * Soar2D.config.kRadarHeight;
	}
	
	public int getHeight() {
		return kCellSize * Soar2D.config.kRadarWidth;
	}

	public void enable() {
		disabled = false;
		
	}

	public void disable() {
		disabled = true;
	}
}
