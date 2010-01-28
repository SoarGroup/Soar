package edu.umich.soar.sproom.soar;

import java.util.HashMap;
import java.util.Map;

import sml.Identifier;
import edu.umich.soar.IntWme;
import edu.umich.soar.StringWme;
import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;
import edu.umich.soar.sproom.command.Comm;
import edu.umich.soar.sproom.command.CommMessage;

public class ReceivedMessagesIL implements InputLinkElement {

	private class MessageIL {
		private final Identifier messageRoot;

		private MessageIL(CommMessage message) {
			messageRoot = root.CreateIdWME(SharedNames.MESSAGE);
			
			StringWme.newInstance(messageRoot, SharedNames.FROM, message.getFrom());
			IntWme.newInstance(messageRoot, SharedNames.ID, (int)message.getId());
			Identifier next = null;
			for (String word : message.getTokens()) {
				// first one is "first", rest are next
				next = (next == null) ? messageRoot.CreateIdWME(SharedNames.FIRST) : next.CreateIdWME(SharedNames.NEXT);
				StringWme.newInstance(next, SharedNames.WORD, word);
			}
			StringWme.newInstance(next, SharedNames.NEXT, SharedNames.NIL);
		}
		
		private void destroy() {
			messageRoot.DestroyWME();
		}
	}
	
	private final Identifier root;
	private Map<CommMessage, MessageIL> all = new HashMap<CommMessage, MessageIL>();

	public ReceivedMessagesIL(Identifier root, Adaptable app) {
		this.root = root;
	}

	@Override
	public void update(Adaptable app) {
		Comm comm = (Comm)app.getAdapter(Comm.class);
		Map<CommMessage, MessageIL> newAll = new HashMap<CommMessage, MessageIL>(all.size());
		
		for(CommMessage message : comm.getMessages().values()) {
			MessageIL msg = all.remove(message);
			if (msg == null) {
				msg = new MessageIL(message);
			}
			newAll.put(message, msg);
		}
		
		for (MessageIL msg : all.values()) {
			msg.destroy();
		}
		
		all = newAll;
	}
}
