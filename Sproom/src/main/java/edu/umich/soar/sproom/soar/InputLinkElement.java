package edu.umich.soar.sproom.soar;

import edu.umich.soar.sproom.Adaptable;

/**
 * A collection of data managed under one top-level identifier on the input link.
 *
 * @author voigtjr@gmail.com
 */
interface InputLinkElement {
	void update(Adaptable app);
	void destroy();
}
