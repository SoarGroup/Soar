package edu.umich.soar.room;


import java.awt.Color;
//import java.awt.event.MouseAdapter;
//import java.awt.event.MouseEvent;
import java.util.ArrayList;
import java.util.List;

import org.flexdock.docking.DockingConstants;

import vis.VisCanvas;
import vis.VisData;
import vis.VisDataPointStyle;
import vis.VisWorld;

//import edu.umich.soar.room.core.Simulation;
import edu.umich.soar.room.map.Robot;
import edu.umich.soar.room.selection.SelectionManager;
import edu.umich.soar.room.selection.SelectionProvider;

public class WorldView extends AbstractAdaptableView implements SelectionProvider, Refreshable, BreadcrumbsProvider {
	
	private static final long serialVersionUID = 4612350578042898852L;
	
//	private final GridMapPanel gridMapPanel;
//	private SelectionManager manager;
//	private final Simulation sim;
	private Robot selectedPlayer;
    private VisWorld vw = new VisWorld();
    private VisCanvas vc = new VisCanvas(vw);

    public WorldView(Adaptable app) {
        super("worldView", "World View");

//		sim = Adaptables.adapt(app, Simulation.class);
		ActionManager am = Adaptables.adapt(app, ActionManager.class);
		am.getAction(ClearBreadcrumbsAction.class).setBreadcrumbsProvider(this);
		am.getAction(ToggleBreadcrumbsAction.class).setBreadcrumbsProvider(this);
		
        addAction(DockingConstants.PIN_ACTION);
        addAction(DockingConstants.CLOSE_ACTION);

//        gridMapPanel = new RoomPanel(app);
//        setContentPane(gridMapPanel);

        setContentPane(vc);

    	ArrayList<double[]> points = new ArrayList<double[]>();
    	points.add(new double[] {0, 0});
    	points.add(new double[] {0, 1});
    	points.add(new double[] {1, 1});
    	
    	VisData vd = new VisData(points,
    				 new VisDataPointStyle(Color.blue, 4));

    	VisWorld.Buffer vb = vw.getBuffer("mybuffer");
    	vb.addBuffered(vd);
    	vb.switchBuffer();

//        gridMapPanel.addMouseListener(new MouseAdapter() {
//        	@Override
//        	public void mousePressed(MouseEvent e) {
//        		int[] xy = gridMapPanel.getCellAtPixel(e.getX(), e.getY());
//        		if (sim.getMap().isInBounds(xy)) {
//            		
//            		if (e.isPopupTrigger()) {
//            			showPopupMenu(e, xy);
//            		} else {
//                		selectPlayer(getPlayerAtCell(xy));
//            			
//            		}
//        		}
//        	}
//
//        	public void mouseReleased(MouseEvent e) {
//        		int[] xy = gridMapPanel.getCellAtPixel(e.getX(), e.getY());
//        		if (sim.getMap().isInBounds(xy)) {
//            		
//            		if (e.isPopupTrigger()) {
//            			showPopupMenu(e, xy);
//            		}
//        		}
//        	}
//        });
    }

//	private Robot getPlayerAtCell(int[] xy) {
//		return this.sim.getMap().getCell(xy).getFirstPlayer();
//	}
//	
//	private void selectPlayer(Robot player) {
//		selectedPlayer = player;
//		if (this.manager != null) {
//			manager.fireSelectionChanged();
//		}
//	}
//
//	private void showPopupMenu(MouseEvent e, int[] xy)
//    {
//		final WorldMenu menu = new WorldMenu(this, this.sim, xy);
//		menu.getPopupMenu().show(e.getComponent(), e.getX(), e.getY());
//    }

	@Override
	public void activate(SelectionManager manager) {
//		this.manager = manager;
	}

	@Override
	public void deactivate() {
//		this.manager = null;
	}

	@Override
	public Object getSelectedObject() {
		return selectedPlayer;
	}

	@Override
	public List<Object> getSelection() {
		List<Object> selection = new ArrayList<Object>(1);
		selection.add(selectedPlayer);
		return selection;
	}

	@Override
	public void refresh() {
//		this.gridMapPanel.repaint();
	}

	@Override
	public boolean areBreadcrumbsEnabled() {
//		return gridMapPanel.areBreadcrumbsEnabled();
		return false;
	}

	@Override
	public void clearBreadcrumbs() {
//		gridMapPanel.clearBreadcrumbs();
	}

	@Override
	public void setBreadcrumbsEnabled(boolean setting) {
//		gridMapPanel.setBreadcrumbsEnabled(setting);
	}
	
}

/*
package edu.umich.soar.sps;

import java.awt.BorderLayout;
import java.awt.Color;
import java.util.ArrayList;

import javax.swing.JFrame;

import vis.VisCanvas;
import vis.VisData;
import vis.VisDataPointStyle;
import vis.VisWorld;

public class Application {


    JFrame jf;

    public Application() {
    	jf = new JFrame("VisExample");
    	jf.setLayout(new BorderLayout());
    	jf.add(vc, BorderLayout.CENTER);

    	jf.setSize(600,400);
    	jf.setVisible(true);

    	ArrayList<double[]> points = new ArrayList<double[]>();
    	points.add(new double[] {0, 0});
    	points.add(new double[] {0, 1});
    	points.add(new double[] {1, 1});
    	
    	VisData vd = new VisData(points,
    				 new VisDataPointStyle(Color.blue, 4));

    	VisWorld.Buffer vb = vw.getBuffer("mybuffer");
    	vb.addBuffered(vd);
    	vb.switchBuffer();
	}
	
	public static void main(String[] args) {
		new Application();
	}

}
*/