package soar2d.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import soar2d.*;

public class MapButtons extends Composite {
	private Button m_ChangeMapButton;

	public MapButtons(Composite parent) {
		super(parent, SWT.NONE);
		
		setLayout(new FillLayout());
		
		m_ChangeMapButton = new Button(this, SWT.PUSH);
		m_ChangeMapButton.setText("Change Map");
		m_ChangeMapButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				FileDialog fd = new FileDialog(MapButtons.this.getShell(), SWT.OPEN);
				fd.setText("Open");
				fd.setFilterPath(Soar2D.config.getMapPath());
				fd.setFilterExtensions(new String[] {Soar2D.config.kEatersMapFilter, "*.*"});
				VisualWorld.internalRepaint = true;
				String map = fd.open();
				VisualWorld.internalRepaint = false;
				if (map != null) {
					Soar2D.control.changeMap(map);
				}
			}
		});
		
		updateButtons();
	}
	
	public void updateButtons() {
		boolean running = Soar2D.control.isRunning();
		
		m_ChangeMapButton.setEnabled(!running);
	}
}
