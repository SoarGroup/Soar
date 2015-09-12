/***************************************************
 * File: SVS/src/change_tracking_list.h
 *
 * Contains:
 *   class ctlist_listener<T>
 *   class change_tracking_list<T>
 *
 * This defines a generic list that tracks changes
 * to itself and which can be listened to by
 * a class implementing the ctlist_listener interface
 ***************************************************/

#ifndef CHANGE_TRACKING_LIST_H
#define CHANGE_TRACKING_LIST_H

#include <string>
#include <list>
#include <map>
#include <sstream>
#include <iterator>
#include <vector>


/*
 * class ctlist_listener<T>
 *
 * Purpose: an interface for an object which listens to changes
 * on a change_tracking_list.
 * The 3 listening functions will be called when an item's
 * status within the list changes
 */
template<class T>
class ctlist_listener
{
    public:
        // Called when an item is added to the list
        virtual void handle_ctlist_add(const T* e) {}
        
        // Called when an item is removed from the list
        virtual void handle_ctlist_remove(const T* e) {}
        
        // Called when an item in the list changes
        // (Occurs when change_tracking_list.change(T) is called)
        virtual void handle_ctlist_change(const T* e) {}
};

/*
 * class change_tracking_list<T>
 *
 * Purpose:
 * A list that keeps track of changes made to it,
 * so that users can respond to only the things changed
 *
 * Items in the list can have the following designations:
 *   added - the item has been recently added to the list
 *   changed - the item has been recently changed
 *   removed - the item has been recently removed
 *   current - all non-removed items in the list
 *   These designations are reset when clear_changes is called
 *
 * Assumptions:
 * - Owns the memory of the items in the list.
 *   This list is responsible for freeing the memory for items removed from it
 * - Users of this list are responsible for using it correctly
 *   This list will not catch or handle errors caused by misuses like:
 *     Adding the same item multiple times
 *     Removing or changing an item that is not in the list
 *     Accessing items out of bounds
 *     Adding an item you previously removed from the list
*/

template<class T>
class change_tracking_list
{
    public:
        typedef ctlist_listener<T> listener;
        
        // Constructor
        change_tracking_list()
        {
            m_added_begin = 0;
        }
        // Destructor
        //   Frees up memory for all items within the list
        virtual ~change_tracking_list()
        {
            clear();
        }
        
        
        ////////////////////////
        // Changing the list
        ////////////////////////
        
        // Puts an item in the list
        //   will invoke ctlist_listener::handle_ctlist_add
        virtual void add(T* v)
        {
            current.push_back(v);
            for (size_t i = 0; i < listeners.size(); ++i)
            {
                listeners[i]->handle_ctlist_add(v);
            }
        }
        
        // Removes an item from the list
        //   will invoke ctlist_listener::handle_ctlist_remove
        //   The item is not actually deleted until clear_changes is called
        virtual void remove(const T* v)
        {
            bool found = false;
            for (size_t i = 0; i < current.size(); ++i)
            {
                if (current[i] == v)
                {
                    removed.push_back(current[i]);
                    current.erase(current.begin() + i);
                    if (i < m_added_begin)
                    {
                        --m_added_begin;
                    }
                    found = true;
                    break;
                }
            }
            assert(found);
            for (size_t i = 0; i < changed.size(); ++i)
            {
                if (changed[i] == v)
                {
                    changed.erase(changed.begin() + i);
                    break;
                }
            }
            for (size_t i = 0; i < listeners.size(); ++i)
            {
                listeners[i]->handle_ctlist_remove(v);
            }
        }
        
        // Marks an item in the list as changed
        //   will invoke ctlist_listener::handle_ctlist_change
        virtual void change(const T* v)
        {
            for (size_t i = 0; i < current.size(); ++i)
            {
                if (current[i] == v)
                {
                    if (i < m_added_begin &&
                            find(changed.begin(), changed.end(), current[i]) == changed.end())
                    {
                        changed.push_back(current[i]);
                        for (size_t i = 0; i < listeners.size(); ++i)
                        {
                            listeners[i]->handle_ctlist_change(current[i]);
                        }
                    }
                    return;
                }
            }
            assert(false);
        }
        
        
        ////////////////////////
        // Managing the list
        ////////////////////////
        
        // Clears all changes in the list so all items remaining are marked as current
        // Updates all lists so there is nothing in the removed or changed lists
        //   and no items are designated as added
        virtual void clear_changes()
        {
            m_added_begin = current.size();
            changed.clear();
            clear_removed();
        }
        
        // Clears all changes and causes all items to be marked as newly added
        virtual void reset()
        {
            changed.clear();
            clear_removed();
            m_added_begin = 0;
        }
        
        // Removes all items from the list and frees their memory
        // (will call ctlist_listener::handle_ctlist_remove on all current items)
        virtual void clear()
        {
            // Clear changed list
            changed.clear();
            
            // Clear current list
            m_added_begin = 0;
            for (size_t i = 0; i < current.size(); i++)
            {
                for (size_t j = 0; j < listeners.size(); ++j)
                {
                    listeners[j]->handle_ctlist_remove(current[i]);
                }
                removed.push_back(current[i]);
            }
            current.clear();
            
            // Clear removed list
            clear_removed();
        }
        
        
        ///////////////////////
        // change listeners
        ///////////////////////
        void listen(ctlist_listener<T>* l)
        {
            listeners.push_back(l);
        }
        
        void unlisten(ctlist_listener<T>* l)
        {
            typename std::vector<ctlist_listener<T>*>::iterator i;
            i = std::find(listeners.begin(), listeners.end(), l);
            if (i != listeners.end())
            {
                listeners.erase(i);
            }
        }
        
        //////////////////////
        // accessors
        /////////////////////
        
        // current:
        //   all items in the list (including added + changed)
        //   but not those that have been removed
        size_t num_current() const
        {
            return current.size();
        }
        
        T* get_current(size_t i)
        {
            return current[i];
        }
        
        const T* get_current(size_t i) const
        {
            return current[i];
        }
        
        // added:
        //   All items added since clear_changes was last called
        //   Added items can be accessed by get_current
        //   from index i where first_added() <= i < to num_current()
        size_t first_added() const
        {
            return m_added_begin;
        }
        
        // changed:
        //   All items changed since clear_changes was last called
        size_t num_changed() const
        {
            return changed.size();
        }
        
        T* get_changed(size_t i)
        {
            return changed[i];
        }
        
        const T* get_changed(size_t i) const
        {
            return changed[i];
        }
        
        // removed:
        //   All items removed since clear_changes was last called
        size_t num_removed() const
        {
            return removed.size();
        }
        
        T* get_removed(size_t i)
        {
            return removed[i];
        }
        
        const T* get_removed(size_t i) const
        {
            return removed[i];
        }
        
        
    protected:
        // Deletes all items in the removed list and clears it
        virtual void clear_removed()
        {
            for (size_t i = 0; i < removed.size(); ++i)
            {
                delete removed[i];
            }
            removed.clear();
        }
        
        std::vector<T*> current;
        std::vector<T*> removed;
        std::vector<T*> changed;
        // changed is a subset of the current list
        // (all items in changed are also in current)
        
        // Index of the first new element in the current list
        size_t m_added_begin;
        
        std::vector<ctlist_listener<T>*> listeners;
};

#endif //CHANGE_TRACKING_LIST_H
