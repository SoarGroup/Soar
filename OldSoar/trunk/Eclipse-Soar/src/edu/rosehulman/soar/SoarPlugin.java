/**
 * The Soar Editor Plug-in is part of the Project Awesome Soar Eclipse Suite.
 * @file SoarEditorPlugin.java
 * @date December 11, 2003
 */
package edu.rosehulman.soar;

import java.util.MissingResourceException;
import java.util.ResourceBundle;

import org.eclipse.core.resources.*;
import org.eclipse.core.runtime.*;
import org.eclipse.ui.plugin.AbstractUIPlugin;
import org.eclipse.swt.dnd.*;
import org.eclipse.swt.widgets.*;

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
	
	
	
	/**
	 * The constructor.
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
			IResourceChangeEvent.PRE_AUTO_BUILD);
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
	
	
	
	public static DataMapClipboard getDataMapClipboard() {
		return _datamapClipboard;
	}

}
