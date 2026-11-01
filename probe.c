#include <windows.h>
#include <stdio.h>

static void show_region(DWORD addr) {
	MEMORY_BASIC_INFORMATION mbi;
	if (VirtualQuery((void *)(ULONG_PTR)addr, &mbi, sizeof(mbi)) == 0) {
		printf("VirtualQuery(%08lx) failed err=%lu\n", addr, GetLastError());
		return;
	}
	printf("region %08lx: base=%p allocbase=%p size=%08lx state=%08lx protect=%08lx type=%08lx\n", addr,
		   mbi.BaseAddress, mbi.AllocationBase, (DWORD)mbi.RegionSize, mbi.State, mbi.Protect, mbi.Type);
}

static void try_map(HANDLE m, DWORD base, DWORD size) {
	SetLastError(0xdeadbeef);
	void *v = MapViewOfFileEx(m, FILE_MAP_ALL_ACCESS, 0, 0, size, (void *)(ULONG_PTR)base);
	DWORD err = GetLastError();
	printf("MapViewOfFileEx(base=%08lx, size=%08lx, last byte=%08lx) -> %p err=%lu\n", base, size, base + size - 1,
		   v, v ? 0 : err);
	if (v)
		UnmapViewOfFile(v);
}

static void try_alloc(DWORD base, DWORD size) {
	SetLastError(0xdeadbeef);
	void *p = VirtualAlloc((void *)(ULONG_PTR)base, size, MEM_RESERVE, PAGE_NOACCESS);
	DWORD err = GetLastError();
	printf("VirtualAlloc(base=%08lx, size=%08lx, last byte=%08lx) -> %p err=%lu\n", base, size, base + size - 1, p,
		   p ? 0 : err);
	if (p)
		VirtualFree(p, 0, MEM_RELEASE);
}

int main(void) {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	printf("max app address %p, granularity %08lx\n", si.lpMaximumApplicationAddress, si.dwAllocationGranularity);
	show_region(0x7FFC0000);
	show_region(0x7FFD0000);
	show_region(0x7FFE0000);
	show_region(0x7FFE1000);
	show_region(0x7FFEF000);

	HANDLE m = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 0x40000, NULL);
	printf("section 0x40000 -> %p err=%lu\n", m, m ? 0 : GetLastError());

	/* Control: a free, fitting range. */
	void *free_spot = VirtualAlloc(NULL, 0x40000, MEM_RESERVE, PAGE_NOACCESS);
	VirtualFree(free_spot, 0, MEM_RELEASE);
	try_map(m, (DWORD)(ULONG_PTR)free_spot, 0x20000);

	/* Ranges below the limit that are (probably) occupied near the top. */
	try_map(m, 0x7FFE0000, 0x10000);
	try_map(m, 0x7FFD0000, 0x20000);
	try_map(m, 0x7FFD0000, 0x10000);
	try_map(m, 0x7FFC0000, 0x10000);
	/* Ranges that run past 0x7FFEFFFF. */
	try_map(m, 0x7FFE0000, 0x10001);
	try_map(m, 0x7FFE0000, 0x20000);
	try_map(m, 0x7FFD0000, 0x20001);
	try_map(m, 0x7FFD0000, 0x30000);
	try_map(m, 0x7FFC0000, 0x40000);
	/* Bases past the limit, for comparison. */
	try_map(m, 0x7FFF0000, 0x10000);
	try_map(m, 0x80000000, 0x10000);

	HANDLE big = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_RESERVE, 0, 0x70000000, NULL);
	printf("section 0x70000000 -> %p err=%lu\n", big, big ? 0 : GetLastError());
	if (big) {
		try_map(big, 0x10000000, 0x6FFF0000); /* last byte 0x7FFEFFFF */
		try_map(big, 0x10000000, 0x70000000); /* last byte 0x7FFFFFFF */
		try_map(big, 0x40000000, 0x40000000); /* last byte 0x7FFFFFFF */
		try_map(big, 0x40000000, 0x3FFF0000); /* last byte 0x7FFEFFFF */
	}

	try_alloc(0x7FFE0000, 0x10000);
	try_alloc(0x7FFE0000, 0x20000);
	try_alloc(0x7FFD0000, 0x30000);
	try_alloc(0x10000000, 0x70000000);
	return 0;
}
