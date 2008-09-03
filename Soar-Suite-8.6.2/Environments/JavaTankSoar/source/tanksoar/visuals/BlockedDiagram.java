package tanksoar.visuals;

import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

import simulation.visuals.*;
import tanksoar.*;

public class BlockedDiagram extends Canvas implements PaintListener {

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
		
		m_Forward = (sound == tank.forward());
		m_Backward = (sound == tank.backward());
		m_Left = (sound == tank.left());
		m_Right = (sound == tank.right());		
	}

	void updateRWaves(TankSoarWorld world, Tank tank) {
		updateCombinedDirection(tank.getRWaves(), tank);
	}
	
	private void updateCombinedDirection(int directions, Tank tank) {
		m_Forward = ((directions & tank.forward()) > 0);
		m_Backward = ((directions & tank.backward()) > 0);
		m_Left = ((directions & tank.left()) > 0);
		m_Right = ((directions & tank.right()) > 0);				
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
