package edu.umich.soar.room;

import javax.swing.JTextArea;
import javax.swing.SwingUtilities;

import edu.umich.soar.room.map.CommListener;
import edu.umich.soar.room.map.CommMessage;

class FilteredCommListener implements CommListener {
	private final JTextArea commOutput;
	private String who;
	
	FilteredCommListener(JTextArea commOutput) {
		this.commOutput = commOutput;
	}
	
	public void setFilter(String who) {
		this.who = who;
	}
	
	@Override
	public void write(final CommMessage message) {
		if (who == null || message.isBroadcast() || message.getDestination().equals(who) || message.getFrom().equals(who)) {
            SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    synchronized(this) // synchronized on outer.this like the flush() method
                    {
                    	commOutput.append(message.toString());
                    	commOutput.append("\n");
                    	commOutput.setCaretPosition(commOutput.getDocument().getLength());
                    }
                }
            });
		}
	}

}
