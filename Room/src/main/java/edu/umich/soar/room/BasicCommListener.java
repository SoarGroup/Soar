package edu.umich.soar.room;

import javax.swing.JTextArea;
import javax.swing.SwingUtilities;

import edu.umich.soar.room.map.CommListener;
import edu.umich.soar.room.map.CommMessage;

class BasicCommListener implements CommListener {
	private final JTextArea commOutput;
	
	BasicCommListener(JTextArea commOutput) {
		this.commOutput = commOutput;
	}
	
	@Override
	public void write(final CommMessage message) {
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
