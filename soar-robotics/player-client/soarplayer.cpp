#include "Console.inl"
#include "SoarPlayerClient.inl"

void updateHandler( sml::smlUpdateEventId, void* pUserData, sml::Kernel*, sml::smlRunFlags )
{
    SoarPlayerClient* pPlayerClient = static_cast< SoarPlayerClient* >( pUserData );
    pPlayerClient->update();
}

int main( int argc, char** argv )
{
    sml::Kernel* kernel = sml::Kernel::CreateKernelInNewThread();
    sml::Agent* agent = kernel->CreateAgent( "player" );
    agent->ExecuteCommandLine( "waitsnc --enable" );

    SoarPlayerClient client( kernel, agent );
    kernel->RegisterForUpdateEvent( sml::smlEVENT_AFTER_ALL_OUTPUT_PHASES, updateHandler, &client );
    
    Console console( client );

    int result = console.run();
    
    kernel->Shutdown();
    delete kernel;
    
    return result;
}

