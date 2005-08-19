package edu.rosehulman.soar.editor.util;

import org.eclipse.jface.text.rules.IWordDetector;

import edu.rosehulman.soar.editor.soar.ISoarSyntax;

public class SoarDocWordDetector implements ISoarSyntax, IWordDetector {

	public boolean isWordStart(char c) {
		if (c == '@') {
			return true;
		}
		return false;
	}

	public boolean isWordPart(char c) {
		
		for (int i = 0; i < SOAR_DOC.length; i++) {
			if (((String)SOAR_DOC[i]).indexOf(c) != -1)  {
				return true;
			}
		}
		return false;
	}

}
