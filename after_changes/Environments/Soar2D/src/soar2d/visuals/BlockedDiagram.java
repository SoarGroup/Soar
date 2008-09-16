package soar2d.visuals;

import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

import soar2d.*;

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
		kDiagram = new Image(parent.getDisplay(), Soar2D.class.getResourceAsStream("/images/tanksoar/blocked-diagram.gif"));
		disable();
		addPaintListener(this);		
	}
	
	void set(int directions, int facing) {
		m_Forward = ((directions & Direction.indicators[facing]) > 0);
		m_Backward = ((directions & Direction.indicators[Direction.backwardOf[facing]]) > 0);
		m_Left = ((directions & Direction.indicators[Direction.leftOf[facing]]) > 0);
		m_Right = ((directions & Direction.indicators[Direction.rightOf[facing]]) > 0);		
		this.redraw();
	}

	void disable() {
		m_Forward = false;
		m_Backward = false;
		m_Left = false;
		m_Right = false;
		this.redraw();
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
