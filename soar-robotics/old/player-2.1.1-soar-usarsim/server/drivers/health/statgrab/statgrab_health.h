
#include <stdint.h>
#include <libplayercore/playercore.h>

extern "C"
{
#include <statgrab.h>
}
	
////////////////////////////////////////////////////////////////////////////////
// The class for the driver
class StatGrabDriver : public Driver
{
  public:

    StatGrabDriver(ConfigFile* cf, int section);
    
    // Must implement the following methods.
    int Setup();
    int Shutdown();
 
  private:
    // Main function for device thread.
    virtual void Main();
    void RefreshData();
    
    // Structure for specific process data 
    
    // Structure holding Swap data 
    sg_swap_stats         *swap_stats;
    
    // Structure holding CPU data  
    sg_cpu_percents       *cpu_percent;
     
    // Structure holding memory stat  
    sg_mem_stats          *mem_data;
    double mem_percent;
   
    // Health Interface
    player_devaddr_t     mHealthId;
    player_health_data_t   mHealth;
   
    // For status checking priviledge
    int                   status;
    int32_t               mSleep;

};
