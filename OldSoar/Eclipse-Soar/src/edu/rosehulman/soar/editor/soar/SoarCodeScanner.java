/**
 * Defines the code scanner for Soar code.
 * @file SoarCodeScanner.java
 * @date December 11, 2003
 */
package edu.rosehulman.soar.editor.soar;

import java.io.StringReader;

import java.util.List;
import java.util.LinkedList;

import org.eclipse.jface.text.BadLocationException;
import org.eclipse.jface.text.IDocument;
import org.eclipse.jface.text.TextAttribute;
import org.eclipse.jface.text.rules.IToken;
import org.eclipse.jface.text.rules.ITokenScanner;
import org.eclipse.jface.text.rules.Token;
import org.eclipse.swt.SWT;


import edu.umich.visualsoar.parser.ASCII_CharStream;
import edu.umich.visualsoar.parser.SoarParserConstants;
import edu.umich.visualsoar.parser.SoarParserTokenManager;
import edu.umich.visualsoar.parser.TokenMgrError;

import edu.rosehulman.soar.SoarPlugin;
import edu.rosehulman.soar.editor.util.ColorProvider;

/**
 * The SoarCodeScanner finds keywords and other attributes in a Soar code
 * document as it is edited by the user.  It makes use of the Soar parsing
 * code of VisualSoar, written by Brad Jones using JavaCC and compiled
 * into Java classes which have now been included in the
 * edu.rosehulman.soar.editor tree.
 * @author Zach Lute <lutezp@cs.rose-hulman.edu>
 */
public class SoarCodeScanner implements ITokenScanner
{
	private class SoarToken
	{
		private String _image;
		private int _kind;
		private int _offset;
		private int _line;
		private int _column;

		public SoarToken()
		{
		}

		public SoarToken(
			SoarToken lastToken, edu.umich.visualsoar.parser.Token token)
		{
			// The parser is not so smart (in my opinion), and chooses
			// to put the newline character in the first column of the
			// line it creates rather than in the line it ends...so
			// we have to fix that little problem here.

			// NOTE: As an afterthought...that might be Eclipse doing that.
			// TODO: Look into what's actually causing this problem.

			setLine(_documentStartLine + token.beginLine);

			//setColumn(token.beginColumn);
			setColumn(
				(_line == _documentStartLine) ? 
					token.beginColumn : token.beginColumn - 1);

			setOffset(calculateOffset(lastToken));			

			setImage(token.image);

			setKind(token.kind);
		}

		private int calculateOffset(SoarToken lastToken)
		{
			// To calculate the offset of the new token, we need to
			// figure out its offset from the old token and work from there.

			int offset = _lastToken.getOffset();

			// If we're on the same line
			if(_line == lastToken.getLine())
			{
				// Add the difference in the two columns to the offset
				offset += _column - lastToken.getColumn();
			}
			// If we're on a different line...
			else
			{
				try
				{
					int currLine = lastToken.getLine();

					// Calculate the offset up to the end of the old line
					offset += _document.getLineLength(currLine)
									- _lastToken.getColumn();
					currLine++;

					// Then add the length of all the lines in between
					while(currLine < _line)
					{
						offset += _document.getLineLength(currLine);
						currLine++;
					}

					// Finally, add the offset up to the current column
					offset += _column;
				}
				catch(BadLocationException ble)
				{
					// This should never, ever happen, provided the framework
					// sends us the right information...which I hope we can rely on.
					ble.printStackTrace();
				}
			}	

			return offset;
		}
		
		public void setImage(String image)
		{
			_image = image;
		}
		
		public void setKind(int kind)
		{
			_kind = kind;
		}
		
		public void setOffset(int offset)
		{
			_offset = offset;
		}
		
		public void setLine(int line)
		{
			_line = line;
		}
		
		public void setColumn(int column)
		{
			_column = column;
		}

		public String getImage()
		{
			return _image;
		}
		
		public int getKind()
		{
			return _kind;
		}
		
		public int getOffset()
		{
			return _offset;
		}
		
		public int getLength()
		{
			return _image.length();
		}

		public int getLine()
		{
			return _line;
		}

		public int getColumn()
		{
			return _column;
		}

		public String toString()
		{
			return "TOKEN: " + getImage() + "\n"
				+ "\tOFFSET: " + getOffset() + "\n"
				+ "\tLENGTH: " + getLength() + "\n"
				+ "\t  LINE: " + getLine() + "\n"
				+ "\tCOLUMN: " + getColumn();
		}
	}

	private ColorProvider _provider;
	
	private List _tokenList;
	private SoarToken _lastToken;
	private int _tokenIndex;

	private SoarToken _currentToken;

	private IDocument _document;
	private int _documentStartLine;
	
	private SoarParserTokenManager _tokenMgr;

	private IToken _keyword;
	private IToken _name;
	private IToken _number;
	private IToken _variable;
	private IToken _identifier;
	private IToken _other;
	
	/**
	 * Creates a code scanner which uses the Soar parsing implementaion
	 * provided by Brad Jones in VisualSoar to create Eclipse-style
	 * tokens, which can be used to manipulate the document.
	 */
	public SoarCodeScanner()
	{
		_provider = SoarPlugin.getDefault().getColorProvider();

		_tokenList = new LinkedList();
		_tokenIndex = 0;

		// NOTE: This used to make keywords bold as well, but
		//       for some reason that caused all kinds of horrible
		//       String out of bounds exceptions under GTK.  So,
		//       I've removed that part for the moment, but it's probably
		//       something that should be looked at in the future.

		_keyword = new Token(
			new TextAttribute(
				_provider.getColor(ColorProvider.KEYWORD),
				_provider.getColor(ColorProvider.BACKGROUND),
				SWT.BOLD));

		_variable = new Token(
			new TextAttribute(
				_provider.getColor(ColorProvider.OPERATOR)));

		_number = new Token(
			new TextAttribute(
				_provider.getColor(ColorProvider.NUMBER)));

		_name = new Token(
			new TextAttribute(
				_provider.getColor(ColorProvider.NAME)));
	
		_identifier = new Token(
			new TextAttribute(
				_provider.getColor(ColorProvider.IDENTIFIER)));

		_other = new Token(
			new TextAttribute(
				_provider.getColor(ColorProvider.DEFAULT)));
	}

	/**
	 * Returns the length of the last token read by this scanner.
	 * @see org.eclipse.jface.text.rules.ITokenScanner#getTokenLength()
	 */
	public int getTokenLength()
	{
		return _currentToken.getLength();
	}

	/**
	 * Returns the offset of the last token read by this scanner.
	 * @see org.eclipse.jface.text.rules.ITokenScanner#getTokenOffset()
	 */
	public int getTokenOffset()
	{
		return _currentToken.getOffset();
	}

	/**
	 * Returns the next token in the document.
	 * @see org.eclipse.jface.text.rules.ITokenScanner#nextToken()
	 */
	public IToken nextToken()
	{
		IToken token = _other;

		if(_tokenIndex >= _tokenList.size())
		{
			System.out.println("Tried to get index greater than size");
			return Token.EOF;
		}

		_currentToken = (SoarToken) _tokenList.get(_tokenIndex);

		// We have to set the offset to 

		// Now we do all of the advanced checking for special coloring
		// not handled by the general case of the parser itself.

		switch(_currentToken.getKind())
		{
			case SoarParserConstants.SP:
				token = _keyword;
				break;
			case SoarParserConstants.RARROW:
				token = _keyword;
				break;
			case SoarParserConstants.SYMBOLIC_CONST:
				// Check to see if it's a symbolic constant we care about
				if(_tokenIndex > 0)
				{
					SoarToken previous = 
						(SoarToken) _tokenList.get(_tokenIndex - 1);

					// Is it part of an identifier?
					// Definition: After a "^" or "."
					if(previous.getKind() == SoarParserConstants.CARET ||
						previous.getKind() == SoarParserConstants.PERIOD)
					{
						token = _identifier;
					}

					// Is it the name of a rule?
					// Definition: After BOTH "sp" and "{"
					else if(previous.getKind() == SoarParserConstants.LBRACE)
					{
						if(_tokenIndex > 1)
						{
							SoarToken beforePrevious = 
								(SoarToken) _tokenList.get(_tokenIndex - 2);

							if(beforePrevious.getKind() == SoarParserConstants.SP)
							{
								token = _name;
							}
						}
					}
				}
				break;
			case SoarParserConstants.VARIABLE:
				token = _variable;
				break;
			case SoarParserConstants.INTEGER_CONST:
				// Fall through
			case SoarParserConstants.FLOATING_POINT_CONST:
				token = _number;
				break;
			case SoarParserConstants.EOF:
				token = Token.EOF;
				break;
			default:
				token = _other;
		}	
			
		_tokenIndex++;

		return token;
	}

	private SoarToken parseNextToken()
	{
		// This function is actually fairly simple.  Because we already
		// have the parser tokenizer to get our tokens for us, all this
		// function has to do is map those tokens to the appropriate
		// tokens for the SoarEditor (by doing a bit more voodoo).
		
		edu.umich.visualsoar.parser.Token parserToken = null;
		
		try
		{
			parserToken = _tokenMgr.getNextToken();
		}
		catch(TokenMgrError tme)
		{
			// According to whoever wrote the rule editor for Visual Soar,
			// if this happens, it just means the syntax wasn't valid at 
			// the current state of entry, so we just assume more is coming 
			// and bail out.

			// I'm inclined to believe whoever that was, so that's what
			// we'll do here.

			return null;
		}

		// Now we run through the kinds of Soar Parser tokens and
		// return the appropriate SoarToken.
		
		//SoarToken token = null;

		if(parserToken == null)
		{
			return null;
		}

		return new SoarToken(_lastToken, parserToken);
	}

	/**
	 * Configures the scanner by providing access to the document 
	 * range that should be scanned.
	 * @see org.eclipse.jface.text.rules.ITokenScanner#setRange(IDocument,int,int)
	 */
	public void setRange(IDocument document, int offset, int length)
	{
		// Here we have to do a little bit of voodoo to make the Eclipse
		// side of things and the parser side of things get along.
		// Mainly, this involves converting the IDocument format over
		// to the ASCII_CharStream format the parser is looking for.
		
		// To do this, we have to get the appropriate String from the
		// IDocument, create a Reader for that string, and then create
		// an ASCII_CharStream from that Reader.  It's ugly, I know,
		// but it means I didn't have to rewrite all the parser code.

		// Yeah, now you understand.

		//System.out.println("****** RESETTING CODE SCANNER ******");
		//System.out.println("****** OFFSET: " + offset);
		//System.out.println("****** LENGTH: " + length);
		try
		{
			_document = document;
			_documentStartLine = document.getLineOfOffset(offset);
		
			_tokenIndex = 0;
		
			_lastToken = new SoarToken();
			_lastToken.setOffset(offset);
			_lastToken.setImage("");
			_lastToken.setLine(_documentStartLine);
			_lastToken.setColumn(
				offset - document.getLineOffset(_lastToken.getLine()));	

			//System.out.println("Start: " + _lastToken.getOffset() + " L: " + _lastToken.getLine() + " C: " + _lastToken.getColumn());
 	
			String toScan = _document.get(offset, length);

			// What I'm about to do is an awful, awful thing.  See,
			// unfortunately the parser converts tabs to 8 columns,
			// and returns its columns accordingly.  The Eclipse side
			// of things doesn't like that very much, and thus, any time
			// you insert a tab, the column count gets off by 7.

			// To fix this problem, I'm replacing all tabs with a single
			// space in the input string, and thus they will only count
			// as one column.  This only works because tab and space are
			// not symantically important in Soar (I hope).

			toScan = toScan.replace('\t', ' ');
		
			//System.out.println("toScan: " + toScan);
	
			StringReader reader = new StringReader(toScan);
			ASCII_CharStream stream = new ASCII_CharStream(reader, 0, 0);
		
			_tokenMgr = new SoarParserTokenManager(stream);

			// Now that we've done all of the setup, we have to do something
			// that isn't very clean, but which must be done in order to
			// get advanced syntax highlighting.  Because the parser we're
			// using doesn't let us run forward to check what's ahead without
			// breaking it, we're going to have to preparse everything
			// in the document, so that nextToken() can do more advanced
			// work in lookahead and the like.  So here we go...

			_tokenList = new LinkedList();

			SoarToken token = parseNextToken();
			while((token != null) && (token.getKind() != SoarParserConstants.EOF))
			{
				//System.out.println(token);
				_tokenList.add(token);
				_lastToken = token;
				token = parseNextToken();
			}
			
			if(token != null)
			{
				_tokenList.add(token);
			}
		}
		catch(BadLocationException ble)
		{
			// This should never, ever happen.
			ble.printStackTrace();
		}
	}
}
