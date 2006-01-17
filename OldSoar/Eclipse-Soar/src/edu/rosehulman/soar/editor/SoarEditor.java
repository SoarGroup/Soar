/**
 * A class defining the main editor plug-in for the Soar Eclipse Project
 * @file SoarEditor.java
 * @date December 11, 2003
 */
package edu.rosehulman.soar.editor;

//import java.util.ResourceBundle;


import org.eclipse.ui.part.*;
import org.eclipse.ui.views.contentoutline.*;
import org.eclipse.ui.texteditor.*;
import org.eclipse.ui.editors.text.*;
import org.eclipse.jface.text.*;
import org.eclipse.jface.text.rules.*;
import org.eclipse.core.runtime.IProgressMonitor;

import edu.rosehulman.soar.*;
import edu.rosehulman.soar.editor.outline.*;
import edu.rosehulman.soar.editor.soar.*;
import edu.rosehulman.soar.errorchecking.*;


/**
 * Defines the main editor plug-in for the Soar Eclipse Project
 * @author lutezp
 * @author Tim Jasko
 */
public class SoarEditor extends TextEditor
{
	private SoarContentOutlinePage _outlinePage;
	
	//private static SoarCodeScanner _soarCodeScanner;
	private static ITokenScanner _soarCodeScanner;

	/**
	 * Instantiates the Soar Editor
	 */
	public SoarEditor()
	{
		super();
		setDocumentProvider(new SoarEditorDocumentProvider());
		setSourceViewerConfiguration(new SoarEditorSourceViewerConfiguration(this));
		_soarCodeScanner = new SoarCodeScanner();
		//_soarCodeScanner = new SoarRulesScanner();
		setRangeIndicator(new DefaultRangeIndicator());
	}
	
	/**
	 * Disposes of the Soar Editor and associated color provider
	 */
	public void disposeColorProvider()
	{
		SoarPlugin.getDefault().disposeColorProvider();
		super.dispose();
	}

	/**
	 * Installs the editor actions
	 *
	 * @see org.eclipse.ui.texteditor.AbstractTextEditor#createActions()
	 */
	protected void createActions()
	{
		super.createActions();
		
		//ResourceBundle bundle =
		//	SoarPlugin.getDefault().getResourceBundle();
/*
		setAction(
			"ContentFormatProposal",
			new TextOperationAction(
				bundle,
				"ContentFormatProposal.",
				this,
				ISourceViewer.FORMAT));

		setAction(
			"ContentAssistProposal",
			new TextOperationAction(
				bundle,
				"ContentAssistProposal.",
				this,
				ISourceViewer.CONTENTASSIST_PROPOSALS));

		setAction(
			"ContentAssistTip",
			new TextOperationAction(
				bundle,
				"ContentAssistTip.",
				this,
				ISourceViewer.CONTENTASSIST_CONTEXT_INFORMATION));
*/	}

	/**
	 * Gets a Soar Code Scanner.
	 * @return SoarCodeScanner
	 */
	/*public static SoarCodeScanner getSoarCodeScanner()
	{
		return _soarCodeScanner;
	}*/
	public static ITokenScanner getSoarCodeScanner()
	{
		return _soarCodeScanner;
	}
	
	public void setFocus() {
		/*SoarContentAssistProcessor sacp =
			(SoarContentAssistProcessor)
			getSourceViewerConfiguration().getContentAssistant(null).
			getContentAssistProcessor(IDocument.DEFAULT_CONTENT_TYPE); */
		
	}
	
	
	public void doSave(IProgressMonitor monitor) {
		super.doSave(monitor);
		
		SoarSourceChecker.checkSource(
			((FileEditorInput) this.getEditorInput()).getFile() );
		
		_outlinePage.update();
		
		this.getVerticalRuler().update();
	}
	
	
	public Object getAdapter(Class adapter) {
		if (adapter.equals(IContentOutlinePage.class)) {
			
			_outlinePage = new SoarContentOutlinePage( this,
				((FileEditorInput) this.getEditorInput()).getFile());
			return _outlinePage;
			
		} else {
			return super.getAdapter(adapter);
		}
	}
	
	
	public void gotoLine(int line) {
		System.out.println("Line:" + line);
		IDocument doc = this.getSourceViewer().getDocument();
		try {
			this.selectAndReveal( doc.getLineOffset(line), doc.getLineLength(line) );
		} catch (BadLocationException e) {
		}
	}
}
