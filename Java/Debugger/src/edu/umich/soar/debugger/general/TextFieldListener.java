/********************************************************************************************
 *
 * TextFieldAdapter.java
 *
 * Created on 	Nov 9, 2003
 *
 * @author 		Doug
 * @version
 *
 * Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
 ********************************************************************************************/
package edu.umich.soar.debugger.general;

import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

/********************************************************************************************
 *
 * Makes attaching a document adapter to a text field simpler, when you just
 * care that it's changed, not how it has changed.
 *
 ********************************************************************************************/
public abstract class TextFieldListener implements DocumentListener
{
    public abstract void textUpdate(DocumentEvent e);

    @Override
    public void insertUpdate(DocumentEvent e)
    {
        textUpdate(e);
    }

    @Override
    public void removeUpdate(DocumentEvent e)
    {
        textUpdate(e);
    }

    @Override
    public void changedUpdate(DocumentEvent e)
    {
        textUpdate(e);
    }
}
