/*
 * Copyright (c) 2009  Dave Ray <daveray@gmail.com>
 *
 * Created on Mar 21, 2009
 */
package edu.umich.soar.room;

import javax.swing.SwingUtilities;

/**
 * Wrap a completion handler in logic to ensure that it executes on the 
 * Swing event thread.
 * 
 * @author ray
 * @param <T>
 */
public class SwingCompletionHandler<T> implements CompletionHandler<T>
{
    private final CompletionHandler<T> inner;
    
    public static <V> CompletionHandler<V> newInstance(CompletionHandler<V> inner) 
    {
        return new SwingCompletionHandler<V>(inner);
    }
    
    private SwingCompletionHandler(CompletionHandler<T> inner)
    {
        this.inner = inner;
    }
    
    /* (non-Javadoc)
     * @see org.jsoar.runtime.Result#finish(java.lang.Object)
     */
    @Override
    public void finish(final T result)
    {
        if(SwingUtilities.isEventDispatchThread())
        {
            inner.finish(result);
        }
        else
        {
            SwingUtilities.invokeLater(new Runnable() {

                @Override
                public void run()
                {
                    inner.finish(result);
                }});
        }
    }

}
