/*
 * Copyright (c) 2009  Dave Ray <daveray@gmail.com>
 *
 * Created on Mar 21, 2009
 */
package edu.umich.soar.room;

/**
 * A callback interface for handling completion of a callable in a threaded
 * agent.
 * 
 * @author ray
 */
public interface CompletionHandler<T>
{
    void finish(T result);
}
