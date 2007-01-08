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
	Color[][] tanks = new Color[Soar2D.config.kRadarWidth][Soar2D.config.kRadarHeight];
	Image question;
	Image tankImage;
	boolean blank = false;
	
	int lastX = 0;
	int lastY = 0;
	public static boolean internalRepaint = false;
	protected boolean disabled = false;
	protected boolean painted = false;

	public TankSoarAgentWorld(Composite parent, int style) {
		super(parent, style);
		question = new Image(WindowManager.display, Soar2D.class.getResourceAsStream("/images/question.gif"));
		tankImage = new Image(WindowManager.display, Soar2D.class.getResourceAsStream("/images/tank-mini.gif"));
		addPaintListener(this);		
	}
	
	private void makeBlank() {
		for (int i = 0; i < radar.length; ++i) {
			for (int j = 0; j < radar[i].length; ++j) {
				radar[i][j] = null;
				tanks[i][j] = null;
			}
		}
	}
	
	public void setRepaint() {
		painted = false;
	}
	
	private Image getImage(RadarCell cell) {
		if (cell == null) {
			return question;
		}
		if (cell.obstacle) {
			return RadarCell.obstacleImage;
		}
		if (cell.energy) {
			return RadarCell.energyImage;
		}
		if (cell.health) {
			return RadarCell.healthImage;
		}
		if (cell.missiles) {
			return RadarCell.missilesImage;
		}
		return RadarCell.openImage;
	}
	
	public void update(Player tank) {
		if (!tank.getRadarSwitch()) {
			if (blank) {
				return;
			}
			
			makeBlank();
			blank = true;
			return;
		}
		
		blank = false;

		RadarCell[][] tankRadar = tank.getRadar();
		int distance = tank.getObservedDistance();
		for(int x = 0; x < Soar2D.config.kRadarWidth; ++x){
			for(int y = 0; y < Soar2D.config.kRadarHeight; ++y){
				if ((y < distance) || (y == distance && x == 1)) {
					if (x == 1 && y == 0) {
						radar[x][y] = tankImage;
						tanks[x][y] = WindowManager.getColor(tank.getColor());
					} else {
						radar[x][y] = getImage(tankRadar[x][y]);
						if (tankRadar[x][y].player == null) {
							tanks[x][y] = null;
						} else {
							tanks[x][y] = WindowManager.getColor(tankRadar[x][y].player.getColor());
						}
					}
				} else {
					radar[x][y] = null;
					tanks[x][y] = null;
				}
			}
		}
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
		
		// Draw world
		for (int x = 0; x < radar.length; ++x) {
			int drawHeight = radar[x].length * kCellSize;
			for (int y = 0; y < radar[x].length; ++y) {
				drawHeight -= kCellSize;
				if (radar[x][y] == null) {
					gc.drawImage(question, x*kCellSize, drawHeight);
				} else {
					gc.drawImage(radar[x][y], x*kCellSize, drawHeight);
				}
				if (tanks[x][y] != null) {
					gc.setBackground(tanks[x][y]);
					gc.fillOval(kCellSize*x + kCellSize/2 - kDotSize/2, drawHeight + kCellSize/2 - kDotSize/2, kDotSize, kDotSize);
					gc.setBackground(WindowManager.widget_background);
				}
			}
		}
		
		painted = true;
	}

	public int getWidth() {
		return kCellSize * Soar2D.config.kRadarWidth;
	}
	
	public int getHeight() {
		return kCellSize * Soar2D.config.kRadarHeight;
	}

	public void enable() {
		disabled = false;
		
	}

	public void disable() {
		disabled = true;
	}
}
