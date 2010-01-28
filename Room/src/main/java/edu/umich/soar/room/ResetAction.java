package edu.umich.soar.room;

import java.awt.event.ActionEvent;

public class ResetAction extends AbstractGridmap2DAction {

	private static final long serialVersionUID = 8971347340846353565L;

	public ResetAction(ActionManager manager) {
		super(manager, "Reset");
	}

	@Override
	public void update() {
        setEnabled(!getApplication().getSim().isRunning());
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		getApplication().getSim().reset();
	}

}
