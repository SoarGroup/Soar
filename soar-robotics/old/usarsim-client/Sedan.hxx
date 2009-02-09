#ifndef SOAR_PLAYER_CLIENT_HXX
#define SOAR_PLAYER_CLIENT_HXX

#include "Sedan.h"

void Sedan::run()
{
	int leg = 0;
	
	while ( !stop )
	{
		m_robot.Read();
		
		assert ( m_read.GetXPos() > 0 );
		
		//std::cout << "leg " << leg << "(" << m_read.GetXPos() << "," << m_read.GetYPos() << "," << m_read.GetYaw() << ")" << std::endl;
		
		switch ( leg )
		{
			case 0: // step on it
				m_write.SetSpeed( 10, 0 );
				leg = 1;
				break;
			
			case 1: // swerve left
				if ( m_read.GetYPos() > -52 )
				{
					m_write.SetSpeed( 10, -0.3 );
					leg = 2;
				}
				break;
			
			case 2: // straight again
				if ( m_read.GetYaw() > -3.925 )
				{
					m_write.SetSpeed( 10, 0 );
					leg = 3;
				}
				break;
			
			case 3: // swerve left
				if ( m_read.GetYPos() > -39 )
				{
					m_write.SetSpeed( 10, -0.15 );
					leg = 4;
				}
				break;
			
			case 4: // straight again
				if ( m_read.GetYaw() > -3.14 )
				{
					m_write.SetSpeed( 10, 0 );
					leg = 5;
				}
				break;
			
			case 5: // stop
				if ( m_read.GetXPos() < 15 )
				{
					m_write.SetSpeed( 0, 0 );
					leg = 6;
				}
				break;
			default:
				break;
		}
		
	}
}

#endif // SOAR_PLAYER_CLIENT_HXX
