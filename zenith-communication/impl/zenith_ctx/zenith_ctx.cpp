#ifndef  ZENITH_CTX_CPP
#define ZENITH_CTX_CPP

#include "../include.hpp"

auto zenith::communicate_t::send_cmd(
	void* data,
	requests code ) -> bool
{
	if ( !data || !code )
		return false;

	IO_STATUS_BLOCK block;
	invoke_data request { 0 };

	request.unique = requests::invoke_unique;
	request.data = data;
	request.code = code;

	auto result =
		direct_device_control(
		this->m_handle, 
		nullptr, 
		nullptr,
		nullptr, 
		&block, 
		0,
		&request,
		sizeof( request ), 
		&request, 
		sizeof( request ) );

	return result;
}

auto zenith::communicate_t::initialize_handle( ) -> bool
{
	this->m_handle = CreateFileA( DEVICE, GENERIC_READ, 0, 0, 3, 0x00000080, 0 );
	if ( this->m_handle != INVALID_HANDLE_VALUE ) {
		return true;
	}
	return false;
}

auto zenith::communicate_t::attach(
	int a_pid ) -> bool
{
	if ( !a_pid )
		return false;

	this->m_pid = a_pid;

	return true;
}

auto zenith::communicate_t::get_image_base(
	const char* module_name ) -> const std::uintptr_t
{
	base_invoke data { 0 };
	
	data.pid = this->m_pid;
	data.handle = 0;
	data.name = module_name;

	this->send_cmd(
		&data,
		invoke_base );

	return data.handle;
}

auto zenith::communicate_t::write_virtual(
	const std::uintptr_t address, 
	void* buffer,
	const std::size_t size ) -> bool
{
	write_invoke data { 0 };

	data.pid = this->m_pid;
	data.address = address;
	data.buffer = buffer;
	data.size = size;

	auto result = 
		this->send_cmd( 
			&data, 
			invoke_write );

	return result;
}

auto zenith::communicate_t::read_virtual(
	const std::uintptr_t address, 
	void* buffer,
	const std::size_t size ) -> bool
{
	read_invoke data { 0 };

	data.pid = this->m_pid;
	data.address = address;
	data.buffer = buffer;
	data.size = size;

	auto result = 
		this->send_cmd( 
			&data, 
			invoke_read );
	
	return result;
}

#endif // ! ZENITH_CTX_CPP
