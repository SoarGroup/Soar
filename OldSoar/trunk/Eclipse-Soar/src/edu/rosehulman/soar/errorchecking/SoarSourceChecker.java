/**
 *
 * @file SoarSourceChecker.java
 * @date Jun 4, 2004
 */
package edu.rosehulman.soar.errorchecking;


import edu.rosehulman.soar.datamap.checker.*; 
import edu.umich.visualsoar.parser.*;

import org.eclipse.core.runtime.*;
import org.eclipse.core.resources.*;

import java.io.*;


/**
 * 
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class SoarSourceChecker {
	public static void checkSource(IFile file) {
		try {
			file.deleteMarkers(IMarker.PROBLEM, false, 0);
			
			SoarParser parser =
				new SoarParser (new InputStreamReader(file.getContents()));
			
			// Now see if it passes muster with the datamap
			DataMapChecker.matches(file, parser.VisualSoarFile());
			
			
		} catch (CoreException e) {
			e.printStackTrace();
			
		} catch (ParseException e) {
			
			//System.out.println("parsing error!");
			
			try {
				IMarker err = file.createMarker(IMarker.PROBLEM);
				err.setAttribute(IMarker.SEVERITY, IMarker.SEVERITY_ERROR);
				
				err.setAttribute(IMarker.LINE_NUMBER, e.currentToken.endLine);
				
				String msg = e.getMessage();
				
				//System.out.println(msg);
				
				if (msg.indexOf("\n") != -1) {
					msg = msg.substring(0, msg.indexOf("\n")-1);
				}
				
				err.setAttribute(IMarker.MESSAGE, msg);
				
			} catch (CoreException e2) {
				e2.printStackTrace();
			} // catch
				
		} // catch
		
	}

} // class
