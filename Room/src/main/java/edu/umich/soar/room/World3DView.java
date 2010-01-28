package edu.umich.soar.room;


import org.flexdock.docking.DockingConstants;

import april.config.Config;
import april.config.ConfigUtil;
import april.viewer.Viewer;

public class World3DView extends AbstractAdaptableView implements BreadcrumbsProvider {
	
	private static final long serialVersionUID = 4612350578042898852L;
	
	//private final Simulation sim;
	private final Viewer viewer;

    public World3DView(Adaptable app) {
        super("worldView", "World View");

		//sim = Adaptables.adapt(app, Simulation.class);
		ActionManager am = Adaptables.adapt(app, ActionManager.class);
		am.getAction(ClearBreadcrumbsAction.class).setBreadcrumbsProvider(this);
		am.getAction(ToggleBreadcrumbsAction.class).setBreadcrumbsProvider(this);
		
        addAction(DockingConstants.PIN_ACTION);
        addAction(DockingConstants.CLOSE_ACTION);

        // TODO uses april not umich config, might need an adaptor or something
        // TODO get path from elsewhere
        Config config = ConfigUtil.getDefaultConfig(new String[] {"config/simulator"});
        viewer = new Viewer(config);

        setContentPane(viewer.getVisCanvas());
    }
    
	@Override
	public boolean areBreadcrumbsEnabled() {
		return false;
	}

	@Override
	public void clearBreadcrumbs() {
	}

	@Override
	public void setBreadcrumbsEnabled(boolean setting) {
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