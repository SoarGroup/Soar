package soar2d.config;


public class EatersConfig implements GameConfig {
	public int vision = 2;
	public int wall_penalty = -5;
	public int jump_penalty = -5;
	public double low_probability = 0.15;
	public double high_probability = 0.65;
	
	public String title() {
		return "Eaters";
	}
	
	public boolean runTilOutput() {
		return false;
	}
}
