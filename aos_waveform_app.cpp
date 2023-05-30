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

	int socket = -1;
	uint16_t expected_sync = 0;
	uint16_t expected_sequence = 0;
	uint16_t write_count = 0;

	aos::waveform_provider::Packet packet;
	auto network = aos::waveform_provider::Network::Unknown;

	bool start_new_file = true;
	std::ofstream ofile;

	while( true )
	{
		if( socket == -1 )
		{
			socket = aos::waveform_provider::open_socket( 0 );
			if( socket == -1 )
			{
				logError( "could not open waveform provider: " << strerror( errno ) );
				std::this_thread::sleep_for( std::chrono::seconds( 30 ) );
				continue;
			}
		}

		ssize_t amount = ::read( socket, &packet, sizeof(packet) );
		if( amount <= 0 )
		{
			::close( socket );
			socket = -1;
			logError( "could not read waveform provider" );
			std::this_thread::sleep_for( std::chrono::seconds( 30 ) );
			continue;
		}

		if( network == aos::waveform_provider::Network::Unknown )
		{
			network = packet.network;
			switch( network )
			{
			case aos::waveform_provider::Network::OnePhaseOneElement:
				logInfo( "u1 and i1 valid" );
				break;
			case aos::waveform_provider::Network::OnePhaseTwoElement:
				logInfo( "u2-u3 and i2-i3 valid" );
				break;
			case aos::waveform_provider::Network::ThreePhase:
				logInfo( "u1-u3 and i1-i3 valid" );
				break;
			default:
				logError( "unexpected network configuration" );
			}
		}

		if( packet.sync != expected_sync or packet.sequence != expected_sequence )
		{
			logWarn( "unexpected packet: " << packet.sync << " " << packet.sequence
				<< ", expected: " << expected_sync << " " << expected_sequence );
			expected_sync = packet.sync;
			expected_sequence = packet.sequence;
			start_new_file = true;
		}

		++expected_sequence;

		if( start_new_file )
		{
			start_new_file = false;
			write_count = 0;
			ofile.close();
			ofile.open( "output", std::ios::binary | std::ios::trunc );
			if( not ofile )
			{
				logError( "could not open output file" );
			}
		}

		if( ofile.is_open() and ofile and write_count < 10 )
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
