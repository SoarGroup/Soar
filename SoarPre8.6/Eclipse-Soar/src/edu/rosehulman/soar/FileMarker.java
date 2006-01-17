package edu.rosehulman.soar;

import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.QualifiedName;

/**
 * Marks up soar files, enabling us to tell if they are a file, operator, or impasse.
 * 
 * @author Tim Jasko
 */
public class FileMarker {

	public static final QualifiedName SOAR_TYPE
		= new QualifiedName("edu.rosehulman.soar", "SoarType");

	/**
	 * Gets the type of soar resource this is.
	 * @param res
	 * @return "file" "operator" or "impasse". Or maybe ""
	 */
	public static String getSoarType(IResource res) {
		try {
			String ret = res.getPersistentProperty(SOAR_TYPE);
			if (ret == null) {
				return "";
			} else {
				return ret;
			}
		} catch (CoreException e) {
			return "";
		}
	}
	
	/**
	 * Marks the resource as belonging to Soar.
	 *  Mark is the type of soar resource, ie: file, operator, impasse.
	 * @param res The resource to mark.
	 * @param mark What to mark this resource as ("file", "operator", "impasse").
	 */
	public static void markResource(IResource res, String mark) {
		
		try {
			/*IMarker marker = res.createMarker(
				"edu.rosehulman.soar.natures.SoarFile");
				
			marker.setAttribute("type", "file"); */
			
			res.setPersistentProperty(SOAR_TYPE, mark);
		} catch (CoreException e) {
			e.printStackTrace();
		} // catch 
	}
}
