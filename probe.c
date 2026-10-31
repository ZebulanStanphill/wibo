#include <windows.h>
#include <stdio.h>
int main(void) {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	printf("max app address %p\n", si.lpMaximumApplicationAddress);
	SetLastError(0xdeadbeef);
	void *p = VirtualAlloc((void *)0x80000000u, 0x10000, MEM_RESERVE, PAGE_NOACCESS);
	printf("VirtualAlloc(0x80000000) -> %p err=%lu\n", p, GetLastError());
	HANDLE m = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 0x10000, NULL);
	SetLastError(0xdeadbeef);
	void *v = MapViewOfFileEx(m, FILE_MAP_ALL_ACCESS, 0, 0, 0x10000, (void *)0x80000000u);
	printf("MapViewOfFileEx(pagefile, 0x80000000) -> %p err=%lu\n", v, GetLastError());
	SetLastError(0xdeadbeef);
	v = MapViewOfFileEx(m, FILE_MAP_ALL_ACCESS, 0, 0, 0x10000, (void *)0x90000000u);
	printf("MapViewOfFileEx(pagefile, 0x90000000) -> %p err=%lu\n", v, GetLastError());
	SetLastError(0xdeadbeef);
	v = MapViewOfFileEx(m, FILE_MAP_ALL_ACCESS, 0, 0, 0x10000, (void *)0x7FFF0000u);
	printf("MapViewOfFileEx(pagefile, 0x7FFF0000) -> %p err=%lu\n", v, GetLastError());
	return 0;
}
