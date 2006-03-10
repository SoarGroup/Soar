package eaters.visuals;

import java.util.*;

import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

import eaters.*;
import simulation.*;
import simulation.visuals.*;

public class EatersVisualWorld extends VisualWorld implements PaintListener {
	static HashMap m_FoodColors;
	
	public static void remapFoodColors(EatersWorld.Food[] food) {
		if (food == null) {
			m_FoodColors = null;
			return;
		}
		m_FoodColors = new HashMap();
		for (int i = 0; i < food.length; ++i) {
			m_FoodColors.put(food[i], WindowManager.getColor(food[i].getColor()));
		}
	}
	
	EatersSimulation m_Simulation;
	EatersWorld m_World;
	Point m_AgentLocation;
	
	public EatersVisualWorld(Composite parent, int style, EatersSimulation simulation, int cellSize) {
		super(parent, style, simulation, cellSize);
		
		m_Simulation = simulation;
		m_World = m_Simulation.getEatersWorld();

		addPaintListener(this);		
	}
	
	public int getMiniWidth() {
		return m_CellSize * ((Eater.kEaterVision * 2) + 1);
	}
	
	public int getMiniHeight() {
		return m_CellSize * ((Eater.kEaterVision * 2) + 1);
	}
	
	public void setAgentLocation(Point location) {
		m_AgentLocation = location;
	}
	
	Eater getEaterAtPixel(int x, int y) {
		x /= m_CellSize;
		y /= m_CellSize;
		EatersWorld.EatersCell cell = m_World.getCell(x, y);
		if (cell.isEater()) {
			return cell.getEater();
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
		for(int x = 0; x < m_World.getWidth(); ++x){
			if (m_AgentLocation != null) {
				if ((x < m_AgentLocation.x - Eater.kEaterVision) || (x > m_AgentLocation.x + Eater.kEaterVision)) {
					continue;
				} 
				xDraw = x + Eater.kEaterVision - m_AgentLocation.x;
			} else {
				xDraw = x;
			}
			
			for(int y = 0; y < m_World.getHeight(); ++y){
				if (m_AgentLocation != null) {
					if ((y < m_AgentLocation.y - Eater.kEaterVision) || (y > m_AgentLocation.y + Eater.kEaterVision)) {
						continue;
					} 
					yDraw = y + Eater.kEaterVision - m_AgentLocation.y;
				} else {
					yDraw = y;
				}
				
				EatersWorld.EatersCell cell = m_World.getCell(x, y);
				if (!cell.isModified() && m_Painted) {
					continue;
				}
				
				if (cell.isWall()) {
				    gc.setBackground(WindowManager.black);
				    gc.fillRectangle(m_CellSize*xDraw + 1, m_CellSize*yDraw + 1, m_CellSize - 2, m_CellSize - 2);
					
				} else if (cell.isEmpty()) {
					gc.setBackground(WindowManager.widget_background);
					gc.fillOval(m_CellSize*xDraw, m_CellSize*yDraw, m_CellSize, m_CellSize);
					
				} else if (cell.isEater()) {
					
					Eater eater = cell.getEater();
					
					gc.setBackground((Color)m_EntityColors.get(eater));
					gc.fillOval(m_CellSize*xDraw, m_CellSize*yDraw, m_CellSize, m_CellSize);
					gc.setBackground(WindowManager.widget_background);
					
					
					switch (eater.getFacingInt()) {
					case WorldEntity.kNorthInt:
						drawEaterMouth(xDraw, yDraw, 1, 0, 1, 1, gc);
						break;
					case WorldEntity.kEastInt:
						drawEaterMouth(xDraw + 1, yDraw, 0, 1, -1, 1, gc);
						break;
					case WorldEntity.kSouthInt:
						drawEaterMouth(xDraw, yDraw + 1, 1, 0, 1, -1, gc);
						break;
					case WorldEntity.kWestInt:
						drawEaterMouth(xDraw, yDraw, 0, 1, 1, 1, gc);
						break;
					default:
						break;
					}
				} else {
				
					EatersWorld.Food food = cell.getFood();
					
					gc.setBackground((Color)m_FoodColors.get(food));
					
					// TODO: this is a lot of string compares, should be integer switch
					switch (food.getShape()) {
					default:
						m_Logger.log("Invalid food shape '" + food.getShapeName() + "', using round.");
					case EatersWorld.Food.kRoundInt:
						fill1 = (int)(m_CellSize/2.8);
						fill2 = m_CellSize - fill1 + 1;
						gc.fillOval(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2, m_CellSize - fill2);
						gc.drawOval(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2 - 1, m_CellSize - fill2 - 1);
						break;
					case EatersWorld.Food.kSquareInt:
						fill1 = (int)(m_CellSize/2.8);
						fill2 = m_CellSize - fill1 + 1;
						gc.fillRectangle(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2, m_CellSize - fill2);
						gc.drawRectangle(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2, m_CellSize - fill2);
						break;
					}
				}
				
				if (cell.checkCollision()) {
					drawExplosion(gc, xDraw, yDraw);
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
		
		Point center = new Point(xBase + halfCell, yBase + halfCell);
		Point north = new Point(center.x, yBase);
		Point east = new Point(xBase + m_CellSize, center.y);
		Point south = new Point(center.x, yBase + m_CellSize);
		Point west = new Point(xBase, center.y);
		
		gc.fillPolygon(new int[] {center.x, center.y, north.x, north.y, center.x + offCenter, center.y});
		gc.fillPolygon(new int[] {center.x, center.y, east.x,  east.y,  center.x, center.y + offCenter});
		gc.fillPolygon(new int[] {center.x, center.y, south.x, south.y, center.x - offCenter, center.y});
		gc.fillPolygon(new int[] {center.x, center.y, west.x,  west.y,  center.x, center.y - offCenter});
	}
	
	void drawEaterMouth(int x, int y, int x_mult, int y_mult, int cx_mult, int cy_mult, GC gc){		
	    switch(m_Simulation.getWorldCount() % 8){
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
 }
