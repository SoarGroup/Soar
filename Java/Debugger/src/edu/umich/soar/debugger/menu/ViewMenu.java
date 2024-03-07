package edu.umich.soar.debugger.menu;

import edu.umich.soar.debugger.MainFrame;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.FontData;
import org.eclipse.swt.widgets.Menu;

import static edu.umich.soar.debugger.menu.ShortcutUtils.SHORTCUT_HINT;

/**
 * Offer the user options for changing the view/layout.
 */
public class ViewMenu {
    private MainFrame m_Frame = null;

    private final AbstractAction m_ChooseFont = new AbstractAction("&Choose text font...") {
        @Override
        public void actionPerformed(ActionEvent e) {
            chooseFontPerformed();
        }
    };

    private final AbstractAction m_DefaultFont = new AbstractAction("Use &default font") {
        @Override
        public void actionPerformed(ActionEvent e) {
            useDefaultFont();
        }
    };

    private static final String ZOOM_IN_HINT = SHORTCUT_HINT + "=";
    private static final String ZOOM_OUT_HINT = SHORTCUT_HINT + "-";

    private final AbstractAction m_ZoomIn = new AbstractAction("Zoom &in\t" + ZOOM_IN_HINT) {
        @Override
        public void actionPerformed(ActionEvent e) {
            Font currentFont = m_Frame.getTextFont();
            FontData[] fontData = currentFont.getFontData();
            if (fontData.length != 1) {
                System.err.println("Expected 1 fontData but found " + fontData.length + "; can't zoom in");
                return;
            }
            FontData fontDatum = fontData[0];
            fontDatum.setHeight(fontDatum.getHeight() + 1);
            m_Frame.setTextFont(fontDatum);
        }
    };

    private final AbstractAction m_ZoomOut = new AbstractAction("Zoom &out\t" + ZOOM_OUT_HINT) {
        @Override
        public void actionPerformed(ActionEvent e) {
            Font currentFont = m_Frame.getTextFont();
            FontData[] fontData = currentFont.getFontData();
            if (fontData.length != 1) {
                System.err.println("Expected 1 fontData but found " + fontData.length + "; can't zoom out");
                return;
            }
            FontData fontDatum = fontData[0];
            fontDatum.setHeight(fontDatum.getHeight() - 1);
            m_Frame.setTextFont(fontDatum);
        }
    };

    /**
     * Create this menu
     */
    public static ViewMenu createMenu(MainFrame frame, String title) {
        ViewMenu menu = new ViewMenu();
        menu.m_Frame = frame;

        menu.makeMenu(frame.getMenuBar(), title);

        return menu;
    }

    private void makeMenu(Menu parent, String title) {
        BaseMenu menu = new BaseMenu(parent, title);

        menu.add(m_ChooseFont);
        menu.add(m_DefaultFont);
        menu.add(m_ZoomIn, ShortcutUtils.SHORTCUT_KEY + '=');
        menu.add(m_ZoomOut, ShortcutUtils.SHORTCUT_KEY + '-');
    }

    private void useDefaultFont() {
        m_Frame.setTextFont(MainFrame.kDefaultFontData);
    }

    private void chooseFontPerformed() {
        FontData data = m_Frame.ShowFontDialog();

        if (data != null) m_Frame.setTextFont(data);
    }
}
