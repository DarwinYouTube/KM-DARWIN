#include "main.h"
#include "event.h"
#include "struct.h"
#include "address.h"

// Точка входа
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pUnicodeString) {

	DbgPrintEx(0, 0, "[+] Driver Loaded\n");

	PsSetLoadImageNotifyRoutine(ImageCallback);

	RtlInitUnicodeString(&dev, L"\\Device\\kmdarwin");
	RtlInitUnicodeString(&dos, L"\\DosDevices\\kmdarwin");

	IoCreateDevice(pDriverObject, 0, &dev, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pDeviceObject);
	IoCreateSymbolicLink(&dos, &dev);

	HideDriver(pDriverObject);
	clearTableEntry(pDriverObject, &dev);

	pDriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCall;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseCall;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;
	pDriverObject->DriverUnload = DriverUnload;

	pDeviceObject->Flags |= DO_DIRECT_IO;
	pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	return STATUS_SUCCESS;
}

// Точка выхода
NTSTATUS DriverUnload(PDRIVER_OBJECT pDriverObject) {

	DbgPrintEx(0, 0, "[+] Driver Unloaded\n");

	PsRemoveLoadImageNotifyRoutine(ImageCallback);
	IoDeleteSymbolicLink(&dos);
	IoDeleteDevice(pDriverObject->DeviceObject);

	clearTableEntry(pDriverObject, &dev);

	return STATUS_SUCCESS;
}


