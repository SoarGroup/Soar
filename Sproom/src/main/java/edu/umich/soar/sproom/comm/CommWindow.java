package edu.umich.soar.sproom.comm;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Point;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.prefs.BackingStoreException;
import java.util.prefs.Preferences;

import javax.swing.DefaultComboBoxModel;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.SwingUtilities;

import edu.umich.soar.sproom.Sproom;

public class CommWindow
{
	private static final String KEY_WIDTH = "width";
	private static final String KEY_HEIGHT = "height";
	private static final String KEY_X = "x";
	private static final String KEY_Y = "y";
	
	private final JFrame frame;
	private final JTextArea trace = new JTextArea();
	private final DefaultComboBoxModel promptModel = new DefaultComboBoxModel();
	private final JComboBox prompt = new JComboBox(promptModel);
	
	public CommWindow(final Messages messages)
	{
		final Preferences pref = Sproom.PREFERENCES;
		
		frame = new JFrame("Communication");

		frame.setSize(pref.getInt(KEY_WIDTH, 800), pref.getInt(KEY_HEIGHT, 400));
		frame.setLocation(pref.getInt(KEY_X, 200), pref.getInt(KEY_Y, 200));
		
		frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
		frame.addWindowListener(new WindowAdapter() {
			@Override
			public void windowClosing(WindowEvent e) {
				Dimension d = frame.getSize();
				pref.putInt(KEY_WIDTH, d.width);
				pref.putInt(KEY_HEIGHT, d.height);

				Point p = frame.getLocation();
				pref.putInt(KEY_X, p.x);
				pref.putInt(KEY_Y, p.y);
				
				try {
					pref.flush();
				} catch (BackingStoreException e1) {
					e1.printStackTrace();
				}
				
				super.windowClosing(e);
			}			
		});
		
		//trace.setFont(new Font("Monospaced", Font.PLAIN, 12));
		trace.setEditable(false);
        trace.setLineWrap(true);
        trace.setWrapStyleWord(true);
		trace.setCaretPosition(trace.getDocument().getLength());

		prompt.setEditable(true);

		prompt.getEditor().addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				messages.sendMessage("human", null, prompt.getEditor().getItem().toString().trim());
			}});

		JPanel panel = new JPanel(new BorderLayout());

		panel.add(new JScrollPane(trace), BorderLayout.CENTER);
		panel.add(prompt, BorderLayout.SOUTH);
		
		frame.add(panel);
		
        messages.registerReceiver(null, new CommReceiver() {
        	@Override
        	public void receiveMessage(final Message message)
        	{
				SwingUtilities.invokeLater(new Runnable() {
					public void run() {
						trace.append(message.toString());
						trace.append("\n");
						// TODO: possibly truncate length
						trace.setCaretPosition(trace.getDocument().getLength());
					}
				});
        	}
        });

        frame.setVisible(true);

		prompt.requestFocus();
	}
}
