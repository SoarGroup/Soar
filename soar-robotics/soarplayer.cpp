#include <iostream>
#include <libplayerc++/playerc++.h>
#include <sml_Client.h>
#include <assert.h>

using namespace PlayerCc;
using namespace sml;

struct RobotData
{
  RobotData()
  : robot( "localhost" )
  , sp( &robot,0 )
  , pp( &robot,0 ) 
  {}

  PlayerClient    robot;
  SonarProxy      sp;
  Position2dProxy pp;
};

void updateHandler( smlUpdateEventId, void* pUserData, Kernel*, smlRunFlags )
{
  RobotData* pRobotData = static_cast< RobotData* >( pUserData );
  PlayerClient&    robot = pRobotData->robot;
  SonarProxy&      sp = pRobotData->sp;
  Position2dProxy& pp = pRobotData->pp;

  double turnrate, speed;

  // read from the proxies
  robot.Read();

  // print out sonars for fun
  std::cout << sp << std::endl;

  // do simple collision avoidance
  if((sp[0] + sp[1]) < (sp[6] + sp[7]))
    turnrate = dtor(-20); // turn 20 degrees per second
  else
    turnrate = dtor(20);

  if(sp[3] < 0.500)
    speed = 0;
  else
    speed = 1.000;

  // command the motors
  pp.SetSpeed(speed, turnrate);
}

int main(int argc, char *argv[])
{
  RobotData robotData;

  Kernel* pKernel = Kernel::CreateKernelInNewThread();
  assert( pKernel );
  if ( pKernel->HadError() )
  {
    std::cerr << pKernel->GetLastErrorDescription() << std::endl;
    return 1;
  }

  Agent* pAgent = pKernel->CreateAgent( "robotics" );
  if ( !pAgent )
  {
    std::cerr << pKernel->GetLastErrorDescription() << std::endl;
    return 1;
  }

  pAgent->ExecuteCommandLine( "waitsnc --enable" );
  pKernel->RegisterForUpdateEvent( smlEVENT_AFTER_ALL_OUTPUT_PHASES, updateHandler, &robotData );

  pKernel->RunAllAgentsForever();

  return 0;
}
