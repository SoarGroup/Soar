/*
 * Created on Jan 18, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap;

import org.eclipse.core.resources.IMarker;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.swt.widgets.*;
import org.eclipse.ui.*;
import org.eclipse.ui.part.*;
import org.eclipse.jface.viewers.*;
import org.eclipse.jface.action.*;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.*; 

import edu.rosehulman.soar.datamap.items.*;
import edu.rosehulman.soar.datamap.actions.*;

/**
 *
 * Editor for the DataMap.
 * 
 * @author Tim Jasko
 * @see DataMapContentProvider
 * @see DataMap
 */
public class DataMapEditor extends EditorPart {
	private FileEditorInput _file;
	private boolean _isDirty = false;
	
	private TreeViewer viewer;
	private DataMapContentProvider _content;
	

	public DataMapEditor() {
		super();
	}

	public void doSave(IProgressMonitor monitor) {
		
		_content.save(monitor);
		
		monitor.done();
		
		_isDirty = false;
		
		firePropertyChange(EditorPart.PROP_DIRTY); // all is well once more!
	}

	public void doSaveAs() {
		//We don't do SaveAs.
	}

	public void gotoMarker(IMarker marker) {
		// I think we're just going to pass on this one.
	}

	public void init(IEditorSite site, IEditorInput input)
		throws PartInitException {
		
		if (! (input instanceof FileEditorInput)) {
			throw new PartInitException("Invalid Input");
		} // if
		
		_file = (FileEditorInput) input;
		
		
		try {
		
			setSite(site);
			setInput(input);
			
			setTitle(_file.getFile().getProject().getName() + " DataMap");
			
		} catch (Exception e) {
			throw new PartInitException(e.getMessage());
		} // catch
	}


	public boolean isDirty() {
		return _isDirty;
	}


	public boolean isSaveAsAllowed() {
		// Heck no
		return false;
	}


	public void createPartControl(Composite parent) {
		viewer = new TreeViewer(parent, SWT.MULTI | SWT.H_SCROLL | SWT.V_SCROLL);
		_content = new DataMapContentProvider(this, _file.getFile().getProject(), _file);
		
		viewer.setContentProvider(_content);
		viewer.setLabelProvider(new DataMapLabelProvider());
		viewer.setSorter(null);
		
		viewer.setInput(ResourcesPlugin.getWorkspace());
		
		viewer.expandToLevel(2);
		
		viewer.getTree().addKeyListener(new DMKeyListener());


		hookContextMenu();

	}
	
	private void hookContextMenu() {
		
		MenuManager menuMgr = new MenuManager("#PopupMenu");
		menuMgr.setRemoveAllWhenShown(true);
		menuMgr.addMenuListener(new IMenuListener() {
			public void menuAboutToShow(IMenuManager manager) {
				DataMapEditor.this.fillContextMenu(manager);
			}
		});
	
		Menu menu = menuMgr.createContextMenu(viewer.getControl());
		viewer.getControl().setMenu(menu);
		getSite().registerContextMenu(menuMgr, viewer);
	}


	/**
	 * Fills the context menu.
	 * 
	 * @param manager The menu manager.
	 */
	private void fillContextMenu(IMenuManager manager) {
		viewer.refresh();
		DMItem target = (DMItem) ((StructuredSelection)viewer.getSelection()).getFirstElement();		
	
	 
		manager.add(new AddItem(this, new DMIdentifier(), target));
		manager.add(new AddItem(this, new DMEnumeration(), target));
		manager.add(new AddItem(this, new DMInteger(), target));
		manager.add(new AddItem(this, new DMFloat(), target));
		manager.add(new AddItem(this, new DMString(), target));
	
		manager.add(new Separator("Search"));
	
		manager.add(new SearchFor(this, target));
	
		manager.add(new Separator("Edit"));


		MenuManager subMenuDMType = new MenuManager("Change DataMap Type...");
		subMenuDMType.setRemoveAllWhenShown(false);
		subMenuDMType.add(new ChangeType(this, new DMIdentifier(), target));
		subMenuDMType.add(new ChangeType(this, new DMEnumeration(), target));
		subMenuDMType.add(new ChangeType(this, new DMInteger(), target));
		subMenuDMType.add(new ChangeType(this, new DMFloat(), target));
		subMenuDMType.add(new ChangeType(this, new DMString(), target));
		
		
		manager.add(subMenuDMType);


		
		manager.add(new DeleteItem(this, target));
		manager.add(new RenameItem(this, target));
		EditValues ev = new EditValues(this, target);
		if(!target.canEditValues())
		{
			ev.setEnabled(false);
		}
		manager.add(ev);
	
		manager.add(new Separator("Comments"));
		manager.add(new EditComment(this, target));
		manager.add(new RemoveComment(this, target));
	
		manager.add(new Separator("Validation"));
		//TODO The following menus items:
		// Validate Entry
		// Validate All
	
		manager.add(new Separator("Additions"));
	}

	

	public void setFocus() {

	}
	
	//*******************************************
	//         Special methods I made up
	//*******************************************
	
	/**
	 * Gets the viewer being used by this editor.
	 */
	public TreeViewer getViewer() {
		return viewer;
	}
	
	/**
	 * Gets the content provider for this editor.
	 * @return The content provider
	 */
	public DataMapContentProvider getContentProvider() {
		return _content;
	}
	
	/**
	 * Signals to the DataMapEditor that we have dirtied the contents of the
	 *  datamap, ie: they have been changed and saving would be in order.
	 *
	 */
	public void defecateUpon() {
		_isDirty = true;
		firePropertyChange(EditorPart.PROP_DIRTY); //Alert the troops!
	}
	
	
	//******************************************
	//             Private classes
	//******************************************
	
	/**
	 * Listens to keypresses and responds accordingly.
	 * 
	 * @author Tim Jasko
	 */
	private class DMKeyListener implements KeyListener {
		public void keyPressed(KeyEvent e) {
			//Meh. Don't do nuthin' fer now.
		}
		
		public void keyReleased(KeyEvent e) {
			ISelection ss = viewer.getSelection();
			
			switch (e.keyCode) {
				
				//Delete key - deletes the item
				case 127:
	
					if (! ss.isEmpty()) {
						DMItem target = (DMItem) ((StructuredSelection)ss).getFirstElement();
						
						new DeleteItem(DataMapEditor.this, target).run();
						
						DataMapEditor.this.getViewer().refresh();
					} // if
				break;
				
				// Enter key - edits the item
				case 13:
	
					if (! ss.isEmpty()) {
						DMItem target = (DMItem) ((StructuredSelection)ss).getFirstElement();
						
						if (target.canEditValues()) {
							new EditValues(DataMapEditor.this, target).run();
						} // if 
						
						DataMapEditor.this.getViewer().refresh();
					} // if
				break;
			} // switch
			//System.out.println("key released:" + e.keyCode);
		}
		
	} // class

} // class
