package edu.umich.soar.room;

import java.awt.event.ActionEvent;

public class StopAction extends AbstractGridmap2DAction {

	private static final long serialVersionUID = 7887235085533160856L;

	public StopAction(ActionManager manager) {
		super(manager, "Stop");
	}

	@Override
	public void update() {
        setEnabled(getApplication().getSim().isRunning());
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		getApplication().getSim().stop();
	}

}
