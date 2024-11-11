#include <ntifs.h>

#define DEVICE_NAME L"\\driver\\TestDevice"
#define SYM_NAME L"\\??\\TestDevice"

#define CODE_CTR_INDEX 0x800
#define TEST_CODE CTL_CODE(FILE_DEVICE_UNKNOWN, \
														CODE_CTR_INDEX, \
														METHOD_BUFFERED, \
														FILE_ANY_ACCESS)

NTSTATUS DriverDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, 0);
	return STATUS_SUCCESS;
}

NTSTATUS Dispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	PIO_STACK_LOCATION ioStack = IoGetCurrentIrpStackLocation(Irp);

	if (ioStack->MajorFunction == IRP_MJ_DEVICE_CONTROL) {
		int size = ioStack->Parameters.DeviceIoControl.InputBufferLength;
		ULONG IoControlCode = ioStack->Parameters.DeviceIoControl.IoControlCode;
		switch (IoControlCode) {
			case TEST_CODE:
			{
				int* buffer = (int*)Irp->AssociatedIrp.SystemBuffer;
				KdPrintEx((77, 0, "[db]: %x\r\n", *buffer));
				break;

			}
		}
	}

	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, 0);
	return STATUS_SUCCESS;
}


VOID DriverUnload(PDRIVER_OBJECT pDriver) {}




NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {

	UNICODE_STRING deviceName = { 0 };
	RtlInitUnicodeString(&deviceName, DEVICE_NAME);

	UNICODE_STRING symName = { 0 };
	RtlInitUnicodeString(&symName, SYM_NAME);

	PDEVICE_OBJECT pDevice = NULL;

	NTSTATUS status = IoCreateDevice(
		pDriver,
		0,
		&deviceName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		pDevice
	);

	if (NT_SUCCESS(status)) {
		KdPrintEx((77, 0, "res: %x\r\n", status));
		return status;
	}

	// °ó¶¨·ûºÅÁ´½Ó
	status = IoCreateSymbolicLink(&symName, &deviceName);

	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(pDevice);
		KdPrintEx((77, 0, "res: %x\r\n", status));
		return status;
	}

	pDevice->Flags &= ~DO_DEVICE_INITIALIZING;
	pDevice->Flags |= DO_BUFFERED_IO;

	pDriver->MajorFunction[IRP_MJ_CREATE] = DriverDispatch;
	pDriver->MajorFunction[IRP_MJ_CLOSE] = DriverDispatch;
	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Dispatch;


	pDriver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}