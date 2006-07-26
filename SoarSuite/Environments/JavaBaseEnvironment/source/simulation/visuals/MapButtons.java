package simulation.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import simulation.*;

public class MapButtons extends Composite {
	private Simulation m_Simulation;
	private Button m_ChangeMapButton;
	
	public MapButtons(Composite parent, Simulation simulation, final String mapFilter) {
		super(parent, SWT.NONE);
		
		m_Simulation = simulation;
		
		setLayout(new FillLayout());
		
		m_ChangeMapButton = new Button(this, SWT.PUSH);
		m_ChangeMapButton.setText("Change Map");
		m_ChangeMapButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				FileDialog fd = new FileDialog(MapButtons.this.getShell(), SWT.OPEN);
				fd.setText("Open");
				fd.setFilterPath(m_Simulation.getMapPath());
				fd.setFilterExtensions(new String[] {mapFilter, "*.*"});
				VisualWorld.internalRepaint = true;
				String map = fd.open();
				VisualWorld.internalRepaint = false;
				if (map != null) {
					m_Simulation.changeMap(map);
				}
			}
		});
		
		updateButtons();
	}
	
	public void updateButtons() {
		boolean running = m_Simulation.isRunning();
		
		m_ChangeMapButton.setEnabled(!running);
	}
}
