/**
 * A damager and repairer for Soar files
 * @file SoarDamagerRepairer.java
 * @date December 15, 2003
 */
package edu.rosehulman.soar.editor;

import org.eclipse.jface.text.BadLocationException;
import org.eclipse.jface.text.DocumentEvent;
import org.eclipse.jface.text.IDocument;
import org.eclipse.jface.text.IRegion;
import org.eclipse.jface.text.ITypedRegion;
import org.eclipse.jface.text.TextAttribute;
import org.eclipse.jface.text.TextPresentation;
import org.eclipse.jface.text.presentation.IPresentationDamager;
import org.eclipse.jface.text.presentation.IPresentationRepairer;
import org.eclipse.jface.text.rules.IToken;
import org.eclipse.jface.text.rules.ITokenScanner;
import org.eclipse.jface.text.rules.Token;

import org.eclipse.swt.custom.StyleRange;

/**
 * A damager and repairer for Soar files
 * @author Zach Lute <lutezp@cs.rose-hulman.edu>
 */
public class SoarDamagerRepairer
	implements IPresentationDamager, IPresentationRepairer
{
	protected IDocument _document;
	protected ITokenScanner _scanner;
	protected TextAttribute _defaultTextAttribute;

	/**
	 * Creates a SoarDamagerRepairer with the given scanner.
	 * @param scanner The scanner for obtaining document tokens
	 */
	public SoarDamagerRepairer(ITokenScanner scanner)
	{
		_scanner = scanner;
		_defaultTextAttribute = new TextAttribute(null);
	}
	
	// Implementing IPresentationDamager
	
	/**
	 * Returns the damage in the document's presentation caused
	 * by the given document change.
	 * @see org.eclipse.jface.text.presentation.IPresentationDamager#getDamageRegion(ITypedRegion,DocumentEvent,boolean)
	 */
	public IRegion getDamageRegion(
		ITypedRegion partition,
		DocumentEvent event,
		boolean documentPartitioningChanged)
	{
		if(!documentPartitioningChanged)
		{
			try
			{
				IRegion info = 
					_document.getLineInformationOfOffset(event.getOffset());
				//int start = 
				//	Math.max(partition.getOffset(), info.getOffset());
				int end =
					event.getOffset() + 
						(event.getText() == null ? event.getLength() : 
														event.getText().length());

				if(info.getOffset() <= end &&
					end <= info.getOffset() + info.getLength())
				{
					// Optimize the case of the same line
					end = info.getOffset() + info.getLength();
				}
				else
				{
					end = endOfLineOf(end);
				}

				end = 
					Math.min(partition.getOffset() + partition.getLength(), end);
			}
			catch(BadLocationException ble)
			{
				// This should never, ever happen.
				ble.printStackTrace();
			}
		}

		return partition;
	}

	/**
	 * Tells the presentation damager/repairer on which document it will work.
	 * @see org.eclipse.jface.text.presentation.IPresentationDamager#setDocument(IDocument)
	 */
	public void setDocument(IDocument document)
	{
		_document = document;
	}

	// Implementing IPresentationRepairer

	/**
	 * Fills the given presentation with the style ranges which 
	 * when applied to the presentation reconciler's text viewer
	 * repair the presentational damage described by the given region.	
	 * @see org.eclipse.jface.text.presentation.IPresentationRepairer#createPresentation(TextPresentation,ITypedRegion)
	 */
	 public void createPresentation(
		TextPresentation presentation, ITypedRegion damage)
	{
		int lastStart = damage.getOffset();
		int length = 0;
		IToken lastToken = Token.UNDEFINED;
		TextAttribute lastAttribute = getTokenTextAttribute(lastToken);

		_scanner.setRange(_document, lastStart, damage.getLength());

		while(true)
		{
			IToken token = _scanner.nextToken();
			if(token.isEOF())
			{
				break;
			}

			TextAttribute attribute = getTokenTextAttribute(token);

			// This is where our implementation differs from that of
			// the DefaultDamagerRepairer.  At this point, it checks
			// to see if the last attribute was the same as this one,
			// and if so it just adds the length of this one on to the
			// last one and continues on.

			// The problem arises in the fact that our tokenizer doesn't
			// return spaces as tokens, so every time it encounters a
			// space, we end up getting off by one.  The simplest way
			// to solve this (although we lose a bit of efficiency) is
			// just to style each of those tokens individually, and so
			// we do.
			
			addRange(presentation, lastStart, length, lastAttribute);
			lastToken = token;
			lastAttribute = attribute;
			lastStart = _scanner.getTokenOffset();
			length = _scanner.getTokenLength();
		}

		addRange(presentation, lastStart, length, lastAttribute);
	}

	protected TextAttribute getTokenTextAttribute(IToken token)
	{
		Object data = token.getData();
		if(data instanceof TextAttribute)
		{
			return (TextAttribute) data;
		}

		return _defaultTextAttribute;
	}

	protected void addRange(
		TextPresentation presentation, 
		int offset, 
		int length, 
		TextAttribute attr)
	{
		if(attr != null)
		{
			presentation.addStyleRange(
				new StyleRange(
					offset, length, attr.getForeground(), 
					attr.getBackground(), attr.getStyle()));
		}
	}

	protected int endOfLineOf(int offset) throws BadLocationException
	{
		IRegion info = _document.getLineInformationOfOffset(offset);
		if(offset <= info.getOffset() + info.getLength())
		{
			return info.getOffset() + info.getLength();
		}

		int line = _document.getLineOfOffset(offset);
		try
		{
			info = _document.getLineInformation(line + 1);
			return info.getOffset() + info.getLength();
		}
		catch(BadLocationException ble)
		{
			return _document.getLength();
		}	
	}
}
