package edu.umich.soar.sproom.gp;

/**
 * Interface for listening for gamepad events.
 *
 * @author voigtjr@gmail.com
 */
public interface GPComponentListener {
	public void stateChanged(GamepadJInput.Id id, float value);
}
