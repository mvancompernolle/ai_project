/* +---------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)               |
   |                          http://www.mrpt.org/                             |
   |                                                                           |
   | Copyright (c) 2005-2014, Individual contributors, see AUTHORS file        |
   | See: http://www.mrpt.org/Authors - All rights reserved.                   |
   | Released under BSD License. See details in http://www.mrpt.org/License    |
   +---------------------------------------------------------------------------+ */

#include "hwdrivers-precomp.h"   // Precompiled headers

#include <mrpt/system/os.h>
#include <mrpt/synch/CCriticalSection.h>
#include <mrpt/hwdrivers/CGPSInterface.h>

using namespace mrpt::hwdrivers;
using namespace mrpt::slam;
using namespace mrpt::system;
using namespace mrpt::synch;
using namespace std;

IMPLEMENTS_GENERIC_SENSOR(CGPSInterface,mrpt::hwdrivers)

/** Auxiliary function: Tokenize a string "str" into commas separated tokens */
void  getNextToken(const std::string &str,std::string &token, unsigned int &parserPos);

/* -----------------------------------------------------
                Constructor
   ----------------------------------------------------- */
CGPSInterface::CGPSInterface( int BUFFER_LENGTH, mrpt::hwdrivers::CSerialPort *outPort, mrpt::synch::CCriticalSection *csOutPort ) :
	m_COM					(),
	m_customInit			(),
	m_COMname				(),
	m_COMbauds				(4800),
	m_GPS_comsWork			(false),
	m_GPS_signalAcquired	(false),
	m_BUFFER_LENGTH			(BUFFER_LENGTH),
	m_buffer				( new char[m_BUFFER_LENGTH] ),
	m_bufferLength			(0),
	m_bufferWritePos		(0),

	m_JAVAD_rtk_src_port	(),
	m_JAVAD_rtk_src_baud	(0),
	m_JAVAD_rtk_format		("cmr"),
	m_useAIMMode            ( false ),
	m_last_timestamp        ( INVALID_TIMESTAMP ),
	m_AIMConfigured         ( false ),
	m_data_period           ( 0.2 ) // 20 Hz
{
	m_sensorLabel = "GPS";
	m_latestGPS_data.clear();
	m_last_UTC_time.hour = 0;
	m_last_UTC_time.minute = 0;
	m_last_UTC_time.sec = 0;

	m_out_COM       = outPort;
	m_cs_out_COM    = csOutPort;
}

/* -----------------------------------------------------
                loadConfig_sensorSpecific
   ----------------------------------------------------- */
void  CGPSInterface::loadConfig_sensorSpecific(
	const mrpt::utils::CConfigFileBase &configSource,
	const std::string	  &iniSection )
{
#ifdef MRPT_OS_WINDOWS
	m_COMname = configSource.read_string(iniSection, "COM_port_WIN", m_COMname, true );
#else
	m_COMname = configSource.read_string(iniSection, "COM_port_LIN", m_COMname, true );
#endif

	m_COMbauds		= configSource.read_int( iniSection, "baudRate",m_COMbauds, true );

	m_customInit	= configSource.read_string( iniSection, "customInit", m_customInit, false );

	m_sensorPose.x( configSource.read_float( iniSection, "pose_x",0, false ) );
	m_sensorPose.y( configSource.read_float( iniSection, "pose_y",0, false ) );
	m_sensorPose.z( configSource.read_float( iniSection, "pose_z",0, false ) );

	m_JAVAD_rtk_src_port = configSource.read_string(iniSection, "JAVAD_rtk_src_port",m_JAVAD_rtk_src_port );
	m_JAVAD_rtk_src_baud = configSource.read_int(iniSection, "JAVAD_rtk_src_baud",m_JAVAD_rtk_src_baud );
	m_JAVAD_rtk_format   = configSource.read_string(iniSection,"JAVAD_rtk_format", m_JAVAD_rtk_format );

    m_useAIMMode = configSource.read_bool( iniSection,"JAVAD_useAIMMode", m_useAIMMode );
    m_data_period = 1.0/configSource.read_double( iniSection,"outputRate", m_data_period );
}



/* -----------------------------------------------------
                Destructor
   ----------------------------------------------------- */
CGPSInterface::~CGPSInterface()
{
	delete[] m_buffer;
}

/* -----------------------------------------------------
				setSerialPortName
----------------------------------------------------- */
void  CGPSInterface::setSerialPortName(const std::string &COM_port)
{
    // MAR'11
    if( useExternCOM() )
    {
        bool res = false;
        {
            CCriticalSectionLocker lock( m_cs_out_COM );
            res = m_out_COM->isOpen();
        }
        if( res )
            THROW_EXCEPTION("Cannot change serial port name when it's already open")
	}
	else
        if (m_COM.isOpen())
            THROW_EXCEPTION("Cannot change serial port name when it's already open")

	m_COMname = COM_port;

//    if (m_COM.isOpen())
//        THROW_EXCEPTION("Cannot change serial port name when it's already open")
//
//	m_COMname = COM_port;

}

/* -----------------------------------------------------
				getSerialPortName
----------------------------------------------------- */
std::string CGPSInterface::getSerialPortName() const
{
	return m_COMname;
}

/* -----------------------------------------------------
				tryToOpenTheCOM
----------------------------------------------------- */
bool  CGPSInterface::tryToOpenTheCOM()
{
    if( useExternCOM() )
    {
        bool res = false;
        {
            CCriticalSectionLocker loc( m_cs_out_COM );
            res = m_out_COM->isOpen();
        }
        if( res )
            return true;	// Already open
    }
    else
        if (m_COM.isOpen())
            return true;	// Already open

    if (m_verbose) cout << "[CGPSInterface] Opening " << m_COMname << " @ " << m_COMbauds << endl;

	m_last_GGA.clear();  // On comms reset, empty this cache


	try
	{
        if( useExternCOM() )
        {
            CCriticalSectionLocker lock( m_cs_out_COM );
            m_out_COM->open(m_COMname);
            // Config:
            m_out_COM->setConfig( m_COMbauds, 0, 8, 1 );
            m_out_COM->setTimeouts( 1, 0, 1, 1, 1 );
        }
        else
        {
            m_COM.open(m_COMname);
            // Config:
            m_COM.setConfig( m_COMbauds, 0, 8, 1 );
            m_COM.setTimeouts( 1, 0, 1, 1, 1 );
        }

		m_latestGPS_data.clear();

		// Do extra initialization?
		if (! OnConnectionEstablished() )
		{
		    if( useExternCOM() )
		    {
		        CCriticalSectionLocker lock( m_cs_out_COM );
                m_out_COM->close();
            }
            else
                m_COM.close();
			return false;
		}

		return true; // All OK!
	}
	catch (std::exception &e)
	{
		std::cerr << "[CGPSInterface::tryToOpenTheCOM] Error opening or configuring the serial port:" << std::endl << e.what();
        if( useExternCOM() )
        {
            CCriticalSectionLocker lock( m_cs_out_COM );
            m_out_COM->close();
        }
        else
            m_COM.close();
		return false;
	}
	catch (...)
	{
		std::cerr << "[CGPSInterface::tryToOpenTheCOM] Error opening or configuring the serial port." << std::endl;
		if( useExternCOM() )
        {
            CCriticalSectionLocker lock( m_cs_out_COM );
            m_out_COM->close();
        }
        else
            m_COM.close();
		return false;
	}
}

/* -----------------------------------------------------
				isGPS_connected
----------------------------------------------------- */
bool  CGPSInterface::isGPS_connected()
{
	return m_GPS_comsWork;
}

/* -----------------------------------------------------
				isGPS_signalAcquired
----------------------------------------------------- */
bool  CGPSInterface::isGPS_signalAcquired()
{
	return m_GPS_signalAcquired;
}

/* -----------------------------------------------------
				doProcess
----------------------------------------------------- */
void  CGPSInterface::doProcess()
{
	// Is the COM open?
	if (!tryToOpenTheCOM())
	{
		m_state = ssError;
		THROW_EXCEPTION("Cannot open the serial port");
	}

	// Read as many bytes as available:
	size_t bytesRead = 1;
	while (bytesRead)
	{
		size_t bytesToRead = (m_BUFFER_LENGTH-10) - m_bufferWritePos;

		try
		{
		    if( useExternCOM() )
		    {
                CCriticalSectionLocker lock( m_cs_out_COM );
                bytesRead = m_out_COM->Read(m_buffer + m_bufferWritePos, bytesToRead);
            }
            else
                bytesRead = m_COM.Read(m_buffer + m_bufferWritePos, bytesToRead);
		}
		catch (...)
		{
			// ERROR:
			printf_debug("[CGPSInterface::doProcess] Error reading COM port: Closing communications\n");
		    if( useExternCOM() )
		    {
                CCriticalSectionLocker lock( m_cs_out_COM );
                m_out_COM->close();
		    }
		    else
                m_COM.close();

            m_GPS_comsWork			= false;
			m_GPS_signalAcquired	= false;
			return;
		}

		// Process the data:
		if (bytesRead)
		{
			m_bufferLength += bytesRead;
			processBuffer( );
		}
	};

	if (m_customInit.empty())
	{ // "Normal" GPS device
		// Write command to buffer:
		if ( m_latestGPS_data.has_GGA_datum ||
			 m_latestGPS_data.has_RMC_datum ||
			 m_latestGPS_data.has_PZS_datum ||
			 m_latestGPS_data.has_SATS_datum )
		{
			// Add observation to the output queue:
			CObservationGPSPtr newObs = CObservationGPSPtr( new  CObservationGPS( m_latestGPS_data ) );
			appendObservation( newObs );

			m_latestGPS_data.clear();
		}
	}
	else
	{	// "Advanced" (RTK,mmGPS) device
		// FAMD
		// Append observation if:
		// 0. the timestamp seems to be correct!
		// 1. it contains both synched GGA and RMC data
		// 2. it contains only GGA or RMC but the next one is not synched with it
		if( m_last_timestamp == INVALID_TIMESTAMP )
		{
			if (m_verbose) cout << "[CGPSInterface] Initial timestamp: " << mrpt::system::timeToString(m_latestGPS_data.timestamp) << endl;
			// Check if the initial timestamp seems to be OK (not a spurio one)
			TTimeStamp tmNow = mrpt::system::now();
			const double tdif = mrpt::system::timeDifference( m_latestGPS_data.timestamp, tmNow );
			if( tdif >= 0 && tdif < 7500 /*Up to two hours*/)
				m_last_timestamp = m_latestGPS_data.timestamp;
			else
				{  if (m_verbose) cout << "[CGPSInterface] Warning: The initial timestamp seems to be wrong! : " << tdif << endl;}
		} // end-if
		else
		{
			const double time_diff = mrpt::system::timeDifference( m_last_timestamp, m_latestGPS_data.timestamp );
			if( time_diff < 0 || time_diff > 300 )     // Assert that the current timestamp is after the previous one and not more than 5 minutes later -> remove spurious
				{ if (m_verbose) cout << "[CGPSInterface ] Bad timestamp difference" << endl; return; }

			if( time_diff-m_data_period > 0.25*m_data_period )
				{ if (m_verbose) cout << "[CGPSInterface] WARNING: According to the timestamps, we probably skipped one frame!" << endl; }

	//        TTimeStamp tnow = mrpt::system::now();
	//        const double now_diff = mrpt::system::timeDifference( m_latestGPS_data.timestamp,tnow );

			// a. These GPS data have both synched RMC and GGA data
			// don't append observation until we have both data
			if( m_latestGPS_data.has_GGA_datum && m_latestGPS_data.has_RMC_datum )
			{
				// Add observation to the output queue:
				CObservationGPSPtr newObs = CObservationGPSPtr( new  CObservationGPS( m_latestGPS_data ) );
				appendObservation( newObs );

				// Reset for the next frame
				m_latestGPS_data.clear();

				m_last_timestamp = m_latestGPS_data.timestamp;

	//            cout << "FAMD: [GPS_GPS____GR3]: " << int(m_latestGPS_data.GGA_datum.UTCTime.hour) << ":" << int(m_latestGPS_data.GGA_datum.UTCTime.minute) << ":" << double(m_latestGPS_data.GGA_datum.UTCTime.sec);
	//            cout /*<< " -> Lat:" << m_latestGPS_data.GGA_datum.latitude_degrees << ", Lon:" << m_latestGPS_data.GGA_datum.longitude_degrees << ", Hei:" << m_latestGPS_data.GGA_datum.altitude_meters*/ << endl;
			}

		} // end-else
	}
}

/* -----------------------------------------------------
					processBuffer
----------------------------------------------------- */
void  CGPSInterface::processBuffer()
{
	unsigned int	i=0, lineStart = 0;

	while (i<m_bufferLength)
	{
		// Loof for the first complete line of text:
		while (i<m_bufferLength && m_buffer[i]!='\r' && m_buffer[i]!='\n')
			i++;

		// End of buffer or end of line?
		if (i<m_bufferLength)
		{
			// It is the end of a line: Build the string:
			std::string		str;
			int				lenOfLine = i-1 - lineStart;
			if (lenOfLine>0)
			{
				str.resize( lenOfLine );
				memcpy( (void*)str.c_str(), m_buffer + lineStart, lenOfLine );

				// Process it!:
				processGPSstring(str);
			}

			// And this means the comms works!
			m_GPS_comsWork			= true;

            m_state = ssWorking;

			// Pass over this end of line:
			while (i<m_bufferLength && (m_buffer[i]=='\r' || m_buffer[i]=='\n'))
				i++;

			// And start a new line:
			lineStart = i;
		}

	};

	// Dejar en el buffer desde "lineStart" hasta "i-1", inclusive.
	size_t	newBufLen = m_bufferLength - lineStart;

	memcpy( m_buffer, m_buffer + lineStart, newBufLen);

	m_bufferLength = newBufLen;

	// Seguir escribiendo por aqui:
	m_bufferWritePos = i - lineStart;

}

/* -----------------------------------------------------
					processGPSstring
----------------------------------------------------- */
void  CGPSInterface::processGPSstring(const std::string &s)
{
	const bool did_have_gga = m_latestGPS_data.has_GGA_datum;
	// Parse:
	CGPSInterface::parse_NMEA(s,m_latestGPS_data, m_verbose);
	
	// Save GGA cache (useful for NTRIP,...)
	const bool has_gga = m_latestGPS_data.has_GGA_datum;
	if (has_gga && !did_have_gga) {
		m_last_GGA = s;
	}

	// Generic observation data:
	m_latestGPS_data.sensorPose     = m_sensorPose;
	m_latestGPS_data.sensorLabel    = m_sensorLabel;
}

/* -----------------------------------------------------
					parse_NMEA
----------------------------------------------------- */
bool CGPSInterface::parse_NMEA(const std::string &s, mrpt::slam::CObservationGPS &out_obs, const bool verbose)
{
    if (verbose)
		cout << "[CGPSInterface] GPS raw string: " << s << endl;

	bool parsed_ok = false;

	// Firstly! If the string does not start with "$GP" it is not valid:
	if (s.size()<7) return parsed_ok;

	if ( s[0]!='$' ||
         s[1]!='G' ) return parsed_ok;

	// Try to determine the kind of command:
	if ( s[3]=='G' &&
         s[4]=='G' &&
	     s[5]=='A' &&
	     s[6]==',')
	{
		// ---------------------------------------------
		//					GGA
		// ---------------------------------------------
		bool all_fields_ok=true;
		unsigned int	parserPos = 7;
		std::string		token;

		// Fill out the output structure:

		// Time:

		getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
		if (token.size()>=6)
		{
			out_obs.GGA_datum.UTCTime.hour		= 10 * (token[0]-'0') + token[1]-'0';
			out_obs.GGA_datum.UTCTime.minute	= 10 * (token[2]-'0') + token[3]-'0';
			out_obs.GGA_datum.UTCTime.sec		= atof( & (token.c_str()[4]) );
		}
		else all_fields_ok = false;

        // Check if there is already RMC datum within this observation.
        // If so, check if the UTC time is the same in both cases
        // If the times are different -> discard the previous RMC datum
        if( out_obs.has_RMC_datum )
        {
            if( out_obs.GGA_datum.UTCTime.hour != out_obs.RMC_datum.UTCTime.hour ||
                out_obs.GGA_datum.UTCTime.minute != out_obs.RMC_datum.UTCTime.minute ||
                out_obs.GGA_datum.UTCTime.sec != out_obs.RMC_datum.UTCTime.sec )
            {
                out_obs.has_RMC_datum = false;
            }
        } // end-if

		// Latitude:
		getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
		if (token.size()>=4)
		{
			double	lat = 10 * (token[0]-'0') + token[1]-'0';
			lat += atof( & (token.c_str()[2]) ) / 60.0;
			out_obs.GGA_datum.latitude_degrees = lat;
		}
		else all_fields_ok = false;

		// N/S:
		getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
		if (token.empty())
			all_fields_ok = false;
		else if (token[0]=='S')
			out_obs.GGA_datum.latitude_degrees = -out_obs.GGA_datum.latitude_degrees;

		// Longitude:
		getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
		if (token.size()>=5)
		{
			double	lat = 100 * (token[0]-'0') + 10 * (token[1]-'0')+ token[2]-'0';
			lat += atof( & (token.c_str()[3]) ) / 60.0;
			out_obs.GGA_datum.longitude_degrees = lat;
		}
		else all_fields_ok = false;

		// E_W:
		getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
		if (token.empty())
			all_fields_ok = false;
		else
		if (token[0]=='W')
			out_obs.GGA_datum.longitude_degrees = -out_obs.GGA_datum.longitude_degrees;

		// fix quality:
		getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
		if (!token.empty())
			out_obs.GGA_datum.fix_quality = (unsigned char)atoi(token.c_str());

		// sats:
		getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
		if (!token.empty())
			out_obs.GGA_datum.satellitesUsed = (unsigned char)atoi( token.c_str() );

		// HDOP:
		getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
		if (!token.empty())
		{
			out_obs.GGA_datum.HDOP = (float)atof( token.c_str() );
			out_obs.GGA_datum.thereis_HDOP = true;
		}

		// Altitude:
		getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
		if (token.empty()) all_fields_ok = false;
		else out_obs.GGA_datum.altitude_meters = atof( token.c_str() );

        // Units of the altitude:
        getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
//        ASSERT_(token == "M");

        // Geoidal separation [B]:
        getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
		if (!token.empty())
			out_obs.GGA_datum.geoidal_distance = atof( token.c_str() );

        // Units of the geoidal separation:
        getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
//        ASSERT_(token == "M");

        // Total altitude [A]+[B] and mmGPS Corrected total altitude Corr([A]+[B]):
        out_obs.GGA_datum.orthometric_altitude =
        out_obs.GGA_datum.corrected_orthometric_altitude =
        out_obs.GGA_datum.altitude_meters + out_obs.GGA_datum.geoidal_distance;

		out_obs.has_GGA_datum = all_fields_ok;
		parsed_ok = all_fields_ok;

        // Only set the timestamp if this data hasn't got it yet
        if( !out_obs.has_RMC_datum )
        {
#if 0
            out_obs.timestamp      = mrpt::system::now();
#else
            // FAMD: use satellite time as timestamp??
            TTimeParts parts;
            timestampToParts( mrpt::system::now(), parts );
            parts.hour      = out_obs.GGA_datum.UTCTime.hour;
            parts.minute    = out_obs.GGA_datum.UTCTime.minute;
            parts.second    = out_obs.GGA_datum.UTCTime.sec;

            out_obs.timestamp = buildTimestampFromParts(parts);
#endif
            // cout << "Timestamp [GGA]: " << timeToString(out_obs.timestamp) << endl;
        }
		// printf_debug("[GPS decoded GGA string] %s\n",s.c_str());
        // cout << "GGA STRING Decoded:" << endl;
        // cout << int(out_obs.GGA_datum.UTCTime.hour) << ":" << int(out_obs.GGA_datum.UTCTime.minute) << ":" << out_obs.GGA_datum.UTCTime.sec << endl;
	}
	else
	{
		// Try to determine the kind of command:
		if ( s[3]=='R' &&
             s[4]=='M' &&
		     s[5]=='C' &&
		     s[6]==',')
		{

			// ---------------------------------------------
			//					GPRMC
			//
			// ---------------------------------------------
			bool all_fields_ok = true;
			unsigned int	parserPos = 7;
			std::string		token;

			// Rellenar la estructura de "ultimo dato RMC recibido"

			// Time:
			getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
			if (token.size()>=6)
			{
				out_obs.RMC_datum.UTCTime.hour		= 10 * (token[0]-'0') + token[1]-'0';
				out_obs.RMC_datum.UTCTime.minute	= 10 * (token[2]-'0') + token[3]-'0';
				out_obs.RMC_datum.UTCTime.sec		= atof( & (token.c_str()[4]) );
			}
			else all_fields_ok = false;

			// Check if there is also GGA data within this observation. If so, check if the UTC time is the same in both cases
            if( out_obs.has_GGA_datum )
            {
                if( out_obs.GGA_datum.UTCTime.hour != out_obs.RMC_datum.UTCTime.hour ||
                    out_obs.GGA_datum.UTCTime.minute != out_obs.RMC_datum.UTCTime.minute ||
                    out_obs.GGA_datum.UTCTime.sec != out_obs.RMC_datum.UTCTime.sec )
                {
                    out_obs.has_GGA_datum = false;
                }
            } // end-if

			// Valid?
			getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
			if (token.empty()) all_fields_ok = false;
			else out_obs.RMC_datum.validity_char = token.c_str()[0];

			// Latitude:
			getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
			if (token.size()>=4)
			{
				double	lat = 10 * (token[0]-'0') + token[1]-'0';
				lat += atof( & (token.c_str()[2]) ) / 60.0;
				out_obs.RMC_datum.latitude_degrees = lat;
			}
			else all_fields_ok = false;

			// N/S:
			getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
			if (token.empty()) all_fields_ok = false;
			else if (token[0]=='S')
				out_obs.RMC_datum.latitude_degrees = -out_obs.RMC_datum.latitude_degrees;

			// Longitude:
			getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
			if (token.size()>=5)
			{
				double	lat = 100 * (token[0]-'0') + 10 * (token[1]-'0')+ token[2]-'0';
				lat += atof( & (token.c_str()[3]) ) / 60.0;
				out_obs.RMC_datum.longitude_degrees = lat;
			}
			else all_fields_ok = false;

			// E/W:
			getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
			if (token.empty()) all_fields_ok = false;
			else if (token[0]=='W')
				out_obs.RMC_datum.longitude_degrees = -out_obs.RMC_datum.longitude_degrees;

			// Speed:
			getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
			if (!token.empty()) out_obs.RMC_datum.speed_knots = atof( token.c_str() );

			// Direction:
			getNextToken(s,token,parserPos); //printf("TOKEN: %s\n",token.c_str());
			if (!token.empty()) out_obs.RMC_datum.direction_degrees= atof( token.c_str() );

			out_obs.has_RMC_datum = all_fields_ok;
			parsed_ok = all_fields_ok;

            // Only set the timestamp if this data hasn't got it yet
            if( !out_obs.has_GGA_datum )
            {
#if 0
                out_obs.timestamp      = mrpt::system::now();
#else
                // FAMD: use satellite time as timestamp??
                TTimeParts parts;
                timestampToParts( mrpt::system::now(), parts );
                parts.hour      = out_obs.RMC_datum.UTCTime.hour;
                parts.minute    = out_obs.RMC_datum.UTCTime.minute;
                parts.second    = out_obs.RMC_datum.UTCTime.sec;

                out_obs.timestamp = buildTimestampFromParts(parts);
#endif
                // cout << "Timestamp [RMC]: " << timeToString(out_obs.timestamp) << endl;
            }
			//printf_debug("[GPS decoded RMC string] %s\n",s.c_str());
//            cout << "RMC STRING Decoded:" << endl;
//            cout << int(out_obs.RMC_datum.UTCTime.hour) << ":" << int(out_obs.RMC_datum.UTCTime.minute) << ":" << out_obs.GGA_datum.UTCTime.sec << endl;
		}
		else
		{
			// ... parse other commands
		}
	}

	// Ensure that both GGA and RMC data have the same timestamp
	if( out_obs.has_RMC_datum && out_obs.has_GGA_datum )
    {
            TTimeParts parts;
            timestampToParts( now(), parts );

            parts.hour              = out_obs.RMC_datum.UTCTime.hour;
            parts.minute            = out_obs.RMC_datum.UTCTime.minute;
            parts.second            = out_obs.RMC_datum.UTCTime.sec;
            TTimeStamp thisRMCTS    = buildTimestampFromParts( parts );

            parts.hour              = out_obs.GGA_datum.UTCTime.hour;
            parts.minute            = out_obs.GGA_datum.UTCTime.minute;
            parts.second            = out_obs.GGA_datum.UTCTime.sec;
            TTimeStamp thisGGATS    = buildTimestampFromParts( parts );

            if( thisRMCTS != thisGGATS )
				{ if (verbose) cout << "[CGPSInterface::doProcess()] WARNING: UTC Times within the frame are different!" << endl;}
    } // end-if

    return parsed_ok;
}

/* -----------------------------------------------------
					processGPSstring
----------------------------------------------------- */
void  getNextToken(
    const std::string	&str,
    std::string			&token,
    unsigned int		&parserPos)
{
	unsigned int	start = parserPos;
	char			c;

	if (parserPos>=str.size())
	{
		token.resize(0);
		return;
	}

	do
	{
		c = str[parserPos++];
	} while (parserPos<str.size() && c!=',');

	unsigned int	tokenSize = parserPos - 1 - start;
	if (tokenSize<1)
	{
		token.resize(0);
	}
	else
	{
		token.resize( tokenSize );
		memcpy( (void*)token.c_str(), str.c_str()+start, tokenSize );
	}

}

/* -----------------------------------------------------
					JAVAD_sendMessage
----------------------------------------------------- */
void CGPSInterface::JAVAD_sendMessage(const char *str, bool waitForAnswer )
{
	if (!str) return;
	const size_t len = strlen(str);

    size_t written;

    if( useExternCOM() )
    {
        CCriticalSectionLocker lock( m_cs_out_COM );
        written = m_out_COM->Write(str,len);
    }
    else
        written = m_COM.Write(str,len);

	if (m_verbose)
		std::cout << "[CGPSInterface] TX: " << str;

	if (written != len )
		throw std::runtime_error(format("Error sending command: '%s'",str).c_str());
	mrpt::system::sleep(5);

	if (!waitForAnswer) return;

	mrpt::system::sleep(200);
	char buf[200];
	buf[0]='\0';

    int bad_counter = 0;
    while(bad_counter < 10)
    {
        size_t nRead;
        if( useExternCOM() )
        {
            CCriticalSectionLocker lock( m_cs_out_COM );
            nRead = m_out_COM->Read(buf,sizeof(buf));
        }
        else
            nRead = m_COM.Read(buf,sizeof(buf));

		if (m_verbose)
			std::cout << "[CGPSInterface] RX: " << buf << std::endl;

        if (nRead<3 )
            throw std::runtime_error(format("ERROR: Invalid response '%s' for command '%s'",buf,str));

        if (nRead>=3 && buf[0]=='R' && buf[1]=='E')
            return; // Ok!
        else
            ++bad_counter;
    }
    throw std::runtime_error(format("ERROR: Invalid response '%s' for command '%s'",buf,str));
}

/* -----------------------------------------------------
					OnConnectionEstablished
----------------------------------------------------- */
bool CGPSInterface::OnConnectionEstablished()
{
	if ( !os::_strcmpi( m_customInit.c_str(), "JAVAD" ) || !os::_strcmpi( m_customInit.c_str(), "TOPCON" ) )
	{
		// Stop messaging:
		JAVAD_sendMessage("%%dm\r\n", false);
		mrpt::system::sleep(500);
		JAVAD_sendMessage("%%dm\r\n",false);
		mrpt::system::sleep(1000);

		// Purge input:
		if( useExternCOM() )
		{
		    CCriticalSectionLocker lock( m_cs_out_COM );
		    m_out_COM->purgeBuffers();
		}
		else
            m_COM.purgeBuffers();

		// Configure RTK mode and source:
		if (m_verbose)
			cout << "[CGPSInterface] Configure RTK options" << endl;

		if (!m_JAVAD_rtk_src_port.empty())
		{
			const int elevation_mask = 5; // Degs

			JAVAD_sendMessage(format("%%%%set,/par/lock/elm,%i\r\n", elevation_mask).c_str());  // Set elevation mask to track satellites
			JAVAD_sendMessage("%%set,/par/base/mode/,off\r\n");  // Set Base Mode off
			JAVAD_sendMessage("%%set,/par/pos/pd/period,1.0\r\n"); // Differential Correction Interval
			//JAVAD_sendMessage("%%set,hd/mode,off\r\n");  // fixed distance to rtk base: Off
			//JAVAD_sendMessage("%%set,/par/pos/pd/hd/mode,off\r\n");  // fixed distance to rtk base: Off  <-- Not working with TopCon GR3! (option disabled)
			JAVAD_sendMessage("%%set,/par/pos/pd/qcheck,off\r\n"); // Set Quality Checks Off
			JAVAD_sendMessage("%%set,/par/pos/mode/cur,pd\r\n");  // Pos Mode Phase Diff
			JAVAD_sendMessage("%%set,/par/pos/pd/textr,10\r\n");  // RTK Extrapolation Limit
			JAVAD_sendMessage("%%set,/par/pos/pd/inuse,-1\r\n");  // Set Rovers Reference Station
			JAVAD_sendMessage("%%set,/par/pos/pd/nrs/mode,y\r\n");  // Enable Nearest Reference Station Mode
			JAVAD_sendMessage("%%set,/par/pos/pd/mode,extrap\r\n");// Enable EXTRAPOLATED mode in RTK corrections

			// Set Differential Correction Source
			JAVAD_sendMessage(format("%%%%set,/par/pos/pd/port,%s\r\n",m_JAVAD_rtk_src_port.c_str()).c_str());

			// Set port bauds:
			if( ! m_useAIMMode && m_JAVAD_rtk_src_baud!=0 && !mrpt::system::strCmp(m_JAVAD_rtk_src_port,"/dev/usb/a") )
				JAVAD_sendMessage(format("%%%%set,/par%s/rate,%u\r\n",m_JAVAD_rtk_src_port.c_str(), m_JAVAD_rtk_src_baud).c_str());

			// Set Input Mode: CMR,RTCM,...
			if( ! m_useAIMMode && !m_JAVAD_rtk_format.empty())
				JAVAD_sendMessage(format("%%%%set,/par%s/imode,%s\r\n", m_JAVAD_rtk_src_port.c_str(), m_JAVAD_rtk_format.c_str()).c_str());
		}

		// Start NMEA messaging:
//		JAVAD_sendMessage("%%em,,/msg/nmea/GGA:0.2\r\n");
//		JAVAD_sendMessage("%%em,,/msg/nmea/RMC:0.2\r\n");
		//JAVAD_sendMessage("%%em,,/msg/jps/PS:0.2\r\n");

		if( m_useAIMMode )
		{
		    if (m_verbose) cout << "[CGPSInterface] Using Advanced Input Mode";
            m_AIMConfigured = setJAVAD_AIM_mode();
            if (m_verbose) cout << "... done" << endl;
		}
		JAVAD_sendMessage(format("%%%%em,,/msg/nmea/GGA:%.1f\r\n", m_data_period ).c_str());
		JAVAD_sendMessage(format("%%%%em,,/msg/nmea/RMC:%.1f\r\n", m_data_period ).c_str());       // FAMD: 10 Hz

		if( m_useAIMMode )
            { if (m_verbose) cout << "[CGPSInterface::OnConnectionEstablished] JAVAD/TopCon commands sent successfully with AIM." << endl;}
		else
            { if (m_verbose) cout << "[CGPSInterface::OnConnectionEstablished] JAVAD/TopCon commands sent successfully." << endl;}

		return true;
	}
	else
	{
		return true; // Nothing extra
	}
}

bool CGPSInterface::unsetJAVAD_AIM_mode()
{
    MRPT_START
    if ( !os::_strcmpi( m_customInit.c_str(), "JAVAD" ) || !os::_strcmpi( m_customInit.c_str(), "TOPCON" ) )
	{
		// Stop messaging:
		JAVAD_sendMessage("%%dm\r\n", false);
		mrpt::system::sleep(500);
		JAVAD_sendMessage("%%dm\r\n",false);
		mrpt::system::sleep(1000);

		// Purge input:
		if( useExternCOM() )
		{
		    CCriticalSectionLocker lock( m_cs_out_COM );
		    m_out_COM->purgeBuffers();
		}
		else
            m_COM.purgeBuffers();

        JAVAD_sendMessage("%%set,/par/cur/term/imode,cmd\r\n");                // set the current port in command mode
        return true;
	}
	else
        return true;
    MRPT_END
} // end-unsetJAVAD_AIM_mode

bool CGPSInterface::setJAVAD_AIM_mode()
{
    MRPT_START
    if ( !os::_strcmpi( m_customInit.c_str(), "JAVAD" ) || !os::_strcmpi( m_customInit.c_str(), "TOPCON" ) )
	{
//		// Stop messaging:
//		JAVAD_sendMessage("%%dm\r\n", false);
//		mrpt::system::sleep(500);
//		JAVAD_sendMessage("%%dm\r\n",false);
//		mrpt::system::sleep(1000);
//
//		// Purge input:
//		if( useExternCOM() )
//		{
//		    CCriticalSectionLocker lock( m_cs_out_COM );
//		    m_out_COM->purgeBuffers();
//		}
//		else
//            m_COM.purgeBuffers();

        JAVAD_sendMessage(format("%%%%set,/par%s/imode,cmd\r\n",m_JAVAD_rtk_src_port.c_str()).c_str());  // set the port in command mode
        JAVAD_sendMessage("%%set,/par/cur/term/jps/0,{nscmd,37,n,\"\"}\r\n");               // any command starting with % will be treated as normal

        ASSERT_(!m_JAVAD_rtk_format.empty())
        cout << "Formato de correcciones para GR3: " << m_JAVAD_rtk_format << endl;
        if( m_JAVAD_rtk_format == "cmr" )
        {
            JAVAD_sendMessage(format("%%%%set,/par/cur/term/jps/1,{cmr,-1,y,%s}\r\n", m_JAVAD_rtk_src_port.c_str()).c_str());   // set corrections type CMR or CMR+
            JAVAD_sendMessage("%%set,/par/cur/term/jps/2,{none,-1,n,\"\"}\r\n");
            JAVAD_sendMessage(format("%%%%set,/par%s/imode,cmr\r\n", m_JAVAD_rtk_src_port.c_str()).c_str());
        }
        else if( m_JAVAD_rtk_format == "rtcm" )
        {
            JAVAD_sendMessage(format("%%%%set,/par/cur/term/jps/1,{rtcm,-1,y,%s}\r\n", m_JAVAD_rtk_src_port.c_str()).c_str());  // set corrections type RTCM
            JAVAD_sendMessage("%%set,/par/cur/term/jps/2,{none,-1,n,\"\"}\r\n");
            JAVAD_sendMessage(format("%%%%set,/par%s/imode,rtcm\r\n", m_JAVAD_rtk_src_port.c_str()).c_str());
        }
        else if( m_JAVAD_rtk_format == "rtcm3" )
        {
            JAVAD_sendMessage(format("%%%%set,/par/cur/term/jps/1,{rtcm3,-1,y,%s}\r\n", m_JAVAD_rtk_src_port.c_str()).c_str()); // set corrections type RTCM 3.x
            JAVAD_sendMessage("%%set,/par/cur/term/jps/2,{none,-1,n,\"\"}\r\n");
            JAVAD_sendMessage(format("%%%%set,/par%s/imode,rtcm3\r\n", m_JAVAD_rtk_src_port.c_str()).c_str());
        }
        else
        {
            cout << "Unknown RTK corrections format. Only supported: CMR, RTCM or RTCM3" << endl;
            return false;
        }
        JAVAD_sendMessage("%%set,/par/cur/term/imode,jps\r\n");                         // sets current port into "JPS" mode

        return true;

	} // end-if
	else
        return true;
    MRPT_END

} // end-setJAVAD_AIM_mode

std::string CGPSInterface::getLastGGA(bool reset)
{
	std::string ret = m_last_GGA;
	if (reset) m_last_GGA.clear();
	return ret;
}
