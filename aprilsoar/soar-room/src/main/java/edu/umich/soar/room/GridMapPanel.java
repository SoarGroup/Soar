package edu.umich.soar.room;

import javax.swing.JPanel;

abstract class GridMapPanel extends JPanel implements BreadcrumbsProvider {

	private static final long serialVersionUID = -6979980443605257397L;

	int [] getCellAtPixel(int x, int y) {
		int [] loc = new int [] {x, y};
		loc[0] /= getCellSize();
		loc[1] /= getCellSize();
		return loc;
	}

	abstract int getCellSize();
}
