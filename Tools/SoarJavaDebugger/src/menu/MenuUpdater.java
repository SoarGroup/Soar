package menu;

public interface MenuUpdater {
	// This is called right before the menu pops down
	// giving the menu a chance to update the state of the items within that menu
	void updateItems() ;
}
