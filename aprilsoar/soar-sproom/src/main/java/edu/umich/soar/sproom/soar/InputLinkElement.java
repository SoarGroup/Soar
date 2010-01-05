package edu.umich.soar.sproom.soar;

import edu.umich.soar.sproom.Adaptable;

interface InputLinkElement {
	void update(Adaptable app);
	// possibly add destroy();
}
