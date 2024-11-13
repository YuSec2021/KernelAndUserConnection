#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

#define CWK_DEV_SYM L"\\\\.\\TestDevice"
#define CRL_CODE_INDEX 0x800
#define TEST_R3TOR0_CODE (ULONG)CTL_CODE(FILE_DEVICE_UNKNOWN, CRL_CODE_INDEX, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define TEST_R0TOR3_CODE (ULONG)CTL_CODE(FILE_DEVICE_UNKNOWN, CRL_CODE_INDEX+1, METHOD_BUFFERED, FILE_ANY_ACCESS)


HANDLE OpenDevice() {
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

	// check DEVICE_OBJECT
	if (pDevice == INVALID_HANDLE_VALUE) {
		printf("Open Device Failed. \r\n");
		return NULL;
	}

	return pDevice;
}

void sendMessage() {
	HANDLE pDevice = OpenDevice();

	char msgArray[] = "Test Message From app.\r\n";
	char* msg = msgArray;
	ULONG retLen = 0;

	BOOLEAN res = DeviceIoControl(pDevice, TEST_R3TOR0_CODE, msg, strlen(msg) + 1, NULL, NULL, &retLen, NULL);
	if (!res) {
		printf("Send Message Failed..\r\n");
	}
	CloseHandle(pDevice);
}

void receiveMessage() {
	HANDLE pDevice = OpenDevice();

	int ret = 0;
	ULONG retLen = 0;

	BOOLEAN res = DeviceIoControl(pDevice, TEST_R0TOR3_CODE, NULL, NULL, &ret, 4, &retLen, NULL);
	if (!res) {
		printf("Receive Message Failed..\r\n");
	}

	printf("retLen: %d\r\n", retLen);
	printf("[db] Receive Message From Ring0: %d\r\n", ret);
	CloseHandle(pDevice);
}


int _tmain(int argc, _TCHAR* argv[]) {
	/*sendMessage();
	receiveMessage();*/

	HANDLE pDevice = OpenDevice();
	DWORD p = 0;

	char sendStr[] = "Send Message To Ring0 By IRP_MJ_WRITE";
	BOOLEAN res = WriteFile(pDevice, sendStr, strlen(sendStr)+1, &p, NULL);
	if (res) {
		printf("Write Success\r\n");
	}
	
	char* recStr = (char*)malloc(0x1000);
	res = ReadFile(pDevice, recStr, 0x1000, &p, NULL);
	if (res) {
		printf("Read Success\r\n");
		printf("[db]: %s\r\n", recStr);
	}

	CloseHandle(pDevice);

	return 0;
}