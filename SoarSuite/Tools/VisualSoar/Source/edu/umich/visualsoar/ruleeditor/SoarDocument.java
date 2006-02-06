package edu.umich.visualsoar.ruleeditor;

import java.io.*;
import java.util.TreeMap;
import java.awt.*;
import javax.swing.text.*;
import java.util.Enumeration;
import edu.umich.visualsoar.parser.*;
import edu.umich.visualsoar.MainFrame;
import edu.umich.visualsoar.misc.*;
import javax.swing.*;

public class SoarDocument extends DefaultStyledDocument
{

    AbstractElement root = (AbstractElement)getDefaultRootElement();
    SyntaxColor[]   colorTable;
    Preferences     prefs = Preferences.getInstance();
    boolean         inRHS = false; // Are we in the RHS of a production?
    
    public SoarDocument()
    {
        colorTable = (SyntaxColor[])Preferences.getInstance().getSyntaxColors().clone();
    }
    
                    
    public void insertString(int offset,
                             String str,
                             AttributeSet a) throws BadLocationException
    {
        super.insertString(offset, str, a);
        
        if (! prefs.isHighlightingEnabled())
        {
            return;
        }
        
        int     length = str.length();
        
        if (length == 1)
        {
            colorSyntax(offset);
        }
        else
        {
            colorSyntax(offset, str.length(), new StringReader(str));

//Commented out for now.  I don't understand this code and it no longer seems
//to work. -:AMN: 24 Nov 03
//              Element currElem = root.getElement(root.getElementIndex(offset));
//              Content data = getContent();
//              String elStr = getElementString(currElem,data);
//              if(elStr.length() > str.length())
//              {
//                  colorSyntax(currElem.getStartOffset(),
//                              elStr.length(),
//                              new StringReader(elStr));
//              }
//              else
//              {
//                  colorSyntax(offset, str.length(), new StringReader(str));
//              }
        }//else

    }//insertString()
    
    public void remove(int offs, int len) throws BadLocationException
    {
        super.remove(offs, len);

        colorSyntax(offs);
    }
            
    
    void replaceRange(String str, int start, int end)
    {
        try
        {
            remove(start, end - start);
            insertString(start, str, null);
        } catch (BadLocationException e)
        {
            throw new IllegalArgumentException(e.getMessage());
        }
    } // replaceRange()
        
    String getElementString(int elementIndex, Content content)
    {
        String              theLine = null;
        
        AbstractElement     element;
        
        if (elementIndex > -1)
        {
            element = (AbstractElement)root.getElement(elementIndex);
        }
        else
        {
            return null;
        }

        try
        {
            theLine = content.getString(element.getStartOffset(),
                                        element.getEndOffset() - element.getStartOffset() - 1);
        } catch (BadLocationException e)
        {
            e.printStackTrace();
        }
        
        return theLine;
    } // getElementString()

    String getElementString(Element element, Content content)
    {
        String              theLine = null;
        
        try
        {
            theLine = content.getString(element.getStartOffset(),
                                        element.getEndOffset() - element.getStartOffset() - 1);
        } catch (BadLocationException e)
        {
            e.printStackTrace();
        }
        
        return theLine;
    } // getElementString()
    
    String getElementString(Element element, Content content, int startOffset)
    {
        String  theLine = null;
        
        try
        {
            theLine = content.getString(startOffset,
                                        element.getEndOffset() - startOffset - 1);
        } catch (BadLocationException e)
        {
            e.printStackTrace();
        }
        
        return theLine;
    } // getElementString()

    void colorRange(int begPos, int length, int kind)
    {
        Color               theColor = colorTable[kind];
        SimpleAttributeSet  attrib = new SimpleAttributeSet();
        String text = null;
        try
        {
            text = getText(begPos,length);
        }
        catch(BadLocationException ble) {}
        
        StyleConstants.setForeground(attrib, theColor);             
        setCharacterAttributes(begPos, length, attrib, false);
    }//colorRange()

    /*
     * If the parser barfs on the input we want to casually just keep reading
     * the file.  So we catch the thrown error and and substitute a dummy token.
     * :AMN: 31 Oct '03  (Boo!)
     */
    Token carefullyGetNextToken(SoarParserTokenManager mgr)
    {
        try
        {
            return mgr.getNextToken();
        }
        catch(TokenMgrError tme)
        {
            return new Token();
        }
                
    }//carefullyGetNextToken()

    void evaluateToken(Token currToken,
                       int startOffset,
                       SoarParserTokenManager mgr)
    {
        Token   nextToken;

        int     begin;
        int     length;
    
        Token startToken = currToken;
        switch (currToken.kind)
        {
            case SoarParserConstants.RARROW :
                begin = startOffset + currToken.beginColumn;
                length = 3;
                colorRange(begin, length, currToken.kind);
                inRHS = true;
                break;
                
            case SoarParserConstants.SP :
                begin = startOffset + currToken.beginColumn;
                length = 2;
                colorRange(begin, length, currToken.kind);
                break;
                                    
            case SoarParserConstants.CARET : // followed by a STRING
                begin = startOffset + currToken.beginColumn;
                colorRange(begin, 1, SoarParserConstants.DEFAULT);

                currToken = carefullyGetNextToken(mgr);
                begin += 1;
                if ((currToken.kind == SoarParserConstants.SYMBOLIC_CONST)
                    || (currToken.kind == SoarParserConstants.INTEGER_CONST)
                    || (currToken.kind == SoarParserConstants.FLOATING_POINT_CONST))
                {
                 
                    length = currToken.image.length();
                    colorRange(begin, length, SoarParserConstants.CARET);                    

                    currToken = carefullyGetNextToken(mgr);
                    while (currToken.kind == SoarParserConstants.PERIOD)
                    {
                        begin += length + 1; // don't color period
                    
                        currToken = carefullyGetNextToken(mgr);
                        length = currToken.image.length();
                        
                        if ((currToken.kind == SoarParserConstants.SYMBOLIC_CONST)
                            || (currToken.kind == SoarParserConstants.INTEGER_CONST)
                            || (currToken.kind == SoarParserConstants.FLOATING_POINT_CONST))
                        {

                            colorRange(begin, length, SoarParserConstants.CARET);
                        }
                        else if (currToken.kind == SoarParserConstants.VARIABLE)
                        {
                            colorRange(begin, length, SoarParserConstants.VARIABLE);
                        }
                        
                        currToken = carefullyGetNextToken(mgr);
                    }
                                    
                }
                else if (currToken.kind == SoarParserConstants.VARIABLE)
                {
                 
                    length = currToken.image.length();
                    colorRange(begin, length, SoarParserConstants.VARIABLE);                     
                    
                    currToken = carefullyGetNextToken(mgr);
                    while (currToken.kind == SoarParserConstants.PERIOD)
                    {
                        begin += length + 1; // don't color period
                    
                        currToken = carefullyGetNextToken(mgr);
                        length = currToken.image.length();
                        
                        if ((currToken.kind == SoarParserConstants.SYMBOLIC_CONST)
                            || (currToken.kind == SoarParserConstants.INTEGER_CONST)
                            || (currToken.kind == SoarParserConstants.FLOATING_POINT_CONST))
                        {

                            colorRange(begin, length, SoarParserConstants.CARET);
                        }
                        else if (currToken.kind == SoarParserConstants.VARIABLE)
                        {
                            colorRange(begin, length, SoarParserConstants.VARIABLE);
                        }
                        
                        currToken = carefullyGetNextToken(mgr);
                    }
                                    
                }
                // We need to make sure that this token is on the same line as the first
                // token
                if(startToken.beginLine != currToken.beginLine)
                {
                    Element rootElement = getDefaultRootElement();
                    Element childElement = rootElement.getElement(currToken.beginLine);
                    evaluateToken(currToken,childElement.getStartOffset()-1,mgr);
                }
                else
                evaluateToken(currToken, startOffset, mgr);  
                break;
                
            case SoarParserConstants.VARIABLE :
                begin = startOffset + currToken.beginColumn;
                // XXX Assumes that tokens do not cross line barriers
                length = currToken.image.length();
                colorRange(begin, length, currToken.kind);
                break;
                
            case SoarParserConstants.SYMBOLIC_CONST :
                begin = startOffset + currToken.beginColumn;
                //NOTE:  This assumes that tokens do not cross line barriers
                length = currToken.image.length();

                //Since the syntax highlighter has to guess which lexical state
                //to start in, me may see an "sp" here being misinterpreted as
                //a symbolic constant.  So we catch that here.
                if (currToken.image.equals("sp"))
                {
                    currToken.kind = SoarParserConstants.SP;
                    colorRange(begin, 2, currToken.kind);
                    inRHS = false;
                }
                else
                {
                    //This is really a symbolic constant
                    colorRange(begin, length, currToken.kind);
                }
                break;

            case SoarParserConstants.RBRACE :
                if (inRHS)
                {
                    //A closing brace in the RHS indicates we've finished
                    //a Soar production and should be back in the
                    //DEFAULT lexical state.  The parser does not do
                    //this for us because we're using it strictly as
                    //a tokenizer right now.
                    mgr.SwitchTo(SoarParserConstants.DEFAULT);
                    inRHS = false;
                }
                break;

            case SoarParserConstants.LPAREN:
            case SoarParserConstants.RPAREN:
            case SoarParserConstants.AMPERSAND:
            case SoarParserConstants.ATSIGN:
            case SoarParserConstants.COMMA:
            case SoarParserConstants.EQUAL:
            case SoarParserConstants.EMARK:
            case SoarParserConstants.GREATER:
            case SoarParserConstants.HYPHEN:
            case SoarParserConstants.LESS:
            case SoarParserConstants.PERIOD:
            case SoarParserConstants.PLUS:
            case SoarParserConstants.QMARK:
            case SoarParserConstants.TILDE:
            case SoarParserConstants.LSQBRACET:
            case SoarParserConstants.RSQBRACET:
            case SoarParserConstants.EXPONENT:
                begin = startOffset + currToken.beginColumn;
                colorRange(begin, 1, SoarParserConstants.DEFAULT);
                break;
                
            case SoarParserConstants.LDISJUNCT:
            case SoarParserConstants.RDISJUNCT:
            case SoarParserConstants.GEQUAL:
            case SoarParserConstants.LEQUAL:
            case SoarParserConstants.NEQUAL:
                begin = startOffset + currToken.beginColumn;
                colorRange(begin, 2, SoarParserConstants.DEFAULT);
                break;

                
            default:
                break;
                
        } // token cases
        
    } // evaluateToken()
    
    public void colorSyntax(Reader r)
    {
        (new ColorSyntaxThread(r)).start();
    }

    class ColorSyntaxThread extends Thread
    {
        Reader  r;
    
        public ColorSyntaxThread(Reader inReader)
        {
            r = inReader;
        }   
        
        public void run()
        {
            colorSyntax();
        }

        //Color the syntax of an entire file
        public void colorSyntax()
        {
            Token                   currToken;
            Element                 currElem;
            int                     currLineNum;
            int                     offset;
            SoarParserTokenManager  mgr =
                new SoarParserTokenManager(new ASCII_CharStream(r,0,0));
                                            
            try
            {
                currToken = mgr.getNextToken();
            }
            catch (TokenMgrError tme)
            {
                /* this just means the syntax wasn't valid at
                 * the current state of entry we assume that more
                 * is coming and give up for now
                 */
                return;
            }
            
            currLineNum = currToken.beginLine;
            currElem = root.getElement(currLineNum);
                            
            while (currToken.kind != SoarParserConstants.EOF)
            {
            
                if (currLineNum != currToken.beginLine)
                {
                    currLineNum = currToken.beginLine;
                    currElem = root.getElement(currLineNum);        
                }
                
                if (currLineNum == 0)
                {
                    offset = currElem.getStartOffset();
                }
                else
                {
                    offset = currElem.getStartOffset() - 1;
                }
                
                evaluateToken(currToken, offset, mgr);
                
                try
                {
                    currToken = mgr.getNextToken();
                } catch (TokenMgrError tme)
                {
                    /* this just means the syntax wasn't valid at
                     * the current state of entry we assume that more
                     * is coming and give up for now
                     */
                    return;
                }
                
            }  // iterate through tokens    
        } // colorSyntax() (whole file)
        
    } // ColorSyntaxThread

    /**
     * The Token class created by JavaCC does not have a copy ctor.
     * Since we can't edit that class we improvise a copy ctor here.
     *
     * @param dest Token to copy to 
     * @param src Token to copy from 
     * @author Andrew Nuxoll
     * 24 Nov 03
     */
    void copyToken(Token dst, Token src)
    {
        dst.kind         = src.kind;
        dst.beginLine    = src.beginLine;
        dst.beginColumn  = src.beginColumn;
        dst.endLine      = src.endLine;
        dst.endColumn    = src.endColumn;
        dst.image        = src.image;
        dst.next         = src.next;
        dst.specialToken = src.specialToken;
    }
    

/**
     * Given a reader, this function attempts to guess the correct lexical state
     * to start the parser in.  Once guessed, the correct first token and
     * token mamager is returned.
     *
     * @param r Reader
     * @param tok Token
     * @return a SoarParserTokenManager
     * @author Andrew Nuxoll
     * 17 Nov 03
     */
    protected SoarParserTokenManager guessLexicalState(Reader r,
                                                       Token tok)
    {
        SoarParserTokenManager mgr = null;
        Token ispTok = new Token(); // Token retrieved using IN_SOAR_PRODCTION
        Token defTok  = new Token(); // Token retrieved using DEFAULT
        
        //Save our position in the reader
        try
        {
            r.mark(4096);
        
            //Since we don't know what lexical state we're in, guess
            //the most likely state: IN_SOAR_PRODUCTION
            mgr = new SoarParserTokenManager(new ASCII_CharStream(r, 0, 0));
            mgr.SwitchTo(SoarParserConstants.IN_SOAR_PRODUCTION);

            //Get a token
            try
            {
                ispTok = mgr.getNextToken();
            }
            catch (TokenMgrError tme)
            {
                //Set the kind so that the other lexical state is tried.
                ispTok.kind = SoarParserConstants.SYMBOLIC_CONST;
            }
        
            //If the token is anything other than SYMBOLIC_CONST then
            //we probably picked the right lexical state.
            if (ispTok.kind != SoarParserConstants.SYMBOLIC_CONST)
            {
                copyToken(tok, ispTok);
                return mgr;
            }

            //Reset the reader and try the DEFAULT lexical state
            r.reset();
            mgr = new SoarParserTokenManager(new ASCII_CharStream(r, 0, 0));
        
            //Get a token
            try
            {
                defTok = mgr.getNextToken();
            }
            catch (TokenMgrError tme)
            {
                //Guess we'd better stick with IN_SOAR_PRODUCTION
                mgr = new SoarParserTokenManager(new ASCII_CharStream(r, 0, 0));
                mgr.SwitchTo(SoarParserConstants.IN_SOAR_PRODUCTION);
                copyToken(tok, ispTok);
                return mgr;
            }

            //Looks like the default state worked
            copyToken(tok,defTok);
        }
        catch(IOException ioe)
        {
            //This exception occurs when we call reset() on a closed reader
            //In this case we should stick with IN_SOAR_PRODUCTION
            mgr = new SoarParserTokenManager(new ASCII_CharStream(r, 0, 0));
            mgr.SwitchTo(SoarParserConstants.IN_SOAR_PRODUCTION);
            copyToken(tok, ispTok);
            return mgr;
        }

        return mgr;
    }//guessLexicalState()
    
    //Color the syntax of a single line
    public void colorSyntax(int caretPos)
    {
        Content                 data = getContent();
        Token                   currToken = new Token();
        Element                 currElem = root.getElement(root.getElementIndex(caretPos));
        String                  currLine;
        int                     offset = currElem.getStartOffset();
        
        currLine = getElementString(currElem, data);

        //Create a token manager. Since we don't know what lexical state we're
        //in, guess the most likely state.
        Reader r = new StringReader(currLine);
        SoarParserTokenManager mgr = guessLexicalState(r, currToken);

        // init all the text to black
        colorRange(offset, currLine.length(), SoarParserConstants.DEFAULT);
                
        while (currToken.kind != SoarParserConstants.EOF)
        {
        
            evaluateToken(currToken, offset, mgr);
            
            try
            {
                currToken = mgr.getNextToken();
            } catch (TokenMgrError tme)
            {
                /* this just means the syntax wasn't valid at
                 * the current state of entry we assume that more
                 * is coming and give up for now
                 */
                return;
            }
            
        }  // iterate through tokens    

        
    } // colorSyntax (one line)
    
    
    //Color the syntax of a specified region
    public void colorSyntax(int caretPos, int length, Reader r)
    {
        Content     data = getContent();
        String      modifiedString;
        Element     currElem;
        int         startLineNum = root.getElementIndex(caretPos);
        int         currLineNum;
        int         startPos;
        int         endPos;
        int         totalLength; 

        currElem = root.getElement(startLineNum);
        startPos = currElem.getStartOffset();
        endPos = root.getElement(root.getElementIndex(caretPos + length)).getEndOffset() - 1;
        totalLength = endPos - startPos; 
        Token currToken = new Token();

        //Create a token manager with our best guess for the current lexical
        //state and get the first token. 
        SoarParserTokenManager  mgr = guessLexicalState(r, currToken);
        
        currLineNum = startLineNum + currToken.beginLine;
                   
        // init all the text to black
        //colorRange(startPos, totalLength, SoarParserConstants.DEFAULT);
        
        while (currToken.kind != SoarParserConstants.EOF)
        {
        
            currLineNum = startLineNum + currToken.beginLine;
            currElem = root.getElement(currLineNum);
            
            if (currToken.beginLine == 0)
            {
                evaluateToken(currToken, currElem.getStartOffset(), mgr);
            }
            else
            {
                evaluateToken(currToken, currElem.getStartOffset() - 1, mgr);
            }
            
            try
            {
                currToken = mgr.getNextToken();
            } catch (TokenMgrError tme)
            {
                /* this just means the syntax wasn't valid at
                 * the current state of entry we assume that more
                 * is coming and give up for now
                 */
                return;
            }
            
        }  // iterate through tokens    
    } // colorSyntax() (specific section)



    /**
     *  Justifies a chunk of text from in the rule editor.
     *  If nothing is highlighted, then justifies the entire document
     *  @param selectionStart the position of the beginning of the highlighted text
     *  @param selectionEnd the position of the end of the highlighted text
     */
    public int justifyDocument(int selectionStart, int selectionEnd)
    {

        Content             data = getContent();

        String              prevLine = null;
        String              currLine = null;
        String              afterCaretString = null;
        String              newCurrLine = null;
        String              indentString = "";

        int         elemIndex = 0;
        int         prevLineIndex = 0;
        int         endIndex = 0;
        int                 numSpaces = 0;
        int                 currLineBegin;
        int                 currLineEnd;


        char                lastChar;
        char[]              indentChars;

        boolean             leftOfText = true;
        boolean       firstProduction = true;

        AbstractElement     currLineElem;

        // check if block of text is selected or not
        if(selectionStart == selectionEnd)
        {
            // nothing selected, start from beginning of file and do every line
            selectionStart = 0;
            selectionEnd = getLength() - 1;
            firstProduction = false;    // makes sure vs doesn't justify stuff before
            // the first production (ie echo)
        }

        elemIndex = root.getElementIndex(selectionStart);
        prevLineIndex = elemIndex -1;
        endIndex = root.getElementIndex(selectionEnd);

        // endIndex is one less if last line just a line feed
        String lastLine = getElementString(((AbstractElement)root.getElement(endIndex)), data);
        if(lastLine.trim().length() == 0)
        {
            endIndex--;
        }

        // This while loop goes through every line of selected text for
        // justification
        while((elemIndex <= endIndex)  && (selectionStart <= (getLength() - 1)))
        {
            currLineElem = (AbstractElement)root.getElement(elemIndex);
            currLineBegin = currLineElem.getStartOffset();
            currLineEnd = currLineElem.getEndOffset() - 1;
            currLine = getElementString(currLineElem, data);
            afterCaretString = getElementString(currLineElem, data, selectionStart);
            indentString = "";
            numSpaces = 0;

            // Get last prevLine that isn't a blank line or comment
            if (elemIndex > 0)
            {
                boolean goodLine = false;
                while((prevLineIndex > -1) && !goodLine)
                {
                    prevLine = getElementString(prevLineIndex, data);
                    if( (prevLine == null)
                        || (prevLine.trim().startsWith("#"))
                        || (prevLine.trim().length() == 0) )
                    {
                        prevLineIndex--;
                    }
                    else
                    {
                        goodLine = true;
                    }
                }   // end of while getting last previous line of Soar code
            }
            newCurrLine = currLine.trim();

            if ((newCurrLine.length() != 0)
                && ( (newCurrLine.charAt(0) == '}')
                     || (newCurrLine.startsWith("-->"))
                     || (newCurrLine.startsWith("sp")) ) )
            {
                if(newCurrLine.startsWith("sp"))
                {
                    firstProduction = true;
                }
                numSpaces = 0;
            }
            else if((newCurrLine.length() != 0)
                    && (newCurrLine.charAt(0) == '#'))
            {
                numSpaces = currLine.indexOf('#');    // don't move comment.
            }
            else if(prevLine == null)
            {
                if(newCurrLine.startsWith("sp"))
                {
                    numSpaces = 0;
                    firstProduction = true;
                }
            }
            // already returned if prevLine == null
            else if (prevLine.trim().length() != 0)
            {
                prevLine = cropComments(prevLine);  // omit comments from the end of the string
                lastChar = prevLine.charAt(prevLine.length() - 1);

                if(prevLine.startsWith("sp") || prevLine.endsWith("-->"))
                {
                    numSpaces = 3;
                }
                else if( (newCurrLine.startsWith("^")
                          || newCurrLine.startsWith("-^")) )
                {
                    String currentLine = prevLine;
                    int currentElementIndex = prevLineIndex;
                    boolean done = false;
                    numSpaces = 0;

                    while(!done && currentLine != null)
                    {
                        int upPos = currentLine.indexOf('^');
                        if(upPos != -1)
                        {
                            numSpaces = upPos;
                            done = true;
                        }
                        else
                        {
                            --currentElementIndex;
                            currentLine = getElementString(currentElementIndex,
                                                           data);
                        }
                    } // while searching previous lines for last instance of '^'

                    // if couldn't find a previous '^', set numspaces to end of
                    // last line
                    if(!done)
                    {
                        prevLine = cropComments(prevLine);
                        // tells us where the code actually starts (skips
                        // leading whitespace)
                        int firstCharLoc = prevLine.indexOf( (prevLine.trim()).charAt(0) );
                        numSpaces = prevLine.trim().length() + firstCharLoc + 1;
                    }
                }
                else if (lastChar == ')')
                {
                    int currentElementIndex = prevLineIndex;
                    boolean done = false;
                    numSpaces = 3;
                    int count = 0;
                    String currentLine = prevLine;

                    while( (!done)
                           && (currentLine != null)
                           && (currentElementIndex > -1) )
                    {
                        for(int i = currentLine.length() - 1; i >=0; --i)
                        {
                            if(currentLine.charAt(i) == ')')
                            {
                                ++count;
                            }
                            else if(currentLine.charAt(i) == '(')
                            {
                                --count;
                                if(count <= 0)
                                {
                                    numSpaces = i;
                                    done = true;
                                    break;
                                }
                            }
                        }
                        --currentElementIndex;

                        // Get the next previous line of valid Soar code
                        boolean soarLine = false;
                        while((currentElementIndex > -1) && !soarLine)
                        {
                            currentLine = getElementString(currentElementIndex, data);
                            if((currentLine == null) || currentLine.trim().startsWith("#") || (currentLine.length() == 0))
                            {
                                currentElementIndex--;
                            }
                            else
                            {
                                soarLine = true;
                            }
                        }   // end of while getting last previous line of Soar code

                    } // end of while
                } // end of else if last char == ')'
                else if (lastChar == '{')
                {
                    numSpaces = prevLine.indexOf('{');
                }
                else if ( ( newCurrLine.startsWith("(")
                            && ((lastChar == ')')
                                || (lastChar == '}')) ) )
                {
                    numSpaces = 3;
                }
                else if (prevLineIndex >= 0)
                {
                    // look for a ^ on previous line
                    String fullPrevLine = getElementString(prevLineIndex, data);  // get the full previous line
                    int caretLocation = fullPrevLine.indexOf('^');
                    if(caretLocation != -1)
                    {
                        // Get position past string that follows the caret (ie ^string )
                        while( (fullPrevLine.charAt(caretLocation) != ' ')
                               && (caretLocation < fullPrevLine.length()) )
                        {
                            caretLocation++;
                        }
                        // look to see if string past ^string is << or { and if
                        // so, use that position
                        while( (fullPrevLine.charAt(caretLocation) == ' ')
                               && (caretLocation < fullPrevLine.length()) )
                        {
                            caretLocation++;
                        }
                        if(fullPrevLine.charAt(caretLocation) == '{')
                        {
                            numSpaces = caretLocation + 2;
                        }
                        else if( (fullPrevLine.charAt(caretLocation) == '<')
                                 && (fullPrevLine.charAt(caretLocation + 1) == '<') )
                        {
                            numSpaces = caretLocation + 3;
                        }
                        else
                        {
                            numSpaces = caretLocation;
                        }
                    } // end of prevLine contains a '^' and thus currLine
                      // is considered a value
                    else
                    {
                        // else line up with previous line
                        numSpaces = fullPrevLine.indexOf( (fullPrevLine.trim()).charAt(0));
                    }
                }    // end of else if prevlineindex > 1
                else  
                {
                    // First line of re
                    numSpaces = 0;
                }
                
                //  Does not fit a previous constraint and therefore is possibly
                //  considered a VALUE of a wme
                
            }   // end of if previous line was not length zero


            //Now that we have the desired position, line up line based on that position
            // properly line up negated conditions
            if ( (newCurrLine.length() != 0)
                 && (newCurrLine.charAt(0) == '-')
                 && (numSpaces > 0) )
            {
                numSpaces--;
            }
            //  Empty line feed found, remove is within production
            if( (!(prevLine == null))
                && (newCurrLine.length() == 0)
                && (prevLine.trim().length() != 0)
                && firstProduction )
            {
                if(!(prevLine.trim().charAt(prevLine.trim().length() - 1) == '}'))
                {
                    indentString = "";
                    newCurrLine = "";
                    try
                    {
                        remove(currLineBegin - 1,
                               currLineEnd - currLineBegin + 1);
                    }
                    catch (BadLocationException e)
                    {
                        throw new IllegalArgumentException(e.getMessage());
                    }
                }
            }
            // no line up char found, or coincidental 3 space indent
            else if ((numSpaces == 3) || (numSpaces < 0))
            {
                indentString = "   ";
                numSpaces = 3;
            }
            // variable indent to line up chars vertically
            else if (numSpaces > 0)
            {
                indentChars = new char[numSpaces];

                for (int i = 0; i < numSpaces; i++)
                {
                    indentChars[i] = ' ';
                }
                indentString = new String(indentChars);
            }
            // no indent for normal chars, or coincidental 0 space indent
            newCurrLine = indentString + newCurrLine;
            if ( !newCurrLine.equals(currLine)
                 && (newCurrLine.length() != 0))
            {
                // not already justified
                replaceRange(newCurrLine, currLineBegin, currLineEnd);
            }

            currLineEnd = currLineElem.getEndOffset() - 1;
            selectionStart = currLineEnd + 1;
            if(selectionStart <= (getLength() - 1))
            {
                elemIndex = root.getElementIndex(selectionStart);
                prevLineIndex = elemIndex -1;
            }
        }     // end of while going through every line

        return 0;
    } // justifyDocument()

    public int autoJustify(int caretPos)
    {
        if(!edu.umich.visualsoar.misc.Preferences.getInstance().isAutoIndentingEnabled())
        return -1;
        Content             data = getContent();

        String              prevLine = null;
        String              currLine = null;
        String              afterCaretString = null;
        String              newCurrLine = null;
        String              indentString = "";
        
                    
        int                 elemIndex = root.getElementIndex(caretPos);
        int         prevLineIndex = elemIndex - 1;
        int                 numSpaces = 0;
        int                 currLineBegin;
        int                 currLineEnd;
        
        
        char                lastChar;
        char[]              indentChars;
        
        boolean             leftOfText = true;
        
        AbstractElement     currLineElem;


        currLineElem = (AbstractElement)root.getElement(elemIndex);
        currLineBegin = currLineElem.getStartOffset();
        currLineEnd = currLineElem.getEndOffset() - 1;
        currLine = getElementString(currLineElem, data);
        afterCaretString = getElementString(currLineElem, data, caretPos);


        // Gets the last line of Soar code (skips blanks lines and comment lines)
        if (elemIndex > 0)
        {
            boolean goodLine = false;
            while((prevLineIndex > -1) && !goodLine)
            {
                prevLine = getElementString(prevLineIndex, data);
                if((prevLine == null) || prevLine.trim().startsWith("#") || (prevLine.trim().length() == 0))
                {
                    prevLineIndex--;
                }
                else
                {
                    goodLine = true;
                }
            }   // end of while getting last previous line of Soar code
        }
        else { // no indent for 1st line
            return -1;
        }
        if (prevLine == null)
        {
            return -1;
        }
        newCurrLine = currLine.trim();

        if( (newCurrLine.length() == 0) )
        {
            //Attempt to indent the appropriate number of spaces
            String trimmed = prevLine.trim();
            if ( ((trimmed.startsWith("sp")) && (trimmed.indexOf("{") != -1))
                 || ((trimmed.startsWith("(")) && (trimmed.endsWith(")")))
                 || ((trimmed.startsWith("^")) && (trimmed.endsWith(")")))
                 || (trimmed.startsWith("-->")) )
            {
                numSpaces = 3;
            }
            else if ( (trimmed.startsWith("(") || trimmed.startsWith("^"))
                      && (trimmed.indexOf(")") == -1)
                      && (trimmed.indexOf("^") != -1) )
            {
                numSpaces = prevLine.indexOf("^");
            }
        }
        else if ( (newCurrLine.length() != 0)
                  && (newCurrLine.charAt(0) == '}')
                  || (newCurrLine.startsWith("-->")))
        {
            numSpaces = 0;
        }
        // already returned if prevLine == null
        else if (prevLine.trim().length() != 0)
        {
            prevLine = cropComments(prevLine);  // omit comments from the end of the string
            lastChar = prevLine.charAt(prevLine.length() - 1);

            if(prevLine.startsWith("sp") || (prevLine.endsWith("-->")) )
            {
                numSpaces = 3;
            }
            else if(newCurrLine.startsWith("^") || newCurrLine.startsWith("-^"))
            {
                String currentLine = prevLine;
                int currentElementIndex = elemIndex;
                boolean done = false;
                numSpaces = 0;
                while(!done && currentLine != null)
                {
                    int upPos = currentLine.indexOf('^');
                    if(upPos != -1)
                    {
                        numSpaces = upPos;
                        done = true;
                    }
                    else
                    {
                        --currentElementIndex;
                        currentLine = getElementString(currentElementIndex,data);
                    }
                }
                // if couldn't find a previous '^', set numspaces to end of last line
                if(!done)
                {
                    prevLine = cropComments(prevLine);
                    int firstCharLoc = prevLine.indexOf( (prevLine.trim()).charAt(0) );
                    numSpaces = prevLine.trim().length() + firstCharLoc;
                }
            }
            else if (newCurrLine.startsWith("<"))
            {
                String currentLine = prevLine;
                int currentElementIndex = elemIndex;
                boolean done = false;
                numSpaces = 0;
                while(!done && currentLine != null)
                {
                    int upPos = currentLine.indexOf('<');
                    if(upPos != -1)
                    {
                        numSpaces = upPos;
                        done = true;
                    }
                    else
                    {
                        --currentElementIndex;
                        currentLine = getElementString(currentElementIndex,data);
                    }
                }
                // if couldn't find a previous '<', set numspaces to end of last line
                if(!done)
                {
                    prevLine = cropComments(prevLine);
                    int firstCharLoc = prevLine.indexOf( (prevLine.trim()).charAt(0) );
                    numSpaces = prevLine.trim().length() + firstCharLoc;
                }
            }
            else if (lastChar == ')')
            {
                int currentElementIndex = elemIndex - 1;
                boolean done = false;
                numSpaces = 0;
                int count = 0;
                String currentLine = prevLine;

                while( (!done && currentLine != null)
                       && (currentElementIndex > -1) )
                {
                    for(int i = currentLine.length() - 1; i >=0; --i)
                    {
                        if(currentLine.charAt(i) == ')')
                        {
                            ++count;
                        }
                        else if(currentLine.charAt(i) == '(')
                        {
                            --count;
                            if(count <= 0)
                            {
                                numSpaces = i;
                                done = true;
                                break;
                            }
                        }
                    }
                    --currentElementIndex;

                    // Get the last previous line of valid Soar code
                    boolean soarLine = false;
                    while((currentElementIndex > -1) && !soarLine)
                    {
                        currentLine = getElementString(currentElementIndex, data);
                        if( (currentLine == null)
                            || (currentLine.trim().startsWith("#"))
                            || (currentLine.length() == 0) )
                        {
                            currentElementIndex--;
                        }
                        else
                        {
                            soarLine = true;
                        }
                    }

                }   // while finding the matching parentheses
            }   // end of else if last char == ')'
            else if (lastChar == '{')
            {
                numSpaces = prevLine.indexOf('{');
            }
            else if ( ( newCurrLine.startsWith("(")
                        && ((lastChar == ')')
                            || (lastChar == '}')) ) )
            {
                numSpaces = 3;
            }
            else if (prevLineIndex >= 0)
            {
                // look for a ^ on previous line
                String fullPrevLine = getElementString(prevLineIndex, data);  // get the full previous line
                int caretLocation = fullPrevLine.indexOf('^');
                if(caretLocation != -1)
                {
                    // Get position past string that follows the caret (ie ^string )
                    while( (fullPrevLine.charAt(caretLocation) != ' ')
                           && (caretLocation < fullPrevLine.length()) )
                    {
                        caretLocation++;
                    }
                    // look to see if string past ^string is << or { and if so, use that position
                    while( (fullPrevLine.charAt(caretLocation) == ' ')
                           && (caretLocation < fullPrevLine.length()) )
                    {
                        caretLocation++;
                    }
                    if(fullPrevLine.charAt(caretLocation) == '{')
                    {
                        numSpaces = caretLocation + 2;
                    }
                    else if( (fullPrevLine.charAt(caretLocation) == '<')
                             && (fullPrevLine.charAt(caretLocation + 1) == '<') )
                    {
                        numSpaces = caretLocation + 3;
                    }
                    else
                    {
                        numSpaces = caretLocation;
                    }
                } // end of prevLine contains a '^' and thus currLine considered
                  // a value
                else
                {
                    // else line up with previous line
                    numSpaces = fullPrevLine.indexOf( (fullPrevLine.trim()).charAt(0));
                }
            }    // end of else if prevlineindex > 1
            else
            {
                // First line of re
                numSpaces = 0;
            } // Does not fit a previous constraint and therefore is possibly
              // considered a VALUE of a wme
        }
        // else { numSpaces = 0; } , already initialized

        // properly line up negated conditions
        if ((newCurrLine.length() != 0) && (newCurrLine.charAt(0) == '-') &&
            (numSpaces > 0))
        {

            numSpaces--;
        }

        // no line up char found, or coincidental 3 space indent
        if ((numSpaces == 3) || (numSpaces < 0))
        {
            indentString = "   ";
            numSpaces = 3;
        }
        // variable indent to line up chars vertically
        else if (numSpaces > 0)
        {
            indentChars = new char[numSpaces];

            for (int i = 0; i < numSpaces; i++)
            {
                indentChars[i] = ' ';
            }

            indentString = new String(indentChars);
        }
        // no indent for normal chars, or coincidental 0 space indent

        newCurrLine = indentString + newCurrLine;

        if (! newCurrLine.equals(currLine)) { // not already justified
            replaceRange(newCurrLine, currLineBegin, currLineEnd);
        }
        
        /* find out if if the caret is left of all the text on the line 
         * check for non-whitespace left of the caret
         */
        for (int i = caretPos - currLineBegin - 1; (i >= 0) && leftOfText; i--)
        {
            if (! Character.isWhitespace(currLine.charAt(i)))
            {
                leftOfText = false;
            }
        }
    
        if (leftOfText)
        {
            return (currLineBegin + numSpaces);
        }
        else { // caret is in the middle, or after text
            return (newCurrLine.lastIndexOf(afterCaretString)
                    + currLineBegin);
        }
        
    } // autoJustify()



    /**
     * Function removes any trailing comments from the end of a string.
     * Comments are defined as anything following a '#', ";#", "; #" or ";  #"
     * Note:  the use of semicolons denoting comments is no longer used in Soar, but
     * still supported.
     * @param prevLine the string that is to be cropped.
     */
    public String cropComments(String prevLine)
    {
        // omit comments from the end of the previous line for testing
        if(!prevLine.startsWith("#"))
        {
            if(prevLine.indexOf(";#") != -1)
            {
                prevLine = prevLine.substring(0, prevLine.indexOf(";#") - 1);
            }
            else if(prevLine.indexOf("; #") != -1)
            {
                prevLine = prevLine.substring(0, prevLine.indexOf("; #") - 1);
            }
            else if(prevLine.indexOf(";  #") != -1)
            {
                prevLine = prevLine.substring(0, prevLine.indexOf(";  #") - 1);
            }
            else if(prevLine.indexOf('#') != -1)
            {
                prevLine = prevLine.substring(0, prevLine.indexOf('#') - 1);
            }
        }
        return prevLine;
    }
} // class SoarDocument
