#include "UrbanCombatInterface.h"
#include <assert.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

UrbanCombatInterface::UrbanCombatInterface() : EffectorKey(2501), PerceptKey(1967)
{
    pPerceptMem = new perceptShm;
    pLivePerceptMem = 0;
    pLiveEffectorMem = 0;
    
    assert(InitEffectorMemory());
    assert(LaunchUrbanCombat());
    assert(InitPerceptMemory());
}

UrbanCombatInterface::~UrbanCombatInterface()
{
    assert(DetachPerceptMemory());
    assert(ClearEffectorMem());
    assert(DetachEffectorMemory());
    
    assert(pPerceptMem);
    
    delete pPerceptMem;
}

void UrbanCombatInterface::UpdatePercepts()
{
    assert(pPerceptMem && pLivePerceptMem);
    
    memcpy(pPerceptMem, pLivePerceptMem, sizeof(perceptShm));
}

effectorShm* UrbanCombatInterface::GetEffectorState()
{
    return pLiveEffectorMem;
}

perceptShm* UrbanCombatInterface::GetPerceptState()
{
    return pLivePerceptMem;
} 
                                   

bool UrbanCombatInterface::InitPerceptMemory()
{
    assert(pLivePerceptMem == 0);
    
    int memID = shmget(PerceptKey, sizeof(perceptShm), 0666 | IPC_CREAT);
    if (memID == -1)
        return false;
    
    pLivePerceptMem = static_cast<perceptShm*>(shmat(memID, 0, 0666));
    if ((int)pLivePerceptMem == -1)
        return false;
    
    return true;
}

bool UrbanCombatInterface::InitEffectorMemory()
{
    assert(pLiveEffectorMem == 0);
    
    int memID = shmget(EffectorKey, sizeof(effectorShm), 0666 | IPC_CREAT);
    if (memID == -1)
        return false;
        
    pLiveEffectorMem = static_cast<effectorShm*>(shmat(memID, 0, 0666));
    if ((int)pLiveEffectorMem == -1)
        return false;
        
    return true;
}

bool UrbanCombatInterface::DetachPerceptMemory()
{
    assert(pLivePerceptMem);
    shmdt(pLivePerceptMem);
    pLivePerceptMem = 0;
}

bool UrbanCombatInterface::DetachEffectorMemory()
{
    assert(pLiveEffectorMem);
    shmdt(pLiveEffectorMem);
    pLiveEffectorMem = 0;
}

bool UrbanCombatInterface::LaunchUrbanCombat()
{
    pid_t pid = fork();
    
    if (pid < 0)
        return false;
    else if (pid == 0)
    {
        if (pLiveEffectorMem)
            DetachEffectorMemory();
        char* pStr = getenv("URBAN_COMBAT_HOME");
        if (!pStr)
            exit(1);
        else
        {
            if (chdir(pStr) < 0)
                exit(1);
            //system("./linuxuct");
            system("./agentstart.sh");
        }
        exit(0);
    }
    else
        return true;
}

bool UrbanCombatInterface::ClearEffectorMem()
{
    if (!pLiveEffectorMem)
        return false;
    pLiveEffectorMem->floats[EFFECTOR_PRE_COUNTER] = 0.0;
    pLiveEffectorMem->floats[EFFECTOR_AMOUNT_1] = 0.0;
    pLiveEffectorMem->floats[EFFECTOR_AMOUNT_2] = 0.0;
    pLiveEffectorMem->floats[EFFECTOR_AMOUNT_3] = 0.0;	
    pLiveEffectorMem->floats[EFFECTOR_AMOUNT_4] = 0.0;
    pLiveEffectorMem->ints[EFFECTOR_COMMAND_1] = DCA_DO_NOTHING;
    pLiveEffectorMem->ints[EFFECTOR_COMMAND_2] = DCA_DO_NOTHING;
    pLiveEffectorMem->ints[EFFECTOR_COMMAND_3] = DCA_DO_NOTHING; 
    pLiveEffectorMem->ints[EFFECTOR_COMMAND_4] = DCA_DO_NOTHING;	
    pLiveEffectorMem->ints[EFFECTOR_POST_COUNTER] = 0;    
    return true;
}
