/**
 * Handles the creation and manipulation of the source viewer options.
 * @file SoarEditorSourceViewerConfiguration.java
 * @date December 11, 2003
 */
package edu.rosehulman.soar.editor;

import org.eclipse.jface.text.*;
import org.eclipse.jface.text.TextAttribute;
import org.eclipse.jface.text.formatter.ContentFormatter;
import org.eclipse.jface.text.formatter.IContentFormatter;
import org.eclipse.jface.text.formatter.IFormattingStrategy;
import org.eclipse.jface.text.presentation.IPresentationReconciler;
import org.eclipse.jface.text.presentation.PresentationReconciler;
import org.eclipse.jface.text.rules.*;
import org.eclipse.jface.text.rules.Token;
import org.eclipse.jface.text.source.ISourceViewer;
import org.eclipse.jface.text.source.SourceViewerConfiguration;
import org.eclipse.jface.text.contentassist.*;

import edu.rosehulman.soar.*;
import edu.rosehulman.soar.editor.soar.*;
import edu.rosehulman.soar.editor.soar.SoarPartitionScanner;
import edu.rosehulman.soar.editor.util.ColorProvider;
import edu.rosehulman.soar.editor.autocomplete.*;


public class SoarEditorSourceViewerConfiguration
	extends SourceViewerConfiguration
{
	private SoarEditor _editor;
	private ContentAssistant _contentAssistant;
	
	
	public SoarEditorSourceViewerConfiguration(SoarEditor editor) {
		super();
		_editor = editor;
		
		
		ContentAssistant ca = new ContentAssistant();
		ca.setContentAssistProcessor(new SoarContentAssistProcessor(_editor), 
			IDocument.DEFAULT_CONTENT_TYPE);
		
		ca.setContentAssistProcessor(new SoarDocContentAssistProcessor(),
				SoarPartitionScanner.SOAR_COMMENT);
		
		ca.enableAutoActivation(true);
		ca.setAutoActivationDelay(200);
		ca.setProposalPopupOrientation(ContentAssistant.CONTEXT_INFO_BELOW);
		ca.setContextInformationPopupOrientation(ContentAssistant.CONTEXT_INFO_BELOW);
		
		_contentAssistant = ca;
	}
	
	
	/**
	 * Single token scanner.
	 * Blatently stolen from org.eclipse.ui.examples.javeditor
	 */
	static class SingleTokenScanner extends BufferedRuleBasedScanner
	{
		public SingleTokenScanner(TextAttribute attribute)
		{
				setDefaultReturnToken(new Token(attribute));
		}
	}

	/**
	 * Configure the presentation reconciler for syntax highlighting
	 * @see org.eclipse.jface.text.source.SourceViewerConfiguration#getPresentationReconciler(ISourceViewer)
	 */
	public IPresentationReconciler getPresentationReconciler(
		ISourceViewer sourceViewer)
	{
		/*ColorProvider provider = 
			SoarPlugin.getDefault().getColorProvider();
 
		PresentationReconciler reconciler = new PresentationReconciler();

		// Rule for default text
		SoarDamagerRepairer dr = 
			new SoarDamagerRepairer(SoarEditor.getSoarCodeScanner());
		reconciler.setDamager(dr, IDocument.DEFAULT_CONTENT_TYPE);
		reconciler.setRepairer(dr, IDocument.DEFAULT_CONTENT_TYPE);

		dr = new SoarDamagerRepairer(
				new SingleTokenScanner(
					new TextAttribute(provider.getColor(ColorProvider.COMMENT))));
		reconciler.setDamager(dr, SoarPartitionScanner.SOAR_COMMENT);
		reconciler.setRepairer(dr, SoarPartitionScanner.SOAR_COMMENT);

		return reconciler; */
		
		ColorProvider provider = 
			SoarPlugin.getDefault().getColorProvider();
 
		PresentationReconciler reconciler = new PresentationReconciler();

		// Rule for default text
		/*DefaultDamagerRepairer dr = 
			new DefaultDamagerRepairer(SoarEditor.getSoarCodeScanner());
		reconciler.setDamager(dr, IDocument.DEFAULT_CONTENT_TYPE);
		reconciler.setRepairer(dr, IDocument.DEFAULT_CONTENT_TYPE); */


		DefaultDamagerRepairer dr = 
			new DefaultDamagerRepairer(SoarEditor.getSoarCodeScanner());
		reconciler.setDamager(dr, IDocument.DEFAULT_CONTENT_TYPE);
		reconciler.setRepairer(dr, IDocument.DEFAULT_CONTENT_TYPE);
		

		dr = new DefaultDamagerRepairer(
				new SingleTokenScanner(
					new TextAttribute(provider.getColor(ColorProvider.COMMENT))));
		dr = new DefaultDamagerRepairer(new SoarCommentScanner());
		reconciler.setDamager(dr, SoarPartitionScanner.SOAR_COMMENT);
		reconciler.setRepairer(dr, SoarPartitionScanner.SOAR_COMMENT);

		return reconciler;
	}

	/**
	 * Configure the content formatter with the formatting strategies.
	 * @see org.eclipse.jface.text.source.SourceViewerConfiguration#getContentFormatter(ISourceViewer)
	 */
	public IContentFormatter getContentFormatter(ISourceViewer sourceViewer)
	{
		ContentFormatter formatter = new ContentFormatter();
		IFormattingStrategy autoalign = new SoarAlignStrategy();
		formatter.setFormattingStrategy(
				autoalign, 
				IDocument.DEFAULT_CONTENT_TYPE);

		return formatter;
	}
	
	public IContentAssistant getContentAssistant(ISourceViewer sourceViewer)
	{
		return _contentAssistant;
	}
	
	public IAutoEditStrategy[] getAutoEditStrategies(ISourceViewer sourceViewer, String contentType) {
		return new IAutoEditStrategy[] {new SoarAutoIndentStrategy() };
	}
	
	
	/*public IAutoIndentStrategy getAutoIndentStrategy(
		ISourceViewer sourceViewer, String contentType) {
		
		return new SoarAutoIndentStrategy();
	}*/
}
