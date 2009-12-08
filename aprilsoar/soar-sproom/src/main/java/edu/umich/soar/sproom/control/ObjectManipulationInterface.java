package edu.umich.soar.sproom.control;

public interface ObjectManipulationInterface {
	boolean get(int id);
	boolean drop(int id);
	boolean diffuse(int id);
	boolean diffuseByWire(int id, String color);
	String reason();
}
