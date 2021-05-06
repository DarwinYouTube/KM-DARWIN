#include "event.h"
#include "struct.h"
#include "address.h"
#include "main.h"

PLOAD_IMAGE_NOTIFY_ROUTINE ImageCallback(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo) {

	if (wcsstr(FullImageName->Buffer, L"\\csgo\\bin\\client.dll")) {
		DbgPrintEx(0, 0, "[+] Image Name: %ls \n", FullImageName->Buffer);
		DbgPrintEx(0, 0, "[+] Process Id: %p \n", ProcessId);

		clientaddress = ImageInfo->ImageBase;
		csgoid = ProcessId;
	}
	return STATUS_SUCCESS;
}

NTSTATUS KeReadVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{
	PSIZE_T Bytes;
	if (NT_SUCCESS(MmCopyVirtualMemory(Process, SourceAddress, PsGetCurrentProcess(),
		TargetAddress, Size, KernelMode, &Bytes)))
		return STATUS_SUCCESS;
	else
		return STATUS_ACCESS_DENIED;
}

NTSTATUS KeWriteVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{
	PSIZE_T Bytes;
	if (NT_SUCCESS(MmCopyVirtualMemory(PsGetCurrentProcess(), SourceAddress, Process,
		TargetAddress, Size, KernelMode, &Bytes)))
		return STATUS_SUCCESS;
	else
		return STATUS_ACCESS_DENIED;
}

NTSTATUS IoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	NTSTATUS Status;
	ULONG BytesIO = 0;

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

	// Code received from user space
	ULONG ControlCode = stack->Parameters.DeviceIoControl.IoControlCode;

	if (ControlCode == IO_READ_REQUEST)
	{
		// Get the input buffer & format it to our struct
		PKERNEL_READ_REQUEST ReadInput = (PKERNEL_READ_REQUEST)Irp->AssociatedIrp.SystemBuffer;
		PKERNEL_READ_REQUEST ReadOutput = (PKERNEL_READ_REQUEST)Irp->AssociatedIrp.SystemBuffer;

		PEPROCESS Process;
		// Get our process
		if (NT_SUCCESS(PsLookupProcessByProcessId(ReadInput->ProcessId, &Process)))
			KeReadVirtualMemory(Process, ReadInput->Address,
				&ReadInput->Response, ReadInput->Size);

		Status = STATUS_SUCCESS;
		BytesIO = sizeof(KERNEL_READ_REQUEST);
	}
	else if (ControlCode == IO_WRITE_REQUEST)
	{
		// Get the input buffer & format it to our struct
		PKERNEL_WRITE_REQUEST WriteInput = (PKERNEL_WRITE_REQUEST)Irp->AssociatedIrp.SystemBuffer;

		PEPROCESS Process;
		// Get our process
		if (NT_SUCCESS(PsLookupProcessByProcessId(WriteInput->ProcessId, &Process)))
			KeWriteVirtualMemory(Process, &WriteInput->Value,
				WriteInput->Address, WriteInput->Size);

		Status = STATUS_SUCCESS;
		BytesIO = sizeof(KERNEL_WRITE_REQUEST);
	}
	else if (ControlCode == IO_GET_ID_REQUEST)
	{
		PULONG OutPut = (PULONG)Irp->AssociatedIrp.SystemBuffer;
		*OutPut = csgoid;

		Status = STATUS_SUCCESS;
		BytesIO = sizeof(*OutPut);
	}
	else if (ControlCode == IO_GET_MODULE_REQUEST)
	{
		PULONG OutPut = (PULONG)Irp->AssociatedIrp.SystemBuffer;
		*OutPut = clientaddress;

		Status = STATUS_SUCCESS;
		BytesIO = sizeof(*OutPut);
	}
	else
	{
		// if the code is unknown
		Status = STATUS_INVALID_PARAMETER;
		BytesIO = 0;
	}

	// Complete the request
	Irp->IoStatus.Status = Status;
	Irp->IoStatus.Information = BytesIO;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Status;
}

NTSTATUS CreateCall(PDEVICE_OBJECT DeviceObject, PIRP irp)
{
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS CloseCall(PDEVICE_OBJECT DeviceObject, PIRP irp)
{
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS clearTableEntry(PDRIVER_OBJECT driver, wchar_t* driverName) {
	__try
	{

		//check first entry
		PLDR_DATA_TABLE_ENTRY currentEntry = (PLDR_DATA_TABLE_ENTRY)(driver->DriverSection);
		PLDR_DATA_TABLE_ENTRY BeginningEntry = currentEntry;

		while ((PLDR_DATA_TABLE_ENTRY)(currentEntry->InLoadOrderLinks.Flink) != BeginningEntry)
		{
			if (!(ULONG)currentEntry->EntryPoint > MmUserProbeAddress)
			{
				currentEntry = (PLDR_DATA_TABLE_ENTRY)(currentEntry->InLoadOrderLinks.Flink);
				continue;
			}
			if (wcsstr(currentEntry->BaseDllName.Buffer, driverName)) {
				currentEntry->BaseDllName.Length = 0;
				return STATUS_SUCCESS;
			}
			currentEntry = (PLDR_DATA_TABLE_ENTRY)(currentEntry->InLoadOrderLinks.Flink);
		}

		return STATUS_UNSUCCESSFUL;

	}

	__except
		(1)
	{
		return STATUS_UNSUCCESSFUL;
	}
}

VOID HideDriver(PDRIVER_OBJECT pDriverObject)
{
	KIRQL irql = KeRaiseIrqlToDpcLevel(); // raises IRQL to DISPATCH level
	PLDR_DATA_TABLE_ENTRY prevEntry, nextEntry, modEntry;
	modEntry = (PLDR_DATA_TABLE_ENTRY)pDriverObject->DriverSection;
	prevEntry = (PLDR_DATA_TABLE_ENTRY)modEntry->InLoadOrderLinks.Blink;
	nextEntry = (PLDR_DATA_TABLE_ENTRY)modEntry->InLoadOrderLinks.Flink;
	// changing pointers of previous and next process in the list to point around our Driver process
	// this means our Driver process will be skipped when a different program tries enumerating the list
	prevEntry->InLoadOrderLinks.Flink = modEntry->InLoadOrderLinks.Flink;
	nextEntry->InLoadOrderLinks.Blink = modEntry->InLoadOrderLinks.Blink;
	// changing our flink and blink pointers to point to our Driver process
	// this is done just to not have any loose ends and accidentally blue screen
	modEntry->InLoadOrderLinks.Flink = (PLIST_ENTRY)modEntry;
	modEntry->InLoadOrderLinks.Blink = (PLIST_ENTRY)modEntry;
	// once everything is done we lower the IRQL level back to normal
	KeLowerIrql(irql);
}