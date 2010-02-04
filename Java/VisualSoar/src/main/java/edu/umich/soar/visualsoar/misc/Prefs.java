package edu.umich.soar.visualsoar.misc;

import java.util.prefs.BackingStoreException;
import java.util.prefs.Preferences;

public enum Prefs {
	openFolder(System.getProperty("user.dir")),
	autoTileEnabled(true),
	horizTile(true),
	highlightingEnabled(true),
	autoIndentingEnabled(true),
	autoSoarCompleteEnabled(true),
	sharedProjectFile(null),
	sharedProjectEnabled(false),
	userName("User");
	
	private static final Preferences preferences = Preferences.userRoot().node("edu/umich/soar/visualsoar");
	private static final SyntaxColor[] colors = SyntaxColor.getDefaultSyntaxColors();
	
	static {
		// start at 1 to skip DEFAULT
		for (int i = 1; i < colors.length; ++i) {
			if (colors[i] == null) {
				continue;
			}
			int rgb = preferences.getInt("syntaxColor" + i, colors[i].getRGB());
			colors[i] = new SyntaxColor(rgb, colors[i]);
		}
	}
	
	public static SyntaxColor[] getSyntaxColors() {
		SyntaxColor[] temp = new SyntaxColor[colors.length];
		System.arraycopy(colors, 0, temp, 0, colors.length);
		return temp;
	}
	
	public static void setSyntaxColors(SyntaxColor[] colors) {
		SyntaxColor[] defaults = SyntaxColor.getDefaultSyntaxColors();
		
		// start at 1 to skip DEFAULT
		for (int i = 1; i < colors.length; ++i) {
			if (colors[i] == null) {
				if (defaults[i] != null) {
					colors[i] = defaults[i];
				} else {
					continue;
				}
			} else {
				continue;
			}
			Prefs.colors[i] = colors[i];
			
			preferences.putInt("syntaxColor" + i, colors[i].getRGB());
		}
	}
	
	private final String def;
	private final boolean defBoolean;
	
	private Prefs(String def) {
		this.def = def;
		this.defBoolean = false;
	}
	
	private Prefs(boolean def) {
		this.def = null;
		this.defBoolean = def;
	}
	
	public String get() {
		return preferences.get(this.toString(), def);
	}
	
	public void set(String value) {
		preferences.put(this.toString(), value);
	}
	
	public boolean getBoolean() {
		return preferences.getBoolean(this.toString(), defBoolean);
	}
	
	public void setBoolean(boolean value) {
		preferences.putBoolean(this.toString(), value);
	}
	
	public static void flush() {
		try {
			preferences.flush();
		} catch (BackingStoreException e) {
			e.printStackTrace();
		}
	}

}
