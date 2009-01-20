package soar2d.config;


public class KitchenConfig implements GameConfig {
	public String title() {
		return "Kitchen";
	}
	
	public boolean runTilOutput() {
		return false;
	}
}
