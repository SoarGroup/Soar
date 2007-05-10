// put me in SoarSuite/Tools/SoarJavaDebugger/modules

/********************************************************************************************
*
* CanvasView.java
* 
* Created on 	Nov 23, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package modules;

import helpers.CommandHistory;
import helpers.FormDataHelper;

import manager.Pane;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.graphics.* ;
import org.eclipse.swt.events.*;

import sml.Agent;
import sml.smlAgentEventId;
import sml.smlSystemEventId;
import sml.smlUpdateEventId;
import sml.smlXMLEventId;
import sml.smlRunEventId;
import sml.Kernel;
import sml.ClientXML;
import sml.WMElement;

import java.util.*;
import java.lang.*;
import java.lang.Math.*;
import debugger.MainFrame;
import dialogs.PropertiesDialog;
import dialogs.ReorderButtonsDialog;
import doc.Document;

import java.io.*;

class LineInfo
{
	public int x0, y0, x1, y1;
	public Color color;
}

class RectInfo
{
	public int x0, y0, width, height;
	public Color color;
}

class PointInfo
{
	public int x0, y0;
	public int angle;
	public Color color;
}

class CircleInfo
{
	public int x, y;
	public int radius;
	public Color color;
}

class TextInfo
{
	public PointInfo point;
	public String text;
}

class PolarCoordinate 
{
	public int x, y;
	public int centroid_x, centroid_y;

	private float getPolar()
	{
       
		return (float) Math.atan2((float)(y - centroid_y), (float) (x - centroid_x));
	}

	// negative if this is < oCompare, positive if this is > oCompare
	public int compareTo(Object oCompare)
	{
		PolarCoordinate leCompare = (PolarCoordinate) oCompare;
		if (this.getPolar() == leCompare.getPolar())
			return 0;
		else if (this.getPolar() < leCompare.getPolar())
			return -1;
		else
			return 1;
	}
}

class Comparer implements Comparator 
{
	public int compare(Object obj1, Object obj2)
	{
		PolarCoordinate pc1 = (PolarCoordinate) obj1;
		PolarCoordinate pc2 = (PolarCoordinate) obj2;

		return pc1.compareTo(pc2);
	}
}

class ConvexInfo
{
	public Vector vertices;
	public int centroid_x, centroid_y;
	public Color color;
}

class PointerInfo
{
	public int x1, x2, x3, y1, y2, y3;
	public Color color;
}

	/********************************************************************************************
	 * 
	 * Shows a series of buttons for issuing commands.
	 * 
	 ********************************************************************************************/
public class CanvasView extends AbstractFixedView implements  Kernel.RhsFunctionInterface, Kernel.AgentEventInterface
{
	protected Canvas m_Canvas1;
	protected Canvas m_Canvas2;
	protected String m_Arguments;
	protected int    m_MapX0, m_MapX1, m_MapY0, m_MapY1, m_CanvasWidth, m_CanvasHeight;
	protected Shell  m_Shell;
	protected Color  m_Black;
	protected Color  m_Blue;
	protected Color  m_Green;
	protected Color  m_Red;
	protected Color  m_White;
	protected Color  m_Orange;
	protected Color m_Brown;
	protected Color m_Purple;
	protected Color m_Yellow;
	protected Display m_Display;
	protected boolean m_ShouldSwap = false;
	protected Image m_StaticImage;
	protected GC m_ImageGC;
	protected String m_FunctionName;
	protected boolean m_AutoDraw = true;
	protected double PI = 3.14159;
    
	protected int     m_drawLineCallBack = -1;
	protected int    m_drawRectCallBack = -1;
	protected int    m_drawPointCallBack = -1;
	protected int    m_setScaleCallBack = -1;
	protected int m_drawPointerCallBack = -1;
	protected int   m_toggleAutoUpdateCallBack = -1;
	protected int   m_drawCircleCallBack = -1;
	protected int   m_drawPolygonCallBack = -1;
	protected int m_drawTextCallBack = -1;
	protected int m_UpdateCallBack = -1;
	protected int    m_outputPhaseCallBack = -1;
	protected int   m_runEventCallBack = -1;
	protected int m_clearCallBack = -1;
	protected int m_initBeforeCallBack = -1;
    
	protected Map m_PointsDrawTo;
	protected Map m_LinesDrawTo;
	protected Map m_RectsDrawTo;
	protected Map m_CirclesDrawTo;
	protected Map m_TextDrawTo;
	protected Map m_PolysDrawTo;
	protected Map m_PointersDrawTo;

	protected LineInfo m_Line;
	protected ConvexInfo m_Convex;
	protected Integer m_ConvexKey;
	protected RectInfo m_Rect;
	protected PointInfo m_Point;
	protected CircleInfo m_Circle;
	protected TextInfo m_Text;

    
	/********************************************************************************************
	 * 
	 * This "base name" is used to generate a unique name for the window.
	 * For example, returning a base name of "trace" would lead to windows named
	 * "trace1", "trace2" etc.
	 * 
	 ********************************************************************************************/
	public String getModuleBaseName() { return "logio" ; }
    
	/********************************************************************************************
	 * 
	 * Initialize this window and its children.
	 * Should call setValues() at the start to complete initialization of the abstract view.
	 * 
	 ********************************************************************************************/
	public void init(MainFrame frame, Document doc, Pane parentPane)
	{
	
		setValues(frame, doc, parentPane) ;
	
		createCanvasWindow(m_Pane.getWindow()) ;	
	
	
	
		m_PointsDrawTo = Collections.synchronizedMap(new HashMap());
		m_LinesDrawTo = Collections.synchronizedMap(new HashMap());
		m_RectsDrawTo = Collections.synchronizedMap(new HashMap());
		m_CirclesDrawTo = Collections.synchronizedMap(new HashMap());
		m_TextDrawTo = Collections.synchronizedMap(new HashMap());
		m_PolysDrawTo = Collections.synchronizedMap(new HashMap());
		m_PointersDrawTo = Collections.synchronizedMap(new HashMap());
	
	}
    
    
    
	public void showProperties()
	{
		m_Frame.ShowMessageBox("Properties", "There are currently no properties for this view") ;
	}
    
	protected void createCanvasWindow(final Composite parent)
	{
		// Allow us to recreate the panel by calling this multiple times
		if (m_Container != null)
		{
			m_Container.dispose() ;
			m_Container = null ;
		}
	
	
		// The container lets us control the layout of the controls
		// within this window
		m_Container	   = new Composite(parent, SWT.NULL) ;
		m_Display	   = parent.getDisplay();
	
	
		m_Black = parent.getDisplay().getSystemColor(SWT.COLOR_BLACK);
		m_Red = parent.getDisplay().getSystemColor(SWT.COLOR_RED);
		m_White = parent.getDisplay().getSystemColor(SWT.COLOR_WHITE);
		m_Blue = parent.getDisplay().getSystemColor(SWT.COLOR_BLUE);
		m_Green = parent.getDisplay().getSystemColor(SWT.COLOR_GREEN);
		m_Orange = new Color(parent.getDisplay(), 255, 127, 0);
		m_Brown = parent.getDisplay().getSystemColor(SWT.COLOR_DARK_YELLOW);
		m_Purple = parent.getDisplay().getSystemColor(SWT.COLOR_MAGENTA);
		m_Yellow = parent.getDisplay().getSystemColor(SWT.COLOR_YELLOW);
	
	
		m_Shell = new Shell(m_Container.getShell());
		m_Shell.setSize(540, 600);
		m_Shell.open();
	
	
		m_Canvas1 = new Canvas(m_Shell, SWT.BORDER | SWT.DOUBLE_BUFFERED);
		m_CanvasHeight = 500;	
		m_CanvasWidth = 500;
		m_Canvas1.setSize(m_CanvasWidth, m_CanvasHeight);
		m_Canvas1.setLocation(20, 20);
		m_Canvas1.setVisible(true);

		m_StaticImage = new Image(m_Display, m_CanvasWidth, m_CanvasHeight);
		m_ImageGC = new GC(m_StaticImage);

		// paint listeners
	
		m_Canvas1.addPaintListener(new PaintListener() 
		{
			public synchronized void paintControl(PaintEvent e) 
			{							
				int x0, y0;
				GC gc = e.gc;

				gc.drawImage(m_StaticImage, 0, 0);
				// use iterators here

				Iterator values = m_PointsDrawTo.values().iterator();
				// Iterator keyValuePairs = m_PointsDrawTo.entrySet.iterator();
				while(values.hasNext())
				{
					// System.out.println("point");
					PointInfo point = (PointInfo) values.next();
					// System.out.println("after point");
					gc.setBackground(point.color);
					gc.drawRectangle(point.x0, point.y0, 3, 3);
					gc.fillRectangle(point.x0, point.y0, 3, 3);
				}

				values = m_LinesDrawTo.values().iterator();
				while(values.hasNext())
				{
					//  System.out.println("Line");
					LineInfo line = (LineInfo) values.next();
					// System.out.println("after Line");
					gc.setForeground(line.color);
			    
					gc.drawLine(line.x0, line.y0, line.x1, line.y1);
				}

				values = m_RectsDrawTo.values().iterator();
				while(values.hasNext())
				{	
					// System.out.println("Rect");
					RectInfo rect = (RectInfo) values.next();
					// System.out.println("after Rect");
			    
					gc.setForeground(rect.color);
			    
			    
					gc.drawRectangle(rect.x0, rect.y0, rect.width, rect.height);
				}

				values = m_CirclesDrawTo.values().iterator();
				while(values.hasNext())
				{
					// System.out.println("circle");
					CircleInfo circle = (CircleInfo) values.next();
					//  System.out.println("after circle");
					gc.setForeground(circle.color);
			    
					gc.drawOval(circle.x, circle.y, circle.radius, circle.radius);
				}

				values = m_TextDrawTo.values().iterator();
				while(values.hasNext()) 
				{
					TextInfo text = (TextInfo) values.next();

					gc.setBackground(m_White);
					gc.setForeground(text.point.color);
					gc.drawText(text.text, text.point.x0, text.point.y0, true);
				}

		    
				values = m_PolysDrawTo.values().iterator();
				while(values.hasNext()) 
				{
					//System.out.println("drawing");
					ConvexInfo convex = (ConvexInfo) values.next();

					// draw
					drawConvexRegion(gc, convex);
				}

				values = m_PointersDrawTo.values().iterator();
				while(values.hasNext()) 
				{
					PointerInfo pointer = (PointerInfo) values.next();
				    
					gc.setForeground(pointer.color);
					gc.drawLine(pointer.x1, pointer.y1,pointer.x2, pointer.y2);
					gc.drawLine(pointer.x2, pointer.y2, pointer.x3, pointer.y3);
					gc.drawLine(pointer.x3, pointer.y3, pointer.x1, pointer.y1);
				}

		    
			
			}
		});
	

		createContextMenu(m_Container) ;
		createContextMenu(m_Shell);
		createContextMenu(m_Canvas1);
	}
    
	private  synchronized void swap()
	{
		Agent agent = this.getAgentFocus();
		if(agent == null)
			return;

		if(this.m_Container.isDisposed())
			return;


		//	System.out.println("redraw");
		m_Display.asyncExec
			(
			new Runnable()
		{
			public void run() 
			{
    
				//System.out.println("redraw");
				//System.out.println(m_AutoDraw);
				m_Canvas1.redraw();
			}
		}
			);
	}

	public  synchronized void update(GC g) 
	{
		System.out.println("in update");
	}
    
    
  
	public  synchronized String rhsFunctionHandler(int eventID, Object userData, String agentName, String functionName, String argument)
	{

		m_Arguments = argument;
		m_FunctionName = functionName;
		/*m_Display.asyncExec
		 (
		 new Runnable()
		 {
		 public void run()
		 {*/
		//System.out.println(m_FunctionName + " a " + m_Arguments + " b ");
		     
		if(m_FunctionName.equals("toggle_update")) 
		{
			m_AutoDraw = false;
		}
		else if(m_FunctionName.equals("update_canvas")) 
		{
			swap();
		}
		else if(m_FunctionName.equals("clear_canvas"))
		{			
			clearCanvas();			
		}
		else if(m_FunctionName.equals("set_scale")) 
		{	
		
			try 
			{
				m_MapX0 = new Integer(getSpaceDelimitedArg()).intValue();
				m_MapX1 = new Integer(getSpaceDelimitedArg()).intValue();			
				m_MapY0 = new Integer(getSpaceDelimitedArg()).intValue();
				m_MapY1 = new Integer(getSpaceDelimitedArg()).intValue();
			}
			catch(Exception exc) 
			{
				System.err.println("Invalid use of set scale: \n(exec set_scale |x_min x_max y_min y_max|) where x_min" +
					" and other variables are hardcoded constants.\n(exec set_scale || <x_min> | | <x_max>" +
					" | | <y_min> | | <y_max> || where <> indcate soar variables");
				System.exit(1);
			}
		}
		else 
		{
			String command = new String();
			try 
			{
				command = getSpaceDelimitedArg();
				if (command != "static" && command != "update" && command != "delete")
				{
					//Dialog error = new Dialog(m_Shell, "boo");
					System.err.println("You used an invalid keyword, please use \n\"static\", \"update\", or \"delete\" " +
										" after the function name");
					System.err.println("The insulting line is: ");
					System.err.println(functionName + " " + argument);
				}
			}
			catch(Exception exc)
			{
				System.out.println("It appears you have forgotten the command word, please use \n\"static\", \"update\", or \"delete\" " +
								   " after the function name");
				System.exit(1);
			}
			
			 
			if(m_FunctionName.equals("draw_line")) 
			{
				// System.out.println("in line");
			     
				m_ShouldSwap = true;
			     
				if(command.equals("update")) 
				{
					// System.out.println("update");
					Integer key;
					key = Integer.valueOf(getSpaceDelimitedArg());
				 
					m_LinesDrawTo.put(key, buildLineData());
				}
				else if(command.equals("static")) 
				{
				 
					final LineInfo line = buildLineData();
						
				 
					// m_StaticLines.add(line);
					m_Display.asyncExec
						(
						new Runnable()
					{
						public  synchronized void run()
						{
								
							m_ImageGC.setForeground(line.color);
							m_ImageGC.drawLine(line.x0, line.y0, line.x1, line.y1);
						}
					}
						);
				}
				else if(command.equals("delete")) 
				{
					Integer key;
					key = Integer.valueOf(getSpaceDelimitedArg());
				 
					m_LinesDrawTo.remove(key);
				}
				else
					System.out.println("Invalid use of command in line: must be 'update', 'delete', or 'static'");
			}
			else if(m_FunctionName.equals("draw_rectangle")) 
			{
				//System.out.println("in rect");
				m_ShouldSwap = true;
			     
				if(command.equals("update")) 
				{
					// System.out.println("update");
					Integer key;
					key = Integer.valueOf(getSpaceDelimitedArg());
				 
					m_RectsDrawTo.put(key, buildRectData());
				}
				else if(command.equals("static")) 
				{
					final RectInfo rect = buildRectData();
									 
					m_Display.asyncExec
						(
						new Runnable()
					{
						public synchronized  void run()
						{
								
							// m_StaticRects.add(rect);
							m_ImageGC.setForeground(rect.color);
							m_ImageGC.drawRectangle(rect.x0, rect.y0, rect.width, rect.height);
						}
					}
						);
				}
				else if(command.equals("delete")) 
				{
				 
					Integer key;
					key = Integer.valueOf(getSpaceDelimitedArg());
				 
					m_RectsDrawTo.remove(key);
				}
				else
					System.out.println("Invalid use of command in rectangle: must be 'update', 'delete', or 'static'");
			}
			else if(m_FunctionName.equals("draw_point")) 
			{
				// System.out.println("in point");
				m_ShouldSwap = true;
			     
				if(command.equals("update")) 
				{
					//System.out.println("update");
					Integer key;
					key = Integer.valueOf(getSpaceDelimitedArg());
				 
					m_PointsDrawTo.put(key, buildPointData());
				}
				else if(command.equals("static")) 
				{
				 
					final PointInfo point = buildPointData();
					
					m_Display.asyncExec
						(
						new Runnable()
					{
						public  synchronized void run()
						{
							//System.out.println("static point");
								
				 
							//m_StaticPoints.add(point);

							m_ImageGC.setBackground(point.color);
						
							m_ImageGC.drawRectangle(point.x0, point.y0, 3, 3);
							m_ImageGC.fillRectangle(point.x0, point.y0, 3, 3);
						}
					}
						);
				}
					
				else if(command.equals("delete")) 
				{
					Integer key;
					key = Integer.valueOf(getSpaceDelimitedArg());
				 
					m_PointsDrawTo.remove(key);
				}
			}
			else if(m_FunctionName.equals("draw_pointer")) 
			{
				// System.out.println("in point");
				m_ShouldSwap = true;
			     
				if(command.equals("update")) 
				{
					//System.out.println("update");
					Integer key;
					key = Integer.valueOf(getSpaceDelimitedArg());
				 
					m_PointersDrawTo.put(key, buildPointerData());
				}
				else if(command.equals("static")) 
				{
				 
					final PointerInfo pointer = buildPointerData();
					
					m_Display.asyncExec
						(
						new Runnable()
					{
						public  synchronized void run()
						{
							//System.out.println("static point");
								
				 
							//m_StaticPoints.add(point);

							    

							m_ImageGC.setForeground(pointer.color);
							m_ImageGC.drawLine(pointer.x1, pointer.y1, pointer.x2, pointer.y2);
							m_ImageGC.drawLine(pointer.x2, pointer.y2, pointer.x3, pointer.y3);
							m_ImageGC.drawLine(pointer.x3, pointer.y3, pointer.x1, pointer.y1);
							    
						}
					}
						);
				}
					
				else if(command.equals("delete")) 
				{
					Integer key;
					key = Integer.valueOf(getSpaceDelimitedArg());
				 
					m_PointsDrawTo.remove(key);
				}
			}
			else if(m_FunctionName.equals("draw_circle")) 
			{
				m_ShouldSwap = true;
			     
				if(command.equals("update")) 
				{
					//System.out.println("update");
					Integer key;
					key = Integer.valueOf(getSpaceDelimitedArg());
				 
					m_CirclesDrawTo.put(key, buildCircleData());
				}
				else if(command.equals("static")) 
				{
					
					final CircleInfo circle = buildCircleData();
					m_Display.asyncExec
						(
						new Runnable()
					{
						public  synchronized void run()
						{
							
							// m_StaticCircles.add(circle);
							m_ImageGC.setForeground(circle.color);
							m_ImageGC.drawOval(circle.x, circle.y, circle.radius, circle.radius);
						}
					}
						);
				}
				else if(command.equals("delete")) 
				{
					Integer key;
					key = Integer.valueOf(getSpaceDelimitedArg());
				 
					m_CirclesDrawTo.remove(key);
				}
			}
			else if(m_FunctionName.equals("draw_text")) 
			{
				m_ShouldSwap = true;

				if(command.equals("update")) 
				{
					Integer key;
					key = Integer.valueOf(getSpaceDelimitedArg());
				 
					m_TextDrawTo.put(key, buildTextData());
				}
				else if(command.equals("static")) 
				{
					final TextInfo text = buildTextData();
						

				 
					m_Display.asyncExec
						(
						new Runnable()
					{
						public  synchronized void run()
						{
								
							m_ImageGC.setForeground(text.point.color);
							m_ImageGC.drawText(text.text, text.point.x0, text.point.y0, true);
						}
					}
						);
				}
				else if(command.equals("delete")) 
				{
					Integer key;
					key = Integer.valueOf(getSpaceDelimitedArg());

					m_TextDrawTo.remove(key);
				}
			}
			else if(m_FunctionName.equals("draw_polygon")) 
			{
				//System.out.println("in poly");
				m_ShouldSwap = true;

				
			     
				if(command.equals("update")) 
				{
					Integer key;
					key = Integer.valueOf(getSpaceDelimitedArg());
					//System.out.println("in update");
					ConvexInfo convex = buildConvexData();
				 
					m_PolysDrawTo.put(key, convex);
				}
				else if(command.equals("static")) 
				{
						
					final ConvexInfo convex; 					    
					convex = buildConvexData();
					    
					// m_Convex = convex;
					m_Display.asyncExec
						(
						new Runnable()
					{
						public  synchronized void run()
						{
								
							drawConvexRegion(m_ImageGC, convex);

							//		convex.id = key.intValue();
							//drawConvexRegion(m_ImageGC, convex);
						}
					}
						);
				}
				else if(command.equals("delete")) 
				{
					Integer key;
					key = Integer.valueOf(getSpaceDelimitedArg());
					m_PolysDrawTo.remove(key);
				}
			}
				 
			/*	}
			 catch(Exception exc) 
			 {
			 System.out.println("It appears that you have forgotten the command word.  \nPlease use 'update'" +
			 " 'static' or 'delete' after the command name.");
			 System.exit(1);
			 }*/
		}
		     
		if(m_AutoDraw)
			swap();
		     
	
		String t = new String("success");
		return t;
		
	}
	
	public void agentEventHandler(int eventID, Object data, String agentName)
	{
		//if(eventID == smlAgentEventId.smlEVENT_BEFORE_AGENT_REINITIALIZED)
		//{
		//System.out.println("clearing canvas");
		m_Display.asyncExec
			( new Runnable() 
		{
			public void run() 
			{
				clearCanvas();
				//System.out.println("canvas cleared");
			}
		});
		//}
	}
    
  
    
	/************************************************************************
	 * 
	 * Create an instance of the class.  It does not have to be fully initialized
	 * (it's the caller's responsibility to finish the initilization).
	 * 
	 *************************************************************************/
	public static CanvasView createInstance()
	{
		return new CanvasView() ;
	}
    
    
	/************************************************************************
	 * 
	 * Register and unregister for Soar events for this agent.
	 * (E.g. a trace window might register for the print event)
	 * 
	 *************************************************************************/
	protected void registerForAgentEvents(Agent agent)
	{
		if(m_drawLineCallBack == -1)
		{
			Kernel kernel = agent.GetKernel();
			m_drawLineCallBack = kernel.AddRhsFunction("draw_line", this, null);
			m_setScaleCallBack = kernel.AddRhsFunction("set_scale", this, null);
			m_drawRectCallBack = kernel.AddRhsFunction("draw_rectangle", this, null);
			m_drawPointCallBack = kernel.AddRhsFunction("draw_point", this, null);
			m_drawTextCallBack = kernel.AddRhsFunction("draw_text", this, null);
			//	m_outputPhaseCallBack = kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, this, this);
			//	m_runEventCallBack = agent.RegisterForRunEvent(smlRunEventId.smlEVENT_AFTER_ELABORATION_CYCLE, this, this);
			m_drawCircleCallBack = kernel.AddRhsFunction("draw_circle", this, null);
			m_toggleAutoUpdateCallBack = kernel.AddRhsFunction("toggle_update", this, null);
			m_UpdateCallBack = kernel.AddRhsFunction("update_canvas", this, null);
			m_drawPolygonCallBack = kernel.AddRhsFunction("draw_polygon", this, null);
			m_drawPointerCallBack = kernel.AddRhsFunction("draw_pointer", this, null);
			m_clearCallBack = kernel.AddRhsFunction("clear_canvas", this, null);
			m_initBeforeCallBack = kernel.RegisterForAgentEvent(smlAgentEventId.smlEVENT_BEFORE_AGENT_REINITIALIZED, this, null) ;
		}
	}
    
	protected void unregisterForAgentEvents(Agent agent)
	{
		boolean ok = true ;
	
		if (m_drawLineCallBack != -1)
			ok = agent.GetKernel().RemoveRhsFunction(m_drawLineCallBack) && ok ;
		if (m_setScaleCallBack != -1)
			ok = agent.GetKernel().RemoveRhsFunction(m_setScaleCallBack) && ok ;
		if (m_drawRectCallBack != -1)
			ok = agent.GetKernel().RemoveRhsFunction(m_drawRectCallBack) && ok ;
		if (m_drawPointCallBack != -1)
			ok = agent.GetKernel().RemoveRhsFunction(m_drawPointCallBack) && ok ;
		if (m_outputPhaseCallBack != -1)
			ok = agent.GetKernel().UnregisterForUpdateEvent(m_outputPhaseCallBack) && ok;
		if (m_runEventCallBack != -1)
			ok = agent.UnregisterForRunEvent(m_runEventCallBack) && ok;
		if (m_drawCircleCallBack != -1)
			ok = agent.GetKernel().RemoveRhsFunction(m_drawCircleCallBack) && ok;
		if (m_drawTextCallBack != -1)
			ok = agent.GetKernel().RemoveRhsFunction(m_drawTextCallBack) && ok;
		if (m_toggleAutoUpdateCallBack != -1)
			ok = agent.GetKernel().RemoveRhsFunction(m_toggleAutoUpdateCallBack) && ok;
		if (m_UpdateCallBack != -1)
			ok = agent.GetKernel().RemoveRhsFunction(m_UpdateCallBack) && ok;
		if (m_drawPolygonCallBack != -1)
			ok = agent.GetKernel().RemoveRhsFunction(m_drawPolygonCallBack) && ok;
		if (m_drawPointerCallBack != -1)
			ok = agent.GetKernel().RemoveRhsFunction(m_drawPointerCallBack) && ok;
		if (m_clearCallBack != -1)
			ok = agent.GetKernel().RemoveRhsFunction(m_clearCallBack) && ok;
		if (m_initBeforeCallBack != -1)
			ok = agent.GetKernel().RemoveRhsFunction(m_initBeforeCallBack) && ok;
		if (!ok)
			throw new IllegalStateException("Error unregistering callbacks in phase canvas view") ;

		clearAgentEvents();
	}
	
	protected void clearAgentEvents()
	{
		m_drawLineCallBack = -1 ;
		m_setScaleCallBack = -1 ;
		m_drawRectCallBack = -1 ;
		m_drawPointCallBack = -1 ;
		m_drawCircleCallBack = -1;
		m_drawTextCallBack = -1;
		m_outputPhaseCallBack = -1;
		m_runEventCallBack = -1;
		m_toggleAutoUpdateCallBack = -1;
		m_UpdateCallBack = -1;
		m_drawPolygonCallBack = -1;
		m_drawPointerCallBack = -1;
		m_clearCallBack = -1;
		m_initBeforeCallBack = -1;
	}

	protected synchronized void clearCanvas()
	{
		// get new image and image gc
		m_StaticImage = new Image(m_Display, m_CanvasWidth, m_CanvasHeight);
		m_ImageGC = new GC(m_StaticImage);
		
		// clear out all the dynamic containers
		m_PointsDrawTo.clear();
		m_LinesDrawTo.clear();
		m_RectsDrawTo.clear();
		m_CirclesDrawTo.clear();
		m_TextDrawTo.clear();
		m_PolysDrawTo.clear();
		m_PointersDrawTo.clear();

		swap();
	}
    
	protected  synchronized String getSpaceDelimitedArg()
	{
		int index = 0;
		String arg1 = new String();
		while(m_Arguments.charAt(index) == ' ')
			index++;
		while(index < m_Arguments.length() && m_Arguments.charAt(index) != ' ')
		{
			arg1 += m_Arguments.charAt(index);
			index++;
		}
		m_Arguments = m_Arguments.substring(index);			
		return arg1;
	}
    
	protected  synchronized int scaleValue(int toScaleInt, int mapMinInt, int mapMaxInt, int CanvasMeasure)
	{
		Integer toScale = new Integer(toScaleInt);
		Integer mapMin = new Integer(mapMinInt);
		Integer mapMax = new Integer(mapMaxInt);
		Integer mapMeasure = new Integer(mapMax.intValue() - mapMin.intValue());
		Integer toScaleAdj = new Integer(toScale.intValue() - mapMin.intValue());
		double scaleRatio = toScaleAdj.doubleValue() / mapMeasure.doubleValue();
		double correctedValue = scaleRatio * CanvasMeasure;
		Double cV = new Double(correctedValue);
		return cV.intValue();
	
	}
    
	protected Color get_color(String color)
	{
		if(color.equals("red"))
			return m_Red;
		else if(color.equals("blue"))
			return m_Blue;
		else if(color.equals("black"))
			return m_Black;
		else if(color.equals("green"))
			return m_Green;
		else if(color.equals("white"))
			return m_White;
		else if(color.equals("orange"))
			return m_Orange;
		else if(color.equals("brown"))
			return m_Brown;
		else if(color.equals("purple"))
			return m_Purple;
		else if(color.equals("yellow"))
			return m_Yellow;
		return m_White;
	}
		       
	protected  synchronized LineInfo buildLineData()
	{
		LineInfo line = new LineInfo(); 
	
		line.x0 = scaleValue(Float.valueOf(getSpaceDelimitedArg()).intValue(), m_MapX0, m_MapX1,  m_CanvasWidth);
		line.y0 = scaleValue(Float.valueOf(getSpaceDelimitedArg()).intValue(), m_MapY0, m_MapY1,  m_CanvasHeight);
		line.x1 = scaleValue(Float.valueOf(getSpaceDelimitedArg()).intValue(), m_MapX0, m_MapX1,  m_CanvasWidth);
		line.y1 = scaleValue(Float.valueOf(getSpaceDelimitedArg()).intValue(), m_MapY0, m_MapY1,  m_CanvasHeight);
		
	
		String color = new String(getSpaceDelimitedArg());
		line.color  = get_color(color);

		return line;
	}

	protected synchronized  RectInfo buildRectData()
	{
		RectInfo rect = new RectInfo();
	
		rect.x0 = scaleValue(Float.valueOf(getSpaceDelimitedArg()).intValue(), m_MapX0, m_MapX1, m_CanvasWidth);
		rect.y0 = scaleValue(Float.valueOf(getSpaceDelimitedArg()).intValue(), m_MapY0, m_MapY1, m_CanvasHeight);
			
		int x1, y1;
		x1 = scaleValue(Float.valueOf(getSpaceDelimitedArg()).intValue(), m_MapX0, m_MapX1, m_CanvasWidth);
		y1 = scaleValue(Float.valueOf(getSpaceDelimitedArg()).intValue(), m_MapY0, m_MapY1, m_CanvasHeight);
	
		String color = new String(getSpaceDelimitedArg());
		rect.color = get_color(color);
	
		rect.width = Math.abs(x1 - rect.x0);
		rect.height = Math.abs(y1 - rect.y0);

		return rect;
	}

	protected synchronized  PointInfo buildPointData()
	{
		
		PointInfo point = new PointInfo();

		point.x0 = scaleValue(Float.valueOf(getSpaceDelimitedArg()).intValue(), m_MapX0, m_MapX1, m_CanvasWidth);
		point.y0 = scaleValue(Float.valueOf(getSpaceDelimitedArg()).intValue(), m_MapY0, m_MapY1, m_CanvasHeight);
		
	
		String color = new String(getSpaceDelimitedArg());
		point.color = get_color(color);

		return point;
	}

	protected  synchronized CircleInfo buildCircleData()
	{
		CircleInfo circle = new CircleInfo();

		int x_center = scaleValue(Float.valueOf(getSpaceDelimitedArg()).intValue(), m_MapX0, m_MapX1, m_CanvasWidth);
		int y_center = scaleValue(Float.valueOf(getSpaceDelimitedArg()).intValue(), m_MapY0, m_MapY1, m_CanvasHeight);
		int radius = Integer.valueOf(getSpaceDelimitedArg()).intValue();
	
		Float rad_temp = new Float(Math.sqrt(Math.pow(radius,2)*(m_MapX1-m_MapX0)*(m_MapY1-m_MapY0)/(m_CanvasWidth*m_CanvasHeight)));
	
		circle.radius = rad_temp.intValue();
		circle.x = x_center - circle.radius/2;
		circle.y = y_center - circle.radius/2;
	
		String color = new String(getSpaceDelimitedArg());
		circle.color = get_color(color);

		return circle;
	}

	protected synchronized  TextInfo buildTextData()
	{
		TextInfo text = new TextInfo();
	
		text.point = buildPointData();
		text.text = new String(getSpaceDelimitedArg());

		return text;
	}

	protected synchronized  ConvexInfo buildConvexData()
	{
		ConvexInfo convex = new ConvexInfo();
		convex.vertices = new Vector();

		int num_vertices = (Integer.valueOf(getSpaceDelimitedArg())).intValue();
		int total_x = 0, total_y = 0;

		int index = 0;
		while(index < num_vertices) 
		{
	    
			PolarCoordinate pc = new PolarCoordinate();
			pc.x = scaleValue(Float.valueOf(getSpaceDelimitedArg()).intValue(), m_MapX0, m_MapX1, m_CanvasWidth);
			pc.y = scaleValue(Float.valueOf(getSpaceDelimitedArg()).intValue(), m_MapY0, m_MapY1, m_CanvasHeight);

			total_x += pc.x;
			total_y += pc.y;

			convex.vertices.add(pc);

			index++;
		}

		// now get the centroid
		convex.centroid_x = total_x / num_vertices;
		convex.centroid_y = total_y / num_vertices;

		/*System.out.println("centroid ");
		 System.out.println(convex.centroid_x);
		 System.out.println(" ");
		 System.out.println(convex.centroid_y);*/

		// give all of the vertices the centroid info
		for(int i = 0; i < num_vertices; i++) 
		{
			((PolarCoordinate) convex.vertices.elementAt(i)).centroid_x = convex.centroid_x;
			((PolarCoordinate) convex.vertices.elementAt(i)).centroid_y = convex.centroid_y;
		}

		Collections.sort(convex.vertices, new Comparer());

		String color = new String(getSpaceDelimitedArg());
		convex.color = get_color(color);
	 
		return convex;	 
	} 

	protected synchronized PointerInfo buildPointerData()
	{
		int x0, y0;

		x0 = scaleValue(Float.valueOf(getSpaceDelimitedArg()).intValue(), m_MapX0, m_MapX1, m_CanvasWidth);
		y0 = scaleValue(Float.valueOf(getSpaceDelimitedArg()).intValue(), m_MapY0, m_MapY1, m_CanvasHeight);
	
		int Iangle = Float.valueOf(getSpaceDelimitedArg()).intValue();	

		PointerInfo pointer = new PointerInfo();
	
		String color = new String(getSpaceDelimitedArg());
		pointer.color = get_color(color);

	
		double angle = Math.toRadians(Iangle);
		angle *= -1;

					    

		pointer.x1 = x0 +  new Float(7*Math.cos(angle)).intValue();
		pointer.y1 = y0 +  new Float(7*Math.sin(angle)).intValue();
		pointer.x2 = x0 +  new Float(3*Math.cos(PI/2+angle)).intValue();
		pointer.y2 = y0 +  new Float(3*Math.sin(angle+PI/2)).intValue();
		pointer.x3 = x0 +  new Float(3*Math.cos(angle-PI/2)).intValue();
		pointer.y3 = y0 +  new Float(3*Math.sin(angle-PI/2)).intValue();
	
		/*System.out.println(x1);
		 System.out.println(y1);
		 System.out.println(x2);
		 System.out.println(y2);
		 System.out.println(x3);
		 System.out.println(y3);
		 System.out.println(new Float(5*Math.sin(angle-90)));*/

		return pointer;
	}

	protected synchronized  void drawConvexRegion(GC gc, ConvexInfo convex)
	{
		int size = convex.vertices.size();
		gc.setForeground(convex.color);
	

		for(int i = 0; i < size - 1; i++) 
		{
			int x_start = ((PolarCoordinate) (convex.vertices.elementAt(i))).x;
			int y_start = ((PolarCoordinate) (convex.vertices.elementAt(i))).y;
			int x_finish = ((PolarCoordinate) (convex.vertices.elementAt(i+1))).x;
			int y_finish = ((PolarCoordinate) (convex.vertices.elementAt(i+1))).y;
			gc.drawLine(x_start, y_start, x_finish, y_finish);
	    
			/*  System.out.println(i);
			 System.out.println(" Line at: ");
			 System.out.println(x_start);
			 System.out.println(" ");
			 System.out.println(y_start);
			 System.out.println(" ");
			 System.out.println(x_finish);
			 System.out.println(" ");
			 System.out.println(y_finish);	     */
		}

		// draw the last connecting line
		gc.drawLine(((PolarCoordinate) (convex.vertices.elementAt(0))).x, ((PolarCoordinate) (convex.vertices.elementAt(0))).y, ((PolarCoordinate) (convex.vertices.elementAt(size - 1))).x, ((PolarCoordinate) (convex.vertices.elementAt(size -1))).y);

		gc.setForeground(m_Black);
	}

    
}
