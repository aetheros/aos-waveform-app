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

void write_packet( std::ofstream& ofile, aos::waveform_provider::Packet const& packet );
void write_OnePhaseOneElement( std::ofstream& ofile, aos::waveform_provider::Packet const& packet );
void write_OnePhaseTwoElement( std::ofstream& ofile, aos::waveform_provider::Packet const& packet );
void write_ThreePhase( std::ofstream& ofile, aos::waveform_provider::Packet const& packet );

int main(int argc, char *argv[])
{
    aos::AppMain appMain;

	logInfo( "starting" );

	int socket = -1;
	uint16_t expected_sync = 0;
	uint16_t expected_sequence = 0;
	uint16_t write_count = 0;

	aos::waveform_provider::Packet packet;

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
			write_packet( ofile, packet );
			++write_count;
			if( write_count >= 10 )
			{
				logInfo( "wrote out samples" );
				ofile.close();
			}
		}
	}
}

void write_packet( std::ofstream& ofile, aos::waveform_provider::Packet const& packet )
{
	switch( packet.network )
	{
	case aos::waveform_provider::Network::OnePhaseOneElement:
		write_OnePhaseOneElement( ofile, packet );
		break;
	case aos::waveform_provider::Network::OnePhaseTwoElement:
		write_OnePhaseTwoElement( ofile, packet );
		break;
	case aos::waveform_provider::Network::ThreePhase:
		write_ThreePhase( ofile, packet );
		break;
	default:
		logError( "unexpected network configuration" );
		ofile.close();
		return;
	}
}

void write_OnePhaseOneElement( std::ofstream& ofile, aos::waveform_provider::Packet const& packet )
{
	auto const uscale = aos::waveform_provider::voltage_scale();
	auto const iscale = aos::waveform_provider::current_scale();

	for( int i=0; i<64; ++i )
	{
		auto u1 = uscale * packet.blocks[i].u1;
		auto i1 = iscale * packet.blocks[i].i1;
		ofile << "u1: " << u1 << ", i1: " << i1 << '\n';
	}
}

void write_OnePhaseTwoElement( std::ofstream& ofile, aos::waveform_provider::Packet const& packet )
{
	auto const uscale = aos::waveform_provider::voltage_scale();
	auto const iscale = aos::waveform_provider::current_scale();

	for( int i=0; i<64; ++i )
	{
		auto u2 = uscale * packet.blocks[i].u2;
		auto i2 = iscale * packet.blocks[i].i2;
		auto u3 = uscale * packet.blocks[i].u3;
		auto i3 = iscale * packet.blocks[i].i3;
		ofile << "u2: " << u2 << ", i2: " << i2 << ", u3: " << u3 << ", i3: " << i3 << '\n';
	}
}

void write_ThreePhase( std::ofstream& ofile, aos::waveform_provider::Packet const& packet )
{
	auto const uscale = aos::waveform_provider::voltage_scale();
	auto const iscale = aos::waveform_provider::current_scale();

	for( int i=0; i<64; ++i )
	{
		auto u1 = uscale * packet.blocks[i].u1;
		auto i1 = iscale * packet.blocks[i].i1;
		auto u2 = uscale * packet.blocks[i].u2;
		auto i2 = iscale * packet.blocks[i].i2;
		auto u3 = uscale * packet.blocks[i].u3;
		auto i3 = iscale * packet.blocks[i].i3;
		ofile << "u1: " << u1 << ", i1: " << i1 << ", u2: " << u2 << ", i2: " << i2 << ", u3: " << u3 << ", i3: " << i3 << '\n';
	}
}
