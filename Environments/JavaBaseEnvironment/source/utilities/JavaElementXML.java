/** 
 * ElementXML.java
 *
 * Description:		Utility class for reading and writing XML values.
 * @author			Doug
 * @version			
 */

package utilities;

import java.util.* ;
import java.io.* ;

/************************************************************************
* 
* IMPORTANT NOTES ON THIS CLASS:
* 	This class was written long before SML existed.
*   During the writing of SML, this class was re-implemented in C++ as ElementXML.
*   In general, the sml.ElementXML class should be the class of choice -- it's much more rigorously tested
*   and will be consistently maintained.
*   So why is this class still here?  There's a lot of code in the debugger that uses this class and we just haven't
*   had time to go through and remove all of the uses of this code and use sml.ElementXML instead.
*   To make sure the two aren't muddled up this one has been renamed to JavaElementXML.
* 
* The ElementXML class is a general utility class for reading and writing XML files.
* It represents an XML element, such as (using square brackets instead of
* angled so that javadoc doesn't have a fit): 
* 
* [author genre="fiction"]
*	"tolkien"
*	[book]"the hobbit"[/book]
* [/author]
* 
* The components here are:
* Tag name   - "author"
* Attribute  - "genre"
* Value      - "fiction"
* Contents   - "tolkien"
* Children   - another element: tag "book", contents "the hobbit".
*
* Note that because XML elements can contain other child XML elements
* that one element (and its children) is generally sufficient to represent
* an entire XML file.
*
* To write an XML file, construct the root XML element (and add all of its
* children).  Then call the WriteToFile function.
*
* To read an XML file call the ReadFromFile function, which builds the
* XML root element (and its children) and then returns it.
*
* Of the two, the read function is much more complex.
* It uses a parser, which in turn uses a lexical analyser.
*
* The job of the lexical analyser is to turn the input stream into
* a series of tokens: 
* E.g. Given "[tag att="value"] the tokens would be
* symbol       - "[",
* identifier   - "tag",
* identifier   - "att",
* symbol       - "="
* quotedString - "value"
* symbol	   - "]"
*
* Once the stream is broken into tokens, the parser then consumes them
* to build the XML element.
*
*************************************************************************/
public class JavaElementXML
{
	// Lexical analyser for XML stream.
	private static class LexXML
	{
		// Token types
		public static final int kSymbol       = 1 ;
		public static final int kIdentifier   = 2 ;
		public static final int kQuotedString = 3 ;
		public static final int kEOF		  = 4 ;

		/************************************************************************
		* 
		* The token class represents one element in the input stream.
		* E.g. Given "[tag att="value"] the tokens would be
		* symbol       - "[",
		* identifier   - "tag",
		* identifier   - "att",
		* symbol       - "="
		* quotedString - "value"
		* symbol	   - "]"
		* 
		*************************************************************************/
		private static class Token
		{			
			private String m_Value ;
			private int	   m_Type ;
			
			public Token(String value, int type)
			{
				m_Value = value ;
				m_Type  = type ;
			}

			public int  getType() 		  { return this.m_Type ; }
			public void setType(int type) { this.m_Type = type ; }
			public String getValue()	  { return this.m_Value ; }
			public void setValue(String s){ this.m_Value = s ; }
		}
		
		protected BufferedReader m_Input       = null ;
		protected String		 m_CurrentLine = null ;
		protected int			 m_Pos = 0 ;
		protected Token			 m_CurrentToken = null ;
	
		/************************************************************************
		* 
		* Constructor for the XML lexical analyser.
		* 
		* @param input			The input stream of raw text from the XML file
		* 
		*************************************************************************/
		public LexXML(BufferedReader input) throws Exception
		{
			m_Input = input ;

			// Prime the lexical analyser with initial values
			m_CurrentLine = m_Input.readLine() ;
			
			// For Soar we'll use \n as the universal line separator
			// (Since this is what the kernel always generates it's easier to just standardize on that)
			// This means we need to replace any \r\n combinations Windows inserts with just \n
			m_CurrentLine.replaceAll(kSystemLineSeparator, kLineSeparator) ;
			
			// Add the newline char(s) to the end of the line we read.
			// This generally only matters when reading quoted strings that
			// cover multiple lines.
			m_CurrentLine += kLineSeparator ;
		
			m_Pos = 0 ;
			
			// Read the first token
			GetNextToken() ;
		}
		
		/************************************************************************
		* 
		* Read the next character from the input file.
		*
		* This function reads lines from the input file and inserts the local
		* line break character.  For example, if you're running on Windows reading
		* a Linux file, the Windows line break chars will appear in the character
		* stream even though the original file used Linux line break char.
		* 
		*************************************************************************/
		protected void GetNextChar() throws java.io.IOException
		{
			// When current line is null, we're at the end of the file.
			if (m_CurrentLine == null)
				return ;
		
			// Move the pointer along to the next char in the current input line
			m_Pos++ ;
			
			// If we moved off the end of the current line, move to the next input line.
			if (m_Pos >= m_CurrentLine.length())
			{
				m_CurrentLine = m_Input.readLine() ;
				
				// Add the newline char(s) to the end of the line we read.
				// This generally only matters when reading quoted strings that
				// cover multiple lines.
				m_CurrentLine += kLineSeparator ;
			
				m_Pos = 0 ;
			}
		}
		
		/************************************************************************
		* 
		* Returns the current character from the input stream.
		* 
		*************************************************************************/
		protected char getCurrentChar()
		{
			return m_CurrentLine.charAt(m_Pos) ;
		}
		
		
		/************************************************************************
		* 
		* Returns true if we're at the end of the file.
		* 
		*************************************************************************/
		protected boolean IsEOF()
		{
			return m_CurrentLine == null ;
		}
		
		/************************************************************************
		* 
		* Returns true if the character is white space.
		* 
		*************************************************************************/
		protected boolean IsWhiteSpace(char ch)
		{
			return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') ;
		}
		
		/************************************************************************
		* 
		* Returns true if this is a symbol (i.e. a special char in XML, like "<")
		* 
		*************************************************************************/
		protected boolean IsSymbol(char ch)
		{
			return (ch == kOpenTagChar || ch == kCloseTagChar || ch == kEndMarkerChar ||
				    ch == kHeaderChar  || ch == kEqualsChar) ;
		}
		
		/************************************************************************
		* 
		* Returns true if this is the quote character (")
		* 
		*************************************************************************/
		protected boolean IsQuote(char ch)
		{
			return (ch == '\"') ;
		}
		
		/************************************************************************
		* 
		* Set the value of the current token.
		* 
		*************************************************************************/
		protected void SetCurrentToken(String value, int type)
		{
			m_CurrentToken = new Token(value, type) ;
		}
		
		/************************************************************************
		* 
		* This is the main lexical analyser functions--gets the next
		* token from the input stream (e.g. an identifier, or a symbol etc.)
		* 
		*************************************************************************/
		public void GetNextToken() throws Exception
		{
			// If we're already at EOF when we ask
			// for the next token, that's an error.
			if (IsEOF() && m_CurrentToken.getType() == kEOF)
			{
				throw new Exception("Unexpected end of file when parsing file") ;
			}
			
			// Skip leading white space
			while (!IsEOF() && IsWhiteSpace(getCurrentChar()))
				GetNextChar() ;
			
			// If we're at the end of file, report that as the next token.
			if (IsEOF())
			{
				SetCurrentToken("", kEOF) ;
				return ;
			}
			
			// Symbol token
			if (IsSymbol(getCurrentChar()))
			{
				SetCurrentToken(String.valueOf(getCurrentChar()), kSymbol) ;
				
				// Consume the symbol.
				GetNextChar() ;
				
				return ;
			}
			
			// Quoted string
			if (IsQuote(getCurrentChar()))
			{
				// Buffer we'll use to build up the string
				StringBuffer buffer = new StringBuffer() ;
				
				// Consume the opening quote.
				GetNextChar() ;
				
				while (!IsEOF() && !IsQuote(getCurrentChar()))
				{
					// Add everything up to the next quote.
					buffer.append(getCurrentChar()) ;
					GetNextChar() ;
				}
				
				// Consume the closing quote.
				GetNextChar() ;
				
				SetCurrentToken(buffer.toString(), kQuotedString) ;
				return ;
			}
			
			// Identifier
			StringBuffer identifier = new StringBuffer() ;
			while (!IsEOF() && !IsWhiteSpace(getCurrentChar()) && !IsSymbol(getCurrentChar()) && !IsQuote(getCurrentChar()))
			{
				identifier.append(getCurrentChar()) ;
				GetNextChar() ;
			}
			
			SetCurrentToken(identifier.toString(), kIdentifier) ;
		}
		
		public Token getCurrentToken()
		{
			return this.m_CurrentToken ;
		}
		
		public int getCurrentTokenType()
		{
			return this.m_CurrentToken.getType() ;
		}
		
		public String getCurrentTokenValue()
		{
			return this.m_CurrentToken.getValue() ;
		}
		
		/************************************************************************
		* 
		* Returns true if the current token matches the given type.
		* E.g. if (Have(kSymbol)) { // Process symbol }
		* 
		* @param type			The type to test against.
		* 
		*************************************************************************/
		public boolean Have(int type) throws Exception
		{
			return (m_CurrentToken.getType() == type) ;
		}
		
		/************************************************************************
		* 
		* Returns true AND consumes the current token, if the values match.
		* We can consume the token, because the parser already knows its value.
		* E.g. if (Have("<")) { // Parse what comes after "<" }
		* 
		* @param value			The string value to test
		* 
		* @return True if value matches current token.
		* 
		*************************************************************************/
		public boolean Have(String value) throws Exception
		{
			if (m_CurrentToken.getValue().equals(value))
			{
				GetNextToken() ;
				return true ;
			}
			return false ;
		}
		
		/************************************************************************
		* 
		* Returns true AND consumes the current token if the value and type match.
		* 
		* @param value			The value to test
		* @param type			The type to test
		* 
		*************************************************************************/
		public boolean Have(String value, int type) throws Exception
		{
			if (m_CurrentToken.getType() == type)
				return Have(value) ;
			return false ;
		}
		
		/************************************************************************
		* 
		* Checks that the current token matches the given value.
		* If not, throws an exception.
		* Used for places in the parse when you know what must come next.
		* E.g. At the end of an XML token : MustBe("/") ; MustBe(">") ;
		* 
		* @param value			The value to test
		* 
		*************************************************************************/
		public void MustBe(String value) throws Exception
		{
			if (!m_CurrentToken.getValue().equals(value))
			{			
				throw new Exception("Looking for " + value + " instead found " + m_CurrentToken.getValue()) ;
			}
			
			GetNextToken() ;
		}
		
		/************************************************************************
		* 
		* Checks that the current token matches the given type.
		* If it does, returns the value (this is often useful when testing
		* for identifiers).
		* If it does not match, throws an exception.
		* 
		* @param type			The type to test (e.g. kSymbol)
		* 
		* @return The value of the current token (if matches type).
		* 
		*************************************************************************/
		public String MustBe(int type) throws Exception
		{
			if (m_CurrentToken.getType() != type)
			{
				throw new Exception("Found incorrect type when parsing token " + m_CurrentToken.getValue()) ;
			}
			
			String result = m_CurrentToken.getValue() ;
			
			GetNextToken() ;
			
			return result ;
		}
		
		/************************************************************************
		* 
		* Checks that both the type and value match.
		* If not, throws an exception.
		* 
		*************************************************************************/
		public void MustBe(String value, int type) throws Exception
		{
			if (m_CurrentToken.getType() != type)
			{
				throw new Exception("Found incorrect type when parsing token " + m_CurrentToken.getValue()) ;
			}
			MustBe(value) ;
		}
	}

	// Some useful attribute names
	public static final String kClassAttribute   = "Class" ;
	public static final String kVersionAttribute = "Version" ;

	protected static final char 	kOpenTagChar    = '<' ;
	protected static final String 	kOpenTagString  = "<" ;
	protected static final char 	kCloseTagChar   = '>' ;
	protected static final String 	kCloseTagString = ">" ;
	protected static final char 	kEndMarkerChar  = '/' ;
	protected static final String	kEndMarkerString="/" ;
	protected static final char 	kHeaderChar 	= '?' ;
	protected static final String	kHeaderString 	= "?" ;
	protected static final char 	kEqualsChar 	= '=' ;
	protected static final String	kEqualsString	= "=" ;
	protected static final String   kLineSeparator = "\n" ;	// For Soar it's better for us to agree on using \n for newlines
	protected static final String   kSystemLineSeparator = System.getProperty("line.separator") ;

	/** List of attribute names and string values */
	protected HashMap m_AttributeList = new HashMap() ;
	
	/** List of elements contained in this element */
	protected ArrayList m_ChildElementList = new ArrayList() ;
	
	/** The tag for this item */
	protected String	m_TagName = null ;
	
	/** The content, lies between the tag start and end [tag]"content"[/tag].  Always quoted in file. */
	protected String	m_Contents = null ;
	
	/** Sometimes its helpful to keep a reference to the object used to create this element.  This is essentially "user data" for the client of this class */
	protected Object	m_Source = null ;
	
	/** The parent node for this element (i.e. the inverse of the child relationship) */
	protected JavaElementXML m_Parent = null ;
	
	/************************************************************************
	* 
	* Constructor -- builds the new XML element and assigns it a tag name.
	* 
	* @param tagName		The name of the tag for this element
	* 
	*************************************************************************/
	public JavaElementXML(String tagName)
	{
		// Check there are no spaces in the tag name -- spaces make it hard to parse when reading the file.
		if (tagName.indexOf(" ") != -1)
			throw new Error("Not allowed tag names with spaces inside them -- at least not for my form of XML") ;

		this.m_TagName = tagName ;
	}

	/** The parent node for this element (i.e. the inverse of the child relationship) */
	public JavaElementXML getParent() { return this.m_Parent ; }

	/** The parent node for this element (i.e. the inverse of the child relationship) */
	public void		  setParent(JavaElementXML element) { this.m_Parent = element ; }
		
	/************************************************************************
	* 
	* These convert invalid XML chars to XML escape sequences and back again
	* (e.g. "<" => "&lt;" and so on.
	* 
	*************************************************************************/	
	protected static String convertToEscapes(String src)
	{
		/*
		static char const* kLT   = "&lt;" ;
		static char const* kGT   = "&gt;" ;
		static char const* kAMP  = "&amp;" ;
		static char const* kQUOT = "&quot;" ;
		static char const* kAPOS = "&apos;" ;
		*/
		
		StringBuffer xmlSafe = new StringBuffer() ;

		int len = src.length() ;
		for (int i = 0 ; i < len ; i++)
		{
			char c = src.charAt(i) ;
			
			switch (c)
			{
				case '<' : xmlSafe.append("&lt;") ; break ;
				case '>' : xmlSafe.append("&gt;") ; break ;
				case '&' : xmlSafe.append("&amp;") ; break ;
				case '"' : xmlSafe.append("&quot;") ; break ;
				case '\'': xmlSafe.append("&apos;") ; break ;
				default: xmlSafe.append(c) ;
			}
		}
		
		return xmlSafe.toString() ;
	}
	
	protected static String convertFromEscapes(String src)
	{
		/*
		static char const* kLT   = "&lt;" ;
		static char const* kGT   = "&gt;" ;
		static char const* kAMP  = "&amp;" ;
		static char const* kQUOT = "&quot;" ;
		static char const* kAPOS = "&apos;" ;
		*/

		StringBuffer orig = new StringBuffer() ;
		
		int len = src.length() ;
		for (int i = 0 ; i < len ; i++)
		{
			char c = src.charAt(i) ;
			
			// Any raw & chars must be start of escape sequences
			if (c == '&' && i < len-2)
			{
				char next1 = src.charAt(i+1) ;
				char next2 = src.charAt(i+2) ;
				switch (next1)
				{
					case 'l' : i += 3 ; orig.append("<") ; break ;
					case 'g' : i += 3 ; orig.append(">") ; break ;
					case 'q' : i += 5 ; orig.append('"') ; break ;
					case 'a' : if (next2 == 'm') { i += 4 ; orig.append("&") ; }
							   else { i += 5 ; orig.append("\'") ; } ;
							   break ;
				}
			}
			else
			{
				orig.append(c) ;
			}
		}
		
		return orig.toString() ;
	}
	
	/************************************************************************
	* 
	* Constructor -- builds the new XML element and assigns it a tag name
	* and initial contents.  Useful for simple tags.
	* 
	* @param tagName		The name of the tag for this element
	* @param contents		The body of this tag ([tag]"contents"[/tag])
	* 
	*************************************************************************/
	public JavaElementXML(String tagName, String contents)
	{
		this(tagName) ;
		
		this.addContents(contents) ;
	}
	
	/************************************************************************
	* 
	* Add a string to the "contents" of the XML element.  The contents is
	* the part falling between the opening and closing tag.
	* E.g. <tag>contents</tag>
	* The contents can be multi-line, but cannot contain any embedded quotes.
	* 
	* @param contents		The string to add to the current contents.
	* 
	*************************************************************************/
	public void addContents(String contents)
	{
		// Check that we're not adding a string that contains an
		// embedded quote character--it would throw us off.
		// BADBAD: I guess we could use an escape sequence, blah, blah, blah.
		// But basically it's a pain to handle.
		int quote = contents.indexOf('\"') ;
		if (quote != -1)
		{
			//Debug.println("Tried to add a contents string with an embedded string character") ;
			return ;
		}
		
		// If we have no existing contents string,
		// just replace it, otherwise add to the existing one.
		if (this.m_Contents == null)
			this.m_Contents = contents ;
		else
			this.m_Contents += contents ;
	}
	
	/************************************************************************
	* 
	* Gets the contents of this XML element.
	* 
	*************************************************************************/
	public String getContents()
	{
		return this.m_Contents ;
	}

	/************************************************************************
	* 
	* Get and set the "source" object -- whose use is up to the client (not required at all)
	* 
	*************************************************************************/
	public Object getSource() { return this.m_Source ; }
	public void   setSource(Object obj) { this.m_Source = obj ; }
	
	/************************************************************************
	* 
	* Returns a Hashtable, mapping from attribute name to attribute value.
	* (String to String).
	* 
	*************************************************************************/
	public HashMap getAttributeMap()
	{
		return this.m_AttributeList ;
	}
	
	/************************************************************************
	* 
	* Returns the value of a named attribute.  The value is always a string.
	* 
	*************************************************************************/
	public String getAttribute(String name)
	{
		return (String)this.m_AttributeList.get(name) ;
	}
	
	/************************************************************************
	* 
	* Returns the value of a named attribute.  The value is always a string.
	* This version throws an exception if the attribute is missing.
	* 
	*************************************************************************/
	public String getAttributeThrows(String name) throws Exception
	{
		String value = (String)this.m_AttributeList.get(name) ;
		
		if (value == null)
			throw new Exception("Could not find attribute " + name + " while parsing XML document") ;
			
		return value ;
	}
	
	/************************************************************************
	* 
	* Returns the value of a named attribute, interpreting it as an double.
	* If the attribute is missing or cannot be parsed as an int, this function
	* throws an exception.
	* 
	*************************************************************************/
	public double getAttributeDoubleThrows(String name) throws Exception
	{
		String val = getAttributeThrows(name) ;
		
		double doubleVal = Double.parseDouble(val) ;
		
		return doubleVal ;
	}
	
	/************************************************************************
	* 
	* Returns the value of a named attribute, interpreting it as an double.
	* If the attribute is missing returns the defaultValue.
	* If the attribute is present but cannot be parsed as an int this throws.
	* 
	*************************************************************************/
	public double getAttributeDoubleDefault(String name, double defaultValue) throws Exception
	{
		String val = this.getAttribute(name) ;
		
		if (val == null)
			return defaultValue ;
		
		double doubleVal = Double.parseDouble(val) ;
		
		return doubleVal ;
	}
	
	/************************************************************************
	* 
	* Returns the value of a named attribute, interpreting it as an int.
	* If the attribute is missing or cannot be parsed as an int, this function
	* throws an exception.
	* 
	*************************************************************************/
	public int getAttributeIntThrows(String name) throws Exception
	{
		String val = getAttributeThrows(name) ;
		
		int intVal = Integer.parseInt(val) ;
		
		return intVal ;
	}
	
	/************************************************************************
	* 
	* Returns the value of a named attribute, interpreting it as an int.
	* If the attribute is missing returns the defaultValue.
	* If the attribute is present but cannot be parsed as an int this throws.
	* 
	*************************************************************************/
	public int getAttributeIntDefault(String name, int defaultValue) throws Exception
	{
		String val = this.getAttribute(name) ;
		
		if (val == null)
			return defaultValue ;
		
		int intVal = Integer.parseInt(val) ;
		
		return intVal ;
	}

	/************************************************************************
	* 
	* Returns the value of a named attribute, interpreting it as a long.
	* If the attribute is missing or cannot be parsed as a long, this function
	* throws an exception.
	* 
	*************************************************************************/
	public long getAttributeLongThrows(String name) throws Exception
	{
		String val = getAttributeThrows(name) ;
		
		long longVal = Long.parseLong(val) ;
		
		return longVal ;
	}
	
	/************************************************************************
	* 
	* Returns the value of a named attribute, interpreting it as a long.
	* If the attribute is missing returns the defaultValue.
	* If the attribute is present but cannot be parsed as an long this throws
	* 
	*************************************************************************/
	public long getAttributeLongDefault(String name, long defaultValue) throws Exception
	{
		String val = this.getAttribute(name) ;
		
		if (val == null)
			return defaultValue ;
		
		long longVal = Long.parseLong(val) ;
		
		return longVal ;
	}

	/************************************************************************
	* 
	* Returns the value of a named attribute, interpreting it as an boolean.
	* If the attribute is missing or cannot be parsed as a bool, this function
	* returns the default value without throwing.
	* 
	*************************************************************************/
	public boolean getAttributeBooleanDefault(String name, boolean defaultValue)
	{
		String val = getAttribute(name) ;
		
		if (val == null)
			return defaultValue ;
		
		if (val.equalsIgnoreCase("true"))
			return true ;
		if (val.equalsIgnoreCase("false"))
			return false ;
		
		return defaultValue ;
	}

	/************************************************************************
	* 
	* Returns the value of a named attribute, interpreting it as an boolean.
	* If the attribute is missing or cannot be parsed as a bool, this function
	* throws an exception.
	* 
	*************************************************************************/
	public boolean getAttributeBooleanThrows(String name) throws Exception
	{
		String val = getAttributeThrows(name) ;
		
		if (val.equalsIgnoreCase("true"))
			return true ;
		if (val.equalsIgnoreCase("false"))
			return false ;
		
		throw new Exception("Could not parse the attribute " + name + ":" + val + " as a boolean") ;
	}
		
	/************************************************************************
	* 
	* Returns the number of children of this element.
	* 
	*************************************************************************/
	public int getNumberChildren()
	{
		return this.m_ChildElementList.size() ;
	}
	
	/************************************************************************
	* 
	* Returns a specific child of this element.
	* 
	* @param index			The position in the list (>=0 and < number of children)
	* 
	* @return The element at the given position.
	* 
	*************************************************************************/
	public JavaElementXML getChild(int index)
	{
		return (JavaElementXML)this.m_ChildElementList.get(index) ;
	}
	
	/************************************************************************
	* 
	* Returns a list of all of the children of this element.
	* The list is of ElementXML objects.
	* 
	*************************************************************************/
	protected ArrayList getChildElementList()
	{
		return this.m_ChildElementList ;
	}
	
	/************************************************************************
	* 
	* Replaces one child with another (in the same position in the list of children).
	* 
	* @param existingChild		The child being replaced
	* @param newChild			The child being added (can be null -> just deletes)
	* 
	* @return True if replacement succeeds (i.e. existingChild is found)
	*************************************************************************/
	public boolean replaceChild(JavaElementXML existingChild, JavaElementXML newChild)
	{
		for (int i = 0 ; i < this.m_ChildElementList.size() ; i++)
		{
			if (getChild(i) == existingChild)
			{
				m_ChildElementList.remove(i) ;
				existingChild.setParent(null) ;
				
				if (newChild != null)
				{
					m_ChildElementList.add(i, newChild) ;
					newChild.setParent(this) ;
				}
				
				return true ;
			}
		}
		
		return false ;
	}

	/************************************************************************
	* 
	* Removes a child from the XML tree
	* 
	* @param existingChild		The child being removed
	* 
	* @return True if removal succeeds (i.e. existingChild is found)
	*************************************************************************/
	public boolean removeChild(JavaElementXML child)
	{
		return replaceChild(child, null) ;
	}
	
	/************************************************************************
	* 
	* Returns the tag name for this element.
	* E.g. <author>Thomas</author> the tag name is "author".
	* 
	*************************************************************************/
	public String getTagName()
	{
		return this.m_TagName ;
	}
	
	/************************************************************************
	* 
	* Add an attribute and value pair.
	* The value is always stored as a string.
	* 
	* @param name			The attribute name
	* @param value			The attribute value
	* 
	*************************************************************************/
	public void addAttribute(String name, String value)
	{
		if (name.indexOf(" ") != -1)
			throw new Error("Can't have attribute names that contain a space -- won't parse correctly") ;
			
		this.m_AttributeList.put(name, value) ;
	}
	
	/************************************************************************
	* 
	* Add a child element to this element's list of children.
	* 
	* @param element		The element to add
	* 
	*************************************************************************/
	public void addChildElement(JavaElementXML element)
	{
		this.m_ChildElementList.add(element) ;
		element.setParent(this) ;
	}
	
	/************************************************************************
	* 
	* Find a child element based on an attribut name
	* 
	* @param attName		The name to match against
	* 
	*************************************************************************/	
	public JavaElementXML findChildByAtt(String attName, String value)
	{
		for (java.util.Iterator iter = m_ChildElementList.iterator() ; iter.hasNext() ; )
		{
			JavaElementXML element = (JavaElementXML)iter.next() ;
			
			String att = element.getAttribute(attName) ;
			if (att != null && att.equalsIgnoreCase(value))
				return element ;
		}
		
		return null ;
	}
	
	/************************************************************************
	* 
	* Find a child element based on a tag name
	* 
	* @param tagName		The name to match against
	* 
	*************************************************************************/	
	public JavaElementXML findChildByName(String tagName)
	{
		for (java.util.Iterator iter = m_ChildElementList.iterator() ; iter.hasNext() ; )
		{
			JavaElementXML element = (JavaElementXML)iter.next() ;
			
			if (element.getTagName().equalsIgnoreCase(tagName))
				return element ;
		}
		
		return null ;
	}
	
	/************************************************************************
	* 
	* Find a child element based on an attribute and value
	* If the child does not exist, this version throws an exception.
	* 
	* @param tagName		The name to match against
	* 
	*************************************************************************/	
	public JavaElementXML findChildByAttThrows(String attName, String value) throws Exception
	{
		JavaElementXML child = findChildByAtt(attName, value) ;
		
		if (child == null)
			throw new Exception("Could not find child node with name: " + attName + " while parsing XML document") ;
		
		return child ;
	}
	
	/************************************************************************
	* 
	* Find a child element based on its tag name (case insensitve).
	* If the child does not exist, this version throws an exception.
	* 
	* @param tagName		The name to match against
	* 
	*************************************************************************/	
	public JavaElementXML findChildByNameThrows(String tagName) throws Exception
	{
		JavaElementXML child = findChildByName(tagName) ;
		
		if (child == null)
			throw new Exception("Could not find child node with name: " + tagName + " while parsing XML document") ;
		
		return child ;
	}
	/************************************************************************
	* 
	* Find a child's contents based on its tag name (case insensitve).
	* This turns out to be a handy utility function.
	* 
	* @param tagName		The name to match against
	* 
	*************************************************************************/	
	public String findContentsByName(String tagName)
	{
		JavaElementXML element = findChildByName(tagName) ;
		
		if (element != null)
			return element.getContents() ;
		
		return null ;
	}
	
	/************************************************************************
	* 
	* Write out the element and all of its children to a given file.
	* 
	* @param filename		The path name of the output file.
	* 
	*************************************************************************/
	public void WriteToFile(String filename) throws java.io.IOException
	{
		// Create the file.
		FileWriter fw = new FileWriter(filename) ;
		BufferedWriter output = new BufferedWriter(fw) ;
		
		// Write out the header
		JavaElementXML.WriteHeader(output) ;
		
		// Write out the stream
		WriteToStream(output, 0) ;
		
		// Clean up
		output.close() ;
		fw.close() ;
	}
	
	public String WriteToString()
	{
		StringWriter sw = new StringWriter() ;
		BufferedWriter output = new BufferedWriter(sw) ;
		
		try
		{
			// Write out the header
			JavaElementXML.WriteHeader(output) ;
			
			// Write out the stream
			WriteToStream(output, 0) ;
			
			// Clean up
			output.close() ;
		} catch (IOException e)
		{
			e.printStackTrace();
			return null ;
		}
		
		return sw.toString() ;
	}
	
	public String toString()
	{
		return WriteToString() ;
	}
	
	/************************************************************************
	* 
	* Reads an XML file and builds the ElementXML object that represents it.
	* As an XML element can contain arbitrary children, the root object returned
	* can contain references to an arbitrarily deep tree of elements.
	* 
	* @param filename		The xml file to read.
	* 
	* @return The root element.
	* 
	*************************************************************************/
	public static JavaElementXML ReadFromFile(String filename) throws Exception
	{
		// Create the file reader
		FileReader fr 		 = new FileReader(filename) ;
		BufferedReader input = new BufferedReader(fr) ;
		
		// Fire up the lexical analyser to read this file.
		LexXML lexXML = new LexXML(input) ;
		
		// Read in the root element (and all children)
		JavaElementXML element = JavaElementXML.ReadFromLex(lexXML) ;
		
		// Clean up
		input.close() ;
		fr.close() ;
		
		// Return the root
		return element ;
	}
	
	/************************************************************************
	* 
	* Parse an XML element and its children.  This is the main parsing
	* function.  The lexical analyser contains a references to the input
	* stream being parsed.  The function calls itself recursively to
	* parse its children.
	* 
	* @param lex		The lexical analyser for this parse.	
	* 
	* @return The element that has just been read from the input stream.
	* 
	*************************************************************************/
	public static JavaElementXML ParseElement(LexXML lex) throws Exception
	{
		// For now ignore header tags
		if (lex.Have(kHeaderString))
		{
			while (!lex.Have(kCloseTagString))
				lex.GetNextToken() ;
			
			return null ;
		}
		
		// Get the tag name
		String tagName = lex.MustBe(LexXML.kIdentifier) ;
		
		// Create the object we'll be returning
		JavaElementXML element = new JavaElementXML(tagName) ;
		
		// Read all of the attributes (if there are any)
		while (lex.Have(LexXML.kIdentifier))
		{
			String name = lex.getCurrentTokenValue() ;
			
			// Consume the name
			lex.GetNextToken() ;
			
			// Must be followed by "="
			lex.MustBe(kEqualsString) ;
			
			// Then must be followed by a string containing the value
			String value = lex.MustBe(LexXML.kQuotedString) ;
			value = convertFromEscapes(value) ;
			
			element.addAttribute(name, value) ;
		}

		// After reading all of the attributes, we must be at the end of the tag.
		lex.MustBe(kCloseTagString) ;
		
		boolean endTag = false ;
		
		while (!endTag)
		{
			if (lex.Have(LexXML.kQuotedString))
			{
				// Quoted strings are used to contain content values
				String contents = lex.getCurrentTokenValue() ;
				contents = convertFromEscapes(contents) ;
				
				element.addContents(contents) ;
				lex.GetNextToken() ;
				continue ;
			}
			
			if (lex.Have(kOpenTagString))
			{
				// After "<" we need to decide if this is a child
				// or the end of the current element.
				if (lex.Have(kEndMarkerString))
				{
					endTag = true ;
				}
				else
				{
					JavaElementXML child = ParseElement(lex) ;
					element.addChildElement(child) ;
				}
				continue ;
			}
			
			// We could report an error here as our files shouldn't have unquoted
			// contents strings, but we'll assume anything else we find here
			// is part of the contents.
			String value = lex.getCurrentTokenValue() ;
			value = convertFromEscapes(value) ;
			
			element.addContents(value) ;
			lex.GetNextToken() ;
		}
		
		// Once we reach the end marker "</" we just need to finish up the stream.
		tagName = lex.MustBe(LexXML.kIdentifier) ;
		
		if (!tagName.equals(element.getTagName()))
			throw new Exception("The closing tag for " + element.getTagName() + " doesn't match the opening tag") ;

		// Then we must have the close tag.
		lex.MustBe(kCloseTagString) ;
		
		// And we're done with this element.
		return element ;
	}
	
	/************************************************************************
	* 
	* Parse the input stream (encoded by the lexical analyser) until we
	* find the first XML element.  Return that element.
	* Note: This assumes the file contains only a single element, with all
	* other elements stored as its children.  BADBAD: If this is not true, this function
	* should be generalized to create a root node and store all of the elements
	* it finds in this root node.
	* 
	* @param lex			The lexical analyser, tied to a specific input stream
	* 
	* @return The root XML element.
	* 
	*************************************************************************/
	public static JavaElementXML ReadFromLex(LexXML lex) throws Exception
	{
		JavaElementXML element = null ;
		
		// Keep parsing till we read the first, valid XML node
		// This is really the root of a tree of nodes.
		while (element == null)
		{
			if (lex.Have(kOpenTagString))
				element = ParseElement(lex) ;
			else
				lex.GetNextToken() ;
		}
	
		return element ;
	}

	/************************************************************************
	* 
	* Write out this XML element to the given output stream.
	* The output is indented by 'indent' spaces so that the output
	* looks more pleasant when viewed in an editor.
	* 
	* @param output			The output stream
	* @param indent			The number of spaces to indent this XML element.
	* 
	*************************************************************************/
	public void WriteToStream(BufferedWriter output, int indent) throws java.io.IOException
	{	
		// Build a buffer for the text we'll output
		StringBuffer buffer = new StringBuffer() ;
	
		// Decide if this tag should all be on one line.
		// We only do this for "simple" tags, e.g. <color>"red"</color>
		// and it's just to make the file more human-readable
		boolean oneLine = (this.getNumberChildren() == 0) ;
		
		// Add the necessary spaces to indent everything
		// Just makes the file easier to read.
		for (int i = 0 ; i < indent ; i++)
		{
			buffer.append("  ") ;
		}
		
		// Add the tag itself "<tag"
		buffer.append(kOpenTagString) ;
		buffer.append(m_TagName) ;

		// Get the list of attributes
		java.util.Set entrySet = this.m_AttributeList.entrySet() ;
		
		// Go through each attribute
		for (java.util.Iterator iter = entrySet.iterator() ; iter.hasNext() ;)
		{
			// Get the next item in the hashtable
			java.util.Map.Entry entry = (java.util.Map.Entry)iter.next() ;
			
			// Retrieve the key and value
			String name  = (String)entry.getKey() ;
			String value = (String)entry.getValue() ;

			// Write out the attribute name and value (e.g. name="value")
			buffer.append(" ") ;
			buffer.append(name) ;
			buffer.append("=") ;
			buffer.append("\"") ;
			buffer.append(convertToEscapes(value)) ;
			buffer.append("\"") ;
		}

		// Close the tag itself
		buffer.append(kCloseTagString) ;
		
		// Write the collected string to the output stream
		output.write(buffer.toString()) ;
		
		if (!oneLine)
			output.newLine() ;
				
		if (this.m_Contents != null)
		{
			// Add the necessary spaces to indent everything
			// Just makes the file easier to read.
			if (!oneLine)
			{
				for (int i = 0 ; i < indent ; i++)
				{
					output.write("  ") ;
				}
			}
				
			// Write out the contents, as a quoted string.
			// We use quotes, so we can indent and in case the string has embedded newlines.
			output.write("\"" + convertToEscapes(this.m_Contents) + "\"") ;
			
			if (!oneLine)
				output.newLine() ;
		}

		// Go through all of the child nodes
		for (java.util.Iterator iter = m_ChildElementList.iterator() ; iter.hasNext() ;)
		{
			JavaElementXML element = (JavaElementXML)iter.next() ;
			
			// Write out each child
			element.WriteToStream(output, indent+1) ;
		}
		
		// Add the necessary spaces to indent everything
		// Just makes the file easier to read.
		if (!oneLine)
		{
			for (int i = 0 ; i < indent ; i++)
			{
				output.write("  ") ;
			}
		}
		
		// Write out the closing tag
		output.write(kOpenTagString + kEndMarkerString + m_TagName + kCloseTagString) ;
		output.newLine() ;
	}

	/************************************************************************
	* 
	* Write out the XML header.  BADBAD: Not sure what needs to go in here
	* to make it fully valid XML.  I think just the version is sufficient.
	* 
	* @param output			The output stream.
	* 
	*************************************************************************/
	public static void WriteHeader(BufferedWriter output) throws java.io.IOException
	{
		output.write("<?xml version=\"1.0\"?>") ;
		output.newLine() ;
	}
	
	/************************************************************************
	* 
	* Given an XML element that represents an object from a class, this function 
	* creates the object.  It does this by looking for a "class" attribute and
	* then instantiating an instance of that class.
	* Note: This will only work if the class (a) exists and (b) has a public default
	* constructor.
	* 
	* @return The object represented by this XML element (or null if there's an error).
	* 
	*************************************************************************/
	public Object CreateObjectFromXMLDefaultConstructorNoThrow()
	{
		// Look up the class name (if this object has one)
		String className = this.getAttribute(kClassAttribute) ;
			
		// If there is no class name, nothing to create.
		if (className == null)
			return null ;

		try
		{
			// Try to find the class (this throws if not successful)
			Class childClass = Class.forName(className) ;
		
			// Create an instance of the class
			Object obj = childClass.newInstance() ;
			
			// Return the newly created object
			return obj ;
		}
		catch (Exception e)
		{
			//String msg = e.getMessage() ;
			//Debug.println("Problem creating object: " + msg + ".  Does it have a default constructor?") ;
			
			// If there's a problem creating the object
			// just return null.
			return null ;
		}
	}
	
	/************************************************************************
	* 
	* Given an XML element that represents an object from a class, this function 
	* creates the object.  It does this by looking for a "class" attribute and
	* then instantiating an instance of that class.
	* Note: This will only work if the class (a) exists and (b) has a "createInstance()"
	* public method that takes no arguments.
	* 
	* I think this is a better solution than the methods which require a default
	* constructor.
	* 
	* @return The object represented by this XML element.
	* 
	*************************************************************************/
	public Object CreateObjectFromXML() throws Exception
	{
		// Look up the class name (if this object has one)
		String className = this.getAttribute(kClassAttribute) ;
			
		// If there is no class name, nothing to create.
		if (className == null)
			throw new Exception("This XML object does not have a class attribute, so no object can be built from it") ;

		try
		{
			// Try to find the class (this throws if not successful)
			Class childClass = Class.forName(className) ;
		
			// Get the creation method
			java.lang.reflect.Method method = childClass.getMethod("createInstance", null) ;
		
			// Create an instance of the class (since the method is static both params are null here)
			Object obj = method.invoke(null, null) ;
			
			// Return the newly created object
			return obj ;
		}
		catch (Exception e)
		{
			//String msg = e.getMessage() ;
			//Debug.println("Problem creating object: " + msg + ".  Does it have a createInstance method?") ;
			
			// If there's a problem creating the object
			// rethrow the exception.
			throw e ;
		}
	}
	
	/************************************************************************
	* 
	* Given an XML element that represents an object from a class, this function 
	* creates the object.  It does this by looking for a "class" attribute and
	* then instantiating an instance of that class.
	* Note: This will only work if the class (a) exists and (b) has a public default
	* constructor.
	* 
	* I have generally decided that I'm not very happy with this because it requires
	* a public default constructor and that removes a nice paradigm where some classes
	* would have a constructor taking arguments, so we could be sure they are initialized
	* correctly.
	* 
	* Instead, I've therefore written a replacement for this which requires a
	* createInstance() static method--not something likely to be used in error by
	* a developer.
	* 
	* @return The object represented by this XML element.
	* 
	*************************************************************************/
	public Object CreateObjectFromXMLDefaultConstructor() throws Exception
	{
		// Look up the class name (if this object has one)
		String className = this.getAttribute(kClassAttribute) ;
			
		// If there is no class name, nothing to create.
		if (className == null)
			throw new Exception("This XML object does not have a class attribute, so no object can be built from it") ;

		try
		{
			// Try to find the class (this throws if not successful)
			Class childClass = Class.forName(className) ;
		
			// Create an instance of the class
			Object obj = childClass.newInstance() ;
			
			// Return the newly created object
			return obj ;
		}
		catch (Exception e)
		{
			//String msg = e.getMessage() ;
			//Debug.println("Problem creating object: " + msg + ".  Does it have a default constructor?") ;
			
			// If there's a problem creating the object
			// rethrow the exception.
			throw e ;
		}
	}
}


/* @(#)ElementXML.java */
