/*
 * Created on Jan 18, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap;

import org.eclipse.core.resources.*;
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
	private IFileEditorInput _fileInput;
	private boolean _isDirty = false;
	
	private TreeViewer viewer;
	private DataMapContentProvider _content;
	
	public static final String ID = "edu.rosehulman.soar.datamap.DataMapEditor";
	

	public DataMapEditor() {
		super();
	}

	public void doSave(IProgressMonitor monitor) {
		
		_content.save(monitor);
		
		monitor.done();
		
		//_isDirty = false;
		
		//firePropertyChange(EditorPart.PROP_DIRTY); // all is well once more!
		
		IWorkbenchPage pages[] =
			PlatformUI.getWorkbench().getActiveWorkbenchWindow().getPages();
		
		IProject proj = this.getFile().getProject();
		
		for (int i=0; i<pages.length; ++i) {
			IEditorPart editors[] = pages[i].getDirtyEditors();
			
			for (int i2=0; i2<editors.length; ++i2) {
				if (editors[i2] instanceof DataMapEditor) {
					DataMapEditor editor = (DataMapEditor) editors[i2];
					
					if (proj.equals( editor.getFile().getProject() )) {
						editor.cleanse();
					}
					
				}
			}
		}
	}

	public void doSaveAs() {
		//We don't do SaveAs.
	}

	public void gotoMarker(IMarker marker) {
		// I think we're just going to pass on this one.
	}

	public void init(IEditorSite site, IEditorInput input)
		throws PartInitException {
		
		if (! (input instanceof IFileEditorInput)) {
			throw new PartInitException("Invalid Input");
		} // if
		
		_fileInput = (IFileEditorInput) input;
		
		
		try {
		
			setSite(site);
			setInput(input);
			
			setPartName(_fileInput.getName());
			
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
		_content = new DataMapContentProvider(this, _fileInput.getFile().getProject(), _fileInput);
		
		viewer.setContentProvider(_content);
		viewer.setLabelProvider(new DataMapLabelProvider());
		viewer.setSorter(null);
		
		viewer.setInput(ResourcesPlugin.getWorkspace());
		
		viewer.expandToLevel(2);
		
		viewer.getTree().addKeyListener(new DMKeyListener());
		viewer.getTree().addMouseListener(new DMMouseListener());


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
		manager.add(new EditValues(this, target));
	
		manager.add(new Separator("Clipboard"));
		manager.add(new CutItem(this, target));
		manager.add(new CopyItem(this, target));
		manager.add(new PasteItem(this, target));
		manager.add(new CreateLink(this, target));
	
		manager.add(new Separator("Comments"));
		manager.add(new EditComment(this, target));
		manager.add(new RemoveComment(this, target));
	
		manager.add(new Separator("Validation"));
		//TODO The following menu items:
		// Validate Entry
		// Validate All
	
		manager.add(new Separator("Additions"));
	}

	

	public void setFocus() {

	}
	
	//*******************************************
	//         Special methods I made up
	//*******************************************
	
	
	public IFile getFile() {
		return _fileInput.getFile();
	}
	
	public DataMap getDataMap() {
		return _content.getDataMap();
	}
	
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
	
	public void cleanse() {
		_isDirty = false;
		firePropertyChange(EditorPart.PROP_DIRTY); //All is well once more!
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
	
	/**
	 * Enables the user to edit Datamap items by double clicking on them.
	 * 
	 * 
	 * @author Tim Jasko &lt;tj9582@yahoo.com&gt;
	 */
	private class DMMouseListener implements MouseListener {
		public void mouseDoubleClick(MouseEvent e) {
			ISelection ss = viewer.getSelection();
			
			if (! ss.isEmpty()) {
				DMItem target = (DMItem) ((StructuredSelection)ss).getFirstElement();
						
				if (target.canEditValues()) {
					new EditValues(DataMapEditor.this, target).run();
				} // if 
						
				DataMapEditor.this.getViewer().refresh();
			} // if
		}
		
		public void mouseDown(MouseEvent e) {
		}
		
		public void mouseUp(MouseEvent e) {
		}
	}

} // class
