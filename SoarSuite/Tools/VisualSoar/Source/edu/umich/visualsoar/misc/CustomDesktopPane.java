package edu.umich.visualsoar.misc;
import edu.umich.visualsoar.graph.*;
import edu.umich.visualsoar.datamap.*;
import javax.swing.JDesktopPane;
import javax.swing.JInternalFrame;
import java.util.*;
import javax.swing.Action;
import javax.swing.AbstractAction;
import java.awt.event.ActionEvent;
import java.awt.Point;

/**
 * This is the new editor pane so we can add functionality
 * of moving windows around like cascading and the like
 * based up Graphic Java 2, Volume II pg 901
 * @author Brad Jones Jon Bauman
 * @version 0.5a 6/29/1999
 */
public class CustomDesktopPane extends JDesktopPane {
    CascadeAction 	cascadeAction = new CascadeAction();
    TileAction 		tileAction    = new TileAction();
    ReTileAction 	reTileAction  = new ReTileAction();
    Preferences 	prefs         = Preferences.getInstance();
    private int 	xoffset       = 20;
    private int         yoffset       = 20;
    private int	        w             = 250;
    private int		h             = 350;

    // Added dataMaps member to keep track of which dataMaps are currently
    // open within the desktop.
    private Hashtable dataMaps = new Hashtable();


	/**
	 * This will cascade all the windows currently in the 
	 * desktop pane
	 */
	class CascadeAction extends PerformableAction {
	    public CascadeAction() {
		super("Cascade");
	    }

	    public void perform() {
		JInternalFrame[] frames = getAllFrames();
		int x = 0, y = 0;
		
		for(int i = 0; i < frames.length; ++i) {
		    if( ! frames[i].isIcon()) {
			try {
			    frames[i].setMaximum(false);
			}
			catch(java.beans.PropertyVetoException pve) {}
			frames[i].setBounds(x,y,w,h);
			x += xoffset;
			y += yoffset;
		    }
		}
	    }
		
	    public void actionPerformed(ActionEvent e) {
		perform();
	    }
	}

    public void performTileAction() {
	tileAction.perform();
    }

    public void performReTileAction() {
	reTileAction.perform();
    }

    public void performCascadeAction() {
	cascadeAction.perform();
    }
		
			
	public void horizontalTile() {
		int 				iconizedHeight = 0;
		int 				paneHeight = getHeight();
		int					paneWidth = getWidth();
		int 				numCols;
		int 				numRows;
		
		JInternalFrame[] 	allFrames = getAllFrames();
		
		
		if (allFrames == null || allFrames.length == 0) {
			return;
		}
		
		Vector frames = new Vector();
		int numFrames = 0;	

		
		for (int i = allFrames.length - 1; i > -1; i--) {
			if (!allFrames[i].isIcon()) {
				frames.add(allFrames[i]);
				numFrames++;
			}
			else {
				iconizedHeight = 30;
			}
		}
			
		if(numFrames == 0) {
			return;
		}
		
		switch (numFrames) {
			case 1:
				numCols = 1;
				numRows = 1;
				break;
			case 2:
				if (paneHeight > paneWidth) {
					numCols = 1;
					numRows = 2;
				}
				else {
					numCols = 2;
					numRows = 1;
				}
				break;
			case 3:
			case 4:
				numCols = 2;
				numRows = 2;
				break;
			default: // > 4
				numCols = 2;
				numRows = (int)Math.ceil(numFrames /  2.0);
				break;
		}

		int colWidth = paneWidth / numCols;
		int rowHeight = (paneHeight - iconizedHeight) / numRows;
		
		Vector ULPoints = new Vector(); // all possible upper, left-hand points
		int x = 0, y = 0;
		for (int col = 0; col < numCols; col++) {
			y = 0;
			for (int row = 0; row < numRows; row++) {
				ULPoints.add(new Point(x,y));
				y += rowHeight;
			}
			x += colWidth;
		}		

		CustomInternalFrame currentFrame;	
		Enumeration framesEnum = frames.elements(), ulpe;
		Point framePoint, ULPoint, bestMatch;
		int shortest = 0, current;
		int shortestPointIndex = 0;
		while (framesEnum.hasMoreElements()) { // find the closest match for each frame
			currentFrame = (CustomInternalFrame)framesEnum.nextElement();
			framePoint = currentFrame.getLocation();
			
			ulpe = ULPoints.elements();
			if (ulpe.hasMoreElements()) { // initialize shortest dist to 1st point
				ULPoint = (Point)ulpe.nextElement();
				shortest = (int)framePoint.distance(ULPoint);
				shortestPointIndex = 0;
			} 
			while (ulpe.hasMoreElements()) {
				ULPoint = (Point)ulpe.nextElement();
			
				current = (int)framePoint.distance(ULPoint);
				if (current < shortest) {
					shortest = current;
					shortestPointIndex = ULPoints.indexOf(ULPoint);
				}
			}
			
			try {
				currentFrame.setMaximum(false);
			}
			catch(java.beans.PropertyVetoException pve) {
				pve.printStackTrace();
			}
			ULPoint = (Point)ULPoints.elementAt(shortestPointIndex);
			currentFrame.setBounds((int)ULPoint.getX(), (int)ULPoint.getY(), colWidth, rowHeight);
			
			ULPoints.remove(shortestPointIndex);
		}
		
		if (! ULPoints.isEmpty()) { // cover wasted space
			int wastedY = (int)((Point)ULPoints.elementAt(0)).getY();
			
			framesEnum = frames.elements();
			while (framesEnum.hasMoreElements()) {
				currentFrame = (CustomInternalFrame)framesEnum.nextElement();
				if (currentFrame.getLocation().getY() == wastedY) {
					currentFrame.setBounds(0, wastedY, 2 * colWidth, rowHeight);
				}
			}
		}
	} // horizontalTile

	public void verticalTile() {
		int 				iconizedHeight = 0;
		int 				paneHeight = getHeight();
		int					paneWidth = getWidth();
		int 				numCols;
		int 				numRows;
		
		JInternalFrame[] 	allFrames = getAllFrames();
		
		
		if (allFrames == null || allFrames.length == 0) {
			return;
		}
		
		Vector frames = new Vector();
		int numFrames = 0;	

		
		for (int i = allFrames.length - 1; i > -1; i--) {
			if (!allFrames[i].isIcon()) {
				frames.add(allFrames[i]);
				numFrames++;
			}
			else {
				iconizedHeight = 30;
			}
		}
			
		if(numFrames == 0) {
			return;
		}
		
		switch (numFrames) {
			case 1:
				numCols = 1;
				numRows = 1;
				break;
			case 2:
				if (paneHeight < paneWidth) {
					numCols = 1;
					numRows = 2;
				}
				else {
					numCols = 2;
					numRows = 1;
				}
				break;
			case 3:
			case 4:
				numCols = 2;
				numRows = 2;
				break;
			default: // > 4
				numRows = 2;
				numCols = (int)Math.ceil(numFrames /  2.0);
				break;
		}

		int colWidth = paneWidth / numCols;
		int rowHeight = (paneHeight - iconizedHeight) / numRows;
		
		Vector ULPoints = new Vector(); // all possible upper, left-hand points
		int x = 0, y = 0;
		for (int col = 0; col < numCols; col++) {
			y = 0;
			for (int row = 0; row < numRows; row++) {
				ULPoints.add(new Point(x,y));
				y += rowHeight;
			}
			x += colWidth;
		}		

		CustomInternalFrame currentFrame;	
		Enumeration framesEnum = frames.elements(), ulpe;
		Point framePoint, ULPoint, bestMatch;
		int shortest = 0, current;
		int shortestPointIndex = 0;
		while (framesEnum.hasMoreElements()) { // find the closest match for each frame
			currentFrame = (CustomInternalFrame)framesEnum.nextElement();
			framePoint = currentFrame.getLocation();
			
			ulpe = ULPoints.elements();
			if (ulpe.hasMoreElements()) { // initialize shortest dist to 1st point
				ULPoint = (Point)ulpe.nextElement();
				shortest = (int)framePoint.distance(ULPoint);
				shortestPointIndex = 0;
			} 
			while (ulpe.hasMoreElements()) {
				ULPoint = (Point)ulpe.nextElement();
			
				current = (int)framePoint.distance(ULPoint);
				if (current < shortest) {
					shortest = current;
					shortestPointIndex = ULPoints.indexOf(ULPoint);
				}
			}
			
			try {
				currentFrame.setMaximum(false);
			}
			catch(java.beans.PropertyVetoException pve) {
				pve.printStackTrace();
			}
			ULPoint = (Point)ULPoints.elementAt(shortestPointIndex);
			currentFrame.setBounds((int)ULPoint.getX(), (int)ULPoint.getY(), colWidth, rowHeight);
			
			ULPoints.remove(shortestPointIndex);
		}
		
		if (! ULPoints.isEmpty()) { // cover wasted space
			int wastedX = (int)((Point)ULPoints.elementAt(0)).getX();
			
			framesEnum = frames.elements();
			while (framesEnum.hasMoreElements()) {
				currentFrame = (CustomInternalFrame)framesEnum.nextElement();
				if (currentFrame.getLocation().getX() == wastedX) {
					currentFrame.setBounds(wastedX, 0, colWidth, 2 * rowHeight);
				}
			}
		}
	} // verticalTile

    /**
     *  Checks to see if a datamap is already open within the desktop
     *  @param id the id that denotes the correct datamap.
     */
    public DataMap dmGetDataMap(int id) {
        return (DataMap) dataMaps.get( new Integer(id) );
    }

    /**
     *  Checks to see if a datamap is already open within the desktop
     *  @param dm the datamap to look for
     */
    public boolean hasDataMap(DataMap dm)
    {
        //Make sure this datamap isn't already there
        JInternalFrame[] jif = getAllFrames();
        for(int i = 0; i < jif.length; ++i) 
        {
            if ( (jif[i] instanceof DataMap)
                 && (((DataMap)jif[i]).getId() == dm.getId()) )
            {
                return true;
            }
        }

        return false;
    }
    
  /**
   *  Adds a dataMap to the hashtable of datamaps that are open within the desktop
   *  @param id the id that denotes the correct datamap
   *  @param dm the datamap that is being stored in the hashtable
   */
    public void dmAddDataMap(int id, DataMap value)
    {
        if (hasDataMap(value)) return;
        
        //Try to add the new datamap
        try
        {
            dataMaps.put(new Integer(id), value);
        }
        catch (NullPointerException npe) {
            System.err.println("error-key or value was null");
        }
    }

  /**
   *  removes a datamap from the hashtable that holds all open datamaps within the desktop
   *  @param id the id that denotes the datamap to be removed
   */
  public void dmRemove(int id) {
    dataMaps.remove(new Integer(id));
  }

    /**
	 * Create a version of the getAllFrames() method to return an array of 
	 * CustomInternalFrame.
	 */
    public CustomInternalFrame[] getAllCustomFrames()
    {
        JInternalFrame[] allFrames = getAllFrames();
        CustomInternalFrame[] allCustomFrames = new CustomInternalFrame[allFrames.length];
        for(int i = 0; i < allFrames.length; i++)
        {
            allCustomFrames[i] = (CustomInternalFrame)allFrames[i];
        }
        
        return allCustomFrames;
    }

	/**
	 * This will tile all the windows in the most fantastically efficient manner 
	 * imaginable (as far as desktop space in concerned of course; this may not be the
	 * most efficient imaginable as far as excecution in concerned). It will skip
	 * windows which are iconized.
	 */	
	class TileAction extends PerformableAction {
	    public TileAction() {
		super("Tile");
	    }		
	
	    public void perform() {
			if (prefs.isHorizontalTilingEnabled()) {
			    horizontalTile();
			}
			else {
			    verticalTile();
			}
	    }

	    public void actionPerformed(ActionEvent e) {
			perform();
	    }
	}
	
	class ReTileAction extends AbstractAction {
	    public ReTileAction() {
			super("ReTile");
	    }		
	
	    public void perform() {	
			if (prefs.isHorizontalTilingEnabled()) {
			    verticalTile();
			}
			else {
			    horizontalTile();
			}
	    }
		
	    public void actionPerformed(ActionEvent e) {
			perform();
	    }
	}

}
