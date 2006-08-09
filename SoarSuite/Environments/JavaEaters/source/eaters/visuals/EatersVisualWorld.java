package eaters.visuals;

import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

import eaters.*;
import simulation.visuals.*;
import utilities.*;

public class EatersVisualWorld extends VisualWorld implements PaintListener {
	
	static Color[] foodColors = new Color[0];
	
	/**
	* Reallocates an array with a new size, and copies the contents
	* of the old array to the new array.
	* @param oldArray  the old array, to be reallocated.
	* @param newSize   the new array size.
	* @return          A new array with the same contents.
	*/
	private static Object resizeArray(Object oldArray, int newSize) {
	   int oldSize = java.lang.reflect.Array.getLength(oldArray);
	   Class elementType = oldArray.getClass().getComponentType();
	   Object newArray = java.lang.reflect.Array.newInstance(elementType,newSize);
	   int preserveLength = Math.min(oldSize,newSize);
	   if (preserveLength > 0) {
	      System.arraycopy (oldArray,0,newArray,0,preserveLength);
	   }
	   return newArray; 
	}
	
	public static void remapFoodColors() {
		
		foodColors = (Color[])resizeArray(foodColors, Food.foodTypeCount());
		for (int i = 0; i < Food.foodTypeCount(); ++i) {
			Food f = Food.getFood(i);
			foodColors[i] = WindowManager.getColor(f.color());
		}
	}
	
	EatersSimulation m_Simulation;
	EatersWorld m_World;
	java.awt.Point m_AgentLocation;
	
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
	
	public void setAgentLocation(java.awt.Point location) {
		m_AgentLocation = location;
	}
	
	Eater getEaterAtPixel(int x, int y) {
		x /= m_CellSize;
		y /= m_CellSize;
		EatersCell cell = m_World.getCell(x, y);
		if (cell.getEater() != null) {
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
		for(int x = 0; x < m_World.getSize(); ++x){
			if (m_AgentLocation != null) {
				if ((x < m_AgentLocation.x - Eater.kEaterVision) || (x > m_AgentLocation.x + Eater.kEaterVision)) {
					continue;
				} 
				xDraw = x + Eater.kEaterVision - m_AgentLocation.x;
			} else {
				xDraw = x;
			}
			
			for(int y = 0; y < m_World.getSize(); ++y){
				if (m_AgentLocation != null) {
					if ((y < m_AgentLocation.y - Eater.kEaterVision) || (y > m_AgentLocation.y + Eater.kEaterVision)) {
						continue;
					} 
					yDraw = y + Eater.kEaterVision - m_AgentLocation.y;
				} else {
					yDraw = y;
				}
				
				EatersCell cell = m_World.getCell(x, y);
				if (!cell.checkDraw() && m_Painted) {
					continue;
				}

				if (cell.getEater() != null) {
					
					Eater eater = cell.getEater();
					
					gc.setBackground((Color)m_EntityColors.get(eater));
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
				} else if (cell.isWall()) {
				    gc.setBackground(WindowManager.black);
				    gc.fillRectangle(m_CellSize*xDraw + 1, m_CellSize*yDraw + 1, m_CellSize - 2, m_CellSize - 2);
					
				} else if (cell.getFood() != null) {
					Food food = cell.getFood();
					
				    gc.setBackground(WindowManager.widget_background);
				    gc.fillRectangle(m_CellSize*xDraw, m_CellSize*yDraw, m_CellSize, m_CellSize);
				    
					gc.setBackground(foodColors[food.id()]);
					
					if (food.shape().equals(Shape.ROUND)) {
						fill1 = (int)(m_CellSize/2.8);
						fill2 = m_CellSize - fill1 + 1;
						gc.fillOval(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2, m_CellSize - fill2);
						gc.drawOval(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2 - 1, m_CellSize - fill2 - 1);
					} else if (food.shape().equals(Shape.SQUARE)) {
						fill1 = (int)(m_CellSize/2.8);
						fill2 = m_CellSize - fill1 + 1;
						gc.fillRectangle(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2, m_CellSize - fill2);
						gc.drawRectangle(m_CellSize*xDraw + fill1, m_CellSize*yDraw + fill1, m_CellSize - fill2, m_CellSize - fill2);
					}

				} else {
					gc.setBackground(WindowManager.widget_background);
					gc.fillRectangle(m_CellSize*xDraw, m_CellSize*yDraw, m_CellSize, m_CellSize);
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
