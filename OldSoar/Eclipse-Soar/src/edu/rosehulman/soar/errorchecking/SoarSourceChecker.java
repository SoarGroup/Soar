/**
 *
 * @file SoarSourceChecker.java
 * @date Jun 4, 2004
 */
package edu.rosehulman.soar.errorchecking;


import edu.rosehulman.soar.datamap.checker.*; 
import edu.rosehulman.soar.navigator.SoarErrorDecorator;
import edu.umich.visualsoar.parser.*;

import org.eclipse.core.runtime.*;
import org.eclipse.core.resources.*;
import org.eclipse.ui.texteditor.*;

import java.io.*;
import java.util.*;


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
				HashMap attr = new HashMap();
				MarkerUtilities.setLineNumber(attr, e.currentToken.endLine);
				attr.put(IMarker.SEVERITY, new Integer(IMarker.SEVERITY_ERROR));
				
				
				String msg = e.getMessage();
				
				if (msg.indexOf("\n") != -1) {
					msg = msg.substring(0, msg.indexOf("\n")-1);
				}
				MarkerUtilities.setMessage(attr, msg);
				
				MarkerUtilities.createMarker(file, attr, IMarker.PROBLEM);
				
				
			} catch (CoreException e2) {
				e2.printStackTrace();
			} // catch
				
		} // catch
		
		SoarErrorDecorator dec = SoarErrorDecorator.getErrorDecorator();
		if (dec != null) {
			dec.refresh(file);
		}
		
	}
	
} // class
