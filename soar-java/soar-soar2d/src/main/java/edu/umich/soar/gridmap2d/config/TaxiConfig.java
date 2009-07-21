package edu.umich.soar.gridmap2d.config;

public class TaxiConfig implements GameConfig{
	public boolean disable_fuel = false;
	public int fuel_starting_minimum = 5;
	public int fuel_starting_maximum = 12;
	public int fuel_maximum = 14;
	
	public String title() {
		return "Taxi";
	}
}
