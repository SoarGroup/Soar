package edu.umich.soar.sproom.soar;

import java.util.HashMap;
import java.util.Map;

import sml.Identifier;
import edu.umich.soar.IntWme;
import edu.umich.soar.StringWme;
import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;
import edu.umich.soar.sproom.comm.Comm;
import edu.umich.soar.sproom.comm.Message;

/**
 * Input link management of received messages.
 *
 * @author voigtjr@gmail.com
 */
public class ReceivedMessagesIL implements InputLinkElement {

	private class MessageIL {
		private final Identifier messageRoot;

		private MessageIL(Message message) {
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
	private Map<Message, MessageIL> all = new HashMap<Message, MessageIL>();

	public ReceivedMessagesIL(Identifier root, Adaptable app) {
		this.root = root;
	}

	@Override
	public void update(Adaptable app) {
		Comm comm = (Comm)app.getAdapter(Comm.class);
		Map<Message, MessageIL> newAll = new HashMap<Message, MessageIL>(all.size());
		
		for(Message message : comm.getMessages()) {
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

	@Override
	public void destroy() {
		this.root.DestroyWME();
	}
}
