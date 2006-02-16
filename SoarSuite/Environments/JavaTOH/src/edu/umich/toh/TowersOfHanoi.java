//-----------------------------------------------------------------------------------
// edu.umich.toh.TowersOfHanoi
// Author: Trevor McCulloch
//   Date: 23 November 2004
//-----------------------------------------------------------------------------------

package edu.umich.toh;

import java.util.List;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ControlEvent;
import org.eclipse.swt.events.ControlListener;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Canvas;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;

import sml.Kernel;


/**
 * This class provides a user interface to watch Soar solve the Towers of Hanoi
 * problem.
 * 
 * @author Trevor McCulloch, University of Michigan
 * @version 1.1
 */
public class TowersOfHanoi
implements PaintListener, GameListener, ControlListener {
    private Game game;
    private ToHRunner runner;
    
    private Display dpy;
    private Shell shell;
    
    private Canvas tohCanvas;
    private Button startButton;
    private Button stopButton;
    private Button stepButton;
    private Button resetButton;
    
    // drawing members
    private Image bufImg;
    private GC bufGC;
    private boolean reinitBuf = true;
    // below are pre-calculated data used to draw the game, based on the size of the
    // canvas we are drawing on
    private int towerCount;  // num towers
    private int towerWidth;  // width of one tower
    private int towerHeight; // height of one tower
    private int towerX[];    // offset of each tower from left edge
    private int diskCount;   // num disks
    private int diskHeight;  // height of each disk
    private int diskWidth[]; // width of disk 1,2,...,num disks
    private int diskXOff[];  // offset from left edge of tower for disk 1,2,...
    
    /**
     * The main method called by Java to start the game.
     * 
     * @param args - arguments to start the game.
     *     -t &lt;num&gt;: number of towers to use.
     *     -d &lt;num&gt;: number of disks to use.
     * <p>NOTE: these arguments are not necessarily supported by the Soar productions
     * used to implement the game.
     */
    public static void main(String [] args) {
        int towerCount = 3;
        int diskCount  = 11;
        
        for (int i = 0; i < args.length; ++i) {
            if (args[i].equals("-t")) {
                if (++i < args.length) {
                    try {
                        towerCount = Integer.parseInt(args[i]);
                    } catch (NumberFormatException nfe) {
                        System.out.println("Bad number for tower count");
                        System.exit(1);
                    }
                } else {
                    System.out.println("No number given for tower count");
                    System.exit(1);
                }
            } else if (args[i].equals("-d")) {
                if (++i < args.length) {
                    try {
                        diskCount = Integer.parseInt(args[i]);
                    } catch (NumberFormatException nfe) {
                        System.out.println("Bad number for disk count");
                        System.exit(1);
                    }
                } else {
                    System.out.println("No number given for disk count");
                    System.exit(1);
                }
            } else {
                System.out.print("Unrecognized option ");
                System.out.println(args[i]);
                System.exit(1);
            }
        }
        
        new TowersOfHanoi(new Game(towerCount, diskCount)).run();
        
        // Explicitly calling System.exit() ensures the javaw process shuts down cleanly.
        // Without this we sometimes have a problem with threads not shutting down on their own.
        // We still need to investigate this more fully (it's an SML-java issue when you run
        // the environment through a remote debugger--something's a little off).
        System.exit(0) ;
    }
    
	public void systemEventHandler(int eventID, Object data, Kernel kernel)
	{
		if (eventID == sml.smlSystemEventId.smlEVENT_SYSTEM_START.swigValue())
		{
			// The callback comes in on Soar's thread and we have to update the buttons
			// on the UI thread, so switch threads.
			dpy.asyncExec(new Runnable() { public void run() { updateButtons(true) ; } } ) ;
		}

		if (eventID == sml.smlSystemEventId.smlEVENT_SYSTEM_STOP.swigValue())
		{
			// The callback comes in on Soar's thread and we have to update the buttons
			// on the UI thread, so switch threads.
			dpy.asyncExec(new Runnable() { public void run() { updateButtons(false) ; } } ) ;
		}
	}
    
    /**
     * Create the interface for Towers of Hanoi, based on the <code>Game</code>
     * object passed.
     * 
     * @param g - the game to show.
     */
    public TowersOfHanoi(Game g) {
        game = g;
        game.addGameListener(this);
        
        // Only doing this now so we can set the buttons to a reasonable state when the
        // user is running from the debugger.  If all buttons were enabled at all times
        // we could skip this.
        game.registerForStartStopEvents(this) ;
        
        dpy = new Display();
        shell = new Shell(dpy, SWT.TITLE | SWT.CLOSE | SWT.MIN | SWT.RESIZE);
        
        GridLayout layout = new GridLayout();
        layout.numColumns = 5;
        shell.setLayout(layout);
        shell.setText("Towers of Hanoi");
        
        // main widget
        tohCanvas = new Canvas(shell, SWT.NONE);
        GridData tcdata = new GridData(GridData.FILL_BOTH);
        tcdata.horizontalSpan = 5;
        tohCanvas.setLayoutData(tcdata);
        tohCanvas.addPaintListener(this);
        tohCanvas.addControlListener(this);
        
        // add the buttons: Start, Stop, Step, Reset
        startButton = new Button(shell, SWT.PUSH);
        startButton.setText("Run");
        GridData bdata = new GridData();
        startButton.setLayoutData(bdata);
        
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
        resetButton.setEnabled(true);
        bdata = new GridData();
        resetButton.setLayoutData(bdata);
        
        // setup button actions
        startButton.addSelectionListener(new SelectionAdapter() {
            public void widgetSelected(SelectionEvent e) {
            	startPressed() ;
            }
        });
        stepButton.addSelectionListener(new SelectionAdapter() {
            public void widgetSelected(SelectionEvent e) {
            	stepPressed() ;
            }
        });
        stopButton.addSelectionListener(new SelectionAdapter() {
            public void widgetSelected(SelectionEvent e) {
            	stopPressed() ;
            }
        });
        resetButton.addSelectionListener(new SelectionAdapter() {
            public void widgetSelected(SelectionEvent e) {
            	resetPressed() ;
            }
        });
        
        shell.setDefaultButton(startButton);
    }
    
    public void updateButtons(boolean running)
    {
    	boolean done = game.isAtGoalState() ;
    	
    	startButton.setEnabled(!running && !done) ;
    	stopButton.setEnabled(running) ;
    	resetButton.setEnabled(!running) ;
    	stepButton.setEnabled(!running && !done) ;
    }
    
    public void startPressed()
    {
    	updateButtons(true) ;
        
        runner = new ToHRunner();
        runner.start();
    }

    public void stepPressed()
    {
    	updateButtons(true) ;

    	game.step(); // runs one decision cycle
        
        updateButtons(false) ;
    }

    public void stopPressed()
    {
        game.stop();        
        updateButtons(false) ;
    }
    
    public void resetPressed()
    {
        game.reset();
        tohCanvas.redraw();
        
        stepButton.setEnabled(true);
        startButton.setEnabled(true);
        resetButton.setEnabled(false);    	
    }
    
    /**
     * Starts and runs the SWT event loop.
     */
    public void run() {
        shell.pack();
        shell.setSize(new Point(500, 300));
        shell.open();
        while (!shell.isDisposed()) {
            if (!dpy.readAndDispatch()) {
                dpy.sleep();
            }
        }
        game.detachSoar();
        dpy.dispose();
    }
    
    /**
     * This class runs the enclosing class' <code>Game</code> in a  separate thread.
     * It also provides a method to allow other threads to stop this thread in a
     * thread-safe manner. It exists to allow Soar decisions to be run continuously
     * without blocking the UI thread.
     * 
     * @author Trevor McCulloch, University of Michigan
     * @version 1.0
     */
    private class ToHRunner extends Thread {
        /**
         * Runs the enclosing class' <code>Game</code> until it is at the goal state
         * or until <code>halt</code> is called.
         */
        public void run() {
            game.run();

            // Update the buttons for the end of the run
            dpy.syncExec(new Runnable() { public void run() { updateButtons(false) ; } });            
        }
    }

    public void paintControl(PaintEvent e) {
        if (reinitBuf)
            reinitBuf();
        bufGC.setClipping(e.x, e.y, e.width, e.height);
        paintBuffer();
        e.gc.drawImage(bufImg,e.x,e.y,e.width,e.height,e.x,e.y,e.width,e.height);
    }

    public void diskMoved(Game g, Tower source, Tower destination) {
        int si = g.getTowers().indexOf(source);
        int di = g.getTowers().indexOf(destination);
        
        // calculate clipping regions on src and dst Towers
        int disk = destination.getTopDisk().getSize() - 1;
        
        Rectangle sr = new Rectangle(0, 0, diskWidth[disk] + 1, diskHeight + 1);
        Rectangle dr = new Rectangle(0, 0, diskWidth[disk] + 1, diskHeight + 1);
        
        // set x coords of clipping rects
        sr.x = towerX[si] + diskXOff[disk];
        dr.x = towerX[di] + diskXOff[disk];
        
        // set y coodrs of clipping rects
        int height = bufImg.getBounds().height;
        sr.y = height - 6 - (diskHeight * (source.getHeight() + 1));
        dr.y = height - 6 - (diskHeight * destination.getHeight());
        
        final Rectangle sRect = new Rectangle(sr.x, sr.y, sr.width, sr.height);
        final Rectangle dRect = new Rectangle(dr.x, dr.y, dr.width, dr.height);
        
        // issue redraws on sr and dr
        dpy.syncExec(new Runnable() {
            public void run() {
                tohCanvas.redraw(sRect.x, sRect.y, sRect.width, sRect.height, true);
                tohCanvas.redraw(dRect.x, dRect.y, dRect.width, dRect.height, true);
            }
        });
    }
    public void atGoalState(Game g) { /* DO NOTHING */}

    public void controlMoved(ControlEvent e) { /* DO NOTHING */ }
    public void controlResized(ControlEvent e) {
        reinitBuf = true;
        tohCanvas.redraw();
    }
    
    private void paintBuffer() {
        int width   = bufImg.getBounds().width;
        int height  = bufImg.getBounds().height;
        Color black = dpy.getSystemColor(SWT.COLOR_BLACK);
        Color white = dpy.getSystemColor(SWT.COLOR_WHITE);
        Color blue  = dpy.getSystemColor(SWT.COLOR_BLUE);
        Color maize = dpy.getSystemColor(SWT.COLOR_YELLOW);
        
        // (1) draw background rectangle
        bufGC.setBackground(white);
        bufGC.fillRectangle(0, 0, width, height);
        bufGC.setForeground(black);
        bufGC.drawRectangle(0, 0, width - 1, height - 1);
        // (2) draw pegs
        bufGC.setBackground(maize);
        for (int i = 0; i < towerX.length; ++i) {
            int x = towerX[i] + towerWidth / 2 - 3;
            bufGC.fillRectangle(x, 6, 6, towerHeight);
            bufGC.drawRectangle(x, 6, 6, towerHeight - 1);
        }
        // (3) draw each disk
        bufGC.setBackground(blue);
        List towers = game.getTowers();
        for (int i = 0; i < towerX.length; ++i) {
            Tower t = (Tower)towers.get(i);
            int y = height - 6 - (diskHeight * t.getHeight());
            Disk d = t.getTopDisk();
            while (d != null) {
                int j = d.getSize() - 1;
                int x = towerX[i] + diskXOff[j];
                bufGC.fillRectangle(x, y, diskWidth[j], diskHeight);
                bufGC.drawRectangle(x, y, diskWidth[j], diskHeight);
                y += diskHeight;
                d = d.getBelow();
            }
        }
    }
    
    private void calculateDrawingParameters() {
        int width = bufImg.getBounds().width;
        int height = bufImg.getBounds().height;
        
        // calculate drawing info:
        // (1) # of towers (calculate width of each tower, bounds)
        towerCount  = game.getTowers().size();
        towerWidth  = (width - 12 - (5 * (towerCount - 1))) / towerCount;
        towerHeight = height - 12;
        towerX = new int[towerCount];
        for (int i = 0, x = 6; i < towerCount; ++i, x += towerWidth + 5)
            towerX[i] = x;
        // (2) # of disks (calculate height of each disk, width based on tower width)
        diskCount  = game.getDiskCount();
        diskHeight = Math.min(towerHeight / diskCount, 20);
        diskWidth = new int[diskCount];
        for (int i = 0; i < diskCount; ++i)
            diskWidth[i] = 12 + (i * (towerWidth - 12)/(diskCount - 1));
        diskXOff = new int[diskCount];
        for (int i = 0; i < diskCount; ++i)
            diskXOff[i] = (diskWidth[diskCount - 1] - diskWidth[i]) / 2;
    }
    
    private void reinitBuf() {
        if (bufGC != null)
            bufGC.dispose();
        if (bufImg != null)
            bufImg.dispose();
        
        bufImg = new Image(dpy,
                tohCanvas.getBounds().width, tohCanvas.getBounds().height);
        bufGC = new GC(bufImg);
        
        calculateDrawingParameters();
        
        reinitBuf = false;
    }
}
