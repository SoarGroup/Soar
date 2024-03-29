/********************************************************************************************
 *
 * CommandsMenu.java
 *
 * Description:
 *
 * Created on 	Apr 12, 2005
 * @author 		Douglas Pearson
 *
 * Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
 ********************************************************************************************/
package edu.umich.soar.debugger.menu;

import org.eclipse.swt.widgets.Menu;

import edu.umich.soar.debugger.MainFrame;
import edu.umich.soar.debugger.doc.Document;

/********************************************************************************************
 *
 * The commands menu
 *
 ********************************************************************************************/
public class CommandsMenu
{
    private MainFrame m_Frame = null;

    private Document m_Document = null;

    private final AbstractAction m_RestartAgent = new AbstractAction(
            "Clear &working memory")
    {
        @Override
        public void actionPerformed(ActionEvent e)
        {
            initSoar(e);
        }
    };

    private final AbstractAction m_ExciseAll = new AbstractAction(
            "Excise &all productions")
    {
        @Override
        public void actionPerformed(ActionEvent e)
        {
            exciseAll(e);
        }
    };

    private final AbstractAction m_ExciseChunks = new AbstractAction("Excise &chunks")
    {
        @Override
        public void actionPerformed(ActionEvent e)
        {
            exciseChunks(e);
        }
    };

    private final AbstractAction m_ExciseTask = new AbstractAction(
            "Excise &task productions")
    {
        @Override
        public void actionPerformed(ActionEvent e)
        {
            exciseTask(e);
        }
    };

    private final AbstractAction m_ExciseUser = new AbstractAction(
            "Excise &user productions")
    {
        @Override
        public void actionPerformed(ActionEvent e)
        {
            exciseUser(e);
        }
    };

    private final AbstractAction m_ExciseDefault = new AbstractAction(
            "Excise &default productions")
    {
        @Override
        public void actionPerformed(ActionEvent e)
        {
            exciseDefault(e);
        }
    };

    /** Create this menu */
    public static CommandsMenu createMenu(MainFrame frame, Document doc,
            String title)
    {
        CommandsMenu menu = new CommandsMenu();
        menu.m_Frame = frame;
        menu.m_Document = doc;

        menu.makeMenu(frame.getMenuBar(), title);

        return menu;
    }

    private BaseMenu makeMenu(Menu parent, String title)
    {
        BaseMenu menu = new BaseMenu(parent, title);

        menu.add(m_RestartAgent);

        menu.addSeparator();
        // BaseMenu excise = menu.addSubmenu("Excise productions") ;
        menu.add(m_ExciseAll);
        menu.add(m_ExciseChunks);
        menu.add(m_ExciseTask);
        menu.add(m_ExciseUser);
        menu.add(m_ExciseDefault);

        return menu;
    }

    private void initSoar(ActionEvent e)
    {
        String sourceLine = m_Document.getSoarCommands().getInitSoarCommand();
        m_Frame.executeCommandPrimeView(sourceLine, true);
    }

    private void exciseAll(ActionEvent e)
    {
        String sourceLine = m_Document.getSoarCommands().getExciseAllCommand();
        m_Frame.executeCommandPrimeView(sourceLine, true);
    }

    private void exciseChunks(ActionEvent e)
    {
        String sourceLine = m_Document.getSoarCommands()
                .getExciseChunksCommand();
        m_Frame.executeCommandPrimeView(sourceLine, true);
    }

    private void exciseTask(ActionEvent e)
    {
        String sourceLine = m_Document.getSoarCommands().getExciseTaskCommand();
        m_Frame.executeCommandPrimeView(sourceLine, true);
    }

    private void exciseUser(ActionEvent e)
    {
        String sourceLine = m_Document.getSoarCommands().getExciseUserCommand();
        m_Frame.executeCommandPrimeView(sourceLine, true);
    }

    private void exciseDefault(ActionEvent e)
    {
        String sourceLine = m_Document.getSoarCommands()
                .getExciseDefaultCommand();
        m_Frame.executeCommandPrimeView(sourceLine, true);
    }

}
