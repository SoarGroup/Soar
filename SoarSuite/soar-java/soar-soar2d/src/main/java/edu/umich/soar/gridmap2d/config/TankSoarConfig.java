package edu.umich.soar.gridmap2d.config;


public class TankSoarConfig implements GameConfig {
	public int max_missiles = 15;
	public int max_energy = 1000;
	public int max_health = 1000;
	public int collision_penalty = -100;
	public int max_missile_packs = 3;
	public int missile_pack_respawn_chance = 5;
	public int shield_energy_usage = -20;
	public int missile_hit_award = 2;
	public int missile_hit_penalty = -1;
	public int frag_award = 3;
	public int frag_penalty = -2;
	public int max_sound_distance = 7;
	public int missile_reset_threshold = 100;
	
	public String title() {
		return "TankSoar";
	}
}
