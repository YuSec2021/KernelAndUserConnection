#include <ntifs.h>

#define CRL_CODE_INDEX 0x800
#define TEST_R3TOR0_CODE (ULONG)CTL_CODE(FILE_DEVICE_UNKNOWN, CRL_CODE_INDEX, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define TEST_R0TOR3_CODE (ULONG)CTL_CODE(FILE_DEVICE_UNKNOWN, CRL_CODE_INDEX+1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define CDO_NAME L"\\Device\\TestDevice"
#define SYM_NAME L"\\??\\TestDevice"

PDEVICE_OBJECT pDevice = NULL;

NTSTATUS TestCreateCDO(PDRIVER_OBJECT pDriver) {
	UNICODE_STRING cdoName = RTL_CONSTANT_STRING(CDO_NAME);
	NTSTATUS status = IoCreateDevice(
		pDriver,
		0,
		&cdoName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&pDevice
	);

	return status;
}

NTSTATUS TestCreateSymbolicLink(PDRIVER_OBJECT drvier) {
	UNICODE_STRING cdoName = RTL_CONSTANT_STRING(CDO_NAME);
	UNICODE_STRING symName = RTL_CONSTANT_STRING(SYM_NAME);
	return IoCreateSymbolicLink(&symName, &cdoName);
}

NTSTATUS InitDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp) {
	NTSTATUS status = STATUS_SUCCESS;
	ULONG retLen = 0;
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = retLen;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS TestDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp) {
	PIO_STACK_LOCATION ioStack = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_SUCCESS;
	ULONG retLen = 0;

	if (ioStack->MajorFunction == IRP_MJ_DEVICE_CONTROL) {

		switch (ioStack->Parameters.DeviceIoControl.IoControlCode) {
			case TEST_R3TOR0_CODE:
			{
				PVOID buffer = Irp->AssociatedIrp.SystemBuffer;
				ULONG inlen = ioStack->Parameters.DeviceIoControl.InputBufferLength;
				// receive Message
				KdPrintEx((77, 0, "[db]:%s\r\n", buffer));
				break;
			}
			case TEST_R0TOR3_CODE:
			{
				PVOID buffer = Irp->AssociatedIrp.SystemBuffer;
				ULONG outlen = ioStack->Parameters.DeviceIoControl.OutputBufferLength;
				// send Message
				ULONG x = 2000;
				memcpy((PULONG)buffer, &x, sizeof(ULONG));
				Irp->IoStatus.Information = outlen; // ÉèÖÃ
				DbgBreakPoint();
				break;
			}
			default:
				status = STATUS_INVALID_PARAMETER;
				break;
		}

	}
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = retLen;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;

}

VOID Unload(PDRIVER_OBJECT driver) {
	UNICODE_STRING symName = RTL_CONSTANT_STRING(SYM_NAME);
	IoDeleteDevice(pDevice);
	IoDeleteSymbolicLink(&symName);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING pReg) {
	pDriver->DriverUnload = Unload;
	pDriver->MajorFunction[IRP_MJ_CREATE] = InitDispatch;
	pDriver->MajorFunction[IRP_MJ_CLOSE] = InitDispatch;
	pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = TestDispatch;

	NTSTATUS status;
	status = TestCreateCDO(pDriver);
	if (!NT_SUCCESS(status)) {
		KdPrint(("create CDO Failed..."));
		return status;
	}
	pDevice->Flags &= ~DO_DEVICE_INITIALIZING;
	pDevice->Flags |= DO_BUFFERED_IO;

	status = TestCreateSymbolicLink(pDriver);
	if (!NT_SUCCESS(status)) {
		KdPrint(("create symboliclink failed..."));
		return status;
	}

	return STATUS_SUCCESS;
}