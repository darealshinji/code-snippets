#ifdef _WIN32
# include <windows.h>
#else
# include <fcntl.h>
# include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>

#define UUID_LEN 36
#define UUID_BYTES 16
static char uuidStr[UUID_LEN + 1] = {0};


const char *gen_uuid()
{
	unsigned char b[UUID_BYTES];
	memset(&uuidStr, 0, sizeof(uuidStr));

#ifdef _WIN32
	HMODULE hLib = LoadLibraryExA("rpcrt4.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (!hLib) return NULL;

	FARPROC myUuidCreate = GetProcAddress(hLib, "UuidCreate");
	if (!myUuidCreate) {
		FreeLibrary(hLib);
		return NULL;
	}

	UUID uuid;
	RPC_STATUS ret = myUuidCreate(&uuid);
	FreeLibrary(hLib);

	if (ret != RPC_S_OK && ret != RPC_S_UUID_LOCAL_ONLY) {
		return NULL;
	}

	unsigned char *p = b;
	memcpy(p, &uuid.Data1, 4);
	memcpy(p+4, &uuid.Data2, 2);
	memcpy(p+6, &uuid.Data3, 2);
	memcpy(p+8, uuid.Data4, 8);
#else
	int fd = open("/dev/random", O_RDONLY);
	if (fd == -1) return NULL;
	ssize_t nread = read(fd, &b, UUID_BYTES);
	close(fd);
	if (nread != UUID_BYTES) return NULL;
#endif

	sprintf(uuidStr, "%02x%02x%02x%02x-"
		"%02x%02x-%02x%02x-%02x%02x-"
		"%02x%02x%02x%02x%02x%02x",
		b[0],b[1],b[2],b[3],
		b[4],b[5], b[6],b[7], b[8],b[9],
		b[10],b[11],b[12],b[13],b[14],b[15]);

	return uuidStr;
}

int main()
{
	const char *uuid = gen_uuid();
	printf("%s\n", uuid ? uuid : "error");

	return 0;
}
