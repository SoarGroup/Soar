package simulation.visuals;

import java.util.HashMap;
import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;

import simulation.*;

public class VisualWorld extends Canvas {
	public static HashMap m_EntityColors = null;
	
	public static void remapEntityColors(WorldEntity[] entities) {
		if (entities == null) {
			m_EntityColors = null;
			return;
		}
		m_EntityColors = new HashMap();
		for (int i = 0; i < entities.length; ++i) {
			m_EntityColors.put(entities[i], WindowManager.getColor(entities[i].getColor()));
		}		
	}

	public static boolean internalRepaint = false;
	
	protected Display m_Display;
	protected int m_CellSize;
	protected boolean m_Disabled = false;
	protected boolean m_Painted = false;
	protected int m_LastX = 0;
	protected int m_LastY = 0;
	private SimulationManager m_Simulation;
	
	public VisualWorld(Composite parent, int style, SimulationManager simulation, int cellSize) {
		super(parent, style | SWT.NO_BACKGROUND);
		m_Simulation = simulation;
		m_Display = parent.getDisplay();
		m_CellSize = cellSize;

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
		return m_CellSize * m_Simulation.getWorld().getSize();
	}
	
	public int getHeight() {
		return m_CellSize * m_Simulation.getWorld().getSize();
	}
	
}
