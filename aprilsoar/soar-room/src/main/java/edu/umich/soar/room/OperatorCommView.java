package edu.umich.soar.room;

import java.awt.BorderLayout;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.imageio.ImageIO;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.flexdock.docking.DockingConstants;

import edu.umich.soar.sps.control.robot.SendMessages;
import edu.umich.soar.sps.control.robot.SendMessagesInterface;
import edu.umich.soar.room.core.Simulation;
import edu.umich.soar.room.core.events.PlayerAddedEvent;
import edu.umich.soar.room.core.events.PlayerRemovedEvent;
import edu.umich.soar.room.events.SimEvent;
import edu.umich.soar.room.events.SimEventListener;
import edu.umich.soar.room.map.CommListener;
import edu.umich.soar.room.map.CommMessage;
import edu.umich.soar.room.map.Robot;

public class OperatorCommView extends AbstractAgentView implements SimEventListener {
	private static final Log logger = LogFactory.getLog(OperatorCommView.class);
	
	private static final long serialVersionUID = -1676542480621263207L;
	private final Simulation sim;
	private final JLabel commWarn = new JLabel();
	private final JTextField commInput = new JTextField();
	private final SendMessagesInterface sendMessages;
	private final JTextArea commOutput = new JTextArea();
	private final JComboBox commDestination = new JComboBox();
	private static final String DESTINATION_ALL = "-ALL-";
	private static final String OPERATOR = "operator";
	private static final String WIRES = "WIRES";
	private static final String PICTURE = "PICTURE";
	private static final String CLEAR_IMAGE = "CLEAR";

    private final CommListener outputListener = new CommListener()
    {
    	@Override
        public void write(final CommMessage message)
        {
    		if (message.isBroadcast() || message.getDestination().equals(OPERATOR) || message.getFrom().equals(OPERATOR)) {
	            SwingUtilities.invokeLater(new Runnable() {
	                public void run() {
	                    synchronized(outputListener) // synchronized on outer.this like the flush() method
	                    {
	                    	List<String> tokens = message.getTokens();
	                    	boolean picture = false;
	                    	for (String tok : tokens) {
	                    		if (picture) {
	                    			activateImage(tok, message.getId(), message.getFrom());
	                    		}
	                    		if (tok.equals(CLEAR_IMAGE)) {
	                    			clearImage();
	                    		} else if (tok.equals(WIRES)) {
	                        		activateImage(WIRES, message.getId(), message.getFrom());
	                    		} else if (tok.equals(PICTURE)) {
	                    			picture = true;
	                    		}
	                    	}
	                    	if (picture) {
	                    		logger.warn("PICTURE command did not have argument");
	                    	}
	                    	
	                    	commOutput.append(message.toString());
	                    	commOutput.append("\n");
	                    	commOutput.setCaretPosition(commOutput.getDocument().getLength());
	                    }
	                }
	            });
    		}
        }
    };

    private final Map<String, Icon> images = new HashMap<String, Icon>();
    
    private Icon getIcon(String key) {
    	Icon ret = images.get(key);
    	if (ret == null) {
        	File file = sim.getConfig().getImageFile(key);
        	if (file == null) {
        		logger.warn("Undefined image key '" + key + "'.");
        		return null;
        	}
        	
            try {
            	ret = new ImageIcon(ImageIO.read(new FileInputStream(file)));
            	if (ret == null) {
            		logger.warn("Error loading image '" + file + "'.");
            		return null;
            	}
            	images.put(key, ret);
    		} catch (IOException e1) {
    			// TODO Auto-generated catch block
    			e1.printStackTrace();
        		logger.warn("IOException loading image '" + file + "'.");
        		return null;
    		}
    	}
    	return ret;
    }

    public OperatorCommView(Adaptable app) {
        super("operatorCommView", "Communication");
        addAction(DockingConstants.PIN_ACTION);
        addAction(DockingConstants.CLOSE_ACTION);

        this.sim = Adaptables.adapt(app, Simulation.class);
        this.sim.getEvents().addListener(PlayerAddedEvent.class, this);
        this.sim.getEvents().addListener(PlayerRemovedEvent.class, this);

        this.sendMessages = Adaptables.adapt(app, SendMessagesInterface.class);
        
        final JPanel p = new JPanel(new BorderLayout());

        p.add(commDestination, BorderLayout.NORTH);
        
        final JPanel outputPanel = new JPanel(new BorderLayout());

        commOutput.setEditable(false);
        commOutput.setRows(4);
        commOutput.setLineWrap(true);
        commOutput.setWrapStyleWord(true);
        this.sim.getWorld().addCommListener(outputListener);
        outputPanel.add(new JScrollPane(commOutput), BorderLayout.CENTER);
        
        commWarn.setHorizontalTextPosition(JLabel.CENTER);
        commWarn.setVerticalTextPosition(JLabel.BOTTOM);
        outputPanel.add(commWarn, BorderLayout.EAST);

        p.add(outputPanel, BorderLayout.CENTER);
        
        commInput.addKeyListener(new java.awt.event.KeyAdapter() {
			public void keyTyped(java.awt.event.KeyEvent e) {
				if (e.getKeyChar() == '\n') {
					e.consume();
					sendMessage();
				}
			}
		});

        p.add(commInput, BorderLayout.SOUTH);
        final JButton commSend = new JButton("Send");
        commSend.addActionListener(new java.awt.event.ActionListener() {
			public void actionPerformed(java.awt.event.ActionEvent e) {
				sendMessage();
			}
        });
        p.add(commSend, BorderLayout.EAST);
        
        setContentPane(p);
    }
    
    private void sendMessage() {
    	Object selectedObject = commDestination.getSelectedItem();
    	String dest = null;
    	if (selectedObject != null) {
    		if (selectedObject instanceof Robot) {
    			dest = selectedObject.toString();
    		}
    	}
    	List<String> tokens = SendMessages.toTokens(commInput.getText());
    	if (!tokens.isEmpty()) {
    		sendMessages.sendMessage(OPERATOR, dest, tokens);
    		commInput.setText("");
    	}
    }
    
    private void activateImage(String key, long id, String from) {
    	commWarn.setIcon(getIcon(key));
    	commWarn.setText(String.format("From: %s, Message: %d", from, id));
    }

    private void clearImage() {
    	commWarn.setIcon(null);
    	commWarn.setText(null);
    }

	@Override
	public void refresh() {
	}

	@Override
	public void onEvent(SimEvent event) {
		commDestination.removeAllItems();
        commDestination.addItem(DESTINATION_ALL);
        for (Robot robot : sim.getWorld().getPlayers()) {
        	commDestination.addItem(robot);
        }
	}
}
