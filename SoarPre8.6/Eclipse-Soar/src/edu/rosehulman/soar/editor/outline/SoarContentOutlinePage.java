/**
 *
 * @file SoarContentOutlinePage.java
 * @date Jun 9, 2004
 */
package edu.rosehulman.soar.editor.outline;

import edu.rosehulman.soar.*;
import edu.rosehulman.soar.editor.*;
import edu.umich.visualsoar.parser.*;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.core.runtime.*;
import org.eclipse.core.resources.*;
import org.eclipse.jface.viewers.*;
import org.eclipse.ui.views.contentoutline.*;

import java.util.*;
import java.io.*;

/**
 * 
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class SoarContentOutlinePage extends ContentOutlinePage {
	private IFile _file;
	private SoarEditor _editor;
	
	
	public SoarContentOutlinePage(SoarEditor editor, IFile file) {
		_editor = editor;
		_file = file;
	}
	
	public void createControl(Composite parent) {
		super.createControl(parent);
		
		
		fillTree();
	}
	
	/**
	 * Parses the file and creates an outline.
	 *  If an error is encountered in parsing, the outline remains unaltered.
	 *
	 */
	private void fillTree() {
		getControl().setRedraw(false);
		
		try {
			SoarParser parser =
				new SoarParser (new InputStreamReader(_file.getContents()));
				
			
			Vector productions = parser.VisualSoarFile();
			
			
			Tree tree = getTreeViewer().getTree();
			ArrayList namesSorter = new ArrayList();
			
			
			for (int i=0; i<productions.size(); ++i) {
				SoarProduction sp = (SoarProduction) productions.get(i);
				
				namesSorter.add(
					new ProductionWrapper(sp.getName(), sp.getStartLine()-1) );
				
			}
			
			sort(namesSorter); // sort the list
			
			ProductionWrapper[] names;
			names = new ProductionWrapper[namesSorter.size()];
			
			namesSorter.toArray(names);
			
			tree.removeAll();
			
			for (int i=0; i<names.length; ++i) {
				TreeItem temp = new TreeItem(tree, SWT.NONE);
				temp.setText( names[i].getName() );
				temp.setData( names[i] );
				temp.setImage( SoarImages.getImage(SoarImages.IMAGE_SP) );
				
			}
			
			
			
		} catch (CoreException e) {
			e.printStackTrace();
		} catch (ParseException e) {
		}
		
		getControl().setRedraw(true);
	} // void fillTree()
	
	/**
	 * Sorts a list using the much-maligned bubble sort.
	 * Our list shouldn't be long enough for this to be an issue.
	 * @param lst The List to be sorted.
	 */
	private void sort(java.util.List lst) {
		for (int i1=0; i1<lst.size(); ++i1) {
			for (int i2=i1; i2<lst.size(); ++i2) {
				Comparable item1 = (Comparable) lst.get(i1);
				Comparable item2 = (Comparable) lst.get(i2);
					
				if (item1.compareTo(item2) > 0) {
					Comparable temp = item1; // this may not be necessary

					lst.set(i1, item2);
					lst.set(i2, temp);
				}
			} // for i2
		} // for i1
	} // void sort(List ls)
	
	
	/**
	 * Updates the content view to reflect changes in the file.
	 *
	 */
	public void update() {
		fillTree();
	} // void update()
	
	
	public void selectionChanged(SelectionChangedEvent event) {
		IStructuredSelection sel = (IStructuredSelection) event.getSelection();
		
		if (! sel.isEmpty()) {
			ProductionWrapper pw = (ProductionWrapper) sel.getFirstElement();
			
			_editor.gotoLine(pw.getLine());
		}
	} // void selectionChanged( ... )
}
