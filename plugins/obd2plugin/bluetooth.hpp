#ifndef OBDBLUETOOTH
#define OBDBLUETOOTH

#include <string>
#include "bluetoothmanagerproxy.h"
#include "bluetoothadapterproxy.h"
#include "bluetoothserialproxy.h"


class ObdBluetoothDevice
{
public:
	std::string getDeviceForAddress(std::string address)
	{

		GError* error = NULL;
		OrgBluezManager* manager = org_bluez_manager_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
																				 G_DBUS_PROXY_FLAGS_NONE,
																				 "org.bluez","/",NULL, &error);

		if(!manager)
		{
			DebugOut(0)<<"Error getting bluetooth manager proxy: "<<error->message<<endl;
			g_error_free(error);
			return "";
		}

		error = NULL;

		gchar* adapterPath;
		if(!org_bluez_manager_call_default_adapter_sync(manager,&adapterPath,NULL,&error))
		{
			DebugOut(0)<<"Error getting bluetooth default adapter: "<<error->message<<endl;
			g_error_free(error);
			return "";
		}

		error = NULL;

		OrgBluezAdapter* adapter = org_bluez_adapter_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
																				 G_DBUS_PROXY_FLAGS_NONE,
																				 "org.bluez",adapterPath,NULL,&error);
		if(!adapter)
		{
			DebugOut(0)<<"Error getting bluetooth adapter proxy: "<<error->message<<endl;
			g_error_free(error);
			return "";
		}

		error = NULL;

		gchar* devicePath;
		if(!org_bluez_adapter_call_find_device_sync(adapter,address.c_str(),&devicePath,NULL,&error) ||
				std::string(devicePath) == "")
		{
			DebugOut(0)<<"Error finding bluetooth device: "<<address<<error->message<<endl;
			g_error_free(error);
			return "";
		}

		error = NULL;

		OrgBluezSerial* serialDevice = org_bluez_serial_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
																					G_DBUS_PROXY_FLAGS_NONE,
																					"org.bluez",devicePath,NULL,&error);

		if(!serialDevice)
		{
			DebugOut(0)<<"Error getting bluetooth serial device proxy: "<<error->message<<endl;
			g_error_free(error);
			return "";
		}

		gchar* serialDeviceName;
		if(!org_bluez_serial_call_connect_sync(serialDevice,"spp",&serialDeviceName,NULL,&error))
		{
			DebugOut(0)<<"Error connecting bluetooth serial device: "<<address<<" - "<<error->message<<endl;
			g_error_free(error);
			return "";
		}

		return serialDeviceName;
	}
};


#endif
