/********************************************************************************************
 *
 * EditMenu.java
 *
 * Created on 	Nov 9, 2003
 *
 * @author 		Doug
 * @version
 *
 * Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
 ********************************************************************************************/
package edu.umich.soar.debugger.menu;

import org.eclipse.swt.widgets.Menu;

import edu.umich.soar.debugger.MainFrame;
import edu.umich.soar.debugger.dialogs.SearchDialog;
import edu.umich.soar.debugger.doc.Document;

import static edu.umich.soar.debugger.menu.ShortcutUtils.SHORTCUT_HINT;
import static edu.umich.soar.debugger.menu.ShortcutUtils.SHORTCUT_KEY;

/********************************************************************************************
 *
 * The edit menu
 *
 ********************************************************************************************/
public class EditMenu
{
    private MainFrame m_Frame = null;

    private SearchDialog m_SearchDialog = null;

    private AbstractAction m_Copy = new AbstractAction("&Copy\t" + SHORTCUT_HINT + "C")
    {
        @Override
        public void actionPerformed(ActionEvent e)
        {
            copy();
        }
    };

    private AbstractAction m_Paste = new AbstractAction("&Paste\t" + SHORTCUT_HINT + "V")
    {
        @Override
        public void actionPerformed(ActionEvent e)
        {
            paste();
        }
    };

    private AbstractAction m_Search = new AbstractAction("&Find...\t" + SHORTCUT_HINT + "F")
    {
        @Override
        public void actionPerformed(ActionEvent e)
        {
            searchPrime();
        }
    };

    /** Create this menu */
    public static EditMenu createMenu(MainFrame frame, Document doc,
            String title)
    {
        EditMenu menu = new EditMenu();
        menu.m_Frame = frame;

        menu.makeMenu(frame.getMenuBar(), title);

        return menu;
    }

    private BaseMenu makeMenu(Menu parent, String title)
    {
        BaseMenu menu = new BaseMenu(parent, title);

        menu.add(m_Copy);
        menu.add(m_Paste);
        menu.addSeparator();
        menu.add(m_Search, SHORTCUT_KEY + 'F');

        return menu;
    }

    public void searchPrime()
    {
        if (m_SearchDialog != null && !m_SearchDialog.isDisposed())
        {
            // If the search dialog is up and we get another request
            // to bring it up (probably a Ctrl-F typed) then do a find.
            m_SearchDialog.find();
            return;
        }

        m_SearchDialog = SearchDialog.showDialog(m_Frame, "Search for text",
                m_Frame.getPrimeView());
    }

    private void copy()
    {
        // Copying from the view with the current focus should give us the place
        // where the user last selected text
        edu.umich.soar.debugger.modules.AbstractView view = m_Frame
                .getMainWindow().getFocusView();

        if (view != null)
            view.copy();
    }

    private void paste()
    {
        // Pasting the view with the focus preserves symmetry with copy.
        // Could reasonably use getPrimeView() here instead.
        edu.umich.soar.debugger.modules.AbstractView view = m_Frame
                .getMainWindow().getFocusView();

        if (view != null)
            view.paste();
    }
}
