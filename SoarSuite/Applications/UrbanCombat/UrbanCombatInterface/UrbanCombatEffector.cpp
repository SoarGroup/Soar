#include "UrbanCombatEffector.h"

UrbanCombatEffector* UrbanCombatEffector::pInstance = 0;

UrbanCombatEffector* UrbanCombatEffector::Instance()
{
    if (pInstance == 0)
        pInstance = new UrbanCombatEffector;
    return pInstance;
}

UrbanCombatEffector::UrbanCombatEffector()
{
    counter = 0;
}

void UrbanCombatEffector::WalkForward(UrbanCombatInterface& UCI)
{
    effectorShm* pMem = UCI.GetEffectorState();
    
    pMem->floats[EFFECTOR_PRE_COUNTER] = (float)counter;
    pMem->floats[EFFECTOR_AMOUNT_1] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_2] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_3] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_4] = 0.0;		
        
    pMem->ints[EFFECTOR_COMMAND_1] = DCA_WALK_FORWARD;
    pMem->ints[EFFECTOR_COMMAND_2] = DCA_DO_NOTHING;
    pMem->ints[EFFECTOR_COMMAND_3] = DCA_DO_NOTHING; 
    pMem->ints[EFFECTOR_COMMAND_4] = DCA_DO_NOTHING;
    pMem->ints[EFFECTOR_POST_COUNTER] = counter;
    
    ++counter;
}

void UrbanCombatEffector::WalkBackwards(UrbanCombatInterface& UCI)
{
    effectorShm* pMem = UCI.GetEffectorState();
    
    pMem->floats[EFFECTOR_PRE_COUNTER] = (float)counter;
    pMem->floats[EFFECTOR_AMOUNT_1] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_2] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_3] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_4] = 0.0;
    
    pMem->ints[EFFECTOR_COMMAND_1] = DCA_WALK_BACKWARD;
    pMem->ints[EFFECTOR_COMMAND_2] = DCA_DO_NOTHING;
    pMem->ints[EFFECTOR_COMMAND_3] = DCA_DO_NOTHING; 
    pMem->ints[EFFECTOR_COMMAND_4] = DCA_DO_NOTHING;	
    pMem->ints[EFFECTOR_POST_COUNTER] = counter;
    
    ++counter;
}

void UrbanCombatEffector::StrafeLeft(UrbanCombatInterface& UCI)
{
    effectorShm* pMem = UCI.GetEffectorState();

    pMem->floats[EFFECTOR_PRE_COUNTER] = (float)counter;
    pMem->floats[EFFECTOR_AMOUNT_1] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_2] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_3] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_4] = 0.0;
        
    pMem->ints[EFFECTOR_COMMAND_1] = DCA_STRAFE_LEFT;
    pMem->ints[EFFECTOR_COMMAND_2] = DCA_DO_NOTHING;
    pMem->ints[EFFECTOR_COMMAND_3] = DCA_DO_NOTHING; 
    pMem->ints[EFFECTOR_COMMAND_4] = DCA_DO_NOTHING;	
    pMem->ints[EFFECTOR_POST_COUNTER] = counter;    
    
    ++counter;
}

void UrbanCombatEffector::StrafeRight(UrbanCombatInterface& UCI)
{
    effectorShm* pMem = UCI.GetEffectorState();

    pMem->floats[EFFECTOR_PRE_COUNTER] = (float)counter;
    pMem->floats[EFFECTOR_AMOUNT_1] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_2] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_3] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_4] = 0.0;
        
    pMem->ints[EFFECTOR_COMMAND_1] = DCA_STRAFE_RIGHT;
    pMem->ints[EFFECTOR_COMMAND_2] = DCA_DO_NOTHING;
    pMem->ints[EFFECTOR_COMMAND_3] = DCA_DO_NOTHING; 
    pMem->ints[EFFECTOR_COMMAND_4] = DCA_DO_NOTHING;	
    pMem->ints[EFFECTOR_POST_COUNTER] = counter;    
    
    ++counter;
}

void UrbanCombatEffector::TurnLeft(UrbanCombatInterface& UCI)
{
    effectorShm* pMem = UCI.GetEffectorState();

    pMem->floats[EFFECTOR_PRE_COUNTER] = (float)counter;
    pMem->floats[EFFECTOR_AMOUNT_1] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_2] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_3] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_4] = 0.0;
    
    pMem->ints[EFFECTOR_COMMAND_1] = DCA_TURN_LEFT;
    pMem->ints[EFFECTOR_COMMAND_2] = DCA_DO_NOTHING;
    pMem->ints[EFFECTOR_COMMAND_3] = DCA_DO_NOTHING; 
    pMem->ints[EFFECTOR_COMMAND_4] = DCA_DO_NOTHING;	
    pMem->ints[EFFECTOR_POST_COUNTER] = counter;
    
    ++counter;
}

void UrbanCombatEffector::TurnRight(UrbanCombatInterface& UCI)
{
    effectorShm* pMem = UCI.GetEffectorState();

    pMem->floats[EFFECTOR_PRE_COUNTER] = (float)counter;
    pMem->floats[EFFECTOR_AMOUNT_1] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_2] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_3] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_4] = 0.0;
        
    pMem->ints[EFFECTOR_COMMAND_1] = DCA_TURN_RIGHT;
    pMem->ints[EFFECTOR_COMMAND_2] = DCA_DO_NOTHING;
    pMem->ints[EFFECTOR_COMMAND_3] = DCA_DO_NOTHING; 
    pMem->ints[EFFECTOR_COMMAND_4] = DCA_DO_NOTHING;	
    pMem->ints[EFFECTOR_POST_COUNTER] = counter;    
    
    ++counter;
}

void UrbanCombatEffector::DoNothing(UrbanCombatInterface& UCI)
{
    effectorShm* pMem = UCI.GetEffectorState();
    
    pMem->floats[EFFECTOR_PRE_COUNTER] = (float)counter;
    pMem->floats[EFFECTOR_AMOUNT_1] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_2] = 0.0;
    pMem->floats[EFFECTOR_AMOUNT_3] = 0.0;	
    pMem->floats[EFFECTOR_AMOUNT_4] = 0.0;
        
    pMem->ints[EFFECTOR_COMMAND_1] = DCA_DO_NOTHING;
    pMem->ints[EFFECTOR_COMMAND_2] = DCA_DO_NOTHING;
    pMem->ints[EFFECTOR_COMMAND_3] = DCA_DO_NOTHING; 
    pMem->ints[EFFECTOR_COMMAND_4] = DCA_DO_NOTHING;	
    pMem->ints[EFFECTOR_POST_COUNTER] = counter;    
    
    ++counter;
}
