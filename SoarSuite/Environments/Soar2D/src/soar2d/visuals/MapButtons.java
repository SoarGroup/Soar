package soar2d.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import soar2d.*;

public class MapButtons extends Composite {
	Controller control = Soar2D.control;
	private Button m_ChangeMapButton;
	Configuration config = Soar2D.config;

	public MapButtons(Composite parent, final String mapFilter) {
		super(parent, SWT.NONE);
		
		setLayout(new FillLayout());
		
		m_ChangeMapButton = new Button(this, SWT.PUSH);
		m_ChangeMapButton.setText("Change Map");
		m_ChangeMapButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				FileDialog fd = new FileDialog(MapButtons.this.getShell(), SWT.OPEN);
				fd.setText("Open");
				fd.setFilterPath(config.getMapPath());
				fd.setFilterExtensions(new String[] {mapFilter, "*.*"});
				VisualWorld.internalRepaint = true;
				String map = fd.open();
				VisualWorld.internalRepaint = false;
				if (map != null) {
					control.changeMap(map);
				}
			}
		});
		
		updateButtons();
	}
	
	public void updateButtons() {
		boolean running = control.isRunning();
		
		m_ChangeMapButton.setEnabled(!running);
	}
}
