package edu.umich.soar.room;

import java.awt.event.ActionEvent;

public class ClearBreadcrumbsAction extends AbstractGridmap2DAction {

	private static final long serialVersionUID = -8850717104077590008L;
	private BreadcrumbsProvider bp;
	
	public ClearBreadcrumbsAction(ActionManager manager) {
		super(manager, "Clear Breadcrumbs");
	}

	@Override
	public void update() {
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		bp.clearBreadcrumbs();
		getApplication().fireUpdate();
	}

	void setBreadcrumbsProvider(BreadcrumbsProvider bp) {
		this.bp = bp;
	}

}
