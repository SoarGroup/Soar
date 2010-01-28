package edu.umich.soar.room;

import java.awt.event.ActionEvent;

public class ToggleBreadcrumbsAction extends AbstractGridmap2DAction {

	private static final long serialVersionUID = -8850717104077590008L;
	private BreadcrumbsProvider bp;

	public ToggleBreadcrumbsAction(ActionManager manager) {
		super(manager, "Toggle Breadcrumbs");
	}

	@Override
	public void update() {
	}

	@Override
	public void actionPerformed(ActionEvent e) {
	}
	
	@Override
	public Object getValue(String key) {
		if (bp == null || key != SELECTED_KEY) {
			return super.getValue(key);
		}
		
		return bp.areBreadcrumbsEnabled();
	}
	
	@Override
	public void putValue(String key, Object newValue) {
		if (bp == null || key != SELECTED_KEY) {
			super.putValue(key, newValue);
			return;
		}
		bp.setBreadcrumbsEnabled((Boolean)newValue);
		getApplication().fireUpdate();
	}
	
	void setBreadcrumbsProvider(BreadcrumbsProvider bp) {
		this.bp = bp;
	}

}
