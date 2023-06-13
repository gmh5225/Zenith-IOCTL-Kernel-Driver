#pragma once
namespace vortex
{	
	auto io_dispatch( 
		PDEVICE_OBJECT device_object, 
		PIRP irp ) -> NTSTATUS
	{		
		UNREFERENCED_PARAMETER( device_object );	

		invoke_data* buffer = reinterpret_cast< invoke_data* >(
			irp->AssociatedIrp.SystemBuffer);
		
		switch ( buffer->code )
		{

		case invoke_base:
		{
			if ( request::get_module_base( buffer ) != ZENITH_SUCCESSFUL_OPERATION )
			{
				irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			}	
			break;
		}

		case invoke_read:
		{			
			if ( request::read_memory( buffer ) != ZENITH_SUCCESSFUL_OPERATION )
			{
				irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			}
			
			break;
		}

		case invoke_write:
		{
			if ( request::write_memory( buffer ) != ZENITH_SUCCESSFUL_OPERATION )
			{
				irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			}
			break;
		}
		}

		irp->IoStatus.Status = STATUS_SUCCESS;
		qtx_import( IofCompleteRequest )(irp, IO_NO_INCREMENT);

		return STATUS_SUCCESS;
	}

	auto io_close( 
		PDEVICE_OBJECT,
		PIRP Irp ) -> NTSTATUS
	{
		Irp->IoStatus.Status = STATUS_SUCCESS;
		Irp->IoStatus.Information = 0;

		qtx_import(IofCompleteRequest)(Irp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}

	auto vortex_cave_entry( 
		PDRIVER_OBJECT driver_obj, 
		PUNICODE_STRING registry_path ) -> NTSTATUS
	{
		UNREFERENCED_PARAMETER( registry_path );

		// update this LOL, better way's to execute.
		BYTE shell_code[ ] = {
			0x90, 0x48, 0xB8,										                 // nop , mov rax 
			0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,                          // address
			0xFF, 0xE0																 // jmp rax
		};

		*reinterpret_cast<void**>(&shell_code [ 3 ]) = &vortex::io_dispatch;
		if ( !NT_SUCCESS( ctx::write_protected_address( globals::entry_point, shell_code, sizeof( shell_code ), true ) ) )
			return ZENITH_FAILED_SANITY_CHECK;

		*reinterpret_cast< void** >(&shell_code [ 3 ]) = &vortex::io_close;
		auto create_close = (globals::cave_base + sizeof( shell_code ));
		if ( !NT_SUCCESS( ctx::write_protected_address( create_close, shell_code, sizeof( shell_code ), true ) ) )
			return ZENITH_FAILED_SANITY_CHECK;

		UNICODE_STRING device;
		UNICODE_STRING dos_device;

		qtx_import(RtlInitUnicodeString)(&device, _( DEVICE_NAME ));
		qtx_import(RtlInitUnicodeString)(&dos_device, _( DOS_NAME ));

		PDEVICE_OBJECT device_obj = nullptr;
		auto status = qtx_import(IoCreateDevice)(driver_obj,
			0,
			&device,
			FILE_DEVICE_UNKNOWN,
			FILE_DEVICE_SECURE_OPEN,
			FALSE,
			&device_obj);

		if ( NT_SUCCESS( status ) )
		{
			SetFlag( driver_obj->Flags, DO_BUFFERED_IO );

			driver_obj->MajorFunction [ IRP_MJ_CREATE ] = reinterpret_cast< PDRIVER_DISPATCH >(create_close);
			driver_obj->MajorFunction [ IRP_MJ_CLOSE ] = reinterpret_cast< PDRIVER_DISPATCH >(create_close);
			driver_obj->MajorFunction [ IRP_MJ_DEVICE_CONTROL ] = reinterpret_cast< PDRIVER_DISPATCH >(globals::entry_point);
			driver_obj->DriverUnload = nullptr; 

			ClearFlag( device_obj->Flags, DO_DIRECT_IO );
			ClearFlag( device_obj->Flags, DO_DEVICE_INITIALIZING );

			status = qtx_import(IoCreateSymbolicLink)( &dos_device , &device );
			if ( !NT_SUCCESS( status ) ) {
				qtx_import( IoDeleteDevice )(device_obj);
			}

		}
		return status;
	}
	
}
