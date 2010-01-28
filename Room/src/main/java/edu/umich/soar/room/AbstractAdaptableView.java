package edu.umich.soar.room;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.util.prefs.Preferences;

import javax.swing.BorderFactory;
import javax.swing.JPanel;

import org.flexdock.view.View;

public abstract class AbstractAdaptableView extends View implements Adaptable {
	private static final long serialVersionUID = 8049528094231200441L;

	public AbstractAdaptableView(String persistentId, String title) {
		super(persistentId, title);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see org.flexdock.view.View#setContentPane(java.awt.Container)
	 */
	@Override
	public void setContentPane(Container c) throws IllegalArgumentException {
		// Give every view a default border ...
		final JPanel wrapper = new JPanel(new BorderLayout());
		wrapper.add(c, BorderLayout.CENTER);
		wrapper.setBorder(BorderFactory.createLineBorder(Color.GRAY, 1));
		super.setContentPane(wrapper);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * org.jsoar.util.adaptables.AbstractAdaptable#getAdapter(java.lang.Class)
	 */
	@Override
	public Object getAdapter(Class<?> klass) {
		return Adaptables.adapt(this, klass, false);
	}
	
    public Preferences getPreferences()
    {
        return Application.PREFERENCES.node("views/" + getPersistentId());
    }
}
