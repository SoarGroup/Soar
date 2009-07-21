package edu.umich.soar.gridmap2d.visuals;

import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.Canvas;
import org.eclipse.swt.widgets.Composite;

import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.Gridmap2D;


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
		kDiagram = new Image(parent.getDisplay(), Gridmap2D.class.getResourceAsStream("/org/msoar/gridmap2d/images/tanksoar/blocked-diagram.gif"));
		disable();
		addPaintListener(this);		
	}
	
	void set(int directions, Direction facing) {
		m_Forward = ((directions & facing.indicator()) > 0);
		m_Backward = ((directions & facing.backward().indicator()) > 0);
		m_Left = ((directions & facing.left().indicator()) > 0);
		m_Right = ((directions & facing.right().indicator()) > 0);		
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
