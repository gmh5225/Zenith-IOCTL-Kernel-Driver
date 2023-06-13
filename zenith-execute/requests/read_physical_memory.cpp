#pragma once

namespace request
{	
    NTSTATUS read_memory( invoke_data* request )
    {
        read_invoke data = { 0 };

        if (!modules::safe_copy( &data, request->data, sizeof( read_invoke ) ))
            return 0;

		if ( !data.address || !data.pid || !data.buffer || !data.size || data.address >= 0x7FFFFFFFFFFF )
			return 0;

		PEPROCESS process = 0;
		if ( !NT_SUCCESS( qtx_import(PsLookupProcessByProcessId)( ( HANDLE ) data.pid, &process ) ) )
		{
			return zenith::status::failed_sanity_check;
		}

		//if ( data.address >= 0x7FFFFFFFFFFF || data.phys )
		//{

		//	ULONGLONG process_base = physical_memory_operations::get_process_cr3( process );
		//	imports::obf_dereference_object( process );

		//	SIZE_T this_offset = NULL;
		//	SIZE_T total_size = data.size;

		//	INT64 physical_address = physical_memory_operations::translate_linear( process_base, ( ULONG64 ) data.address + this_offset );
		//	if ( !physical_address )
		//		return dolby::status::failed_sanity_check;;

		//	ULONG64 final_size = physical_memory_operations::find_min( PAGE_SIZE - (physical_address & 0xFFF), total_size );
		//	SIZE_T bytes_trough = NULL;

		//	if ( !NT_SUCCESS( physical_memory_operations::read( PVOID( physical_address ), ( PVOID ) ((ULONG64) reinterpret_cast< read_invoke* > (request->data)->buffer + this_offset), final_size, &bytes_trough ) ) )
		//	{
		//		imports::obf_dereference_object( process ); 

		//		return dolby::status::failed_sanity_check;
		//	}

		//	return dolby::status::failed_sanity_check;
		//}

		//printf( "mmcopyvirt: cp, addr %llx, pid %i, %llx, size %i\n", data.address, data.pid, reinterpret_cast<write_invoke*>(request->data)->buffer, data.size );
		size_t bytes = 0;
		if ( qtx_import(MmCopyVirtualMemory)( 
			process, 
			(void*)data.address,
			qtx_import(IoGetCurrentProcess)( ), 
			(void*)reinterpret_cast<read_invoke*> (request->data)->buffer, 
			data.size, 
			UserMode, 
			&bytes ) != STATUS_SUCCESS || bytes != data.size)
		{
			qtx_import( ObfDereferenceObject )(process);
			return zenith::status::failed_sanity_check;
		}

		qtx_import(ObfDereferenceObject)( process );

        return zenith::status::successful_operation;
    }
}