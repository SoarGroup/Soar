/**
 *
 * @file SoarAutoIndentStrategy.java
 * @date Feb 24, 2004
 */
package edu.rosehulman.soar.editor;

import org.eclipse.jface.text.*;

/**
 * Auto-indents soar code.
 * 
 * @author Tim Jasko
 */
public class SoarAutoIndentStrategy implements IAutoIndentStrategy {


	public void customizeDocumentCommand( IDocument doc, DocumentCommand comm) {
		try {
			
			
			
			String prevCodeLine = getPrevCodeLine(doc, comm.offset); 
			
			//this is the first line; we have nothing to do
			if (prevCodeLine == "") {
				return;
			}
			
			String prevDocText = doc.get(0, comm.offset);
			char prevLastChar = prevCodeLine.charAt(prevCodeLine.length() - 1);
			
			int currLineNumber = doc.getLineOfOffset(comm.offset);
			int currLineOffset = doc.getLineOffset(currLineNumber);
			
			String currLine = getLine(doc, doc.getLineOfOffset(comm.offset));
			
			int insertion = comm.offset - currLineOffset;
			String temp = currLine.substring(0, insertion) + comm.text;
			
			if (insertion < currLine.length()) {
				temp += currLine.substring(insertion+1);
			} 
			
			currLine = temp;
			
			String newCurrLine = trimLeading(currLine);
			
			
			
			int numSpaces = 0;
			String indentString = "";
			
			/*System.out.println("*************************************");
			System.out.println("text: '" + comm.text + "'");
			System.out.println("prevCodeLine: '" + prevCodeLine + "'");
			System.out.println("prevLastChar: '" + prevLastChar + "'");
			System.out.println("currLine: '" + currLine + "'");
			System.out.println("currLineNumber: '" + currLineNumber + "'");
			System.out.println("currLineOffset: '" + currLineOffset + "'");
			System.out.println("newCurrLine: '" + newCurrLine + "'"); */
			
			//If there's nothing here, there's nothing to do
			if( (newCurrLine.length() == 0)) {
				return;
			
			} else if ( newCurrLine.charAt(0) == '}' || newCurrLine.startsWith("-->")
				|| newCurrLine.startsWith("sp")) {
					
				numSpaces = 0;
			
			} else {
				//indent by 3 if the previous line was "sp" or "-->" 
				if(prevCodeLine.startsWith("sp") || (prevCodeLine.endsWith("-->")) ) {
					numSpaces = 3;
					
				
				//Deal with "^" and "-^"
				} else if(newCurrLine.startsWith("^") || newCurrLine.startsWith("-^")) {
					
					
					int caretPos =
						prevDocText.substring(0, currLineOffset).lastIndexOf("^");
					
					// indent to the same place as the previous carret
					if (caretPos != -1) {
						int caretLine = doc.getLineOfOffset(caretPos);
						numSpaces = caretPos - doc.getLineOffset(caretLine);
					
					// if couldn't find a previous '^', set numspaces to end of last line
					} else {
						int firstCharLoc = prevCodeLine.indexOf(
							(prevCodeLine.trim()).charAt(0) );
						
						numSpaces = prevCodeLine.trim().length() + firstCharLoc;
					}
					
				
				//Deal with an ending ")"
				} else if (prevLastChar == ')') {
					int workingOffset = comm.offset - 1;
					boolean done = false;
					int parenCount = 0;
					
					
					while (!done && workingOffset > -1) {
						if (prevDocText.charAt(workingOffset) == ')') {
							++parenCount;
						} else if (prevDocText.charAt(workingOffset) == '(') {
							--parenCount;
							
							if (parenCount <= 0) {
								
								int lineOff = doc.getLineOffset(
									doc.getLineOfOffset(workingOffset));
								
								numSpaces = workingOffset - lineOff;
								done = true;
							}
						}
						
						--workingOffset;
					}
					
				
				} else if (prevLastChar == '{') {
					numSpaces = prevCodeLine.indexOf('{');
				
				
				} else if (newCurrLine.startsWith("(")
					&& ((prevLastChar == ')') || (prevLastChar == '}'))) {
					 numSpaces = 3;
				
				
				} else if (currLineNumber >= 0) {
					// look for a ^ on previous line
					int caretLocation = prevCodeLine.indexOf('^');
					
					if(caretLocation != -1) {
						// Get position past string that follows the caret (ie ^string )
						while((prevCodeLine.charAt(caretLocation) != ' ')
							&& (caretLocation < prevCodeLine.length())) {
							
							caretLocation++;
						}
						// look to see if string past ^string is << or { and if so, use that position
						while(prevCodeLine.charAt(caretLocation) == ' '
							&& (caretLocation < prevCodeLine.length()) ) {
							
							caretLocation++;
						}
						
						if(prevCodeLine.charAt(caretLocation) == '{') {
							numSpaces = caretLocation + 2;
						
						} else if( (prevCodeLine.charAt(caretLocation) == '<')
							&& (prevCodeLine.charAt(caretLocation + 1) == '<')) {
								
							numSpaces = caretLocation + 3;
							
						} else {
							numSpaces = caretLocation;
						}
					}  // end of prevLine contains a '^' and thus currLine considered a value
					
					// else line up with previous line
					else {
						numSpaces = prevCodeLine.indexOf( (prevCodeLine.trim()).charAt(0));
					}
					
				}    // end of else if prevlineindex > 1
				else {    // First line of re
					numSpaces = 0;
				} // Does not fit a previous constraint and therefore is possibly considered a VALUE of a wme
			}
			
			// properly line up negated conditions
			if ((newCurrLine.length() != 0) && (newCurrLine.charAt(0) == '-') &&
				(numSpaces > 0)) {

				numSpaces--;
			}
			
			// no line up char found
			/*if (numSpaces < 0) {
				numSpaces = 3;
			} */
			
			// variable indent to line up chars vertically
			for (int i = 0; i < numSpaces; i++) {
				indentString += ' ';
			}

			
			
			newCurrLine = indentString + newCurrLine;
			
			//System.out.println("newCurrLine: '" + newCurrLine + "'");
			

			if (! newCurrLine.equals(currLine)) { // not already justified
				System.out.println("replacing: '" +
					doc.get(currLineOffset, doc.getLineLength(currLineNumber)) + "'");
				doc.replace(currLineOffset, doc.getLineLength(currLineNumber),
					newCurrLine);
				
				comm.offset += numSpaces;
				
				if (Character.isWhitespace(comm.text.charAt(comm.text.length()-1))) {
					System.out.println("is whitespace");
					doc.replace(currLineOffset + newCurrLine.length(), 0, comm.text);
				} else {
					System.out.println("not whitespace");
				}
				
				comm.offset += comm.text.length();
				comm.text = "";
			}
			
			
		} catch (BadLocationException e) {
			e.printStackTrace();
		} catch (Exception e) {
			System.out.println("mysterious exception");
			e.printStackTrace();
		}
		

	} // void customizeDocumentCommand( ... )
	
	

	

	
	/*
	//The preceeding function was an Eclipse translation of the following
	// Visual Soar algorithm, reproduced below for when things go horribly wrong
	
	public int autoJustify(int caretPos, IDocument doc) {
		
		Content 			data = getContent();

		String				prevLine = null;
		String				currLine = null;
		String				afterCaretString = null;
		String				newCurrLine = null;
		String				indentString = "";
		
		            
		int 				elemIndex = root.getElementIndex(caretPos);
		int         prevLineIndex = elemIndex - 1;
		int					numSpaces = 0;
		int					currLineBegin;
		int					currLineEnd;
		
		
		char				lastChar;
		char[]				indentChars;
		
		boolean				leftOfText = true;
		
		AbstractElement 	currLineElem;


		currLineElem = (AbstractElement)root.getElement(elemIndex);
		currLineBegin = currLineElem.getStartOffset();
		currLineEnd = currLineElem.getEndOffset() - 1;
		currLine = getElementString(currLineElem, data);
		afterCaretString = getElementString(currLineElem, data, caretPos);


		// Gets the last line of Soar code (skips blanks lines and comment lines)
		if (elemIndex > 0) {
			boolean goodLine = false;
			while ((prevLineIndex > -1) && !goodLine) {
				prevLine = getElementString(prevLineIndex, data);
				
				if((prevLine == null) || prevLine.trim().startsWith("#") || (prevLine.trim().length() == 0)) {
					prevLineIndex--;
				}
				else {
					goodLine = true;
				}
			}   // end of while getting last previous line of Soar code
		}
		else { // no indent for 1st line
			return -1;
		}
		if (prevLine == null) {
			return -1;
		}
		
		newCurrLine = currLine.trim();

		if( (newCurrLine.length() == 0)) {
			return -1;
		}
		else if ((newCurrLine.length() != 0) &&
			(newCurrLine.charAt(0) == '}') || (newCurrLine.startsWith("-->"))) {
			numSpaces = 0;
		}
		// already returned if prevLine == null
		else if (prevLine.trim().length() != 0) {
			prevLine = cropComments(prevLine);  // omit comments from the end of the string
			lastChar = prevLine.charAt(prevLine.length() - 1);

			if(prevLine.startsWith("sp") || (prevLine.endsWith("-->")) ) {
				numSpaces = 3;
			}
			else if(newCurrLine.startsWith("^") || newCurrLine.startsWith("-^")) {
				String currentLine = prevLine;
				int currentElementIndex = elemIndex;
				boolean done = false;
				numSpaces = 0;
				while(!done && currentLine != null) {
					int upPos = currentLine.indexOf('^');
					if(upPos != -1) {
						numSpaces = upPos;
						done = true;
					}
					else {
						--currentElementIndex;
						currentLine = getElementString(currentElementIndex,data);
					}
				}
				// if couldn't find a previous '^', set numspaces to end of last line
				if(!done) {
					prevLine = cropComments(prevLine);
					int firstCharLoc = prevLine.indexOf( (prevLine.trim()).charAt(0) );
					numSpaces = prevLine.trim().length() + firstCharLoc;
				}
			}
			else if (lastChar == ')') {
				int currentElementIndex = elemIndex - 1;
				boolean done = false;
				numSpaces = 0;
				int count = 0;
				String currentLine = prevLine;

				while(!done && currentLine != null && (currentElementIndex > -1)) {
					for(int i = currentLine.length() - 1; i >=0; --i) {
						if(currentLine.charAt(i) == ')') {
							++count;
						}
						else if(currentLine.charAt(i) == '(') {
							--count;
							if(count <= 0) {
								numSpaces = i;
								done = true;
								break;
							}
						}
					}
					--currentElementIndex;

					// Get the last previous line of valid Soar code
					boolean soarLine = false;
					while((currentElementIndex > -1) && !soarLine) {
						currentLine = getElementString(currentElementIndex, data);
						if((currentLine == null) || (currentLine.trim().startsWith("#")) || (currentLine.length() == 0)) {
						  currentElementIndex--;
						}
						else {
						  soarLine = true;
						}
					}

				}   // while finding the matching parentheses
			}   // end of else if last char == ')'
			else if (lastChar == '{') {
				numSpaces = prevLine.indexOf('{');
			}
			else if (newCurrLine.startsWith("(")  && ((lastChar == ')') || (lastChar == '}'))) {
				 numSpaces = 3;
			}
			else if (prevLineIndex >= 0) {
				// look for a ^ on previous line
				String fullPrevLine = getElementString(prevLineIndex, data);  // get the full previous line
				int caretLocation = fullPrevLine.indexOf('^');
				if(caretLocation != -1) {
					// Get position past string that follows the caret (ie ^string )
					while((fullPrevLine.charAt(caretLocation) != ' ') && (caretLocation < fullPrevLine.length())) {
						caretLocation++;
					}
					// look to see if string past ^string is << or { and if so, use that position
					while(fullPrevLine.charAt(caretLocation) == ' ' && (caretLocation < fullPrevLine.length()) ) {
						caretLocation++;
					}
					if(fullPrevLine.charAt(caretLocation) == '{') {
						numSpaces = caretLocation + 2;
					}
					else if( (fullPrevLine.charAt(caretLocation) == '<') && (fullPrevLine.charAt(caretLocation + 1) == '<')) {
						numSpaces = caretLocation + 3;
					}
					else {
						numSpaces = caretLocation;
					}
				}  // end of prevLine contains a '^' and thus currLine considered a value
				else {
					// else line up with previous line
					numSpaces = fullPrevLine.indexOf( (fullPrevLine.trim()).charAt(0));
				}
				
			}    // end of else if prevlineindex > 1
			else {    // First line of re
				numSpaces = 0;
			}     //  Does not fit a previous constraint and therefore is possibly considered a VALUE of a wme
		} 
		// else { numSpaces = 0; } , already initialized

		// properly line up negated conditions
		if ((newCurrLine.length() != 0) && (newCurrLine.charAt(0) == '-') &&
			(numSpaces > 0)) {

			numSpaces--;
		}

		// no line up char found, or coincidental 3 space indent
		if ((numSpaces == 3) || (numSpaces < 0)) {
			indentString = "   ";
			numSpaces = 3;
		}
		// variable indent to line up chars vertically
		else if (numSpaces > 0) {
			indentChars = new char[numSpaces];

			for (int i = 0; i < numSpaces; i++) {
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
		 */ /*
		for (int i = caretPos - currLineBegin - 1; (i >= 0) && leftOfText; i--) {
			if (! Character.isWhitespace(currLine.charAt(i))) {
				leftOfText = false;
			}
		}
	
		if (leftOfText) {
			return (currLineBegin + numSpaces);
		}
		else { // caret is in the middle, or after text
			return (newCurrLine.lastIndexOf(afterCaretString)
											+ currLineBegin);
		}
		
	} // autoJustify() */



	/**
	 * Function removes any trailing comments from the end of a string.
	 * Comments are defined as anything following a '#', ";#", "; #" or ";  #"
	 * Note:  the use of semicolons denoting comments is no longer used in Soar, but
	 * still supported.
	 * @param prevLine the string that is to be cropped.
	 */
	public String cropComments(String prevLine) {
		// omit comments from the end of the previous line for testing
		if(!prevLine.startsWith("#")) {
			if(prevLine.indexOf(";#") != -1) {
				prevLine = prevLine.substring(0, prevLine.indexOf(";#") - 1);
			}
			else if(prevLine.indexOf("; #") != -1) {
				prevLine = prevLine.substring(0, prevLine.indexOf("; #") - 1);
			}
			else if(prevLine.indexOf(";  #") != -1) {
				prevLine = prevLine.substring(0, prevLine.indexOf(";  #") - 1);
			}
			else if(prevLine.indexOf('#') != -1) {
				prevLine = prevLine.substring(0, prevLine.indexOf('#') - 1);
			}
		}
		return prevLine;
	} // String cropComments(String prevLine)
	
	
	/**
	 * Trims the leading whitespace from a string, leaving any whitespace at the end.
	 * @param victim The string to trim.
	 * @return The string with leading whitespace removed.
	 */
	private String trimLeading(String victim) {
		int i=0;
		
		for (i=0; i<victim.length(); i++) {
			if (! Character.isWhitespace(victim.charAt(i))) {
				break;
			}
		}
		
		return victim.substring(i);
	}


	/**
	 * Returns the previous line of code, ignoring blank or comment lines.
	 * 
	 * @param doc The document containing the code.
	 * @param current The current offset (<i>not</i> the current line of code).
	 * @return The previous line of code.
	 */
	private String getPrevCodeLine( IDocument doc, int current) {
		try {
			int currLine = doc.getLineOfOffset(current);
			
			for (int lineNum = currLine-1; lineNum >= 0; lineNum--) {
				
				String line = getLine(doc, lineNum);
				if (! ( line.equals("") 
					|| line.trim().startsWith("#") || line.trim().length() == 0) ) {
					
					return cropComments(line);
				}
			} // for lineNum--
			
			return "";
			
		} catch (BadLocationException e) {
			return "";
		}
		
	} // String getPrevCodeLine( ... )
	
	
	/**
	 * Returns the line number of the previous line of code. Ignores blank
	 *  or comment lines.
	 * 
	 * @param doc The document containing the code.
	 * @param current The current offset (<i>not</i> the current line of code). 
	 * @return The number of the previous line of code.
	 */
	private int getPrevCodeLineNumber(IDocument doc, int current) {
		try {
			int currLine = doc.getLineOfOffset(current);
			
			for (int lineNum = currLine-1; lineNum >= 0; lineNum--) {
				
				String line = getLine(doc, lineNum);
				if (! ( line.equals("") 
					|| line.trim().startsWith("#") || line.trim().length() == 0) ) {
					
					return lineNum;
				}
			} // for lineNum--
			
			return -1;
			
		} catch (BadLocationException e) {
			return -1;
		}
		
	} // String getPrevCodeLineNumber( ... )
	
	/**
	 * Gets the text of the given line.
	 * 
	 * @param doc The document containing the current line.
	 * @param lineNum The line number.
	 * @return The given line.
	 */
	private String getLine(IDocument doc, int lineNum) {
		try {
			IRegion reg = doc.getLineInformation(lineNum);
			
			return doc.get(reg.getOffset(), reg.getLength());
			
		} catch (BadLocationException e) {
			return "";
		}
	}
	



	/*
	System.out.println( "customizeDocumentCommand( ... ): '" + comm.text + "'" );
		
	try {
		IRegion lineReg = doc.getLineInformationOfOffset(comm.offset);
			
		if (comm.text.indexOf("\n") != -1 ) {
			System.out.println("newline");
		}
			
		//Indent the '(' if it starts the line
		if (comm.text.equals("(") && comm.offset == lineReg.getOffset()) {
			comm.text = "   (";
			
		//Indent that ^
		} else if (comm.text.equals("^")) {
				
		}
			
	} catch (Exception e) {
		e.printStackTrace();
	} */

} // class
