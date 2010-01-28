package edu.umich.soar.room;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.JMenu;

import edu.umich.soar.room.core.Simulation;
import edu.umich.soar.room.map.CellObject;

public class WorldMenu extends JMenu {

	private static final long serialVersionUID = -964317378923488739L;

	private final WorldView wv;
	private final Simulation sim;
	private final int[] xy;
	
	public WorldMenu(WorldView wv, Simulation sim, int[] xy) {
		super("World Menu");

		this.wv = wv;
		this.sim = sim;
		this.xy = xy;
		
		if (sim.getMap().isAvailable(xy)) {
			for (CellObject template : sim.getWorld().getMap().getAllTemplates()) {
				assert template.hasProperty("name");
				if (template.getProperty("movable", Boolean.FALSE, Boolean.class)) {
					String name = template.getProperty("name");
					add(new CreateObject(name));
				}
			}
			
		} else {
			add(new NotEmpty());
		}
	}
	
	private final class NotEmpty extends AbstractAction {
		private static final long serialVersionUID = 1L;
		
		NotEmpty() {
			super("Cell is not empty");
			this.setEnabled(false);
		}
		
		@Override
		public void actionPerformed(ActionEvent e) {
		}
	}
	
	private final class CreateObject extends AbstractAction {

		private static final long serialVersionUID = 1L;
		private final String name;
		
		CreateObject(String name) {
			super("Create " + name);
			this.name = name;
		}
		
		@Override
		public void actionPerformed(ActionEvent e) {
			CellObject object = sim.getMap().createObjectByName(name);
			sim.getMap().getCell(xy).addObject(object);
			wv.refresh();
		}
		
	}
}
