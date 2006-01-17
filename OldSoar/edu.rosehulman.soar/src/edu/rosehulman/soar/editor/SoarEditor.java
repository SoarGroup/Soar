/**
 * A class defining the main editor plug-in for the Soar Eclipse Project
 * @file SoarEditor.java
 * @date December 11, 2003
 */
package edu.rosehulman.soar.editor;

//import java.util.ResourceBundle;


import org.eclipse.ui.texteditor.AbstractTextEditor;
import org.eclipse.ui.texteditor.DefaultRangeIndicator;
import org.eclipse.jface.text.IDocument;

import edu.rosehulman.soar.*;
import edu.rosehulman.soar.editor.soar.SoarCodeScanner;
import edu.rosehulman.soar.editor.autocomplete.*;

/**
 * Defines the main editor plug-in for the Soar Eclipse Project
 * @author lutezp
 * @author Tim Jasko
 */
public class SoarEditor extends AbstractTextEditor
{
	private static SoarCodeScanner _soarCodeScanner;

	/**
	 * Instantiates the Soar Editor
	 */
	public SoarEditor()
	{
		super();
		setDocumentProvider(new SoarEditorDocumentProvider());
		setSourceViewerConfiguration(new SoarEditorSourceViewerConfiguration(this));
		_soarCodeScanner = new SoarCodeScanner();
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
	public static SoarCodeScanner getSoarCodeScanner()
	{
		return _soarCodeScanner;
	}
	
	public void setFocus() {
		SoarContentAssistProcessor sacp =
			(SoarContentAssistProcessor)
			getSourceViewerConfiguration().getContentAssistant(null).
			getContentAssistProcessor(IDocument.DEFAULT_CONTENT_TYPE);
		
		sacp.refreshDatamap();
	}
	
}
