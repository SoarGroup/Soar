/**
 *
 * @file SoarPerspectiveFactory.java
 * @date Mar 23, 2004
 */
package edu.rosehulman.soar.perspective;

import edu.rosehulman.soar.*;

import org.eclipse.ui.*;

/**
 * Creates the Soar perspective, with all of our pretty views.
 * 
 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
 */
public class SoarPerspectiveFactory implements IPerspectiveFactory {


	public void createInitialLayout(IPageLayout layout) {
		
		// Get the editor area.
		String editorArea = layout.getEditorArea();

		// Top left: Resource Navigator view
		IFolderLayout topLeft = layout.createFolder("topLeft", IPageLayout.LEFT, 0.25f,
			editorArea);
		topLeft.addView(SoarPlugin.ID_NAVIGATOR);

		// Bottom left: Outline view and Property Sheet view
		IFolderLayout bottomLeft = layout.createFolder("bottomLeft", IPageLayout.BOTTOM, 0.70f,
			"topLeft");
		bottomLeft.addView(IPageLayout.ID_OUTLINE);
		//bottomLeft.addView(IPageLayout.ID_PROP_SHEET);

		// Bottom right: Task List view
		layout.addView(IPageLayout.ID_PROBLEM_VIEW, IPageLayout.BOTTOM, 0.75f, editorArea);

	}

}
