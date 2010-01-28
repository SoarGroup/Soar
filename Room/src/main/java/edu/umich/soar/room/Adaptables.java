package edu.umich.soar.room;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

public class Adaptables {

    /**
     * Adapt an object to the given class. This is equivalent to o.getAdapter(klass)
     * but o may be null or not adaptable and casting is handled automatically with 
     * generics.
     * 
     * @param <T> The desired type
     * @param o The object to adapt. If not Adaptable, then a simple instanceof test
     *      is performed. May be null.
     * @param klass The desired class. May not be null
     * @return An object of the desired type, or null.
     */
    public static <T> T adapt(Object o, Class<T> klass)
    {
        return adapt(o, klass, true);
    }
    
    public static <T> T adapt(Object o, Class<T> klass, boolean recurse)
    {
        if(o == null)
        {
            return null;
        }
        else if(klass.isInstance(o))
        {
            return klass.cast(o);
        }
        else if(recurse && (o instanceof Adaptable))
        {
            return klass.cast(((Adaptable) o).getAdapter(klass));
        }
        return null;
    }
    
    /**
     * Filter a collection of objects to those that are adaptable to a particular
     * type.
     * 
     * @param <T> Desired type
     * @param collection Collection to filter
     * @param klass Desired type class
     * @return List of objects that are adaptable to T.
     */
    public static <T> List<T> adaptCollection(Collection<?> collection, Class<T> klass)
    {
        List<T> r = new ArrayList<T>();
        
        for(Object o : collection)
        {
            T t = adapt(o, klass);
            if(t != null)
            {
                r.add(t);
            }
        }
        
        return r;
    }

    /**
     * Search a collection for the first object that is adaptable to a particular
     * type.
     * 
     * @param <T> Desired type
     * @param collection Collection to search
     * @param klass Desired type class
     * @return First object in collection that is adaptable to T, or null if
     *  none was found.
     */
    public static <T> T findAdapter(Collection<?> collection, Class<T> klass)
    {
        for(Object o : collection)
        {
            T t = adapt(o, klass);
            if(t != null)
            {
                return t;
            }
        }
        return null;
    }
    
    public static <T> T require(Class<?> requiringClass, Object context, Class<T> requiredClass)
    {
        final T t = Adaptables.adapt(context, requiredClass);
        if(t == null)
        {
            throw new IllegalStateException(requiringClass.getName() + " requires " + requiredClass.getCanonicalName());
        }
        return t;
    } 


}
