package edu.umich.soar.room;

import java.awt.event.ActionEvent;

public class CreatePlayerAction extends AbstractGridmap2DAction {

	private static final long serialVersionUID = -8786603520740096430L;

	public CreatePlayerAction(ActionManager manager) {
		super(manager, "Create Player");
	}

	@Override
	public void update() {
        setEnabled(!getApplication().getSim().isRunning());
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		getApplication().createPlayer();
	}
}
