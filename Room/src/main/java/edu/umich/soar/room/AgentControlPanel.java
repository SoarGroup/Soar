package edu.umich.soar.room;

import java.awt.BorderLayout;

import javax.swing.JPanel;
import javax.swing.JToolBar;

public class AgentControlPanel extends JPanel {

	private static final long serialVersionUID = -7260588716833856406L;

	public AgentControlPanel(Adaptable app) {
        super(new BorderLayout());
        
        final JToolBar bar = new JToolBar();
        bar.setFloatable(false);
        
        final ActionManager am = Adaptables.adapt(app, ActionManager.class);
        bar.add(am.getAction(CreatePlayerAction.class));
        bar.add(am.getAction(ClonePlayerAction.class));
        bar.add(am.getAction(RemovePlayerAction.class));
        
        add(bar, BorderLayout.WEST);
	}
}
