#ifndef ENTRYPOINT_CPP
#define ENTRYPOINT_CPP
#define debug false

#include <ntifs.h>
#include <windef.h>
#include <cstdint>
#include <intrin.h>
#include <ntimage.h>

#include "kernel/xor.h"
#include "kernel/structures.hpp"
#include "impl/imports.h"

#include "impl/communication/interface.h"

#include "impl/scanner.h"
#include "impl/modules.h"

#include "requests/get_module_base.cpp"
#include "requests/read_physical_memory.cpp"
#include "requests/write_physical_memory.cpp"
#include "requests/signature_scanner.cpp"

#include "impl/invoked.h"

_declspec(noinline) auto initialize_hook( ) -> zenith::status
{	
	auto ntos_base_address = modules::get_ntos_base_address( );
	if ( !ntos_base_address ) {
		return ZENITH_FAILED_SANITY_CHECK;
	}
	globals::ntos_image_base = ntos_base_address;

	auto io_create_drive = reinterpret_cast< PVOID >(modules::get_kernel_export( ntos_base_address, _( "IoCreateDriver" ) ));
	if ( qtx_import( MmIsAddressValid )(io_create_drive) ) {
		*( PVOID* ) &globals::io_create_driver = io_create_drive;
	}

	// sc start sometimes this isn't loaded? I would attack a vulnrable driver that doesn't cause issues to code cave.
	const uintptr_t hd_aud_bus = modules::get_kernel_module( _( "storqosflt.sys" ) );
	if ( !hd_aud_bus ) {
		print_dbg( _(" [log] -> failed to find module 2.\n") );
		return ZENITH_FAILED_SANITY_CHECK;
	}

	BYTE section_char[ ] = { 'I', 'N', 'I', 'T','\0' };
	globals::cave_base = modules::find_section( hd_aud_bus, reinterpret_cast< char* >(section_char) );

	if ( globals::cave_base ) {
		globals::cave_base = globals::cave_base - 0x30; // look for padded CC bytes :skull:
	}
	crt::kmemset( &section_char, 0, sizeof( section_char ) );

	// custom shellcode
	BYTE shell_code[ ] = {
		0x90, 0x48, 0xB8,										                 // nop, mov rax 
		0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,                          // address
		0xFF, 0xE0																 // jmp rax
	};

	globals::entry_point = globals::cave_base;

	*reinterpret_cast<void**>(&shell_code [ 3 ]) = &vortex::vortex_cave_entry;
	ctx::write_protected_address( globals::entry_point, shell_code, sizeof( shell_code ), true );

	if ( debug ) {
		print_dbg( _(" [ntoskrnl_image_base] -> 0x%p\n"), globals::ntos_image_base );
		print_dbg( _( " [addr_] -> 0x%p\n"), io_create_drive );
		print_dbg( _( " [image_base] -> 0x%p\n"), hd_aud_bus );
		print_dbg( _( " [code_cave] -> 0x%p\n"), globals::cave_base );
	}

	if ( !NT_SUCCESS( globals::io_create_driver( 
		NULL, 
		reinterpret_cast< PDRIVER_INITIALIZE >(globals::entry_point) ) ) )
	{
		print_dbg( _(" [log] -> check failed.\n") );

		return ZENITH_FAILED_SANITY_CHECK;
	}

	return ZENITH_SUCCESSFUL_OPERATION;
}

auto driver_entry( PDRIVER_OBJECT, PUNICODE_STRING ) -> NTSTATUS
{
	if ( initialize_hook( ) != ZENITH_SUCCESSFUL_OPERATION )
		return ZENITH_FAILED_DRIVER_ENTRY;

	print_dbg( _(" [driver] -> initialized.\n") );

	return STATUS_SUCCESS;
}

#endif // ENTRYPOINT_CPP