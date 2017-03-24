/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  mempool_allocator.h
 *
 * =======================================================================
 */

#ifndef MEMPOOL_ALLOCATOR_H
#define MEMPOOL_ALLOCATOR_H


#include "kernel.h"
#include "memory_manager.h"

#include <map>
#include <string>
#include <set>
#include <list>
#include <functional>
#include <assert.h>
#include <cmath>
#include <iostream>

// separates this functionality
// just for Soar modules
namespace soar_module
{

    ///////////////////////////////////////////////////////////////////////////
    // Memory Pool Allocators
    ///////////////////////////////////////////////////////////////////////////

#ifdef USE_MEM_POOL_ALLOCATORS

    template <class T>
    class soar_memory_pool_allocator
    {
        public:
            typedef T           value_type;
            typedef size_t      size_type;
            typedef ptrdiff_t   difference_type;

            typedef T*          pointer;
            typedef const T*    const_pointer;

            typedef T&          reference;
            typedef const T&    const_reference;

        public:
            soar_memory_pool_allocator() : mem_pool(NULL), memory_manager(NULL)
        {
                memory_manager = &(Memory_Manager::Get_MPM());
                mem_pool = memory_manager->get_memory_pool(sizeof(value_type));
        }

            soar_memory_pool_allocator(agent* new_agent): mem_pool(NULL), memory_manager(NULL)
            {
                // useful for debugging
                // std::string temp_this( typeid( value_type ).name() );
                memory_manager = &(Memory_Manager::Get_MPM());
                mem_pool = memory_manager->get_memory_pool(sizeof(value_type));
            }

            soar_memory_pool_allocator(const soar_memory_pool_allocator& obj): mem_pool(NULL), memory_manager(NULL)
            {
                // useful for debugging
                // std::string temp_this( typeid( value_type ).name() );
                memory_manager = &(Memory_Manager::Get_MPM());
                mem_pool = memory_manager->get_memory_pool(sizeof(value_type));
            }

            template <class _other>
            soar_memory_pool_allocator(const soar_memory_pool_allocator<_other>& other): mem_pool(NULL), memory_manager(NULL)
            {
                    // useful for debugging
                    // std::string temp_this( typeid( T ).name() );
                    // std::string temp_other( typeid( _other ).name() );
                    memory_manager = &(Memory_Manager::Get_MPM());
                    mem_pool = memory_manager->get_memory_pool(sizeof(value_type));
            }

            pointer allocate(size_type n, const void* = 0)
            {
                //assert(memory_manager);
                //assert (n==1);
                pointer t;
                //assert(mem_pool);
                memory_manager->allocate_with_pool_ptr(mem_pool, &t);
                // assert(t);
                return t;
            }

            void deallocate(void* p, size_type n)
            {
                //assert(memory_manager && mem_pool);
                if (p)
                {
                    //assert (n==1);
                    memory_manager->free_with_pool_ptr(mem_pool, p);
                }
            }

            void construct(pointer p, const_reference val)
            {
                new(p) T(val);
            }

            void destroy(pointer p)
            {
                p->~T();
            }

            size_type max_size() const
            {
                return static_cast< size_type >(-1);
            }

            const_pointer address(const_reference r) const
            {
                return &r;
            }

            pointer address(reference r) const
            {
                return &r;
            }

            template <class U>
            struct rebind
            {
                    typedef soar_memory_pool_allocator<U> other;
            };


        private:
            //            agent* thisAgent;
            Memory_Manager* memory_manager;
            memory_pool* mem_pool;

    };

    template <class T>
    class soar_memory_pool_allocator_n
    {
        public:
            typedef T           value_type;
            typedef size_t      size_type;
            typedef ptrdiff_t   difference_type;

            typedef T*          pointer;
            typedef const T*    const_pointer;

            typedef T&          reference;
            typedef const T&    const_reference;

        public:
            soar_memory_pool_allocator_n() : mem_pool(NULL), memory_manager(NULL)
        {
                memory_manager = &(Memory_Manager::Get_MPM());
                mem_pool = memory_manager->get_memory_pool(sizeof(value_type));
        }

            soar_memory_pool_allocator_n(agent* new_agent): mem_pool(NULL), memory_manager(NULL)
            {
                // useful for debugging
                // std::string temp_this( typeid( value_type ).name() );
                memory_manager = &(Memory_Manager::Get_MPM());
                mem_pool = memory_manager->get_memory_pool(sizeof(value_type));
            }

            soar_memory_pool_allocator_n(const soar_memory_pool_allocator_n& obj): mem_pool(NULL), memory_manager(NULL)
            {
                // useful for debugging
                // std::string temp_this( typeid( value_type ).name() );
                memory_manager = &(Memory_Manager::Get_MPM());
                mem_pool = memory_manager->get_memory_pool(sizeof(value_type));
            }

            template <class _other>
            soar_memory_pool_allocator_n(const soar_memory_pool_allocator_n<_other>& other): mem_pool(NULL), memory_manager(NULL)
            {
                    // useful for debugging
                    // std::string temp_this( typeid( T ).name() );
                    // std::string temp_other( typeid( _other ).name() );
                    memory_manager = &(Memory_Manager::Get_MPM());
                    mem_pool = memory_manager->get_memory_pool(sizeof(value_type));
            }

            pointer allocate(size_type n, const void* = 0)
            {
                //assert(memory_manager);
                pointer t;
                if (n == 1)
                {
                    //assert(mem_pool);
                    memory_manager->allocate_with_pool_ptr(mem_pool, &t);
                } else {
                    memory_pool* lMem_pool = memory_manager->get_memory_pool(n*sizeof(value_type));
                    //assert(lMem_pool);
                    memory_manager->allocate_with_pool_ptr(lMem_pool, &t);
                }
                //std::cout << "Dynamic memory pool allocation of size " << n << " * " << sizeof(value_type) << " requested!" << std::endl;
                //assert(t);
                return t;
            }

            void deallocate(void* p, size_type n)
            {
                //assert(memory_manager && mem_pool);
                if (p)
                {
                    if (n == 1)
                    {
                        memory_manager->free_with_pool_ptr(mem_pool, p);
                    } else {
                        memory_pool* lMem_pool = memory_manager->get_memory_pool(n*sizeof(value_type));
                        //assert(lMem_pool);
                        memory_manager->free_with_pool_ptr(lMem_pool, p);
                    }
                }
            }

            void construct(pointer p, const_reference val)
            {
                new(p) T(val);
            }

            void destroy(pointer p)
            {
                p->~T();
            }

            size_type max_size() const
            {
                return static_cast< size_type >(-1);
            }

            const_pointer address(const_reference r) const
            {
                return &r;
            }

            pointer address(reference r) const
            {
                return &r;
            }

            template <class U>
            struct rebind
            {
                    typedef soar_memory_pool_allocator_n<U> other;
            };


        private:
            //            agent* thisAgent;
            Memory_Manager* memory_manager;
            memory_pool* mem_pool;

    };
    template<class T> bool operator==(const soar_memory_pool_allocator_n<T>&, const soar_memory_pool_allocator_n<T>&) { return true; }
    template<class T> bool operator!=(const soar_memory_pool_allocator_n<T>&, const soar_memory_pool_allocator_n<T>&) { return false; }
    template<class T> bool operator==(const soar_memory_pool_allocator<T>&, const soar_memory_pool_allocator<T>&) { return true; }
    template<class T> bool operator!=(const soar_memory_pool_allocator<T>&, const soar_memory_pool_allocator<T>&) { return false; }
#endif
}
#endif
