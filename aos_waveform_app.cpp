// vim: sw=4 expandtab
// Copyright (c) Aetheros, Inc.  See COPYRIGHT
#include <aos/AppMain.hpp>
#include <aos/Log.hpp>
#include <aos/WaveformProvider.hpp>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <thread>

using namespace std::chrono_literals;

int main(int argc, char *argv[])
{
    aos::AppMain appMain;

	logInfo( "starting" );

	auto socket = aos::waveform_provider::open_socket( 0 );
	if( socket == -1 )
	{
		logError( "could not open waveform provider: " << strerror( errno ) );
		exit( -1 );
	}

	aos::waveform_provider::Packet packet;

	std::ofstream ofile;

	uint16_t expected_sync = 0;
	uint16_t expected_sequence = 0;
	uint16_t write_count = 0;

	while( true )
	{
		ssize_t amount = ::read( socket, &packet, sizeof(packet) );
		if( amount <= 0 )
		{
			logError( "could not read waveform provider" );
			exit( 1 );
		}

		if( packet.sync != expected_sync or packet.sequence != expected_sequence )
		{
			logWarn( "unexpected packet: " << packet.sync << " " << packet.sequence
				<< ", expected: " << expected_sync << " " << expected_sequence );
			ofile.close();
			expected_sync = packet.sync;
			expected_sequence = packet.sequence;
		}

		++expected_sequence;

		if( not ofile )
		{
			ofile.open( "output", std::ios::binary | std::ios::trunc );
			if( not ofile )
			{
				logError( "could not open output file" );
				exit( 1 );
			}
			write_count = 0;
		}

		if( ofile and write_count < 10 )
		{
			for( int i=0; i<64; ++i )
			{
				ofile << aos::waveform_provider::voltage_scale() * packet.blocks[i].u1 << '\n';
			}
			++write_count;
			if( write_count >= 10 )
			{
				logInfo( "wrote out samples" );
				ofile.close();
			}
		}
	}
}
