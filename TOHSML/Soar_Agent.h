/* Soar_Agent.h
 *
 * Author : Mitchell Keith Bloch, Soar Group at U-M
 * Date   : June/July 2008
 *
 * Simple RAII-pattern 'sml::Agent *' wrapper
 */

#ifndef SOAR_AGENT_H
#define SOAR_AGENT_H

#include "sml_Client.h"
#include <string>

/** This class can be used transparently as a sml::Agent object,
  * with the added benefit that it will self destruct when it 
  * goes out of scope.
  */
class Soar_Agent {
  /// Disabled (No Implementation)
  Soar_Agent(const Soar_Agent &);
  Soar_Agent & operator=(const Soar_Agent &);

public:
  inline Soar_Agent(sml::Kernel &kernel,
                    const std::string &name);
  inline Soar_Agent(sml::Agent *const &agent_ptr);
  inline ~Soar_Agent();
  
  const sml::Agent & operator*() const          {return *m_agent_ptr;}
  sml::Agent & operator*()                      {return *m_agent_ptr;}
  const sml::Agent * const & operator->() const {
	  const sml::Agent* const* a = &m_agent_ptr;
	  return *a;
  }
  sml::Agent * operator->()                     {return  m_agent_ptr;}

  operator const sml::Agent & () const          {return *m_agent_ptr;}
  operator sml::Agent & ()                      {return *m_agent_ptr;}
  operator const sml::Agent * const & () const  {
	  const sml::Agent* const* a = &m_agent_ptr;
	  return *a;
  }
  operator sml::Agent * const & ()              {return  m_agent_ptr;}

  const sml::Kernel * const & get_kernel() const {
	  const sml::Kernel* const* k = &m_kernel_ptr;
	  return *k;
  }
  sml::Kernel * const & get_kernel()             {return m_kernel_ptr;}
  
  inline void LoadProductions(const std::string &productions);

private:
  sml::Agent * const m_agent_ptr;

  sml::Kernel * const m_kernel_ptr;
};

/// For inlines
#include "TOH_Game.h"

Soar_Agent::Soar_Agent(sml::Kernel &kernel, const std::string &name)
  // Create an arbitrarily named Soar agent
  : m_agent_ptr(kernel.CreateAgent(name.c_str())),
  m_kernel_ptr(&kernel)
{
  // Check that nothing went wrong
  // NOTE: No agent gets created if there's a problem, so we have to check for
  // errors through the kernel object.
  if(!m_agent_ptr || kernel.HadError()) {
    std::cerr << kernel.GetLastErrorDescription() << std::endl;
    abort();
  }
}

Soar_Agent::Soar_Agent(sml::Agent *const &agent_ptr)
  // Create an arbitrarily named Soar agent
  : m_agent_ptr(agent_ptr),
  m_kernel_ptr(agent_ptr ? agent_ptr->GetKernel() : 0)
{
  // Check that nothing went wrong
  // NOTE: No agent gets created if there's a problem, so we have to check for
  // errors through the kernel object.
  if(!m_agent_ptr) {
    std::cerr << "Soar Agent could not be created." << std::endl;
    abort();
  }
}

Soar_Agent::~Soar_Agent() {
  m_kernel_ptr->DestroyAgent(m_agent_ptr);
}

void Soar_Agent::LoadProductions(const std::string &productions) {
  // Load the TOH productions
  if(!m_agent_ptr->LoadProductions(productions.c_str()) ||
    m_agent_ptr->HadError()) {
    std::cerr << m_agent_ptr->GetLastErrorDescription() << std::endl;
    abort();
  }
}

#endif
