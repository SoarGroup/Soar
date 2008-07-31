#include "nimu.h"
#include <stdlib.h>
#include <stdio.h>

nimu::nimu()
{
	nimu_dev = NULL;
	usb_init();

}
nimu::~nimu()
{
	if (nimu_dev)
		Close();
}

int nimu::Open()
{
	int ret;
	ret = usb_find_busses();	
	if (ret < 0)
		return ret;
	int NumDevices = usb_find_devices();
	if (NumDevices < 0)
		return NumDevices;
	
	struct usb_bus *busses = usb_get_busses();

	struct usb_bus *bus;
//	int c, i, a;

	/* ... */

	for (bus = busses; bus; bus = bus->next) {
		struct usb_device *dev;

		for (dev = bus->devices; dev; dev = dev->next) {
			if (dev->descriptor.idVendor == NIMU_VENDORID && dev->descriptor.idProduct == NIMU_PRODUCTID)
			{
				nimu_dev = usb_open(dev);
				if (nimu_dev)
				{
					ret = usb_set_configuration(nimu_dev,1);
					if (ret < 0)
					{
						printf("Error setting configuration: %d (%s)\n",ret,usb_strerror());
						return ret;
					}
					ret = usb_claim_interface(nimu_dev,0);
					if (ret < 0)
					{
						printf("Error claiming interface: %d (%s)\n",ret,usb_strerror());
						return ret;
					}

					// configure the device for data output
					ret = usb_control_msg(nimu_dev,0,0x02,0x02,0," ",0x0,1000);
					if (ret < 0)
					{
						printf("Error sending control message2: %d (%s)\n",ret,usb_strerror());
						return ret;
					}					
				}
				else
				{
					printf("Error opening device\n");
					return -1;
				}
				return 0;
			}
		}
	}
	printf("Error, couldnt find device\n");
	return -1;
}

int nimu::Close()
{
	int ret = 0;
	if (nimu_dev)
	{
		usb_release_interface(nimu_dev,0);
		usb_close(nimu_dev);
		nimu_dev = NULL;
	}
	return ret;
}

nimu_data nimu::GetData()
{
	int ret;
	nimu_data ret_data;
	unsigned char data[NIMU_DATA_SIZE];

	ret = usb_bulk_read(nimu_dev, 0x82, (char*)data, NIMU_DATA_SIZE, 1000);
	if (ret < 0)
	{
		printf("Error reading data: %d (%s)\n",ret,usb_strerror());
		return ret_data;
	}

	ret_data.DeviceID = data[5];
	ret_data.MessageID = data[6];
	ret_data.SampleTimer = ntohs(*(uint16_t*)&data[7]);
	ret_data.GyroX = (short)ntohs(*(uint16_t*)&data[13]);
	ret_data.GyroY = (short)ntohs(*(uint16_t*)&data[15]);
	ret_data.GyroZ = (short)ntohs(*(uint16_t*)&data[17]);
	ret_data.AccelX = (short)ntohs(*(uint16_t*)&data[19]);
	ret_data.AccelY = (short)ntohs(*(uint16_t*)&data[21]);
	ret_data.AccelZ = (short)ntohs(*(uint16_t*)&data[23]);
	ret_data.MagX = (short)ntohs(*(uint16_t*)&data[25]);
	ret_data.MagY = (short)ntohs(*(uint16_t*)&data[27]);
	ret_data.MagZ = (short)ntohs(*(uint16_t*)&data[29]);
	ret_data.GyroTempX = (short)ntohs(*(uint16_t*)&data[31]);
	ret_data.GyroTempY = (short)ntohs(*(uint16_t*)&data[33]);
	ret_data.GyroTempZ = (short)ntohs(*(uint16_t*)&data[35]);

	return ret_data;
}



