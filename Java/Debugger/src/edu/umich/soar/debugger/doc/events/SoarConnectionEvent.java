/********************************************************************************************
 *
 * SoarConnectionEvent.java
 * 
 * Description:	
 * 
 * Created on 	Jan 29, 2005
 * @author 		Douglas Pearson
 * 
 * Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
 ********************************************************************************************/
package edu.umich.soar.debugger.doc.events;

import java.util.EventObject;

/************************************************************************
 * 
 * Fired when a kernel is created/destroyed or we connect/disconnect etc.
 * 
 ************************************************************************/
public class SoarConnectionEvent extends EventObject
{
    private static final long serialVersionUID = -3797441927796559669L;

    /** True if connecting, false if disconnecting */
    private boolean m_Connect;

    /** True if remote, false if local */
    private boolean m_Remote;

    public SoarConnectionEvent(Object source, boolean connect, boolean remote)
    {
        super(source);

        m_Connect = connect;
        m_Remote = remote;
    }

    public boolean isConnect()
    {
        return m_Connect;
    }

    public boolean isRemote()
    {
        return m_Remote;
    }
}
