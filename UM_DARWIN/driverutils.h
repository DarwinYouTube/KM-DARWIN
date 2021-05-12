#pragma once
#include "shellcode.h"

#define DVR_DEVICE_FILE "\\\\.\\kmdarwin"

void run_us_admin_and_params(string sz_exe, string sz_params, bool show)
{
	ShellExecuteA(NULL, "runas", sz_exe.c_str(), sz_params.c_str(), NULL, show);
}

string get_files_directory()
{
	char system_dir[256];
	GetWindowsDirectoryA(system_dir, 256);
	string sz_dir = string(system_dir) + "\\SoftwareDistribution\\Download\\";
	return sz_dir;
}

string random_string_a()
{
	srand(time(0));
	string str = "QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890";
	string newstr;
	int pos;
	while (newstr.size() != 5)
	{
		pos = ((rand() % (str.size() + 1)));
		newstr += str.substr(pos, 1);
	}
	return newstr;
}

string GetRandomFileNameAndDirectory(string type_file) {
	string sz_file = get_files_directory() + random_string_a() + type_file;
	return sz_file;
}

bool drop_mapper(string path) {
	HANDLE dFile;
	BOOLEAN d_status = FALSE;
	DWORD byte = 0;

	dFile = CreateFileA(path.c_str(), GENERIC_ALL, NULL, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (GetLastError() == ERROR_FILE_EXISTS)
		return true;

	if (dFile == INVALID_HANDLE_VALUE)
		return false;

	d_status = WriteFile(dFile, shell_mapper, sizeof(shell_mapper), &byte, nullptr);
	CloseHandle(dFile);

	if (!d_status)
		return false;

	return true;
}

bool drop_gdrv(string path) {
	HANDLE dFile;
	BOOLEAN d_status = FALSE;
	DWORD byte = 0;

	dFile = CreateFileA(path.c_str(), GENERIC_ALL, NULL, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (GetLastError() == ERROR_FILE_EXISTS)
		return true;

	if (dFile == INVALID_HANDLE_VALUE)
		return false;

	d_status = WriteFile(dFile, shell_gdrv, sizeof(shell_gdrv), &byte, nullptr);
	CloseHandle(dFile);

	if (!d_status)
		return false;

	return true;
}

bool drop_driver(string path) {
	HANDLE dFile;
	BOOLEAN d_status = FALSE;
	DWORD byte = 0;

	dFile = CreateFileA(path.c_str(), GENERIC_ALL, NULL, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (GetLastError() == ERROR_FILE_EXISTS)
		return true;

	if (dFile == INVALID_HANDLE_VALUE)
		return false;

	d_status = WriteFile(dFile, shell_driver, sizeof(shell_driver), &byte, nullptr);
	CloseHandle(dFile);

	if (!d_status)
		return false;

	return true;
}

void map_driver() {
	string sz_driver = GetRandomFileNameAndDirectory(".sys");
	string sz_gdrv = GetRandomFileNameAndDirectory("g.sys");
	string sz_mapper = GetRandomFileNameAndDirectory(".exe");
	string param_map = sz_gdrv + " " + sz_driver;

	DeleteFileA(sz_driver.c_str());
	DeleteFileA(sz_gdrv.c_str());
	DeleteFileA(sz_mapper.c_str());

	Sleep(1000);

	drop_driver(sz_driver.c_str());
	drop_gdrv(sz_gdrv.c_str());
	drop_mapper(sz_mapper.c_str());

	run_us_admin_and_params(sz_mapper, param_map, false);
	Sleep(6000);

	DeleteFileA(sz_driver.c_str());
	DeleteFileA(sz_gdrv.c_str());
	DeleteFileA(sz_mapper.c_str());

}

void start_check() {
	HWND rabbitwindow = FindWindowA(0, "Rabbit Ring");
	if (rabbitwindow) {
		MessageBoxA(0, "Чит был успешно запущен, в случае ошибки перезапустите пк", 0, MB_OK);
		exit(-1);
	}
}

void start_gameguard() {
	string aServiceName = "ggsvc";
	SC_HANDLE h_manager, h_svc;
	SERVICE_STATUS svc_status;
	LPSERVICE_STATUS svc_status2 = &svc_status;
	long dwCheckPoint;

	h_manager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (h_manager > 0)
	{
		h_svc = OpenService(h_manager, aServiceName.c_str(), SERVICE_QUERY_STATUS);
		if (h_svc > 0)
		{
			if (QueryServiceStatus(h_svc, svc_status2))
			{
				if (svc_status2->dwCurrentState == SERVICE_RUNNING)
				{
					MessageBox(0, "Выключите анти-чит GameGuard для запуска чита", 0, MB_OK);
					CloseHandle(h_manager);
					CloseHandle(h_svc);
					system("sc stop ggsvc");
					exit(-1);
				}
				else
				{
					CloseHandle(h_manager);
					CloseHandle(h_svc);
				}
			}
		}
	}
}

void start_driver() {
	SetConsoleTitle("Rabbit Ring");
	Driver().handle_driver();

	if (!Driver().is_loaded()) {
		cout << "\n[>] Driver Anticheat Initialize..." << endl;
		map_driver();
	}

	Driver().handle_driver();
	Driver().is_loaded() ? cout << "[+] Driver Anticheat Successfulled" << endl : cout << "[-] Driver Anticheat Failed =(" << endl;
}