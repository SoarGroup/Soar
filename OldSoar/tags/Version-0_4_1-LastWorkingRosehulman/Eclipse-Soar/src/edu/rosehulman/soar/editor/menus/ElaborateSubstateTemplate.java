/*
 * Created on Oct 25, 2003
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.editor.menus;

import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.text.*;
import org.eclipse.ui.*;
import org.eclipse.ui.texteditor.AbstractTextEditor;

/**
 * Menu item for Elaborate Substate template. 
 * 
 * @author Tim Jasko
 *
 */
public class ElaborateSubstateTemplate implements IEditorActionDelegate {
	private AbstractTextEditor editor;
	/* (non-Javadoc)
	 * @see org.eclipse.ui.IEditorActionDelegate#setActiveEditor(org.eclipse.jface.action.IAction, org.eclipse.ui.IEditorPart)
	 */
	public void setActiveEditor(IAction action, IEditorPart targetEditor) {
		editor = (AbstractTextEditor) targetEditor;
	} // void setActiveEditor(IAction action, IEditorPart targetEditor)

	/* (non-Javadoc)
	 * @see org.eclipse.ui.IActionDelegate#run(org.eclipse.jface.action.IAction)
	 */
	public void run(IAction action) {
		IDocument doc = editor.getDocumentProvider().getDocument(
		  editor.getEditorInput());
		
		ITextSelection ts = (ITextSelection)
		  editor.getSelectionProvider().getSelection();
		
		try {
			doc.replace(ts.getOffset(),0,
			  Utility.getFileTemplate(
			    ((IFileEditorInput)editor.getEditorInput()).getFile(),
			    "elaborate-substate.soar"));
		} catch (BadLocationException e){
		} // catch
		
		
	} // void run (IAction action)

	/* (non-Javadoc)
	 * @see org.eclipse.ui.IActionDelegate#selectionChanged(org.eclipse.jface.action.IAction, org.eclipse.jface.viewers.ISelection)
	 */
	public void selectionChanged(IAction action, ISelection selection) {

	} // void selectionChanged(IAction action, ISelection selection)

}
