/**
 * The Soar Editor Plug-in is part of the Project Awesome Soar Eclipse Suite.
 * You need this to make everything else go, but it really doesn't do all that
 *  much itself.
 * @file SoarEditorPlugin.java
 * @date December 11, 2003
 */
package edu.rosehulman.soar;

import java.util.*;

import org.eclipse.core.resources.*;
import org.eclipse.core.runtime.*;
import org.eclipse.ui.plugin.AbstractUIPlugin;

import edu.rosehulman.soar.editor.util.ColorProvider;
import edu.rosehulman.soar.sourcing.*;
import edu.rosehulman.soar.datamap.*;


/**
 * The main plugin class to be used in the desktop.
 */
public class SoarPlugin extends AbstractUIPlugin
{
	
	public static final String ID_NAVIGATOR = "edu.rosehulman.soar.navigator.SoarNavigator";
	
	// The shared instance.
	private static SoarPlugin _plugin;
	
	// Resource bundle.
	private ResourceBundle _resourceBundle;
	private ColorProvider _colorProvider;
	private static DataMapClipboard _datamapClipboard;
	private static Map _dataMapRegistry = new HashMap(1);
	
	
	
	/**
	 * The constructor.
	 */
	public SoarPlugin()
	{
		super();
		
		_plugin = this;
		
		try 
		{
			_resourceBundle = 
				ResourceBundle.getBundle(
					"edu.rosehulman.soar.editor.SoarPluginResources");
		} 
		catch (MissingResourceException e)
		{
			_resourceBundle = null;
		}

		_colorProvider = new ColorProvider();
		
		
		//Make us a clipboard
		
		_datamapClipboard = new DataMapClipboard();
		
		//Set up a listener for resource changes.
		IResourceChangeListener listener = new SoarChangeListener();
		ResourcesPlugin.getWorkspace().addResourceChangeListener(listener,
			IResourceChangeEvent.PRE_BUILD);
	}
	
	/**
	 * The other constructor.
	 */
	public SoarPlugin(IPluginDescriptor descriptor)
	{
		super(descriptor);
		
		_plugin = this;
		
		try 
		{
			_resourceBundle = 
				ResourceBundle.getBundle(
					"edu.rosehulman.soar.editor.SoarPluginResources");
		} 
		catch (MissingResourceException e)
		{
			_resourceBundle = null;
		}

		_colorProvider = new ColorProvider();
		
		
		//Make us a clipboard
		
		_datamapClipboard = new DataMapClipboard();
		
		//Set up a listener for resource changes.
		IResourceChangeListener listener = new SoarChangeListener();
		ResourcesPlugin.getWorkspace().addResourceChangeListener(listener,
			IResourceChangeEvent.PRE_BUILD);
	}


	/**
	 * Returns the shared instance.
	 */
	public static SoarPlugin getDefault() 
	{
		return _plugin;
	}
	

	/**
	 * Returns the workspace instance.
	 */
	public static IWorkspace getWorkspace() 
	{
		return ResourcesPlugin.getWorkspace();
	}


	/**
	 * Returns the string from the plugin's resource bundle,
	 * or 'key' if not found.
	 */
	public static String getResourceString(String key) 
	{
		ResourceBundle bundle = SoarPlugin.getDefault().getResourceBundle();
		try 
		{
			return bundle.getString(key);
		} 
		catch (MissingResourceException e) 
		{
			return key;
		}
	}


	/**
	 * Returns the plugin's resource bundle,
	 */
	public ResourceBundle getResourceBundle() 
	{
		return _resourceBundle;
	}


	/**
	 * Disposes of the color provider.
	 */
	public void disposeColorProvider()
	{
		_colorProvider.dispose();
	}


	/**
	 * Returns the Color provider.
	 * @return ColorProvider
	 */
	public ColorProvider getColorProvider()
	{
		return _colorProvider;
	}
	
	/**
	 * Returns the DataMapClipboard.
	 * @return
	 */
	public static DataMapClipboard getDataMapClipboard() {
		return _datamapClipboard;
	}
	
	
	/**
	 * Returns the DataMap associated with the given resource's project.
	 *  SoarPlugin maintains a HashMap of DataMaps keyed by their project.
	 *  This allows all datamap editors for a project to be working
	 *  with the same data, thus avoiding them from over-writing
	 *  each other when saving. It also saves memory.
	 * @param res
	 * @return
	 */
	public static DataMap getDataMap(IResource res) {
		IProject proj = res.getProject();
		
		DataMap ret = (DataMap) _dataMapRegistry.get(proj);
		
		if (ret == null) {
			//System.out.println("datamap not found, constructing");
			
			try {
				IFile dmFile = proj.getFile(new Path("datamap.xdm"));
				
				if (dmFile.exists()) {
					ret = new DataMap(dmFile);
				
					_dataMapRegistry.put(proj, ret);
					return ret;
					
				} else {
					return null;
				}
			} catch (Exception e) {
				e.printStackTrace();
				
				return null;
			}
		}
		
		//System.out.println("datamap found");
		return ret;
	}
	
	/**
	 * Removes the DataMap that has been associated with the given
	 *  resource's project. It will be reloaded when asked for again.
	 *  Use this function when a project is deleted. That way,
	 *  if a new project is during the same session with the same name,
	 *  it won't be associated with the DataMap for the now-defunct
	 *  project.
	 * @param res
	 */
	public static void removeDataMap(IResource res) {
		IProject proj = res.getProject();
		
		_dataMapRegistry.remove(proj);
	}

}
