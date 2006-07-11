/////////////////////////////////////////////////////////////////
// matches command file.
//
// Author: Andrew Nuxoll
// Date  : 2005
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include <assert.h>

#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

#ifdef SOAR_WMEM_ACTIVATION

/*KLUDGE*KLUDGE*KLUDGE*KLUDGE*KLUDGE*KLUDGE*KLUDGE*KLUDGE*KLUDGE*

  These declarations are needed to print the decay settings.  I've
  copied them wholesale from activate.h in the kernel.  This means
  that the two bits of code must be kept in synch.

 *KLUDGE*KLUDGE*KLUDGE*KLUDGE*KLUDGE*KLUDGE*KLUDGE*KLUDGE*KLUDGE*/

#define MAX_DECAY 200
#define DECAY_ARRAY_SIZE (MAX_DECAY + 1)
#define DECAY_HISTORY_SIZE 10
#define DECAY_POWER_ARRAY_SIZE 270
#define DECAY_ACTIVATION_CUTOFF -1.6
#define DECAY_EXPONENT_DIVISOR 1000.0
#define DECAY_DEFAULT_EXPONENT -800
#define DECAY_ACTIVATION_LOG_SIZE 10

#define DECAY_WME_CRITERIA_O_SUPPORT_ONLY       0
#define DECAY_WME_CRITERIA_O_ARCH               1
#define DECAY_WME_CRITERIA_ALL                  2
#define DECAY_DEFAULT_WME_CRITERIA              DECAY_WME_CRITERIA_O_SUPPORT_ONLY

#define DECAY_DEFAULT_ALLOW_FORGETTING          1

#define DECAY_I_SUPPORT_MODE_NONE               0
#define DECAY_I_SUPPORT_MODE_NO_CREATE          1
#define DECAY_I_SUPPORT_MODE_UNIFORM            2
#define DECAY_DEFAULT_I_SUPPORT_MODE            DECAY_I_SUPPORT_MODE_NONE

#define DECAY_DEFAULT_PERSISTENT_ACTIVATION         0

#define DECAY_PRECISION_HIGH                    0
#define DECAY_PRECISION_LOW                     1
#define DECAY_DEFAULT_PRECISION                 DECAY_PRECISION_HIGH

#define DECAY_DEFAULT_LOGGING                   0


//*** END OF KLUDGE * END OF KLUDGE * END OF KLUDGE * END OF KLUDGE ***/

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseDecay(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
    Options optionsData[] = {
        {'n', "on",             0},
        {'o', "off",            0},
        {'e', "exponent",       1},
        {'a', "activation",     1},
        {'i', "i-support-mode", 1},
        {'f', "forgetting",     1},
        {'w', "wme-criteria",   1},
        {'p', "precision",      1},
        {'l', "log",            1},
        {0, 0, 0}
    };

    double exp;
    long arg = 0;
    FILE *fileHandle = NULL;
    int optCount = 0;
    std::string msg;

    for (;;) {
        if (!ProcessOptions(argv, optionsData)) return false;
        if (m_Option == -1) break;

        optCount++;
        switch (m_Option) {
            case 'n':
                DoDecay(pAgent, DECAY_ON, 0);
                break;
            case 'o':
                DoDecay(pAgent, DECAY_OFF, 0);
                break;
            case 'e':
                if (!IsFloat(m_OptionArgument))
                {
                    SetError(CLIError::kFloatExpected);
                    return false;
                }
                exp = atof(m_OptionArgument.c_str());
                if (exp <= 0)
                {
                    m_Result << "Exponent not set: should be a positive number";
                }
                else
                {
                    DoDecay(pAgent,
                            DECAY_EXPONENT,
                            (long) (-DECAY_EXPONENT_DIVISOR * exp));
                }
                break;
            case 'a':
                if (m_OptionArgument == "once"   || m_OptionArgument == "0")
                {
                    m_Result << "Instantiations will only boost WMEs on the cycle they are created.\n";
                    arg = 0;
                }
                else if (m_OptionArgument == "persistent" || m_OptionArgument == "1")
                {
                    m_Result << "Instantiations will activate wmes each cycle until retraction.";
                    arg = 1;
                }
                else
                {
                    SetErrorDetail("Invalid activation mode specified: " + m_OptionArgument + "Current options are: 'once' or 'persistent'");
                    SetError(CLIError::kInvalidMode);
                    break;
                }
                DoDecay(pAgent, DECAY_ACTIVATION, arg);
                break;
            case 'i':
                if (m_OptionArgument == "none" || m_OptionArgument == "0")
                {
                    m_Result << "I-supported WMEs will not affect activation levels.\n";
                    arg = 0;
                }
                else if (m_OptionArgument == "no-create" || m_OptionArgument == "1")
                {
                    m_Result << "I-supported WMEs will boost supporting WMEs except when first created.\n";
                    arg = 1;
                }
                else if (m_OptionArgument == "uniform" || m_OptionArgument == "2")
                {
                    m_Result << "I-supported WMEs will always boost supporting WMEs when referenced.\n";
                    arg = 2;
                }
                else
                {
                    SetErrorDetail("Invalid i-support-mode specified: " + m_OptionArgument + "Current options are: 'none', 'no-create' or 'uniform'");
                    SetError(CLIError::kInvalidMode);
                    break;
                }
                DoDecay(pAgent, DECAY_I_SUPPORT_MODE, arg);
                break;
            case 'f':
                if (m_OptionArgument == "off"   || m_OptionArgument == "0")
                {
                    m_Result << "Forgotten WMEs will NOT be removed from working memory.\n";
                    arg = 0;
                }
                else if (m_OptionArgument == "on"     || m_OptionArgument == "1")
                {
                    m_Result << "Forgotten WMEs will be removed from working memory.\n";
                    arg = 1;
                }
                else
                {
                    SetErrorDetail("Invalid forgetting specified: " + m_OptionArgument + "Current options are: 'on' or 'off'");
                    SetError(CLIError::kInvalidMode);
                    break;
                }
                DoDecay(pAgent, DECAY_FORGETTING, arg);
                break;
            case 'w':
                if (m_OptionArgument == "o-support" || m_OptionArgument == "0")
                {
                    m_Result << "Only o-supported WMEs will be activated.\n";
                    arg = 0;
                }
                else if (m_OptionArgument == "architectural" || m_OptionArgument == "1")
                {
                    m_Result << "O-supported and architectural WMEs will be activated.\n";
                    arg = 1;
                }
                else if (m_OptionArgument == "all" || m_OptionArgument == "2")
                {
                    m_Result << "All WMEs in working memory will be activated.\n";
                    arg = 2;
                }
                else
                {
                    SetErrorDetail("Invalid wme-criteria specified: " + m_OptionArgument + "Current options are: 'o-support', 'architectural' or 'all'");
                    SetError(CLIError::kInvalidMode);
                    break;
                }
                DoDecay(pAgent, DECAY_WME_CRITERIA, arg);
                break;
            case 'p':
                if (m_OptionArgument == "high" || m_OptionArgument == "0")
                {
                    m_Result << "WME activation levels will be calculated with high precision (slower).\n";
                    arg = 0;
                }
                else if (m_OptionArgument == "low" || m_OptionArgument == "1")
                {
                    m_Result << "WME activation levels will be calculated with low precision (faster).\n";
                    arg = 1;
                }
                else
                {
                    SetErrorDetail("Invalid activation precision specified: " + m_OptionArgument + "Current options are: 'high' or 'low'");
                    SetError(CLIError::kInvalidMode);
                    break;
                }
                DoDecay(pAgent, DECAY_PRECISION, arg);
                break;
            case 'l':
                if (m_OptionArgument == "off" || m_OptionArgument == "0")
                {
                    m_Result << "Log file closed.";
                    arg = 0;
                    fileHandle = 0;
                }
                else
                {
                    fileHandle = fopen(m_OptionArgument.c_str(), "aw");
                    if (fileHandle == NULL)
                    {
                        msg = "Error opening file: ";
                        msg += m_OptionArgument;
                        m_Result << msg.c_str();
                        return false;
                    }
                     msg = "Logging activation levels to file: ";
                    msg += m_OptionArgument;
                    m_Result << msg.c_str();
                }
                
                DoDecay(pAgent, DECAY_LOG, (long)fileHandle);
                break;
            default:
                return SetError(CLIError::kGetOptError);
        }
    }

	if (optCount == 0)
    {
        DoDecay(pAgent, DECAY_REPORT_SETTINGS, 0);
    }
    
    return true;
}

/*
 *----------------------------------------------------------------------
 *
 * PrintCurrentDecaySettings
 *
 *	This procedure prints the current decay status.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Prints the current decay removal list
 *
 *----------------------------------------------------------------------
 */
void CommandLineInterface::PrintCurrentDecaySettings(gSKI::IAgent* pAgent)
{
//    int first_spot, last_spot, i;
    char buf[1024];
    long sp_val = 0;
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

    m_Result << "Current decay settings:\n";
    sp_val = pKernelHack->GetSysparam(pAgent, WME_DECAY_SYSPARAM);
    if (sp_val)
    {
        m_Result << "   --on\n";
    }
    else
    {
        m_Result << "   --off\n";
    }

    sp_val = pKernelHack->GetSysparam(pAgent, WME_DECAY_EXPONENT_SYSPARAM);
    sprintf(buf, "   Decay exponent set to %f \n", (double) (-1.0 * (double)sp_val / DECAY_EXPONENT_DIVISOR));
    m_Result << buf;

    sp_val = pKernelHack->GetSysparam(pAgent, WME_DECAY_WME_CRITERIA_SYSPARAM);
    switch(sp_val)
    {
        case DECAY_WME_CRITERIA_O_SUPPORT_ONLY:
            m_Result << "   Only o-supported WMEs are being activated.\n";
            break;
        case DECAY_WME_CRITERIA_O_ARCH:
            m_Result << "   O-supported and architectural WMEs are being activated.\n";
            break;
        case DECAY_WME_CRITERIA_ALL:
            m_Result << "   All WMEs in working memory are being activated.\n";
            break;
    }
    
    sp_val = pKernelHack->GetSysparam(pAgent, WME_DECAY_ALLOW_FORGETTING_SYSPARAM);
    if(sp_val)
    {
        m_Result << "   Forgotten WMEs will be removed from working memory.\n";
    }
    else
    {
        m_Result << "   Forgotten WMEs will NOT be removed from working memory.\n";
    }

    sp_val = pKernelHack->GetSysparam(pAgent, WME_DECAY_I_SUPPORT_MODE_SYSPARAM);
    switch(sp_val)
    {
        case DECAY_I_SUPPORT_MODE_NONE:
            m_Result << "   I-supported WMEs will not affect activation levels.\n";
            break;
        case DECAY_I_SUPPORT_MODE_NO_CREATE:
            m_Result << "   I-supported WMEs will boost supporting WMEs except when first created.\n";
            break;
        case DECAY_I_SUPPORT_MODE_UNIFORM:
            m_Result << "   I-supported WMEs will always boost supporting WMEs when referenced.\n";
            break;
    }
    
    sp_val = pKernelHack->GetSysparam(pAgent, WME_DECAY_PERSISTENT_ACTIVATION_SYSPARAM);
    if(sp_val)
    {
        m_Result << "   Instantiations will boost WMEs each cycle until retraction.\n";
    }
    else
    {
        m_Result << "   Instantiations will only boost WMEs on the cycle they are created.\n";
    }
    
    sp_val = pKernelHack->GetSysparam(pAgent, WME_DECAY_PRECISION_SYSPARAM);
    switch(sp_val)
    {
        case DECAY_PRECISION_HIGH:
            m_Result << "   WME activation levels will be calculated with high precision (slower).\n";
            break;
        case DECAY_PRECISION_LOW:
            m_Result << "   WME activation levels will be calculated with low precision (faster).\n";
            break;
    }

    sp_val = pKernelHack->GetSysparam(pAgent, WME_DECAY_LOGGING_SYSPARAM);
    if(sp_val)
    {
        m_Result << "   WME activation levels are being logged.\n";
    }
    else
    {
        m_Result << "   WME activation levels are not being logged.\n";
    }
    
    
    
//  #ifndef NO_TIMING_STUFF
//      m_Result << "\nTiming Information:\n";

//      sprintf(buf, "   Time spent on decay: %11.3f secs. \n", timer_value(&(pAgent->total_decay_time)));
//      m_Result << buf;
//      sprintf(buf, "          for new wmes: %11.3f secs. \n", timer_value(&(pAgent->total_decay_new_wme_time)));
//      m_Result << buf;
//      sprintf(buf, "     for move / remove: %11.3f secs. \n", timer_value(&(pAgent->total_decay_move_remove_time)));
//      m_Result << buf;
//      sprintf(buf, "               for rhs: %11.3f secs. \n", timer_value(&(pAgent->total_decay_rhs_time)));
//      m_Result << buf;
//      sprintf(buf, "               for lhs: %11.3f secs. \n", timer_value(&(pAgent->total_decay_lhs_time)));
//      m_Result << buf;
//      sprintf(buf, "        for deallocate: %11.3f secs. \n", timer_value(&(pAgent->total_decay_deallocate_time)));
//      m_Result << buf;
//      sprintf(buf, "          deallocate 2: %11.3f secs. \n", timer_value(&(pAgent->total_decay_deallocate_time_2)));
//      m_Result << buf;

//  #endif


#ifdef MEMORY_POOL_STATS
    sprintf(buf, "\nUsing %i slots in the memory pool \n", (pAgent->decay_element_pool).used_count);
    m_Result << buf;
#endif
       
    sp_val = pKernelHack->GetSysparam(pAgent, WME_DECAY_SYSPARAM);
    if (sp_val)
    {
//          first_spot = (pAgent->current_decay_timelist_element)->position;
//          last_spot = (first_spot - 1 + DECAY_ARRAY_SIZE) % DECAY_ARRAY_SIZE;

//          sprintf(buf, "\nCurrently activated WMEs (cycle %i): \n", (pAgent->current_decay_timelist_element)->time);
//          m_Result << buf;
//          m_Result << "   forget on   decay  kernel                         \n";
//          m_Result << "     cycle      refs   refs            WME           \n";
//          m_Result << "   ---------   -----  ------  -----------------------\n";
        
//          i = first_spot;
//          while(i != last_spot)
//          {
//              print_this = (pAgent->decay_timelist[i]).first_decay_element;

//              while(print_this != NIL)
//              {
//                  int j;
//                  int num_refs = print_this->num_references;
                
//                  //If the reference count has already been set to zero, we'll
//                  //need to calculate how many times this WME was referenced this
//                  //cycle using the boost_history.
//                  if (num_refs == 0)
//                  {
//                      for(j = 0; j < DECAY_HISTORY_SIZE; j++)
//                      {
//                          if (print_this->boost_history[j] == (unsigned long)(pAgent->current_decay_timelist_element)->time)
//                          {
//                              num_refs++;
//                          }
//                      }
//                  }
                
//                  sprintf(buf, "   %8i   %5i  %6i ", (pAgent->decay_timelist[i]).time, num_refs, print_this->this_wme->reference_count);
//                  m_Result << buf;
//                  print_with_symbols(pAgent, "  (%y ^%y %y) ", print_this->this_wme->id, print_this->this_wme->attr, print_this->this_wme->value);
//                  if (print_this->this_wme->preference == NIL)
//                  {
//                      m_Result << " architectural\n";
//                  }
//                  else if (print_this->this_wme->preference->o_supported)
//                  {
//                      m_Result << " o-supported\n";
//                  }
//                  else
//                  {
//                      m_Result << " i-supported\n";
//                  }
                    
//                  print_this = print_this->next;
//              }//while
            
//              i = (i + 1) % DECAY_ARRAY_SIZE;
//          }//while
        m_Result << "\n\n";
    }//if
  
}//print_current_decay_settings


bool CommandLineInterface::DoDecay(gSKI::IAgent* pAgent,
                                   enum eDecayOptions setting,
                                   long arg)
{

    if (!RequireAgent(pAgent)) return false;

    long sp_val = 0;
    char buf[1024];
    
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

    switch( setting )
    {
        case DECAY_REPORT_SETTINGS:
            PrintCurrentDecaySettings(pAgent);
            break;
        
        case DECAY_ON:
            // okay, this is questionable, if we reinit decay, then all the data
            // structures will be set up correctly, but I think that there may be
            // a memory leak where the old memory pool was... think about
            // it... only call init if decay off really no need to set the
            // sysparam either... whatever -MRJ
            sp_val = pKernelHack->GetSysparam(pAgent, WME_DECAY_SYSPARAM);
            if (!sp_val)
            {
                pKernelHack->DecayInit(pAgent);
            }

            m_Result << "Decay is On";
            pKernelHack->SetSysparam(pAgent, WME_DECAY_SYSPARAM, 1);
            break;
          
        case DECAY_OFF:
            // if decay was on, then need to set the decay_elements for all of the
            // wmes to NIL, otherwise there will be dangling pointers if decay is
            // turned back on.
            sp_val = pKernelHack->GetSysparam(pAgent, WME_DECAY_SYSPARAM);
            if (sp_val)
            {
                pKernelHack->DecayDeInit(pAgent);
            }
            m_Result << "Decay is Off";
            pKernelHack->SetSysparam(pAgent, WME_DECAY_SYSPARAM, 0);
            break;
        case DECAY_EXPONENT:
            sp_val = pKernelHack->GetSysparam(pAgent, WME_DECAY_SYSPARAM);
            if (sp_val)
            {
                m_Result << "Cannot set decay exponent while decay mechanism is turned on."; 
            }
            else
            {
                // okay, store the decay exponent as a sysparam, which is really
                // 1000 * the true decay exponent (sysparams are long, not double)
                // this param can only be set while decay off, so init_decay
                // will recalc everything right when turned back on...
                pKernelHack->SetSysparam(pAgent, WME_DECAY_EXPONENT_SYSPARAM, arg);

                sp_val = pKernelHack->GetSysparam(pAgent, WME_DECAY_EXPONENT_SYSPARAM);
                sprintf(buf, "   Decay exponent set to %f \n", (double) (-1.0 * (double)sp_val / DECAY_EXPONENT_DIVISOR));
                m_Result << buf;

            }
            break;

        case DECAY_WME_CRITERIA:
            pKernelHack->SetSysparam(pAgent, WME_DECAY_WME_CRITERIA_SYSPARAM, arg);
            break;
          
        case DECAY_FORGETTING:
            pKernelHack->SetSysparam(pAgent, WME_DECAY_ALLOW_FORGETTING_SYSPARAM, arg);
            break;
          
        case DECAY_I_SUPPORT_MODE:
            pKernelHack->SetSysparam(pAgent, WME_DECAY_I_SUPPORT_MODE_SYSPARAM, arg);
            break;
        case DECAY_ACTIVATION:
            pKernelHack->SetSysparam(pAgent, WME_DECAY_PERSISTENT_ACTIVATION_SYSPARAM, arg);
            break;
        case DECAY_PRECISION:
            pKernelHack->SetSysparam(pAgent, WME_DECAY_PRECISION_SYSPARAM, arg);
            break;
        case DECAY_LOG:
            sp_val = pKernelHack->GetSysparam(pAgent, WME_DECAY_LOGGING_SYSPARAM);
            if (sp_val)
            {
                fclose((FILE *)sp_val);
            }
            pKernelHack->SetSysparam(pAgent, WME_DECAY_LOGGING_SYSPARAM, arg);
            break;
            
    }//switch

    return true;
}
 #endif // SOAR_WMEM_ACTIVATION
