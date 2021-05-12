#include "main.h"
#define DVR_DEVICE_FILE "\\\\.\\kmdarwin"

DarwinTap::DarwinTap() {

}

void DarwinTap::handle_driver() {
	hDriver = CreateFileA(DVR_DEVICE_FILE, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
}

DarwinTap::~DarwinTap() {
	CloseHandle(hDriver);
}

DarwinTap& DarwinTap::singleton()
{
	static DarwinTap p_object;
	return p_object;
}