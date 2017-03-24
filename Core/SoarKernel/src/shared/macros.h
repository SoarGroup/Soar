/* -------------- Macros for safe counters ------------*/

#define increment_counter(counter) counter++; if (counter == 0) counter = 1;
#define add_to_counter(counter, amt) uint64_t lastcnt = counter; counter += amt; if (counter < lastcnt) counter = amt;

/* ---------------------------------------------------------------------
     Macros for Inserting and Removing Stuff from Doubly-Linked Lists

   Note: fast_remove_from_dll() is the same as remove_from_dll() except
   slightly faster.  I (RBD) only realized this after writing all the
   original code.  With fast_remove_from_dll(), you have to tell it
   the type (i.e., structure name) of the item being spliced out of
   the list.  At some point we might want to go through all the code
   and have it use fast_remove_from_dll(), but it's probably not worth
   the effort right now.
-------------------------------------------------------------------- */

/* This macro cannot be easily converted to an inline function.
   Some additional changes are required.
*/
#define insert_at_head_of_dll(header,item,next_field_name,prev_field_name) { \
        ((item)->next_field_name) = (header) ; \
        ((item)->prev_field_name) = NIL ; \
        if (header) ((header)->prev_field_name) = (item) ; \
        (header) = (item) ; }
/*template <typename T>
inline void insert_at_head_of_dll(T header, T item, T next_field_name,
                                  T prev_field_name)
{
  ((item)->next_field_name) = (header);
  ((item)->prev_field_name) = NIL;
  if (header) ((header)->prev_field_name) = (item);
  (header) = (item);
}*/

/* This macro cannot be easily converted to an inline function.
   Some additional changes are required.
*/
#define remove_from_dll(header,item,next_field_name,prev_field_name) { \
        if ((item)->next_field_name) \
            ((item)->next_field_name->prev_field_name) = ((item)->prev_field_name); \
        if ((item)->prev_field_name) { \
            ((item)->prev_field_name->next_field_name) = ((item)->next_field_name); \
        } else { \
            (header) = ((item)->next_field_name); \
        } }
/*template <typename T>
inline void remove_from_dll(T header, T item, T next_field_name,
                            T prev_field_name)
{
  if ((item)->next_field_name)
    ((item)->next_field_name->prev_field_name) = ((item)->prev_field_name);
  if ((item)->prev_field_name) {
    ((item)->prev_field_name->next_field_name) = ((item)->next_field_name);
  } else {
    (header) = ((item)->next_field_name);
  }
}*/

/* This macro cannot be easily converted to an inline function.
   Some additional changes are required.
*/
#define fast_remove_from_dll(header,item,typename,next_field_name,prev_field_name) { \
        typename *tempnext, *tempprev; \
        tempnext = (item)->next_field_name; \
        tempprev = (item)->prev_field_name; \
        if (tempnext) tempnext->prev_field_name = tempprev; \
        if (tempprev) { \
            tempprev->next_field_name = tempnext; \
        } else { \
            (header) = tempnext; } }


// Useful for converting enumerations to string
#define stringify( name ) # name
