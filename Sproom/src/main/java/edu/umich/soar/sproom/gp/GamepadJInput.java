package edu.umich.soar.sproom.gp;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import net.java.games.input.Component;
import net.java.games.input.Controller;
import net.java.games.input.ControllerEnvironment;
import net.java.games.input.ControllerEvent;
import net.java.games.input.ControllerListener;

/**
 * Gamepad abstraction using jinput.
 *
 * @author voigtjr@gmail.com
 */
public class GamepadJInput {
	private static final Log logger = LogFactory.getLog(GamepadJInput.class);
	private static final float DEAD_ZONE = 0.15f;

	private static class HandlerData {
		HandlerData(Id id, Component component) {
			this.id = id;
			this.component = component;
			this.oldValue = component.getPollData();
			this.minValue = this.oldValue;
			this.maxValue = this.oldValue;
		}
		
		Component component;
		Id id;
		float oldValue;
		float minValue;
		float maxValue;
		final List<GPComponentListener> listeners = new ArrayList<GPComponentListener>();
		
		float getOld() {
			return oldValue;
		}
		
		float setOld(float v) {
			oldValue = v;
			
			if (!component.isAnalog()) {
				return v;
			}
			
			minValue = Math.min(minValue, v);
			maxValue = Math.max(maxValue, v);
			
			if (logger.isTraceEnabled()) {
				logger.trace(String.format("v%2.2f min%2.2f max%2.2f", v, minValue, maxValue));
			}
			
			// normalize to -1..1
			// there is positively a faster way to do this.
			float range = maxValue - minValue;
			if (Float.compare(range, 0) == 0) {
				return 0;
			}
			float pct = (v - minValue) / range;
			
			if (logger.isTraceEnabled()) {
				logger.trace(String.format("r%2.2f p%2.2f", range, pct));
			}
			
			float value = pct * 2.0f - 1.0f;
			return Math.abs(value) <= DEAD_ZONE ? 0 : value;
		}
		
		@Override
		public String toString() {
			StringBuilder sb = new StringBuilder();
			sb.append(component);
			sb.append(" ");
			sb.append(oldValue);
			sb.append(" ");
			sb.append(listeners.size());
			return sb.toString();
		}
	}
	
	public enum Id {
		OVERRIDE(Component.Identifier.Button._0), 
		SOAR(Component.Identifier.Button._1), 
		GPMODE(Component.Identifier.Button._2), 
		SLOW(Component.Identifier.Button._3), 
		LX(Component.Identifier.Axis.X), 
		LY(Component.Identifier.Axis.Y), 
		RX(Component.Identifier.Axis.Z), 
		RY(Component.Identifier.Axis.RZ);
		
		private Component.Identifier cid;
		
		private Id(Component.Identifier cid) {
			this.cid = cid;
		}
		
		private Component.Identifier getCId() {
			return cid;
		}
	}

	private Controller controller;
	private final List<HandlerData> components = new ArrayList<HandlerData>();
	private final ScheduledExecutorService exec = Executors.newSingleThreadScheduledExecutor();
	
	public GamepadJInput() {
		ControllerEnvironment ce = ControllerEnvironment.getDefaultEnvironment();
		
		ce.addControllerListener(new ControllerListener() {
			@Override
			public void controllerAdded(ControllerEvent e) {
				synchronized (components) {
					if (controller == null) {
						if (e.getController().getType() == Controller.Type.GAMEPAD) {
							controller = e.getController();
						}
					}
				}
			}
			
			@Override
			public void controllerRemoved(ControllerEvent e) {
				synchronized (components) {
					if (e.getController().equals(controller)) {
						controller = null;
						components.clear();
					}
				}
			}
		});

		synchronized (components) {
			for (Controller c : ce.getControllers()) {
				if (c.getType() != Controller.Type.GAMEPAD) {
					continue;
				}
				controller = c;
			}
			
			if (controller == null) {
				return;
			}
		}
		
		exec.scheduleAtFixedRate(new Runnable() {
			@Override
			public void run() {
				synchronized (components) {
					controller.poll();
	
					for (HandlerData data : components) {
						if (logger.isTraceEnabled()) {
							logger.trace(data);
						}
						float pollValue = data.component.getPollData();
						if (Float.compare(pollValue, data.getOld()) != 0) {
							float value = data.setOld(pollValue);
							for (GPComponentListener listener : data.listeners) {
								listener.stateChanged(data.id, value);
							}
						}
					}
				}
			}
		}, 0, 50, TimeUnit.MILLISECONDS);
	}
	
	public boolean addComponentListener(Id id, GPComponentListener listener) {
		
		synchronized (components) {
			if (controller == null) {
				logger.debug("can't add listener: no controller");
				return false;
			}
			
			logger.debug("adding listener for " + id.getCId());
			Component component = controller.getComponent(id.getCId());
			if (component == null) {
				logger.debug("add failed: no such component");
				return false;
			}
			
			for (HandlerData existing : components) {
				if (existing.component.equals(component)) {
					existing.listeners.add(listener);
					logger.debug("added to existing");
					return true;
				}
			}
			
			HandlerData newHandler = new HandlerData(id, component);
			newHandler.listeners.add(listener);
			components.add(newHandler);
			logger.debug("added new");
			return true;
		}
	}
	
	public void removeComponentListener(GPComponentListener listener) {
		synchronized (components) {
			for (HandlerData existing : components) {
				Iterator<GPComponentListener> iter = existing.listeners.iterator();
				while (iter.hasNext()) {
					GPComponentListener candidate = iter.next();
					if (candidate.equals(listener)) {
						iter.remove();
					}
				}
			}
		}
	}
}
