package soar2d.visuals;

import java.util.*;
import org.eclipse.swt.*;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.widgets.*;

import soar2d.*;
import soar2d.player.Player;
import soar2d.world.*;

public class VisualWorld extends Canvas implements PaintListener {
	public static HashMap<Player, Color> playerColors = new HashMap<Player, Color>();
	
	public static void remapPlayerColors() {
		ArrayList<Player> players = Soar2D.simulation.world.getPlayers();
		playerColors.clear();
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			String color = player.getColor();
			playerColors.put(player, WindowManager.getColor(color));
		}
	}

	public static boolean internalRepaint = false;
	
	java.awt.Point m_AgentLocation;
	
	protected Display m_Display;
	protected int m_CellSize;
	protected boolean m_Disabled = false;
	protected boolean m_Painted = false;
	protected int m_LastX = 0;
	protected int m_LastY = 0;
	
	public VisualWorld(Composite parent, int style, int cellSize) {
		super(parent, style | SWT.NO_BACKGROUND);
		m_Display = parent.getDisplay();
		m_CellSize = cellSize;

		addPaintListener(this);		
	}

	public int getMiniWidth() {
		return m_CellSize * ((Soar2D.config.kEaterVision * 2) + 1);
	}
	
	public int getMiniHeight() {
		return m_CellSize * ((Soar2D.config.kEaterVision * 2) + 1);
	}
	
	public void setAgentLocation(java.awt.Point location) {
		m_AgentLocation = location;
	}
	
	Player getPlayerAtPixel(int x, int y) {
		x /= m_CellSize;
		y /= m_CellSize;
		Cell cell = Soar2D.simulation.world.getCell(x, y);
		if (cell.getPlayer() != null) {
			return cell.getPlayer();
		}
		return null;
	}

	public void paintControl(PaintEvent e){
		if (m_AgentLocation != null || m_LastX != e.x || m_LastY != e.y || internalRepaint) {
			m_LastX = e.x;
			m_LastY = e.y;
			setRepaint();
		}
		
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
		int fill1, fill2, xDraw, yDraw;
		for(int x = 0; x < Soar2D.simulation.world.getSize(); ++x){
			if (m_AgentLocation != null) {
				if ((x < m_AgentLocation.x - Soar2D.config.kEaterVision) || (x > m_AgentLocation.x + Soar2D.config.kEaterVision)) {
					continue;
				} 
				xDraw = x + Soar2D.config.kEaterVision - m_AgentLocation.x;
			} else {
				xDraw = x;
			}
			
			for(int y = 0; y < Soar2D.simulation.world.getSize(); ++y){
				if (m_AgentLocation != null) {
					if ((y < m_AgentLocation.y - Soar2D.config.kEaterVision) || (y > m_AgentLocation.y + Soar2D.config.kEaterVision)) {
						continue;
					} 
					yDraw = y + Soar2D.config.kEaterVision - m_AgentLocation.y;
				} else {
					yDraw = y;
				}
				
				Cell cell = Soar2D.simulation.world.getCell(x, y);
				if ((cell.removeObject(Names.kRedraw) == null) && m_Painted) {
					continue;
				}
				
				ArrayList<CellObject> drawList = cell.getAllWithProperty(Names.kPropertyShape);
				// TODO: support multiple objects
				assert drawList.size() < 2;
				
				if (cell.getPlayer() != null) {
					
					Player eater = cell.getPlayer();
					
					gc.setBackground(playerColors.get(eater));
					gc.fillOval(m_CellSize*xDraw, m_CellSize*yDraw, m_CellSize, m_CellSize);
					gc.setBackground(WindowManager.widget_background);
					
					
					switch (eater.getFacingInt()) {
					case Direction.kNorthInt:
						drawEaterMouth(xDraw, yDraw, 1, 0, 1, 1, gc);
						break;
					case Direction.kEastInt:
						drawEaterMouth(xDraw + 1, yDraw, 0, 1, -1, 1, gc);
						break;
					case Direction.kSouthInt:
						drawEaterMouth(xDraw, yDraw + 1, 1, 0, 1, -1, gc);
						break;
					case Direction.kWestInt:
						drawEaterMouth(xDraw, yDraw, 0, 1, 1, 1, gc);
						break;
					default:
						break;
					}
				} else if (!cell.enterable()) {
				    gc.setBackground(WindowManager.black);
				    gc.fillRectangle(m_CellSize*xDraw + 1, m_CellSize*yDraw + 1, m_CellSize - 2, m_CellSize - 2);
					
				} else if (drawList.size() > 0) {
					CellObject object = drawList.get(0);
					
				    gc.setBackground(WindowManager.widget_background);
				    gc.fillRectangle(m_CellSize*xDraw, m_CellSize*yDraw, m_CellSize, m_CellSize);
				    
				    Color color = WindowManager.getColor(object.getStringProperty(Names.kPropertyColor));
				    if (color == null) {
				    	//TODO: draw outline!
				    }
					gc.setBackground(color);
					
					Shape shape = Shape.getShape(object.getStringProperty(Names.kPropertyShape));
					if (shape.equals(Shape.ROUND)) {
						fill1 = (int)(m_CellSize/2.8);
						fill2 = m_CellSize - fill1 + 1;
						gc.fillOval(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2, m_CellSize - fill2);
						gc.drawOval(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2 - 1, m_CellSize - fill2 - 1);
						
					} else if (shape.equals(Shape.SQUARE)) {
						fill1 = (int)(m_CellSize/2.8);
						fill2 = m_CellSize - fill1 + 1;
						gc.fillRectangle(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2, m_CellSize - fill2);
						gc.drawRectangle(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2, m_CellSize - fill2);
					}

				} else {
					gc.setBackground(WindowManager.widget_background);
					gc.fillRectangle(m_CellSize*xDraw, m_CellSize*yDraw, m_CellSize, m_CellSize);
				}
				
				if (cell.removeObject(Names.kExplosion) != null) {
					drawExplosion(gc, xDraw, yDraw);
					if (!cell.hasObject(Names.kRedraw)) {
						cell.addCellObject(new CellObject(Names.kRedraw, false, true));
					}
				}
			}
		}
		m_Painted = true;
	}
	
	void drawExplosion(GC gc, int x, int y) {
		gc.setBackground(WindowManager.red);
		int offCenter = m_CellSize/4;
		int xBase = m_CellSize*x;
		int yBase = m_CellSize*y;
		int halfCell = m_CellSize/2;
		
		java.awt.Point center = new java.awt.Point(xBase + halfCell, yBase + halfCell);
		java.awt.Point north = new java.awt.Point(center.x, yBase);
		java.awt.Point east = new java.awt.Point(xBase + m_CellSize, center.y);
		java.awt.Point south = new java.awt.Point(center.x, yBase + m_CellSize);
		java.awt.Point west = new java.awt.Point(xBase, center.y);
		
		gc.fillPolygon(new int[] {center.x, center.y, north.x, north.y, center.x + offCenter, center.y});
		gc.fillPolygon(new int[] {center.x, center.y, east.x,  east.y,  center.x, center.y + offCenter});
		gc.fillPolygon(new int[] {center.x, center.y, south.x, south.y, center.x - offCenter, center.y});
		gc.fillPolygon(new int[] {center.x, center.y, west.x,  west.y,  center.x, center.y - offCenter});
	}
	
	void drawEaterMouth(int x, int y, int x_mult, int y_mult, int cx_mult, int cy_mult, GC gc){		
	    switch(Soar2D.simulation.world.getWorldCount() % 8){
			case(0):{
			    gc.fillPolygon(new int[]{m_CellSize*x, m_CellSize*y,
			            m_CellSize*x + x_mult*m_CellSize, m_CellSize * y + y_mult*m_CellSize,
			            m_CellSize*x + cx_mult*m_CellSize/2, m_CellSize*y + cy_mult*m_CellSize/2});
			    break;
			}
			case(1):{
			    gc.fillPolygon(new int[]{m_CellSize*x + x_mult*m_CellSize/8, m_CellSize*y + y_mult*m_CellSize/8,
			            m_CellSize*x + x_mult*(m_CellSize - m_CellSize/8), m_CellSize * y + y_mult*(m_CellSize - m_CellSize/8),
			            m_CellSize*x + cx_mult*m_CellSize/2, m_CellSize*y + cy_mult*m_CellSize/2});
			    break;
			}
			case(2):{
			    gc.fillPolygon(new int[]{m_CellSize*x + x_mult*m_CellSize/4, m_CellSize*y + y_mult*m_CellSize/4,
			            m_CellSize*x + x_mult*(m_CellSize - m_CellSize/4), m_CellSize * y + y_mult*(m_CellSize - m_CellSize/4),
			            m_CellSize*x + cx_mult * m_CellSize/2, m_CellSize*y + cy_mult * m_CellSize/2});
			    break;
			}
			case(3):{
			    gc.fillPolygon(new int[]{m_CellSize*x + x_mult * 3*m_CellSize/8, m_CellSize*y + y_mult * 3*m_CellSize/8,
			            m_CellSize*x + x_mult*(m_CellSize - 3*m_CellSize/8), m_CellSize * y + y_mult*(m_CellSize - 3*m_CellSize/8),
			            m_CellSize*x + cx_mult*m_CellSize/2, m_CellSize*y + cy_mult*m_CellSize/2});
			    break;
			}
			case(4): break;
			case(5):{
			    gc.fillPolygon(new int[]{m_CellSize*x + x_mult * 3*m_CellSize/8, m_CellSize*y + y_mult * 3*m_CellSize/8,
			            m_CellSize*x + x_mult*(m_CellSize - 3*m_CellSize/8), m_CellSize * y + y_mult*(m_CellSize - 3*m_CellSize/8),
			            m_CellSize*x + cx_mult*m_CellSize/2, m_CellSize*y + cy_mult*m_CellSize/2});
			    break;
			}
			case(6):{
			    gc.fillPolygon(new int[]{m_CellSize*x + x_mult*m_CellSize/4, m_CellSize*y + y_mult*m_CellSize/4,
			            m_CellSize*x + x_mult*(m_CellSize - m_CellSize/4), m_CellSize * y + y_mult*(m_CellSize - m_CellSize/4),
			            m_CellSize*x + cx_mult * m_CellSize/2, m_CellSize*y + cy_mult * m_CellSize/2});
			    break;
			}
			case(7):{
			    gc.fillPolygon(new int[]{m_CellSize*x + x_mult*m_CellSize/8, m_CellSize*y + y_mult*m_CellSize/8,
			            m_CellSize*x + x_mult*(m_CellSize - m_CellSize/8), m_CellSize * y + y_mult*(m_CellSize - m_CellSize/8),
			            m_CellSize*x + cx_mult*m_CellSize/2, m_CellSize*y + cy_mult*m_CellSize/2});
			    break;	
			}
		}
	}
	
	public void setRepaint() {
		m_Painted = false;
	}
	
	public void disable() {
		m_Disabled = true;
	}
	
	public void enable() {
		m_Disabled = false;
	}
	
	public int getWidth() {
		return m_CellSize * Soar2D.simulation.world.getSize();
	}
	
	public int getHeight() {
		return m_CellSize * Soar2D.simulation.world.getSize();
	}
	
}
