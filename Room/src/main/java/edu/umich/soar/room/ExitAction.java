package edu.umich.soar.room;

import java.awt.event.ActionEvent;

public class ExitAction extends AbstractGridmap2DAction {

	private static final long serialVersionUID = -8850717104077590008L;

	public ExitAction(ActionManager manager) {
		super(manager, "Exit");
	}

	@Override
	public void update() {
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		getApplication().exit();
	}

}
