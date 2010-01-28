package edu.umich.soar.room;

import java.awt.BorderLayout;

import javax.swing.JPanel;
import javax.swing.JToolBar;

public class RunControlPanel extends JPanel {

	private static final long serialVersionUID = -5768132637851782263L;

	public RunControlPanel(Adaptable app) {
        super(new BorderLayout());
        
        final JToolBar bar = new JToolBar();
        bar.setFloatable(false);
        
        final ActionManager am = Adaptables.adapt(app, ActionManager.class);
        bar.add(am.getAction(RunAction.class));
        bar.add(am.getAction(StepAction.class));
        bar.add(am.getAction(StopAction.class));
        bar.add(am.getAction(ResetAction.class));
        
        add(bar, BorderLayout.CENTER);
	}
}
