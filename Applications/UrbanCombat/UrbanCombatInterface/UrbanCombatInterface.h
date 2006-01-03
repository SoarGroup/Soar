
#ifndef _URBAN_COMBAT_INTERFACE_
#define _URBAN_COMBAT_INTERFACE_

#include "../dca_common.h"

class UrbanCombatInterface
{
public:
    UrbanCombatInterface();     
    ~UrbanCombatInterface();

    void UpdatePercepts();              // copy the hot percept memory to buffer, necessary to
                                        // keep various percepts sync'd

    effectorShm* GetEffectorState();    // allow low-level access or higher-level access
    perceptShm* GetPerceptState();      // through UrbanCombatEffector and UrbanCombatPercepts
                                        // classes
private:
    bool InitPerceptMemory();
    bool DetachPerceptMemory();
    
    bool InitEffectorMemory();
    bool DetachEffectorMemory();
    
    bool LaunchUrbanCombat();

    bool ClearEffectorMem();
    
    perceptShm* pPerceptMem;            // buffer to read from
    perceptShm* pLivePerceptMem;        // shared memory segment
    
    effectorShm* pLiveEffectorMem;      // shared memory segment, we write to it hot

    const int EffectorKey;     // hard coded from provided
    const int PerceptKey;      // sample code
};

#endif
