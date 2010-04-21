package edu.umich.soar.sproom.comm;

import java.awt.BorderLayout;

import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

public class CommWindow
{
	public CommWindow(Messages messages)
	{
		JFrame jf = new JFrame("Communication");
		jf.setLayout(new BorderLayout());

		final JTextArea commOutput = new JTextArea();
        commOutput.setEditable(false);
        commOutput.setRows(4);
        commOutput.setLineWrap(true);
        commOutput.setWrapStyleWord(true);
        jf.add(new JScrollPane(commOutput), BorderLayout.CENTER);

        final JTextField commInput = new JTextField();
        commInput.addKeyListener(new java.awt.event.KeyAdapter() {
			public void keyTyped(java.awt.event.KeyEvent e) {
				if (e.getKeyChar() == '\n') {
					e.consume();
					commInput.getText();
					commInput.setText("");
				}
			}
		});
        jf.add(commInput, BorderLayout.SOUTH);
        
        messages.registerReceiver(null, new CommReceiver() {
        	@Override
        	public void receiveMessage(Message message)
        	{
        		commOutput.append(message.toString());
        		commOutput.append("\n");
            	commOutput.setCaretPosition(commOutput.getDocument().getLength());
        	}
        });
        
		jf.setSize(600, 500);
		jf.setVisible(true);
	}
}
