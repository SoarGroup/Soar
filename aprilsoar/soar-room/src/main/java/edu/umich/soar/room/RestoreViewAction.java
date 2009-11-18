package edu.umich.soar.room;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;

public class RestoreViewAction extends AbstractAction {

	private static final long serialVersionUID = 1L;
	
	public RestoreViewAction(AbstractAdaptableView view) {
		super("Restore " + view.getTitle());
	}

	@Override
	public void actionPerformed(ActionEvent e) {
	}
}
