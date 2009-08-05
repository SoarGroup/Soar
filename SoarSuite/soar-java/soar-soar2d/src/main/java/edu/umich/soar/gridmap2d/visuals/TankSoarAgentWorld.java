package edu.umich.soar.gridmap2d.visuals;

import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.Canvas;
import org.eclipse.swt.widgets.Composite;

import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.players.RadarCell;
import edu.umich.soar.gridmap2d.players.Tank;
import edu.umich.soar.gridmap2d.players.TankState;


public class TankSoarAgentWorld extends Canvas implements PaintListener {

	private static final int kDotSize = 7;
	private static final int kCellSize = 20;
	
	Image[][] radar;
	Color[][] tanks;
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
		
		radar = new Image[TankState.RADAR_WIDTH][TankState.RADAR_HEIGHT];
		tanks = new Color[TankState.RADAR_WIDTH][TankState.RADAR_HEIGHT];

		question = new Image(WindowManager.display, Gridmap2D.class.getResourceAsStream("/edu/umich/soar/gridmap2d/images/tanksoar/question.gif"));
		tankImage = new Image(WindowManager.display, Gridmap2D.class.getResourceAsStream("/edu/umich/soar/gridmap2d/images/tanksoar/tank-mini.gif"));
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
	
	public void update(Tank tank) {
		TankState state = tank.getState();
		if (!state.getRadarSwitch()) {
			if (blank) {
				return;
			}
			
			makeBlank();
			blank = true;
			return;
		}
		
		blank = false;

		RadarCell[][] tankRadar = state.getRadar();
		int distance = state.getObservedPower();
		for(int x = 0; x < TankState.RADAR_WIDTH; ++x){
			for(int y = 0; y < TankState.RADAR_HEIGHT; ++y){
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
		return kCellSize * TankState.RADAR_WIDTH;
	}
	
	public int getHeight() {
		return kCellSize * TankState.RADAR_HEIGHT;
	}

	public void enable() {
		disabled = false;
		
	}

	public void disable() {
		disabled = true;
	}
}
