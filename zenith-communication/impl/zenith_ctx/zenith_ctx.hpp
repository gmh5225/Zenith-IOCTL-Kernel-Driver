#ifndef ZENITH_CTX_HPP
#define ZENITH_CTX_HPP
#define DEVICE "\\\\.\\{033f5028-c00d-4332-ab7f-b72d6ca908c4}"

struct IO_STATUS_BLOCK {
	union {
		NTSTATUS Status;
		PVOID Pointer;
	};
	ULONG_PTR Information;
};

using PIO_STATUS_BLOCK = IO_STATUS_BLOCK*;
using PIO_APC_ROUTINE = void(NTAPI*)(
	PVOID, 
	PIO_STATUS_BLOCK, 
	std::uint32_t);

extern "C" __int64 direct_device_control(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	std::uint32_t IoControlCode,
	PVOID InputBuffer,
	std::uint32_t InputBufferLength,
	PVOID OutputBuffer,
	std::uint32_t OutputBufferLength );

namespace zenith
{
	class communicate_t
	{
		typedef enum _requests
		{
			invoke_start,
			invoke_base,
			invoke_read,
			invoke_write,
			invoke_success,
			invoke_unique,

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

		typedef struct _invoke_data
		{
			uint32_t unique;
			requests code;
			void* data;
		}invoke_data, * pinvoke_data;
		
		std::int32_t m_pid = 0;
		void* m_handle = 0;
		
	public:
		
		std::uintptr_t image_base = 0;
		
		[[nodiscard]] bool send_cmd( void* data, requests code );
		[[nodiscard]] bool initialize_handle( );
		[[nodiscard]] bool attach( int a_pid );
		[[nodiscard]] const std::uintptr_t get_image_base( const char* module_name );

		[[nodiscard]] bool read_virtual( const uintptr_t address, void* buffer, const size_t size );
		[[nodiscard]] bool write_virtual( const uintptr_t address, void* buffer, const size_t size );

		template <typename t>
		[[nodiscard]] auto write( const uintptr_t address, t value ) -> bool
		{
			return write_virtual( address, &value, sizeof( t ) );
		}

		template <typename t>
		[[nodiscard]] auto read( const uintptr_t address ) -> t
		{
			t response {};
			read_virtual( address, &response, sizeof( t ) );
			return response;
		}

		template <typename t>
		[[nodiscard]] auto read_array( const uintptr_t address, t buffer, size_t size ) -> bool
		{
			return read_virtual( address, buffer, size );
		}
	};
}

static zenith::communicate_t* ctx = new zenith::communicate_t( );

#endif // include guard