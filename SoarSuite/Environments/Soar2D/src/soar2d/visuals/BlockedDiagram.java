package soar2d.visuals;

import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

import soar2d.*;
import soar2d.player.*;

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
		kDiagram = new Image(parent.getDisplay(), Soar2D.class.getResourceAsStream("/images/blocked-diagram.gif"));
		disable();
		addPaintListener(this);		
	}
	
	void updateBlocked(World world, Player tank) {
		//TODO
		//updateCombinedDirection(world.getBlockedByLocation(tank), tank);
	}

	void updateIncoming(World world, Tank tank) {
		//TODO
		//updateCombinedDirection(world.getIncomingByLocation(tank.getLocation()), tank);
	}

	void updateSound(World world, Tank tank) {
		//TODO
		//int sound = world.getSoundNear(tank);
		int sound = 0;
		
		m_Forward = (sound == tank.getFacingInt());
		m_Backward = (sound == Direction.backwardOf[tank.getFacingInt()]);
		m_Left = (sound == Direction.leftOf[tank.getFacingInt()]);
		m_Right = (sound == Direction.rightOf[tank.getFacingInt()]);		
	}

	void updateRWaves(World world, Tank tank) {
		//updateCombinedDirection(tank.getRWaves(), tank);
	}
	
	private void updateCombinedDirection(int directions, Tank tank) {
		m_Forward = ((directions & Direction.indicators[tank.getFacingInt()]) > 0);
		m_Backward = ((directions & Direction.indicators[Direction.backwardOf[tank.getFacingInt()]]) > 0);
		m_Left = ((directions & Direction.indicators[Direction.leftOf[tank.getFacingInt()]]) > 0);
		m_Right = ((directions & Direction.indicators[Direction.rightOf[tank.getFacingInt()]]) > 0);				
	}

	void disable() {
		m_Forward = false;
		m_Backward = false;
		m_Left = false;
		m_Right = false;
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
