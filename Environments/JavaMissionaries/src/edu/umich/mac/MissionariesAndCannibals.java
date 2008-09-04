//-----------------------------------------------------------------------------------
// edu.umich.mac.MissionariesAndCannibals
// Author: Trevor McCulloch
// Date Created: 24 March 2005
//------------------------------------------------------------------------------

package edu.umich.mac;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Canvas;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;

/**
 * This class provides an interface for the Missionaries & Cannibals problem,
 * allowing the user to watch the decisions Soar makes in an attempt to solve
 * the problem.
 * 
 * @author Trevor McCulloch, University of Michigan
 * @version 1.0
 */
public class MissionariesAndCannibals
implements Runnable, PaintListener, MacEnvironmentListener {
    private MacEnvironment me;
    
    private Display dpy;
    private Shell shell;
    
    private Canvas macCanvas;
    private Image bufImg;
    private GC bufGC;
    
    private Button runButton;
    private Button stopButton;
    private Button stepButton;
    private Button resetButton;

    // DJP: Using this form of image loading allows us to run in Eclipse and from a JAR file.
    // Eclipse looks for the "mac" folder to be under "bin" (with the current Eclipse project settings)
    // while the JAR build looks for it at the top level, so this is a bit clumsy.
    // If anyone really knows how this getResource stuff really works go ahead and clean this up :)
    private static Image landImg;
    private static Image boatImg;
    private static Image missionaryImg;
    private static Image cannibalImg;
/*
    private static Image landImg = new Image(dpy,
            ClassLoader.getSystemResourceAsStream("/mac/land.gif"));
    private static Image boatImg = new Image(dpy,
            ClassLoader.getSystemResourceAsStream("/mac/boat.gif"));
    private static Image missionaryImg = new Image(dpy,
            ClassLoader.getSystemResourceAsStream("/mac/missionary.gif"));
    private static Image cannibalImg = new Image(dpy,
            ClassLoader.getSystemResourceAsStream("/mac/cannibal.gif"));
*/    
    private static int[] BOAT_X;
    private static int BOAT_Y;
    private static int[] CANNIBAL_X;
    private static int CANNIBAL_Y;
    private static int[] MISSIONARY_X;
    private static int MISSIONARY_Y;
    
    static {
        //Display.setAppName("Missionaries & Cannibals");
        
        // initialize X positions for the boat, cannibals and missionaries
        BOAT_X = new int[2];
        CANNIBAL_X = new int[6];
        MISSIONARY_X = new int[6];
        
    }
    
    public void initializeImages()
    {
    	landImg = new Image(dpy,
                MissionariesAndCannibals.class.getResourceAsStream("/mac/land.gif"));
        boatImg = new Image(dpy,
        		MissionariesAndCannibals.class.getResourceAsStream("/mac/boat.gif"));
        missionaryImg = new Image(dpy,
        		MissionariesAndCannibals.class.getResourceAsStream("/mac/missionary.gif"));
        cannibalImg = new Image(dpy,
        		MissionariesAndCannibals.class.getResourceAsStream("/mac/cannibal.gif"));
    
        
        BOAT_Y = 400;
        CANNIBAL_Y = 300;
        MISSIONARY_Y = 330;
        
        {
            Display.setAppName("Missionaries & Cannibals");
            
            // initialize X positions for the boat, cannibals and missionaries
            BOAT_X[0] = 60;
            BOAT_X[1] = landImg.getBounds().width - boatImg.getBounds().width - 60;
            
            int x_adj = landImg.getBounds().width - cannibalImg.getBounds().width;
            CANNIBAL_X[0] = 45;
            CANNIBAL_X[1] = 85;
            CANNIBAL_X[2] = 125;
            CANNIBAL_X[3] = x_adj - 125;
            CANNIBAL_X[4] = x_adj - 85;
            CANNIBAL_X[5] = x_adj - 45;
            
            x_adj = landImg.getBounds().width - missionaryImg.getBounds().width;
            MISSIONARY_X[0] = 50;
            MISSIONARY_X[1] = 75;
            MISSIONARY_X[2] = 100;
            MISSIONARY_X[3] = x_adj - 100;
            MISSIONARY_X[4] = x_adj - 75;
            MISSIONARY_X[5] = x_adj - 50;
        }
    }
    
    /**
     * Creates a new <code>MissionariesAndCannibals</code> object and calls the
     * <code>run</code> method, which puts the user interface on the screen.
     * 
     * @param args - ignored
     */
    public static void main(String[] args) {
        new MissionariesAndCannibals().run();
        
        // Explicitly calling System.exit() ensures the javaw process shuts down cleanly.
        // Without this we sometimes have a problem with threads not shutting down on their own.
        // We still need to investigate this more fully (it's an SML-java issue when you run
        // the environment through a remote debugger--something's a little off).
        System.exit(0) ;
    }
    
    /**
     * Creates a new <code>MissionariesAndCannibals</code> object, which creates
     * the user interface and the environment Soar run in, and coordinates
     * communication between Soar's environment and the UI.
     */
    public MissionariesAndCannibals() {
        me = new MacEnvironment();
        me.addEnvironmentListener(this);
        
        dpy = new Display();
        initializeImages();
        
        // init SWT and a window
        shell = new Shell(dpy, SWT.TITLE | SWT.CLOSE | SWT.MIN);
        shell.setText("Missionaries & Cannibals");
        
        GridLayout layout = new GridLayout();
        layout.numColumns = 5;
        shell.setLayout(layout);
        
        // set up our canvas...
        // Specifying SWT.DOUBLE_BUFFERED to reduce flicker
        // SWT.NO_BACKGROUND also seems to work on Windows
        // and OS X, but is slow and causes tearing on Linux
        macCanvas = new Canvas(shell, SWT.DOUBLE_BUFFERED);
        macCanvas.setBounds(0, 0, 640, 480);
        macCanvas.addPaintListener(this);
        
        GridData mcdata = new GridData(GridData.FILL_BOTH);
        mcdata.horizontalSpan = 5;
        macCanvas.setLayoutData(mcdata);
        
        // ...and initialize our double buffer
        // (note: we are no longer using our own buffer
        // because SWT's native double buffer support (see above)
        // works better, but we're leaving the code in case we
        // want to revisit this alternative in the future; see
        // paintControl below to see how it was used)
        //bufImg = new Image(dpy, landImg.getBounds());
        //bufGC = new GC(bufImg);
        
        // create the button controls
        runButton = new Button(shell, SWT.PUSH);
        runButton.setText("Run");
        GridData bdata = new GridData();
        runButton.setLayoutData(bdata);
        
        stopButton = new Button(shell, SWT.PUSH);
        stopButton.setText("Stop");
        stopButton.setEnabled(false);
        bdata = new GridData();
        stopButton.setLayoutData(bdata);
        
        stepButton = new Button(shell, SWT.PUSH);
        stepButton.setText("Step");
        bdata = new GridData();
        stepButton.setLayoutData(bdata);
        
        resetButton = new Button(shell, SWT.PUSH);
        resetButton.setText("Reset");
        resetButton.setEnabled(false);
        bdata = new GridData();
        resetButton.setLayoutData(bdata);
        
        // add actions to all of the buttons
        runButton.addSelectionListener(new SelectionAdapter() {
            public void widgetSelected(SelectionEvent e) {
                runPressed();
            }
        });
        stopButton.addSelectionListener(new SelectionAdapter() {
            public void widgetSelected(SelectionEvent e) {
                stopPressed();
            }
        });
        stepButton.addSelectionListener(new SelectionAdapter() {
            public void widgetSelected(SelectionEvent e) {
                stepPressed();
            }
        });
        resetButton.addSelectionListener(new SelectionAdapter() {
            public void widgetSelected(SelectionEvent e) {
                resetPressed();
            }
        });
        
        shell.setDefaultButton(runButton);
    }

    /**
     * Open's the user interface's main window. This method blocks until the
     * window is closed.
     */
    public void run() {
        shell.pack();
        shell.setSize(640, 540);
        shell.open();
        while (!shell.isDisposed()) {
            if (!dpy.readAndDispatch()) {
                dpy.sleep();
            }
        }
        me.detachSoar();
        dpy.dispose();
        System.exit(0);
    }
    
    
    private void runPressed() {
        me.runSystem();
        
        updateButtons(true);
    }
    
    private void stopPressed() {
        me.stopSystem();
        
        updateButtons(false);
    }
    
    private void stepPressed() {
        updateButtons(true);
        // run the environment through a decision cycle
        me.step();
        
        updateButtons(false);
    }
    
    private void resetPressed() {
        me.reset();
        
        updateButtons(false);
    }
    
    public void updateButtons(boolean running) {
        boolean done = me.isAtGoalState();
        
        runButton.setEnabled(!running && !done);
        stopButton.setEnabled(running);
        resetButton.setEnabled(!running);
        stepButton.setEnabled(!running && !done);
    }

    public void paintControl(PaintEvent e) {
    	// The commented out code in this method was used to implement a double
    	// buffer, but it didn't seem to work right on all platforms, and it
    	// isn't necessary -- we now specify SWT.DOUBLE_BUFFERED in the canvas
    	// creation above.
    	
        // clip drawing in our buffer to the affected area.
        //bufGC.setClipping(e.x, e.y, e.width, e.height);
        //paintBuffer(bufGC);
        // copy from the buffer to the screen
        //e.gc.drawImage(bufImg,e.x,e.y,e.width,e.height,e.x,e.y,e.width,e.height);
        
        paintBuffer(e.gc);
    }
    
    /**
     * Paints the specified buffer with background, the positions of the boat and
     * all of the missionaries and cannibals.
     */
    private void paintBuffer(GC gc) {
        gc.drawImage(landImg, 0, 0);
        
        // draw the boat
        gc.drawImage(boatImg,
                BOAT_X[me.getRightBank().getBoatCount()], BOAT_Y);
        
        // draw the cannibals on both sides of the river
        for (int i = 0; i < me.getLeftBank().getCannibalCount(); ++i)
            gc.drawImage(cannibalImg, CANNIBAL_X[i], CANNIBAL_Y);
        for (int i = 0; i < me.getRightBank().getCannibalCount(); ++i)
            gc.drawImage(cannibalImg, CANNIBAL_X[i + 3], CANNIBAL_Y);
        
        // draw the missionaries on both sides of the river
        for (int i = 0; i < me.getLeftBank().getMissionaryCount(); ++i)
            gc.drawImage(missionaryImg, MISSIONARY_X[i], MISSIONARY_Y);
        for (int i = 0; i < me.getRightBank().getMissionaryCount(); ++i)
            gc.drawImage(missionaryImg, MISSIONARY_X[i + 3], MISSIONARY_Y);
    }

    /**
     * Modify the UI so that it appears the "start" button has just been pressed.
     */
    public void systemStarted(MacEnvironment e) {
        dpy.syncExec(new Runnable() {
            public void run() { updateButtons(true); }
        });
    }

    /**
     * Modify the UI so that it appears the "stop" button has just been pressed.
     */
    public void systemStopped(MacEnvironment e) {
        dpy.syncExec(new Runnable() {
            public void run() { updateButtons(false); }
        });
    }
    
    /**
     * Notes the new positions of the boat and all of the missonaries and
     * cannibals, and arranges for the screen to be redrawn in the appropriate
     * places.
     */
    public void boatMoved(final MacEnvironment e, final RiverBank fromBank,
            final int missionaries, final int cannibals, final int boats) {
        // this should be a lot of fun
        dpy.syncExec(new Runnable() {
            public void run() {
                // redraw the two boat positions
                Rectangle rect = boatImg.getBounds();
                macCanvas.redraw(BOAT_X[0], BOAT_Y, rect.width, rect.height,
                        true);
                macCanvas.redraw(BOAT_X[1], BOAT_Y, rect.width, rect.height,
                        true);
                
                // redraw the cannibals right-most positions on each bank
                rect = cannibalImg.getBounds();
                for (int i = 0; i < cannibals; ++i) {
                    int fb = i + fromBank.getCannibalCount();
                    int tb = fromBank.getOppositeBank().getCannibalCount()-i-1;
                    if (fromBank == e.getLeftBank())
                        tb += 3;
                    else
                        fb += 3;
                    
                    macCanvas.redraw(CANNIBAL_X[fb], CANNIBAL_Y,
                            rect.width, rect.height, true);
                    macCanvas.redraw(CANNIBAL_X[tb], CANNIBAL_Y,
                            rect.width, rect.height, true);
                }
                
                // redraw the missionaries right-most positions on each bank
                rect = missionaryImg.getBounds();
                for (int i = 0; i < missionaries; ++i) {
                    int fb = i + fromBank.getMissionaryCount();
                    int tb = fromBank.getOppositeBank().getMissionaryCount()-i-1;
                    if (fromBank == e.getLeftBank())
                        tb += 3;
                    else
                        fb += 3;
                    
                    macCanvas.redraw(MISSIONARY_X[fb], MISSIONARY_Y,
                            rect.width, rect.height, true);
                    macCanvas.redraw(MISSIONARY_X[tb], MISSIONARY_Y,
                            rect.width, rect.height, true);
                }
            }
        });
    }

    public void atGoalState(MacEnvironment e) { /* DO NOTHING */ }
    
    protected void finalize() throws Throwable {
        landImg.dispose();
        boatImg.dispose();
        missionaryImg.dispose();
        cannibalImg.dispose();
        
        bufGC.dispose();
        bufImg.dispose();
    }
}
