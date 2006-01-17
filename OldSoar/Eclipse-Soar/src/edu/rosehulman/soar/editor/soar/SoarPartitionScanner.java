/**
 * A scanner designed to partition Soar files
 * @file SoarPartitionScanner.java
 * @date December 11, 2003
 */
package edu.rosehulman.soar.editor.soar;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.jface.text.rules.RuleBasedPartitionScanner;
import org.eclipse.jface.text.rules.EndOfLineRule;
import org.eclipse.jface.text.rules.IPredicateRule;
import org.eclipse.jface.text.rules.IToken;
//import org.eclipse.jface.text.rules.MultiLineRule;
//import org.eclipse.jface.text.rules.SingleLineRule;
import org.eclipse.jface.text.rules.Token;

/**
 * A scanner designed to partition Soar files
 * @author lutezp
 */
public class SoarPartitionScanner extends RuleBasedPartitionScanner
{
	public final static String SOAR_COMMENT = "soar_comment";
	
	/**
	 * Creates the partitioner and sets up the necessary rules.
	 */
	public SoarPartitionScanner()
	{
		super();

		IToken comment = new Token(SOAR_COMMENT);
		
		List rules = new ArrayList();

		// The rule for comments
		rules.add(new EndOfLineRule("#", comment));
		
		// The rule for strings
		//rules.add(new SingleLineRule("\"", "\"", Token.UNDEFINED));

		IPredicateRule[] result = new IPredicateRule[rules.size()];
		rules.toArray(result);
		setPredicateRules(result);
	}
}
