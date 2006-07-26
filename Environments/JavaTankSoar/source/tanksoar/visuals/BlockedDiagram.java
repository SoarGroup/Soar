package tanksoar.visuals;

import java.util.logging.*;

import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

import simulation.visuals.*;
import tanksoar.*;
import utilities.Direction;

public class BlockedDiagram extends Canvas implements PaintListener {

	private static Logger logger = Logger.getLogger("simulation");
	
	private static Image kDiagram;
	
	static final int kSquareWidth = 10;
	static final int kSquareHeight = 7;
	static final int kForwardX = 20;
	static final int kForwardY = 4;
	static final int kRightX = 36;
	static final int kRightY = 21;
	static final int kBackwardX = 20;
	static final int kBackwardY = 39;
	static final int kLeftX = 4;
	static final int kLeftY = 21;
	
	
	boolean m_Forward;
	boolean m_Backward;
	boolean m_Left;
	boolean m_Right;
	
	public BlockedDiagram(Composite parent, int style) {
		super(parent, style);
		kDiagram = new Image(parent.getDisplay(), TankSoar.class.getResourceAsStream("/images/blocked-diagram.gif"));
		disable();
		addPaintListener(this);		
	}
	
	void updateBlocked(TankSoarWorld world, Tank tank) {
		updateCombinedDirection(world.getBlockedByLocation(tank), tank);
	}

	void updateIncoming(TankSoarWorld world, Tank tank) {
		updateCombinedDirection(world.getIncomingByLocation(tank.getLocation()), tank);
	}

	void updateSound(TankSoarWorld world, Tank tank) {
		int sound = world.getSoundNear(tank);
		
		m_Forward = (sound == tank.getFacingInt());
		m_Backward = (sound == Direction.backwardOf[tank.getFacingInt()]);
		m_Left = (sound == Direction.leftOf[tank.getFacingInt()]);
		m_Right = (sound == Direction.rightOf[tank.getFacingInt()]);		
	}

	void updateRWaves(TankSoarWorld world, Tank tank) {
		updateCombinedDirection(tank.getRWaves(), tank);
	}
	
	private void updateCombinedDirection(int directions, Tank tank) {
		m_Forward = ((directions & tank.getFacingInt()) > 0);
		m_Backward = ((directions & Direction.backwardOf[tank.getFacingInt()]) > 0);
		m_Left = ((directions & Direction.leftOf[tank.getFacingInt()]) > 0);
		m_Right = ((directions & Direction.rightOf[tank.getFacingInt()]) > 0);				
	}

	void disable() {
		m_Forward = m_Backward = m_Left = m_Right = false;
	}
	
	public void paintControl(PaintEvent e) {
		GC gc = e.gc;		
		gc.setBackground(WindowManager.black);
		gc.drawImage(kDiagram, 0, 0);
		
		if (m_Forward) {
			gc.fillRectangle(kForwardX, kForwardY, kSquareWidth, kSquareHeight);
		}
		if (m_Backward) {
			gc.fillRectangle(kBackwardX, kBackwardY, kSquareWidth, kSquareHeight);
		}
		if (m_Left) {
			gc.fillRectangle(kLeftX, kLeftY, kSquareWidth, kSquareHeight);
		}
		if (m_Right) {
			gc.fillRectangle(kRightX, kRightY, kSquareWidth, kSquareHeight);
		}
	}
}
