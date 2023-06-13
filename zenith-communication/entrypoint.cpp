#ifndef ENTRYPOINT_CPP
#define ENTRYPOINT_CPP

#include "impl/include.hpp"

auto main( ) -> void
{
	auto result = ctx->initialize_handle( );
	if ( !result ) {
		std::printf( "\n [log] -> failed to initialize driver.\n" );
		std::cin.get( );
	}
	std::printf( "\n [log] -> driver initialized.\n" );

	// replace this with the PID to attack.
	auto pid = GetCurrentProcessId( );
	if ( !pid ) {
		std::printf( " [log] -> failed to get pid.\n" );
	}
	
	auto ret = ctx->attach( pid );
	if ( !ret ) {
		std::printf( " [log] -> failed to attach to pid.\n" );
	}
	std::printf( " [pid] -> %i\n", pid );

	auto base_address = ctx->get_image_base( nullptr );
	if ( !base_address ) {
		std::printf( " [log] -> failed to get base address.\n" );
	}
	std::printf( " [base_address] -> %I64d\n", base_address );

	// rpm speed
	const auto rpm_time = [ & ] ( ) -> float
	{
		auto time_now = std::chrono::high_resolution_clock::now( );
		for ( auto x = 0ull; x < 0x10000; x++ ) {
			ctx->read<std::uintptr_t>( base_address + x );
		}
		auto time_span =
			std::chrono::duration_cast< std::chrono::duration< float> >(std::chrono::high_resolution_clock::now( ) - time_now);
		return time_span.count( );
	};
	
	std::printf( " [(0x10,000)] -> %fs\n", rpm_time( ) );

	const auto rpm_time2 = [ & ] ( ) -> float
	{
		auto time_now = std::chrono::high_resolution_clock::now( );
		for ( auto x = 0ull; x < 0x100000; x++ ) {
			ctx->read<std::uintptr_t>( base_address + x );
		}
		auto time_span =
			std::chrono::duration_cast< std::chrono::duration< float> >(std::chrono::high_resolution_clock::now( ) - time_now);
		return time_span.count( );
	};
	
	std::printf( " [(0x100,000)] -> %fs\n", rpm_time2( ) );

	std::cin.get( );
}

#endif // !ENTRYPOINT_CPP
