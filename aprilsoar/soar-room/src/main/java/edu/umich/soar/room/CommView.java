package edu.umich.soar.room;

import java.awt.BorderLayout;

import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

import org.flexdock.docking.DockingConstants;

import edu.umich.soar.room.core.Simulation;
import edu.umich.soar.room.map.CommListener;

public class CommView extends AbstractAgentView {
	private static final long serialVersionUID = -1676542480621263207L;
	private final Simulation sim;
	private final JTextArea commOutput = new JTextArea();

    private final CommListener outputListener = new BasicCommListener(commOutput);
    
    public CommView(Adaptable app) {
        super("commView", "All Communication");
        addAction(DockingConstants.PIN_ACTION);
        addAction(DockingConstants.CLOSE_ACTION);

        this.sim = Adaptables.adapt(app, Simulation.class);

        final JPanel p = new JPanel(new BorderLayout());
        
        commOutput.setEditable(false);
        commOutput.setRows(4);
        commOutput.setLineWrap(true);
        commOutput.setWrapStyleWord(true);
        this.sim.getWorld().addCommListener(outputListener);

        p.add(new JScrollPane(commOutput), BorderLayout.CENTER);
        setContentPane(p);
    }
        
	@Override
	public void refresh() {
	}
}
