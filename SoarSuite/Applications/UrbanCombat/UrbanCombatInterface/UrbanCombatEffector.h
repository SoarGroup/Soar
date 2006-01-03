
#ifndef _URBAN_COMBAT_EFFECTOR_
#define _URBAN_COMBAT_EFFECTOR_

#include "UrbanCombatInterface.h"

class UrbanCombatEffector
{
public:
    static UrbanCombatEffector* Instance();
    
    void WalkForward(UrbanCombatInterface& UCI);
    void WalkBackwards(UrbanCombatInterface& UCI);
    void StrafeLeft(UrbanCombatInterface& UCI);
    void StrafeRight(UrbanCombatInterface& UCI);
    void TurnLeft(UrbanCombatInterface& UCI);
    void TurnRight(UrbanCombatInterface& UCI);

    void DoNothing(UrbanCombatInterface& UCI);

private:
    UrbanCombatEffector();
    static UrbanCombatEffector* pInstance;
    int counter;
};

#endif



