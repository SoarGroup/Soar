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

import java.util.*;



/**
 *
 * Provides autocompletion support for Soar. Be careful with this bugger.
 *  If it throws an exception, Eclipse complains when you try to save the file.
 *  Currently, I just catch the offending exceptions, but somebody should really
 *  go over this code and prevent them sometime. It's not too high a priority,
 *  though, as this still produces the desired behavior just fine.
 * 
 * @author Tim Jasko
 * @see org.eclipse.jface.text.contentassist.IContentAssistProcessor
 * 
 */
public class SoarContentAssistProcessor implements IContentAssistProcessor {
	private SoarEditor _editor;
	private DataMap _dm = null;
	
	private char[] _propActivate;
	private char[] _infoActivate = null;
	
	private String _errorMsg=null;

	/**
	 * Constructor
	 * 
	 * @param editor The Soar file editor that is using this class.
	 */
	public SoarContentAssistProcessor(SoarEditor editor) {
		_editor = editor;

		
		ArrayList temp = new ArrayList(70);
		temp.add(new Character('^'));
		temp.add(new Character('.'));
		temp.add(new Character('-'));
		
		
		
		for (char num='0'; num<'9'; num++) {
			temp.add(new Character(num));
		}
		for (char lcase='a'; lcase<'z'; lcase++) {
			temp.add(new Character( lcase));
		}
		for (char ucase='A'; ucase<'A'; ucase++) {
			temp.add(new Character( ucase ));
		}

		
		/*_propActivate[0] = '^';
		_propActivate[1] = '.';
		_propActivate[2] = '-';
		
		int index = 3;
		for (char num='0'; num<'9'; num++) {
			_propActivate[index] = num;
			index++;
		}
		for (char lcase='a'; lcase<'z'; lcase++) {
			_propActivate[index] = lcase;
			index++;
		}
		for (char ucase='A'; ucase<'A'; ucase++) {
			_propActivate[index] = ucase;
			index++;
		}*/
		
		_propActivate = new char[temp.size()];
		for (int i=0; i<temp.size(); ++i) {
			_propActivate[i] = ((Character)temp.get(i)).charValue();
		}
	}
	
	

	public ICompletionProposal[] computeCompletionProposals(
		ITextViewer viewer, int documentOffset) {
		
		DataMap dm = getDataMap();
		
		IFile source =
			((FileEditorInput) _editor.getEditorInput()).getFile();
				
		DMItem start = _dm.getAssociatedVertex(source);
		
		//System.out.println( "start: " + start);
		
		try {
			if (dm == null) {
				System.out.println("Datamap not found.");
				return null;
			}
			
			String docText = viewer.getDocument().get();
			char activator = docText.charAt(documentOffset-1);
				
			if (activator == '^' || activator == '.') {
				
				
				ArrayList suggestions;
				
				if (activator == '^' ) {
					
					suggestions = start.getChildren();
				} else {
				
					ArrayList names = getNames(docText, documentOffset-2);
					
					/*System.out.println("names:");
					for (int i=0; i<names.size(); i++) {
						System.out.println( "\"" + names.get(i) + "\"");
					}*/
					
					//Go through the DataMap for our suggestions
					suggestions = getSuggestions(start, names);
					//suggestions = _dm.find(names, start);
					
					/*System.out.println("suggestions");
					for (int i=0; i<suggestions.size(); i++) {
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
			
			
			
			//Handle partial completion
			} else {
				
				ArrayList suggestions;
				
			
				ArrayList names = getNames(docText, documentOffset-1);
				
				/*for (int i=0; i<names.size(); i++) {
					System.out.println( "\"" + names.get(i) + "\"");
				}*/
				
				//This is the beginning of the attribute the user is looking for.
				String partial = (String) names.get(names.size()-1);
				
				names.remove(names.size()-1);
				
				//Go through the DataMap for our suggestions
				suggestions = _dm.find(names, start);
				//suggestions = getSuggestions(start, names);
				
				/*System.out.println("**suggestions before");
				for (int i=0; i<suggestions.size(); i++) {
					System.out.println(suggestions.get(i));
				}*/
				
				//get rid of any suggestions that do not start with partial
				for (int i=0; i<suggestions.size(); ++i) {
					String name = ((DMItem) suggestions.get(i)).getName();
					if (! name.startsWith(partial)) {
						suggestions.remove(i);
						--i;
					}
				}
	
				/*System.out.println("**suggestions after");
				for (int i=0; i<suggestions.size(); i++) {
					System.out.println(suggestions.get(i));
				}*/
				
				CompletionProposal[] ret = new CompletionProposal[suggestions.size()];
				
				//Build the completion proposal
				for (int i=0; i<ret.length; i++) {
					DMItem sugg = (DMItem) suggestions.get(i);
					
					//System.out.println(sugg);
					
					String replacement = sugg.getName().substring(partial.length());
					
					ret[i] = new CompletionProposal(
						replacement,
						documentOffset,
						0,
						replacement.length(),
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
			}
		} catch (IndexOutOfBoundsException e) {
			return null;
		} catch (NullPointerException e) {
			return null;
		}

	} // ICompletionProposal[] computeCompletionProposals( ... )
	

	private ArrayList getSuggestions(DMItem start, ArrayList names) {
		ArrayList ret = new ArrayList();
		ArrayList results = _dm.find(names, start);
		
		//System.out.println("**find results:");
		for (int i=0; i<results.size(); ++i) {
			DMItem result = (DMItem) results.get(i);
			//System.out.println(result);
			ret.addAll(result.getChildren());
		}
		
		return ret;
	}
	
	
	
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
	/*public void refreshDatamap() {
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
	} // void refreshDatamap() */
	
	
	/**
	 * Retrieves the DataMap we need to look at.
	 * 
	 * @return The DataMap to for this file, or <code>null</code> if one could not be found.
	 */
	private DataMap getDataMap() {
		_dm = DataMap.getAssociatedDatamap( 
			((FileEditorInput) _editor.getEditorInput()).getFile());

		return _dm;
	} // DataMap getDataMap()


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
