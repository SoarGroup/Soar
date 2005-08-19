package edu.rosehulman.soar.editor.autocomplete;


import edu.rosehulman.soar.*;
import edu.rosehulman.soar.editor.soar.*;

import org.eclipse.jface.text.ITextViewer;
import org.eclipse.jface.text.contentassist.*;

public class SoarDocContentAssistProcessor implements IContentAssistProcessor {

	public ICompletionProposal[] computeCompletionProposals(ITextViewer viewer,
			int offset) {
		CompletionProposal[] ret = new CompletionProposal[ISoarSyntax.SOAR_DOC.length];
		
		for (int i=0; i<ret.length; ++i) {
			String word = ISoarSyntax.SOAR_DOC[i]; 
			ret[i] = new CompletionProposal(word, offset, 0, word.length(),
					SoarImages.getImage(SoarImages.SOAR_DOC), word, null, "");
		}
		return ret;
	}

	public IContextInformation[] computeContextInformation(ITextViewer viewer,
			int offset) {
		return null;
	}

	public char[] getCompletionProposalAutoActivationCharacters() {
		return new char[] { '@' };
	}

	public char[] getContextInformationAutoActivationCharacters() {
		return null;
	}

	public String getErrorMessage() {
		return null;
	}

	public IContextInformationValidator getContextInformationValidator() {
		return null;
	}

}
