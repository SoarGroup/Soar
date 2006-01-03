#include "UrbanCombatPercepts.h"

UrbanCombatPercepts* UrbanCombatPercepts::pInstance = 0;

UrbanCombatPercepts* UrbanCombatPercepts::Instance()
{
    if (pInstance == 0)
        pInstance = new UrbanCombatPercepts;
    return pInstance;
}

UrbanCombatPercepts::UrbanCombatPercepts()
{
}

void UrbanCombatPercepts::GetLocation(UrbanCombatInterface& UCI, float& x, float& y, float& z)
{
    perceptShm* pState = UCI.GetPerceptState();
    x = pState->floats[PERCEPT_X];
    y = pState->floats[PERCEPT_Y];
    z = pState->floats[PERCEPT_Z];
}

void UrbanCombatPercepts::GetAngles(UrbanCombatInterface& UCI, float& pitch, float& yaw, float& roll)
{
    perceptShm* pState = UCI.GetPerceptState();
    pitch = pState->floats[PERCEPT_PITCH];
    yaw = pState->floats[PERCEPT_YAW];
    roll = pState->floats[PERCEPT_ROLL];
}

void UrbanCombatPercepts::GetVelocity(UrbanCombatInterface& UCI, float& dx, float& dy, float& dz)
{
    perceptShm* pState = UCI.GetPerceptState();
    dx = pState->floats[PERCEPT_VELOCITY_X];
    dy = pState->floats[PERCEPT_VELOCITY_Y];
    dz = pState->floats[PERCEPT_VELOCITY_Z];
}
