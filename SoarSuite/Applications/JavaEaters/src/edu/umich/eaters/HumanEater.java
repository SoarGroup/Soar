/* File: HumanEater.java
 * Jul 13, 2004
 */

package edu.umich.eaters;

import edu.umich.JavaBaseEnvironment.SimulationControlListener;
import edu.umich.JavaBaseEnvironment.SoarAgent;
import edu.umich.JavaBaseEnvironment.Location;

import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.*;
import org.eclipse.swt.events.*;

/**
 * Each eater is an agent in the Eaters game. This class provides an interface to
 * the user and returns to the EaterControl the decisions the user makes regarding
 * movement, which EaterControl uses to update itself.
 * @author jduchi
 */
public class HumanEater extends Eater implements SimulationControlListener, SoarAgent{

	/**
	 * Simple nested class for a linked list of integers.
	 */
	private class intp{
		int data;
		intp next = null;
		
		private intp(int value){
			data = value;
		}
	}
	
	private Button jumpEastB;
	private Button moveEastB;
	private Button jumpWestB;
	private Button moveWestB;
	private Button jumpSouthB;
	private Button moveSouthB;
	private Button moveNorthB;
	private Button jumpNorthB;
	private Button noDecisionB;
	private Button destroyEaterB;
	
	public boolean movedThisCycle = false;
	
	private intp myMoveLog;
	private Display myDisplay;
	private Shell myShell;
	private Composite upperLeft;
	private Label myNotifier;
	/**
	 * Creates a user-controlled instance of eater, as well as the interface
	 * with which the user can control the eater.
	 * @param loc The location at which this instance of Eater has been created
	 * @param ec The <code>EaterControl</code> in which this <code>Eater</code> exists
	 * @param colorName The <code>String</code> name of the color of this <code>Eater</code>
	 */
	public HumanEater(Location loc, EaterControl ec, String colorName){
		super(loc, ec, colorName);
		myEC.addSimulationControlListener(this);
	}
	/**
	 * Creates a user-controlled instance of eater, as well as the interface
	 * with which the user can control the eater.
	 * @param loc The location at which this instance of Eater has been created
	 * @param ec The <code>EaterControl</code> in which this <code>Eater</code> exists
	 * @param colorName The <code>String</code> name of the color of this <code>Eater</code>
	 * @param disp The display this HumanEater will use to display controls to
	 * the user.
	 */
	public HumanEater(Location loc, EaterControl ec, String colorName, Display disp){
	    this(loc, ec, colorName);
	    initControls(disp);
	}
	
	/**
	 * Initializes the controls for the HumanEater.
	 * Sets up move and jump buttons for north, south, east, and west, and
	 * allows a destroy eater and no decision buttons.
	 * @param disp The display to be used by this HumanEater to show user things.
	 * Expected to be managed elsewehere.
	 */
	public void initControls(Display disp){
		myDisplay = disp;
		myShell = new Shell(myDisplay);
		GridLayout gl = new GridLayout(3, false);
		myShell.setLayout(gl);
		setButtons();
		myShell.setText(myColorName + " eater");
		myShell.addShellListener(new ShellAdapter(){
			public void shellClosed(ShellEvent e){
				if(myEC.isQuitting() || commandFromAbove){
					e.doit = true;
					return;
				}
				MessageBox mb = destroyMessageBox();
				if(mb.open() == SWT.OK){
					if(myEC.canDestroyAgent()){
						myEC.destroyAgent(HumanEater.this);
						myEC.removeSimulationControlListener(HumanEater.this);
						e.doit = true;
					} else {
						/* Eaters cannot be destroyed while an EaterControl is running */
						MessageBox mb2 = new MessageBox(myShell, SWT.OK);
						mb2.setText("Oops");
						String tempString;
						if(myEC.isRunning()){
							tempString = " running";
						} else {
							tempString = " in a step";
						}
						mb2.setMessage("Cannot destroy eater while " + tempString);
						mb2.open();
						e.doit = false;
					}
				} else {
					e.doit = false;
				}
			}
		});
		myShell.pack();
		myShell.open();
	}
	
	/**
	 * Creates a new message box to query the user if he actually wants to destroy the
	 * HumanEater.
	 * @return A new OK/Cancel message box.
	 */
	private MessageBox destroyMessageBox(){
		MessageBox mb = new MessageBox(myShell,
				SWT.OK | SWT.CANCEL);
		mb.setMessage("OK to kill human-controlled eater?");
		mb.setText("Destroy Eater");
		return mb;
	}
	
	private void setButtons(){
		upperLeft = new Composite(myShell, SWT.NONE);
		upperLeft.setLayout(new RowLayout(SWT.HORIZONTAL));
		myNotifier = new Label(upperLeft, SWT.NONE);
		myNotifier.setText("------------------");
		myNotifier.setToolTipText("Says, \'Your decision\' when it is your turn.");
		/* NORTH BUTTONS */
		Composite c2 = new Composite(myShell, SWT.NONE);
		c2.setLayout(new RowLayout(SWT.VERTICAL));
		jumpNorthB = new Button(c2, SWT.PUSH);
		jumpNorthB.setToolTipText("Jump eater north");
		jumpNorthB.setText("Jump N");
		setMoveListener(jumpNorthB, JumpNorth);
		moveNorthB = new Button(c2, SWT.PUSH);
		moveNorthB.setText("Move N");
		moveNorthB.setToolTipText("Move eater north");
		setMoveListener(moveNorthB, MoveNorth);
		c2.pack();
		Composite c3 = new Composite(myShell, SWT.NONE);
		
		/* WEST BUTTONS */
		Composite c4 = new Composite(myShell, SWT.NONE);
		c4.setLayout(new RowLayout(SWT.HORIZONTAL));
		jumpWestB = new Button(c4, SWT.PUSH);
		jumpWestB.setText("Jump W");
		jumpWestB.setToolTipText("Jump eater west");
		setMoveListener(jumpWestB, JumpWest);
		moveWestB = new Button(c4, SWT.PUSH);
		moveWestB.setText("Move W");
		moveWestB.setToolTipText("Move eater west");
		setMoveListener(moveWestB, MoveWest);
		c4.pack();
		
		/* CENTER BUTTON */
		Composite c5 = new Composite(myShell, SWT.NONE);
		c5.setLayout(new RowLayout());
		noDecisionB = new Button(c5, SWT.PUSH);
		noDecisionB.setText("Nothing");
		noDecisionB.setToolTipText("Eater makes no decision");
		setMoveListener(noDecisionB, NoDecision);
		c5.pack();
		
		/* EAST BUTTONS */
		Composite c6 = new Composite(myShell, SWT.NONE);
		c6.setLayout(new RowLayout(SWT.HORIZONTAL));
		moveEastB = new Button(c6, SWT.PUSH);
		moveEastB.setText("Move E");
		moveEastB.setToolTipText("Move eater east");
		setMoveListener(moveEastB, MoveEast);
		jumpEastB = new Button(c6, SWT.PUSH);
		jumpEastB.setText("Jump E");
		jumpEastB.setToolTipText("Jump eater east");
		setMoveListener(jumpEastB, JumpEast);
		c6.pack();
		Composite c7 = new Composite(myShell, SWT.NONE);
		
		/* SOUTH BUTTONS */
		Composite c8 = new Composite(myShell, SWT.NONE);
		c8.setLayout(new RowLayout(SWT.VERTICAL));
		moveSouthB = new Button(c8, SWT.PUSH);
		moveSouthB.setText("Move S");
		moveSouthB.setToolTipText("Move eater south");
		setMoveListener(moveSouthB, MoveSouth);
		jumpSouthB = new Button(c8, SWT.PUSH);
		jumpSouthB.setText("Jump S");
		jumpSouthB.setToolTipText("Jump eater south");
		setMoveListener(jumpSouthB, JumpSouth);
		c8.pack();
		
		/* Destruction button */
		Composite c9 = new Composite(myShell, SWT.NONE);
		c9.setLayout(new RowLayout());
		destroyEaterB = new Button(c9, SWT.PUSH);
		destroyEaterB.setText("Destroy eater");
		destroyEaterB.setToolTipText("Destroys the " + myColorName + " human-controlled eater.");
		destroyEaterB.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				myShell.close();
			}
		});
	}
	
	/**
	 * Takes a button and sets it to update this instance of HumanEater's movement
	 * log when the button is clicked.
	 * @param b The button to which we are adding a listener.
	 * @param decision The value of the decision that will be associated with this button.
	 */
	private void setMoveListener(Button b, final int decision)
	{
		b.addSelectionListener(new SelectionAdapter(){
			public void widgetSelected(SelectionEvent e){
				if (!myEC.isRunning())
					new Thread()
					{
						public void run()
						{
							myEC.singleStep();
						}
					}.start();
				if(myMoveLog == null){
					myMoveLog = new intp(decision);
				} else {
					intp temp = new intp(decision);
					temp.next = myMoveLog;
					myMoveLog = temp;
				}
				
				movedThisCycle = true;
				checkForEndOfCycle();
				if (movedThisCycle)
					enableMoves(false);
			}
		});
	}
	
	
	/**
	 * Sets all the moving buttons to enabled status specified.
	 * @param able True to enable the buttons, false to disable.
	 */
	private void enableMoves(final boolean able){
	    if(myDisplay.isDisposed()) return;
	    myDisplay.asyncExec(new Runnable(){
	        public void run(){
	        	if(myShell.isDisposed()) return;
	    	    jumpNorthB.setEnabled(able);
	    	    jumpSouthB.setEnabled(able);
	    	    jumpWestB.setEnabled(able);
	    	    jumpEastB.setEnabled(able);
	    	    moveNorthB.setEnabled(able);
	    	    moveSouthB.setEnabled(able);
	    	    moveEastB.setEnabled(able);
	    	    moveWestB.setEnabled(able);
	    	    noDecisionB.setEnabled(able);
	        }
	    });
	}
	
	protected EaterOutputInfo getDecision(EaterInputInfo sensors){
		EaterOutputInfo result = new EaterOutputInfo();
	    boolean labeled = false;
		while(myMoveLog == null && !myEC.soarShouldStop){
			if(!labeled && myNotifier != null){
				myDisplay.asyncExec(new Runnable(){
					public void run(){
					    if(!myNotifier.getText().equals("Your decision")){
							myNotifier.setText("Your decision");
					    }
					}
				});
				labeled = true;
			}
			if (myEC.soarShouldStop)
				return (result);
			if(!myEC.isRunning() && !myEC.isStepping()){
			    return(result);
			}
		}
		if(myDisplay.isDisposed()) return (result);
		myDisplay.syncExec(new Runnable(){
			public void run(){
				myNotifier.setText("------------------");
			}
		});
		int decision = myMoveLog.data;
		myMoveLog = myMoveLog.next;
		myRecentDecision = decision;
		return (new EaterOutputInfo(decision));
	}
	
	/**
	 * Boolean to indicate whether the EaterControl called this method on us.
	 */
	private boolean commandFromAbove = false;
	
	public void destroyEater(){
		myDisplay.asyncExec(new Runnable(){
			public void run(){
			    commandFromAbove = true;
			    if(!myShell.isDisposed()){
			        myShell.close();
			    }
			}
		});
		myEC.removeSimulationControlListener(this);
	}
	
	public void locationChanged(Location loc) { /* IGNORED */ }
	
	public void simStarted() {
		if(myDisplay.isDisposed()) return;
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if(destroyEaterB.isDisposed()) return;
				destroyEaterB.setEnabled(false);
			}
		});
	}

	
	public void simEnded(String message){
		myDisplay.asyncExec(new Runnable(){
			public void run(){
				if(myEC.hasQuit() || destroyEaterB.isDisposed()) return;
				destroyEaterB.setEnabled(true);
			}
		});
		if(myMoveLog == null){
			myMoveLog = new intp(NoDecision);
		}
	}
	
	public void agentCreated(SoarAgent created) { /* IGNORED */ }
	
	public void simQuit(){
		/* IGNORED because someone else will kill myDisplay, so this gets disposed. */
	}
	
	public void agentDestroyed(SoarAgent destroyed){
		/* IGNORED:
		 * Since destroyEater is invoked by EaterControl, this needs to do
		 * nothing.
		 */
	}
	
	public void worldCountChanged(int worldCount) { /* IGNORED */ }
	
	public void newMap(String message){ /* IGNORED */ }
	
	private void checkForEndOfCycle()
	{
		boolean doneMoving = true;
		for (int i = 0;i < myEC.getAllEaters().length && doneMoving;i++)
		{
			if (myEC.getAllEaters()[i] instanceof HumanEater && !((HumanEater)myEC.getAllEaters()[i]).movedThisCycle)
				doneMoving = false;
		}
		
		if (doneMoving)
		{
			for (int i = 0;i < myEC.getAllEaters().length;i++)
			{
				if (myEC.getAllEaters()[i] instanceof HumanEater)
				{
					((HumanEater)myEC.getAllEaters()[i]).movedThisCycle = false;
					((HumanEater)myEC.getAllEaters()[i]).enableMoves(true);
				}				
			}		
		}
	}

}
