/*
 * Created on Feb 16, 2004
 *
 */
package edu.rosehulman.soar.editor.autocomplete;

import edu.rosehulman.soar.editor.*;
import edu.rosehulman.soar.datamap.*;
import edu.rosehulman.soar.datamap.items.*;

import org.eclipse.jface.text.*;
import org.eclipse.jface.text.contentassist.*;
import org.eclipse.core.resources.*;
import org.eclipse.ui.part.*;
import org.eclipse.core.runtime.*;

import java.util.*;



/**
 *
 * Provides autocompletion support for Soar.
 * 
 * @author Tim Jasko
 * @see org.eclipse.jface.text.contentassist.IContentAssistProcessor
 * 
 */
public class SoarContentAssistProcessor implements IContentAssistProcessor {
	private SoarEditor _editor;
	private DataMap _dm = null;
	
	private char[] _propActivate = {'.', '^'};
	private char[] _infoActivate = null;
	
	private String _errorMsg;

	/**
	 * Constructor
	 * 
	 * @param editor The Soar file editor that is using this class.
	 */
	public SoarContentAssistProcessor(SoarEditor editor) {
		_editor = editor;
	}
	
	

	public ICompletionProposal[] computeCompletionProposals(
		ITextViewer viewer, int documentOffset) {
		
		DataMap dm = getDataMap();
		
		if (dm == null) {
			System.out.println("Datamap not found.");
			return null;
			
		} else {
			
			// Fetch the text of this file, then parse.
			String docText = viewer.getDocument().get();
			ArrayList suggestions;
			
			if (docText.charAt(documentOffset-1) == '^' ) {
				suggestions = dm.getRoot().getChildren();
			} else {
			
				ArrayList names = getNames(docText, documentOffset-2);
				
				/*for (int i=0; i<names.size(); i++) {
					System.out.println( "\"" + names.get(i) + "\"");
				} */
				
				//Go through the DataMap for our suggestions
				suggestions = getSuggestions(dm.getRoot(), names, 0);
				
				/*for (int i=0; i<suggestions.size(); i++) {
					System.out.println(suggestions.get(i));
				} */
			} // else
			
			
			CompletionProposal[] ret = new CompletionProposal[suggestions.size()];
			
			
			for (int i=0; i<ret.length; i++) {
				DMItem sugg = (DMItem) suggestions.get(i);
				
				//System.out.println(sugg);
				
				ret[i] = new CompletionProposal(
					sugg.getName(),
					documentOffset,
					0,
					sugg.getName().length(),
					ItemImages.getImage(sugg),
					sugg.toString(),
					null,
					"");
				
			} // for i
			

			if (ret.length == 0) {
				return null;
			} else {
				return ret;
			} // else
			
		} // else
	} // ICompletionProposal[] computeCompletionProposals( ... )
	
	
	/**
	 * Retrieves completion suggestions.
	 *  Should be called with the DataMap root node and an index of 0. 
	 * 
	 * @param node The node to search. Use the DataMap root.
	 * @param names A list of the names to seek through in the DataMap
	 * @param index For recursion purposes. Use 0 here.
	 * @return An ArrayList of all possible suggestions.
	 */
	private ArrayList getSuggestions(DMIdentifier node, ArrayList names, int index) {
		
		ArrayList kids = node.getChildren();
		String name = (String) names.get(index);
		
		//System.out.println(node + ": " + index);
		
		//This is the final item! Yipee!
		if (index == names.size()-1) {
			
			
			ArrayList ret = new ArrayList();
			
			for (int i=0; i<kids.size(); i++) {
				DMItem kid = (DMItem) kids.get(i);
				
				if (kid.getName().equals(name)) {
					ret.addAll(kid.getChildren());
				} // if
			} // for
			
			return ret;
		
		//Not the final item. There's some recursing to be doing.
		// I would like to point out that this would be prettier in Scheme.
		} else {
			ArrayList ret = new ArrayList();
			
			for (int i=0; i<kids.size(); i++) {
				DMItem kid = (DMItem) kids.get(i);
	
				if (kid.getName().equals(name) && kid instanceof DMIdentifier) {
					ret.addAll( getSuggestions((DMIdentifier)kid, names, index+1));
				} // if
			} // for
			
			return ret;
		} // else
	} //ArrayList getSuggestions(DataMap dm, ArrayList names)
	
	
	
	/**
	 * Gets the list of attribute names leading up the offset point.
	 * 
	 * @param text The document text containing
	 * @param offset The offset to mave backwards from.
	 * @return An ArrayList containing the attribute names up to the offset point.
	 */
	private ArrayList getNames(String text, int offset) {
		ArrayList ret = new ArrayList();
		
		String acc = new String("");
		
		//Iterate from the offset backwards, building the name,
		// then adding it to the list.
		for (int i=offset; i>0; i--) {
			char c = text.charAt(i);
			
			if (c == '.') {
				//System.out.println("adding: " + acc);
				ret.add(0, acc);
				acc = new String("");
				
			} else if (c == '-' || Character.isLetterOrDigit(c) ) {
				acc = c + acc;
				
			} else {
				//We're done; get out of this loop
				//System.out.println("break");
				break;
				
			} // else
			
		} // for i--

		//Add that final name to the list
		//System.out.println("final add: " + acc);
		ret.add(0, acc);

		return ret;		
	} // String prevName(String text, int offset)
	
	
	/**
	 * Refreshes the Datamap information.
	 * Should be called whenever the editor regains focus so that we have
	 *  the latest changes to the datamap.
	 *
	 */
	public void refreshDatamap() {
		FileEditorInput input = (FileEditorInput) _editor.getEditorInput();
		IContainer folder = input.getFile().getParent();
		
		try {
			IFile dmFile = folder.getFile(new Path("datamap.xdm"));
			
			if (dmFile.exists()) {
				DataMap dm = new DataMap(dmFile);
			
				_dm = dm;
				
			//If that file does not exist, we'll use the one in the parent folder
			} else {
				IFile parentDM = folder.getParent().getFile(new Path("datamap.xdm"));
				DataMap dm = new DataMap(parentDM);
			
				_dm = dm;
			} // else
		
		//If that one doesn't exist either, we pack our bags and go home.
		} catch (Exception e) {
			e.printStackTrace();
			
			_dm = null;
		}
	} // void refreshDatamap()
	
	
	/**
	 * Retrieves the DataMap we need to look at. First it checks the file's
	 *  folder for datamap.xdm; if that does not exist, it looks in that folder's
	 *  parent folder. If there is still no DataMap found,
	 *  <code>null</code> is returned.<p>
	 * 
	 * @return The DataMap to for this file, or <code>null</code> if one could not be found.
	 */
	private DataMap getDataMap() {
		return _dm;
	} // DataMap getDataMap(ITextViewer viewer)


	public IContextInformation[] computeContextInformation(
		ITextViewer viewer, int documentOffset) {

		//For now, we don't do any context information

		return null;
	}


	public char[] getCompletionProposalAutoActivationCharacters() {
		return _propActivate;
	}


	public char[] getContextInformationAutoActivationCharacters() {
		return _infoActivate;
	}


	public String getErrorMessage() {
		return _errorMsg;
	}


	public IContextInformationValidator getContextInformationValidator() {
		//We don't currently worry about context information
		
		return null;
	}

}
