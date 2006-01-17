/**
 * Defines the DocumentProvider for the Soar Editor Plug-in
 * @file SoarEditorDocumentProvider.java
 * @date December 11, 2003
 */
package edu.rosehulman.soar.editor;

import org.eclipse.core.runtime.CoreException;
import org.eclipse.jface.text.IDocument;
import org.eclipse.jface.text.IDocumentPartitioner;
import org.eclipse.jface.text.rules.*;
import org.eclipse.ui.editors.text.FileDocumentProvider;

import edu.rosehulman.soar.editor.soar.SoarPartitionScanner;

/**
 * Defines the DocumentProvider for the Soar Editor Plug-in
 * @author lutezp
 */
public class SoarEditorDocumentProvider extends FileDocumentProvider
{
	private static SoarPartitionScanner _scanner = null;
	
	private final static String[] TYPES = 
		new String[] { //SoarPartitionScanner.SOAR_CODE,
							SoarPartitionScanner.SOAR_COMMENT };
	
	/**
	 * Creates a new document for the given element of input and
	 * connects the partitioner to the document.
	 * @param element The element for which we are creating a document
	 * @see org.eclipse.ui.texteditor.AbstractDocumentProvider#createDocument(Object)
	 */
	protected IDocument createDocument(Object element) throws CoreException
	{
		IDocument document = super.createDocument(element);
		
		//BUG: partitioning the comments...
		//all it seems to do is make the auto-indenter not work on comments.
		//so, removing the partitioner, examining the results of that.
		//RESULTS: comments not colored, act like code.
		//I guess I need to figure out how to make the autoindentstrategy fire here, too
		
		if(document != null)
		{
			IDocumentPartitioner partitioner = createSoarPartitioner();
			partitioner.connect(document);
			document.setDocumentPartitioner(partitioner);
		}
		
		return document;
	}

	/**
	 * Creates a partitioner for Soar files.
	 * @return A partitioner designed to partition Soar files
	 */
	private FastPartitioner createSoarPartitioner()
	{
		return new FastPartitioner(getSoarPartitionScanner(), TYPES);
	}

	/**
	 * Creates a scanner for Soar partitions
	 * @return A scanner for Soar partitions
	 */
	private SoarPartitionScanner getSoarPartitionScanner()
	{
		if(_scanner == null)
		{
			_scanner = new SoarPartitionScanner();
		}

		return _scanner;
	}
}
