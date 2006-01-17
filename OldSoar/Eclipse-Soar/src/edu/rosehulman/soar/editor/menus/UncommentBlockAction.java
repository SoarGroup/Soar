/*
 * Created on Oct 29, 2003
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
 * Menu item to uncomment a block of text.
 * 
 * @author Tim Jasko
 *
 */
public class UncommentBlockAction implements IEditorActionDelegate {
	private AbstractTextEditor editor;
	

	public void setActiveEditor(IAction action, IEditorPart targetEditor) {
		editor = (AbstractTextEditor) targetEditor;
	} // void setActiveEditor(IAction action, IEditorPart targetEditor)



	public void run(IAction action) {
		IDocument doc = editor.getDocumentProvider().getDocument(
		  editor.getEditorInput());
		
		ITextSelection ts = (ITextSelection) editor.getSelectionProvider().getSelection();
		
		int endLine = ts.getEndLine();
		
		for (int i = ts.getStartLine(); i<= endLine; i++) {
			try {
				int offset = doc.getLineOffset(i);
				if (doc.getChar(offset) == '#') {
					doc.replace(offset, 1, "");
				}
			} catch (BadLocationException e) {
				e.printStackTrace();
			} // catch
		} // for i
		
	} // void run (IAction action)



	public void selectionChanged(IAction action, ISelection selection) {

	} // void selectionChanged(IAction action, ISelection selection)

} // class UncommentBlockAction
