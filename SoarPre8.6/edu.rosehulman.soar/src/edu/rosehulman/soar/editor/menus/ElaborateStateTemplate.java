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
 * Menu item for the Elaborate State template.
 * 
 * @author Tim Jasko
 *
 */
public class ElaborateStateTemplate implements IEditorActionDelegate {
	private AbstractTextEditor editor;


	public void setActiveEditor(IAction action, IEditorPart targetEditor) {
		editor = (AbstractTextEditor) targetEditor;
	} // void setActiveEditor(IAction action, IEditorPart targetEditor)


	public void run(IAction action) {
		IDocument doc = editor.getDocumentProvider().getDocument(
		  editor.getEditorInput());
		
		ITextSelection ts = (ITextSelection)
		  editor.getSelectionProvider().getSelection();
		
		try {
			doc.replace(ts.getOffset(),0,
			  Utility.getFileTemplate(
			    ((IFileEditorInput)editor.getEditorInput()).getFile(),
			    "elaborate-state.soar"));
		} catch (BadLocationException e){
		} // catch
		
		
	} // void run (IAction action)


	public void selectionChanged(IAction action, ISelection selection) {

	} // void selectionChanged(IAction action, ISelection selection)

}
