/*
 * Copyright (c) 2009 Dave Ray <daveray@gmail.com>
 *
 * Created on Jul 21, 2009
 */
package edu.umich.soar.room.events;

/**
 * Utility methods for use with SoarEvent classes.
 * 
 * @see SimEvent
 * @see SimEventManager
 * @see SimEventListener
 * @author ray
 */
public class SimEvents
{
    /**
     * Adds an event listener that listens for a single event and then removes 
     * itself from the event manager. Note that the event may be called from 
     * another thread, possibly before this method even returns.
     *  
     * @param <T> The event type to register for.
     * @param manager the event manager
     * @param klass the event type
     * @param listener the listener
     */
    public static <T extends SimEvent> void listenForSingleEvent(final SimEventManager manager, final Class<T> klass, final SimEventListener listener)
    {
        manager.addListener(klass, new SimEventListener() {

            @Override
            public void onEvent(SimEvent event)
            {
                try
                {
                    listener.onEvent(event);
                }
                finally
                {
                    manager.removeListener(klass, this);
                }
            }});
    }
}
