package edu.umich.soar.room;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.io.Writer;
import java.util.concurrent.atomic.AtomicBoolean;

import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.SwingUtilities;

import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;
import org.apache.log4j.WriterAppender;
import org.flexdock.docking.DockingConstants;

public class LogView extends AbstractAgentView {
	private static final long serialVersionUID = -4860644595772659771L;
	private final JTextArea theLog = new JTextArea();
	private final JButton clearButton = new JButton();
    private final Writer outputWriter = new Writer()
    {
        private StringBuilder buffer = new StringBuilder();
        private AtomicBoolean flushing = new AtomicBoolean(false);
        
        @Override
        public void close() throws IOException
        {
        }

        @Override
        synchronized public void flush() throws IOException
        {
            // If there's already a runnable headed for the UI thread, don't send another
        	if (!flushing.compareAndSet(false, true)) {
        		return;
        	}
            
            SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    synchronized(outputWriter) // synchronized on outer.this like the flush() method
                    {
                    	theLog.append(buffer.toString());
                    	theLog.setCaretPosition(theLog.getDocument().getLength());
                        buffer.setLength(0);
                        flushing.set(false);
                    }
                }
            });
        }

        @Override
        synchronized public void write(char[] cbuf, int off, int len) throws IOException
        {
            buffer.append(cbuf, off, len);
        }
    };

    public LogView(Adaptable app) {
        super("logView", "Log");
        
        addAction(DockingConstants.PIN_ACTION);
        addAction(DockingConstants.CLOSE_ACTION);

        final JPanel p = new JPanel(new BorderLayout());
        
        theLog.setEditable(false);
        theLog.setRows(4);
        
        Logger logger = Logger.getRootLogger();
        logger.addAppender(new WriterAppender(new PatternLayout("%-4r %m%n"), outputWriter));
        p.add(new JScrollPane(theLog), BorderLayout.CENTER);
        
        clearButton.setText("Clear");
        clearButton.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				synchronized(outputWriter) {
					theLog.setText("");
				}
			}
		});
        p.add(clearButton, BorderLayout.EAST);
        
        setContentPane(p);
    }
  
    @Override
	public void refresh() {
	}
}
