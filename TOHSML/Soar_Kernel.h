/* Soar_Kernel.h
 *
 * Author : Mitchell Keith Bloch, Soar Group at U-M
 * Date   : June/July 2008
 *
 * Simple RAII-pattern 'sml::Kernel *' wrapper
 */

#ifndef SOAR_KERNEL_H
#define SOAR_KERNEL_H

// Generally only need this one header file
#include "sml_Client.h"

/** This class can be used transparently as a sml::Kernel object,
  * with the added benefit that it will self destruct when it 
  * goes out of scope.
  */
class Soar_Kernel {
  /// Disabled (No Implementation)
  Soar_Kernel(const Soar_Kernel &);
  Soar_Kernel & operator=(const Soar_Kernel &);

public:
  /// Create a kernel; sml::Kernel::CreateRemoteConnection() can be useful for debugging
  inline Soar_Kernel(sml::Kernel * const &new_kernel = sml::Kernel::CreateKernelInCurrentThread(true));
  inline ~Soar_Kernel();

  const sml::Kernel & operator*() const          {return *m_kernel_ptr;}
  sml::Kernel & operator*()                      {return *m_kernel_ptr;}
  const sml::Kernel * const & operator->() const {
	  const sml::Kernel* const* k = &m_kernel_ptr;
	  return *k;
  }
  sml::Kernel * operator->()                     {return  m_kernel_ptr;}

  operator const sml::Kernel & () const          {return *m_kernel_ptr;}
  operator sml::Kernel & ()                      {return *m_kernel_ptr;}
  operator const sml::Kernel * const & () const  {
	  const sml::Kernel* const* k = &m_kernel_ptr;
	  return *k;
  }
  operator sml::Kernel * const & ()              {return  m_kernel_ptr;}

private:
  sml::Kernel * const m_kernel_ptr;
};

/// For inlines
#include <iostream>

Soar_Kernel::Soar_Kernel(sml::Kernel * const &new_kernel)
  : m_kernel_ptr(new_kernel)
{
  if(!m_kernel_ptr)
    abort();

  // Check that nothing went wrong. We will always get back a kernel object
  // even if something went wrong and we have to abort.
  if(m_kernel_ptr->HadError()) {
    std::cerr << m_kernel_ptr->GetLastErrorDescription() << std::endl;
    abort();
  }

  m_kernel_ptr->SetAutoCommit(false);
}

Soar_Kernel::~Soar_Kernel() {
  // Shutdown and clean up
  m_kernel_ptr->Shutdown(); // Deletes all agents (unless using a remote connection)
  delete m_kernel_ptr; // Deletes the kernel itself
}

#endif
