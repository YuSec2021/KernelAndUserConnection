#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

#define CWK_DEV_SYM L"\\\\.\\TestDevice"
#define CRL_CODE_INDEX 0x800
#define TEST_CODE (ULONG)CTL_CODE(FILE_DEVICE_UNKNOWN, CRL_CODE_INDEX, METHOD_BUFFERED, FILE_ANY_ACCESS)

int _tmain(int argc, _TCHAR* argv[]) {
	HANDLE pDevice = NULL;

	pDevice = CreateFile(
		CWK_DEV_SYM,
		GENERIC_READ | GENERIC_WRITE,
		NULL, 
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_SYSTEM,
		0
	);

	if (pDevice == INVALID_HANDLE_VALUE) {
		printf("coworker demo: Open device failed. \r\n");
		return -1;
	}

	char msgArray[] = "Test message from app.\r\n";
	char* msg = msgArray;
	ULONG retLen = 0;

	if (!DeviceIoControl(pDevice, TEST_CODE, msg, strlen(msg) + 1, NULL, 0, &retLen, 0)) {
		printf("send failed..\r\n");
		CloseHandle(pDevice);
		return -1;
	}

	return 1;
}