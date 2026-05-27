/// \file
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.


#include "RakNetStatistics.h"
#include <stdio.h> // sprintf
#include "BitStream.h" // BITS_TO_BYTES
#include "GetTime.h"
#include "RakString.h"

using namespace RakNet;

// Verbosity level currently supports 0 (low), 1 (medium), 2 (high)
// Buffer must be hold enough to hold the output string.  See the source to get an idea of how many bytes will be output
void RAK_DLL_EXPORT StatisticsToString( RakNetStatistics *s, char *buffer, int verbosityLevel )
{
	if ( s == 0 )
	{
		sprintf( buffer, "stats is a NULL pointer in statsToString\n" );
		return ;
	}

	if (verbosityLevel==0)
	{
		sprintf(buffer,
			"Bytes per second sent     %I64u\n"
			"Bytes per second received %I64u\n"
			"Current packetloss        %.0f%%\n",
			s->valueOverLastSecond[ACTUAL_BYTES_SENT],
			s->valueOverLastSecond[ACTUAL_BYTES_RECEIVED],
			s->packetlossLastSecond
			);
	}
	else if (verbosityLevel==1)
	{
		sprintf(buffer,
			"Actual bytes per second sent       %I64u\n"
			"Actual bytes per second received   %I64u\n"
			"Message bytes per second queued    %I64u\n"
			"Total actual bytes sent            %I64u\n"
			"Total actual bytes received        %I64u\n"
			"Total message bytes queued         %I64u\n"
			"Current packetloss                 %.0f%%\n"
			"Average packetloss                 %.0f%%\n"
			"Elapsed connection time in seconds %I64u\n",
			s->valueOverLastSecond[ACTUAL_BYTES_SENT],
			s->valueOverLastSecond[ACTUAL_BYTES_RECEIVED],
			s->valueOverLastSecond[USER_MESSAGE_BYTES_PUSHED],
			s->runningTotal[ACTUAL_BYTES_SENT],
			s->runningTotal[ACTUAL_BYTES_RECEIVED],
			s->runningTotal[USER_MESSAGE_BYTES_PUSHED],
			s->packetlossLastSecond,
			s->packetlossTotal,
			(uint64_t)((RakNet::GetTimeUS()-s->connectionStartTime)/1000000)
			);

		if (s->BPSLimitByCongestionControl!=0)
		{
			char buff2[128];
			sprintf(buff2,
				"Send capacity                    %I64u (%.0f%%)\n",
				s->BPSLimitByCongestionControl,
				100.0f * s->valueOverLastSecond[ACTUAL_BYTES_SENT] / s->BPSLimitByCongestionControl
				);
			strcat(buffer,buff2);
		}
		if (s->BPSLimitByOutgoingBandwidthLimit!=0)
		{
			char buff2[128];
			sprintf(buff2,
				"Send limit                       %I64u (%.0f%%)\n",
				s->BPSLimitByOutgoingBandwidthLimit,
				100.0f * s->valueOverLastSecond[ACTUAL_BYTES_SENT] / s->BPSLimitByOutgoingBandwidthLimit
				);
			strcat(buffer,buff2);
		}
	}	
	else
	{
		sprintf(buffer,
			"Actual bytes per second sent         %I64u\n"
			"Actual bytes per second received     %I64u\n"
			"Message bytes per second sent        %I64u\n"
			"Message bytes per second resent      %I64u\n"
			"Message bytes per second queued      %I64u\n"
			"Message bytes per second processed   %I64u\n"
			"Message bytes per second ignored     %I64u\n"
			"Total bytes sent                     %I64u\n"
			"Total bytes received                 %I64u\n"
			"Total message bytes sent             %I64u\n"
			"Total message bytes resent           %I64u\n"
			"Total message bytes queued           %I64u\n"
			"Total message bytes received         %I64u\n"
			"Total message bytes ignored          %I64u\n"
			"Messages in send buffer, by priority %i,%i,%i,%i\n"
			"Bytes in send buffer, by priority    %i,%i,%i,%i\n"
			"Messages in resend buffer            %i\n"
			"Bytes in resend buffer               %I64u\n"
			"Current packetloss                   %.0f%%\n"
			"Average packetloss                   %.0f%%\n"
			"Elapsed connection time in seconds   %I64u\n",
			s->valueOverLastSecond[ACTUAL_BYTES_SENT],
			s->valueOverLastSecond[ACTUAL_BYTES_RECEIVED],
			s->valueOverLastSecond[USER_MESSAGE_BYTES_SENT],
			s->valueOverLastSecond[USER_MESSAGE_BYTES_RESENT],
			s->valueOverLastSecond[USER_MESSAGE_BYTES_PUSHED],
			s->valueOverLastSecond[USER_MESSAGE_BYTES_RECEIVED_PROCESSED],
			s->valueOverLastSecond[USER_MESSAGE_BYTES_RECEIVED_IGNORED],
			s->runningTotal[ACTUAL_BYTES_SENT],
			s->runningTotal[ACTUAL_BYTES_RECEIVED],
			s->runningTotal[USER_MESSAGE_BYTES_SENT],
			s->runningTotal[USER_MESSAGE_BYTES_RESENT],
			s->runningTotal[USER_MESSAGE_BYTES_PUSHED],
			s->runningTotal[USER_MESSAGE_BYTES_RECEIVED_PROCESSED],
			s->runningTotal[USER_MESSAGE_BYTES_RECEIVED_IGNORED],
			s->messageInSendBuffer[IMMEDIATE_PRIORITY],s->messageInSendBuffer[HIGH_PRIORITY],s->messageInSendBuffer[MEDIUM_PRIORITY],s->messageInSendBuffer[LOW_PRIORITY],
			(unsigned int) s->bytesInSendBuffer[IMMEDIATE_PRIORITY],(unsigned int) s->bytesInSendBuffer[HIGH_PRIORITY],(unsigned int) s->bytesInSendBuffer[MEDIUM_PRIORITY],(unsigned int) s->bytesInSendBuffer[LOW_PRIORITY],
			s->messagesInResendBuffer,
			s->bytesInResendBuffer,
			s->packetlossLastSecond,
			s->packetlossTotal,
			(uint64_t)((RakNet::GetTimeUS()-s->connectionStartTime)/1000000)
			);

		if (s->BPSLimitByCongestionControl!=0)
		{
			char buff2[128];
			sprintf(buff2,
				"Send capacity                    %I64u (%.0f%%)\n",
				s->BPSLimitByCongestionControl,
				100.0f * s->valueOverLastSecond[ACTUAL_BYTES_SENT] / s->BPSLimitByCongestionControl
				);
			strcat(buffer,buff2);
		}
		if (s->BPSLimitByOutgoingBandwidthLimit!=0)
		{
			char buff2[128];
			sprintf(buff2,
				"Send limit                       %I64u (%.0f%%)\n",
				s->BPSLimitByOutgoingBandwidthLimit,
				100.0f * s->valueOverLastSecond[ACTUAL_BYTES_SENT] / s->BPSLimitByOutgoingBandwidthLimit
				);
			strcat(buffer,buff2);
		}
	}
}
