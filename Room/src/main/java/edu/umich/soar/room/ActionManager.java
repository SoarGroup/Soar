package edu.umich.soar.room;

/*
 * Based on org.jsoar.debugger.actions.ActionManager
 */

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.room.selection.SelectionListener;
import edu.umich.soar.room.selection.SelectionManager;

/**
 * @author ray
 */
public class ActionManager {
	private static final Log logger = LogFactory.getLog(ActionManager.class);

	private Application app;
	private List<AbstractGridmap2DAction> actions = new ArrayList<AbstractGridmap2DAction>();
	private Map<String, AbstractGridmap2DAction> actionCache = new HashMap<String, AbstractGridmap2DAction>();

	private static class ObjectActionPair {
		AbstractGridmap2DAction action;
		Class<?> klass;
		public boolean adapt;
	};

	private List<ObjectActionPair> objectActions = new ArrayList<ObjectActionPair>();

	/**
	 * @param app
	 *            The owning application
	 */
	public ActionManager(Application app) {
		this.app = app;
		
        SelectionManager selectionManager = this.app.getSelectionManager();
        selectionManager.addListener(new SelectionListener() {

            public void selectionChanged(SelectionManager manager)
            {
                updateActions();
            }});
	}

	/**
	 * @return The owning application
	 */
	public Application getApplication() {
		return app;
	}

    /**
     * @return the selection manager
     */
    public SelectionManager getSelectionManager()
    {
        return app.getSelectionManager();
    }
    
	public AbstractGridmap2DAction getAction(String id) {
		AbstractGridmap2DAction r = actionCache.get(id);
		if (r != null) {
			return r;
		}

		for (AbstractGridmap2DAction action : actions) {
			if (id.equals(action.getId())) {
				r = action;
				break;
			}
		}

		if (r != null) {
			actionCache.put(r.getId(), r);
		}

		return r;
	}

	public <T extends AbstractGridmap2DAction> T getAction(Class<T> klass) {
		return klass.cast(getAction(klass.getCanonicalName()));
	}

	/**
	 * Add an action that is managed by the application
	 * 
	 * @param action
	 *            The action to add
	 */
	public void addAction(AbstractGridmap2DAction action) {
		if (!actionCache.containsKey(action.getId())) {
			actionCache.put(action.getId(), action);
		}
		actions.add(action);
	}

	public void updateActions() {
		for (AbstractGridmap2DAction action : actions) {
			action.update();
		}
	}

	public void executeAction(String id) {
		AbstractGridmap2DAction action = getAction(id);
		if (action != null) {
			action.actionPerformed(null);
		} else {
			logger.error("No action found with id '" + id + "'");
		}
	}

	/**
	 * Register an action associated with a particular object class.
	 * 
	 * @param action
	 *            The action
	 * @param klass
	 *            The class of object this action is associated with.
	 * @param adapt
	 *            If true, the class is located through adapters in addition to
	 *            the usual instanceof test.
	 */
	public void addObjectAction(AbstractGridmap2DAction action, Class<?> klass,
			boolean adapt) {
		addAction(action);

		ObjectActionPair pair = new ObjectActionPair();
		pair.action = action;
		pair.klass = klass;
		pair.adapt = adapt;

		objectActions.add(pair);
	}

	/**
	 * Return a list of actions applicable to the given object. These are
	 * actions previously installed with a call to
	 * {@link #addObjectAction(AbstractDebuggerAction, Class, boolean)}.
	 * 
	 * @param o
	 *            The object
	 * @return The list of applicable actions.
	 */
	public List<AbstractGridmap2DAction> getActionsForObject(Object o) {
		List<AbstractGridmap2DAction> result = new ArrayList<AbstractGridmap2DAction>();
		for (ObjectActionPair pair : objectActions) {
			if (pair.adapt) {
				if (Adaptables.adapt(o, pair.klass) != null) {
					result.add(pair.action);
				}
			} else if (pair.klass.isInstance(o)) {
				result.add(pair.action);
			}
		}
		return result;
	}
}
