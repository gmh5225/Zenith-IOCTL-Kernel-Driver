
#define DEVICE_NAME L"\\Device\\{033f5028-c00d-4332-ab7f-b72d6ca908c4}"
#define DOS_NAME L"\\DosDevices\\{033f5028-c00d-4332-ab7f-b72d6ca908c4}"

typedef enum _requests
{
	invoke_start,
	invoke_base,
	invoke_read,
	invoke_write,
	invoke_pattern,
	invoke_success,
	invoke_unique,
	invoke_mouse,

}requests, * prequests;

typedef struct _read_invoke {
	uint32_t pid;
	uintptr_t address;
	void* buffer;
	size_t size;
} read_invoke, * pread_invoke;

typedef struct _write_invoke {
	uint32_t pid;
	uintptr_t address;
	void* buffer;
	size_t size;
} write_invoke, * pwrite_invoke;

typedef struct _base_invoke {
	uint32_t pid;
	uintptr_t handle;
	const char* name;
	size_t size;
} base_invoke, * pbase_invoke;

typedef struct _pattern_invoke
{
	int pid;
	uintptr_t base;
	char signature [ 260 ];
	uintptr_t address;
}pattern_invoke, * ppattern_invoke;

typedef struct _mouse_invoke
{
	long x;
	long y;
	unsigned short button_flags;
} mouse_invoke, * pmouse_invoke;

typedef struct _invoke_data
{
	uint32_t unique;
	requests code;
	void* data;
}invoke_data, * pinvoke_data;