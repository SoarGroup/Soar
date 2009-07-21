package org.msoar.gridmap2d.visuals;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.FileDialog;
import org.msoar.gridmap2d.Gridmap2D;


public class MapButtons extends Composite {
	private Button m_ChangeMapButton;

	public MapButtons(Composite parent) {
		super(parent, SWT.NONE);
		
		setLayout(new FillLayout());
		
		m_ChangeMapButton = new Button(this, SWT.PUSH);
		m_ChangeMapButton.setText("Change Map");
		m_ChangeMapButton.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				MapButtons.this.changeMap();
			}
		});
		
		updateButtons();
	}
	
	void changeMap() {
		FileDialog fd = new FileDialog(getShell(), SWT.OPEN);
		fd.setText("Open");
		fd.setFilterPath(Gridmap2D.simulation.getMapPath());
		VisualWorld.internalRepaint = true;
		String map = fd.open();
		VisualWorld.internalRepaint = false;
		if (map != null) {
			try {
				Gridmap2D.simulation.changeMap(map);
			} catch (Exception e) {
				Gridmap2D.control.errorPopUp(e.getMessage());
			}
		}
	}
	
	public void updateButtons() {
		boolean running = Gridmap2D.control.isRunning();
		
		m_ChangeMapButton.setEnabled(!running);
	}
}
