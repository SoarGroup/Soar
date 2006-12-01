#ifndef SMART_POINTER_H
#define SMART_POINTER_H

/* Author: David Kieras */

/* An intrusive reference-counting Smart_Pointer class template
see R.B. Murray, C++ Strategy and Tactics. Addison-Wesley, 1993

Usage:

1. Inherit classes from Reference_Counted_Object

	class My_class : public Reference_Counted_Object {
	// rest of declaration as usual
	};

2. Always allocate objects with new and assign to a Smart_Pointer

		Smart_Pointer<const My_class> ptr = new My_class;
		or
		Smart_Pointer<const My_class> ptr(new My_class);
		
3. Use Smart_Pointers with the same syntax as built-in pointers;
will convert to Smart_Pointers of another type, or built-in types
where legal; can be stored in Standard Library Containers.

4. When the last Smart_Pointer pointing to an object is destructed,
or set to 0, pointed-to object will be deleted automatically.

5. Don'ts: 
Never explicitly delete the pointed-to object; set the pointer to zero instead. 
Don't assign to a built-in pointer type except for purely temporary use, and 
then use the get_raw_ptr accessor to help make it clear what is happening. 
Never assign a Smart_Pointer to point to a stack object. 

The effects of breaking these rules is undefined.
*/

/*
Reference_Counted_Objects should only be allocated using new, never the stack.
Other classes increment and decrement the use count.
If the use count hits zero as a result of decrement, the object deletes itself.
The reference count is declared mutable to allow increment/decrement_ref_count
to be declared const, so that using a Smart_Pointer on an object does not
require it to be non-const.
*/
class Reference_Counted_Object {
public:
	Reference_Counted_Object () : ref_count(0)
		{}
	Reference_Counted_Object (const Reference_Counted_Object&) : ref_count(0)
		{}
	virtual ~Reference_Counted_Object()
		{}
	void increment_ref_count() const
		{++ref_count;}
	void decrement_ref_count() const
		// suicidal - destroys this object
		{if (--ref_count == 0) delete this;}
	long get_ref_count() const
		{return ref_count;}
private:
	mutable long ref_count;	
};

/* Template for Smart_Pointer class
overloads *, ->, and =, provides conversion to built-in type.
Simply increments and decrements the reference count when Smart_Pointers
are initialized, copied, assigned, and destructed.
*/
template <class T> class Smart_Pointer {
public:
	// constructor with pointer argument - copy and increment_ref_count count
	Smart_Pointer(T* arg = 0) : ptr(arg)
		{if (ptr) ptr->increment_ref_count();}
	// copy constructor - copy and increment_ref_count
	Smart_Pointer(const Smart_Pointer<T>& other): ptr(other.ptr) 
		{if (ptr) ptr->increment_ref_count();}
	// templated constructor to support implicit conversions to other Smart_Pointer type
	template <class U> Smart_Pointer(const Smart_Pointer<U> other) : ptr(other.get_raw_ptr()) 
		{if (ptr) ptr->increment_ref_count();}
	// destructor - decrement ref count
	~Smart_Pointer()
		{if (ptr) ptr->decrement_ref_count();}
	// assignment - decrement lhs, increment rhs
	const Smart_Pointer<T>& operator= (const Smart_Pointer<T>& rhs)
		{
			if (ptr != rhs.ptr) {						//check for aliasing
				if (ptr) ptr->decrement_ref_count();
				ptr = rhs.ptr;
				if (ptr) ptr->increment_ref_count();
				}
			return *this;
		}
	// overloaded operators
	T& operator* () {return *ptr;}
	T* operator-> () const {return ptr;}
	// these operators make converting to the pointer type less necessary
	// conversion to bool to allow test for pointer non-zero
	operator bool() const {return ptr != 0;}
	// Smart_Pointers are equal if internal pointers are equal
	bool operator== (const Smart_Pointer<T>& rhs) const {return ptr == rhs.ptr;}
	// Smart_Pointers < if internal pointers are <
	bool operator< (const Smart_Pointer<T>& rhs) const {return ptr < rhs.ptr;}
	// use these with caution - preferably only in a call
	T* get_raw_ptr() const {return ptr;}	// accessor
	operator T*() const {return ptr;}   	// conversion to pointer type
private:
	T* ptr;
};

#endif


