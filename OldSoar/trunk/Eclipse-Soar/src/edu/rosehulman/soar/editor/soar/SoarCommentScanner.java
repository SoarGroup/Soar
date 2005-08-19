package edu.rosehulman.soar.editor.soar;

import java.util.ArrayList;

import org.eclipse.jface.text.TextAttribute;
import org.eclipse.jface.text.rules.*;
import org.eclipse.swt.SWT;

import edu.rosehulman.soar.SoarPlugin;
import edu.rosehulman.soar.editor.util.*;

public class SoarCommentScanner extends RuleBasedScanner {
	public SoarCommentScanner() {
		ColorProvider cp = SoarPlugin.getDefault().getColorProvider();
		ArrayList rules = new ArrayList();
		IRule temp;
		
		
		//SoarDoc
		IToken soarDoc =
			new Token(new TextAttribute(cp.getColor(ColorProvider.SOAR_DOC),
				cp.getColor(ColorProvider.BACKGROUND), SWT.BOLD));
		
		temp = new WordRule(new SoarDocWordDetector());
		for (int i=0; i<ISoarSyntax.SOAR_DOC.length; ++i) {
			((WordRule)temp).addWord("@" + ISoarSyntax.SOAR_DOC[i], soarDoc);
		}
		rules.add(temp);
		
		
		//	 Comments
		IToken comment =
			new Token(new TextAttribute(cp.getColor(ColorProvider.COMMENT)));
		
		this.fDefaultReturnToken = comment;
		
		// Package the rules and send them off
		IRule[] rules2 = new IRule[rules.size()];
		rules.toArray(rules2);
		
		
		setRules(rules2);
	}

}
