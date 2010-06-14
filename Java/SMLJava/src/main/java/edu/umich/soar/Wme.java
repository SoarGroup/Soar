package edu.umich.soar;

/**
 * Simple interface for simple wrappers for valued working memory elements
 * in SML. These wrappers are slightly more convenient to work with than
 * the default SML valued elements.
 * 
 * @author Jonathan Voigt <voigtjr@gmail.com>
 *
 */
public interface Wme
{
    /**
     * Remove the working memory element from memory. It is legal to 
     * call update() later to set a new value.
     */
    public void destroy();
}
