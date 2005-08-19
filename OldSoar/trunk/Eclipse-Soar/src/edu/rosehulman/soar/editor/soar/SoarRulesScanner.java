/**
 *
 * @file SoarRulesScanner.java
 * @date Apr 17, 2004
 */
package edu.rosehulman.soar.editor.soar;

import edu.rosehulman.soar.*;
import edu.rosehulman.soar.editor.util.*;

import org.eclipse.jface.text.rules.*;
import org.eclipse.jface.text.*;
import org.eclipse.swt.*;

import java.util.*;

/**
 * 
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class SoarRulesScanner extends RuleBasedScanner {
	public SoarRulesScanner() {
		ColorProvider cp = SoarPlugin.getDefault().getColorProvider();
		ArrayList rules = new ArrayList();
		IRule temp;
		
		// Comments
		IToken comment =
			new Token(new TextAttribute(cp.getColor(ColorProvider.COMMENT)));
		
		temp = new EndOfLineRule("#", comment);
		rules.add(temp);
		
		//SoarDoc
		IToken soarDoc =
			new Token(new TextAttribute(cp.getColor(ColorProvider.KEYWORD),
				cp.getColor(ColorProvider.BACKGROUND), SWT.BOLD));
		
		temp = new WordRule(new SoarWordDetector());
		for (int i=0; i<ISoarSyntax.SOAR_DOC.length; ++i) {
			((WordRule)temp).addWord(ISoarSyntax.SOAR_DOC[i], soarDoc);
		}
		rules.add(temp);
		
		
		// String
		IToken str = 
			new Token(new TextAttribute(cp.getColor(ColorProvider.STRING)));
		
		temp = new SingleLineRule("\"", "\"", str);
		rules.add(temp);
		
		
		//Attribute
		IToken attribute = 
			new Token(new TextAttribute(cp.getColor(ColorProvider.ATTRIBUTE)));
		
		temp = new SingleLineRule("^", " ", attribute);
		rules.add(temp);
		
		//Attribute
		IToken variable = 
			new Token(new TextAttribute(cp.getColor(ColorProvider.VARIABLE)));
		
		temp = new SingleLineRule("<", ">", variable);
		rules.add(temp);
		
		
		// Key words
		IToken keyword =
			new Token(new TextAttribute(cp.getColor(ColorProvider.KEYWORD),
				cp.getColor(ColorProvider.BACKGROUND), SWT.BOLD));
		
		temp = new WordRule(new SoarWordDetector());
		for (int i=0; i<ISoarSyntax.RESERVED_WORDS.length; ++i) {
			((WordRule)temp).addWord(ISoarSyntax.RESERVED_WORDS[i], keyword);
		}
		rules.add(temp);
		
		
		// Special Characters
		/*IToken schar =
			new Token(new TextAttribute(cp.getColor(ColorProvider.SPECIAL_CHARACTER)));
		
		temp = new WordRule(new SoarWordDetector());
		for (int i=0; i<ISoarSyntax.SPECIAL_CHARACTERS.length; ++i) {
			((WordRule)temp).addWord(ISoarSyntax.SPECIAL_CHARACTERS[i], schar);
		}
		rules.add(temp); */
		
		
		// Package the rules and send them off
		IRule[] rules2 = new IRule[rules.size()];
		rules.toArray(rules2);
		
		
		setRules(rules2);
	}

}
