
#ifndef _URBAN_COMBAT_PERCEPTS_
#define _URBAN_COMBAT_PERCEPTS_

#include "UrbanCombatInterface.h"

class UrbanCombatPercepts
{
public:
    static UrbanCombatPercepts* Instance();

    void Update();

    void GetLocation(UrbanCombatInterface& UCI, float& x, float& y, float& z);
    void GetAngles(UrbanCombatInterface& UCI, float& pitch, float& yaw, float& roll);
    void GetVelocity(UrbanCombatInterface& UCI, float& dx, float& dy, float& dz);

private:
    UrbanCombatPercepts();
    static UrbanCombatPercepts* pInstance;
};

#endif
