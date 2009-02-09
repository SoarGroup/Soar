#include <usb.h>
#include <stdint.h>
#include <stdio.h>
#include <arpa/inet.h>


#define NIMU_VENDORID 0x10c4
#define NIMU_PRODUCTID 0xEA61

#define NIMU_DATA_SIZE 38

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;



class nimu_data
{
	public:
		uint8_t DeviceID;
		uint8_t MessageID;
		uint16_t SampleTimer;
		short GyroX;
		short GyroY;
		short GyroZ;
		short AccelX;
		short AccelY;
		short AccelZ;
		short MagX;
		short MagY;
		short MagZ;
		short GyroTempX;
		short GyroTempY;
		short GyroTempZ;

		void Print() {
			printf("%04X %04X %04X,%04X %04X %04X\n",GyroX,GyroY,GyroZ,AccelX,AccelY,AccelZ);
		}
};

class nimu
{
	public:
		nimu();
		~nimu();

		int Open();
		int Close();

		nimu_data GetData();

	private:
		usb_dev_handle * nimu_dev;

};

