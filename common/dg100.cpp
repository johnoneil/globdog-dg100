#include "dg100.h"
#include <stdlib.h>

namespace DG100
{

	namespace MSG
	{ 

		int GetIntegerValue(const QByteArray& d,int start)
		{
			if(start+3 >=d.size())
			{
				return -1;
			}
			int value = (d[start] & 0x000000ff)<<24;
			value |= (d[start+1] & 0x000000ff)<<16;
			value |= (d[start+2] & 0x000000ff)<<8;
			value |= (d[start+3] & 0x000000ff);
			return value;
		};
		
		void SetIntegerValue(QByteArray& d, int start,int value)
		{
			if(start+3 >= d.size())
			{
				return;
			}
			d[start] = (char)((value & 0xff000000)>>24);
			d[start+1]=(char)((value & 0x00ff0000)>>16);
			d[start+2]=(char)((value & 0x0000ff00)>>8);
			d[start+3]=(char)(value & 0x000000ff);
		};
		
		short GetShortValue(const QByteArray& d,int start)
		{
			if(start+1 >=d.size())
			{
				return -1;
			}
			short value = (d[start] & 0x000000ff)<<8;
			value |= (d[start+1] & 0x000000ff);
			return value;
		};
		
		void SetShortValue(QByteArray&d,int start,short value)
		{
			if(start+1 >= d.size())
			{
				return;
			}
			d[start] = (char)((value & 0x0000ff00)>>8);
			d[start+1]=(char)(value & 0x000000ff);
		};
		
		char GetCharValue(const QByteArray& d, int pos)
		{
			return d[pos];
		};
		
		bool GetBooleanValue(const QByteArray& d,int pos)
		{
			if(d[pos]==0)
			{
				return false;
			}else{
				return true;
			}
		};
		
		void SetBooleanValue(QByteArray& d, int pos, bool value)
		{
			if(d.size()<=pos)
			{
				return;
			}
			if(value == true)
			{
				d[pos]=(char)0x1;
			}else{
				d[pos]=(char)0x0;
			}
		};
		
		char GetCommandType(const QByteArray& full_msg)
		{
			if(full_msg.size()<7)
			{
				return 0;
			}else{
				return full_msg[4];
			}
		};		

		short int GetPayloadSize(const QByteArray& array)
		{
			if( array.size() < 9)
			{
				return -1;
			}
			short int lw = array[3];
			short int uw = array[2];
			short int return_value = ( ( uw <<16 ) | (lw & 0x00ff) );
			if(return_value<1)
			{
				return -1;
			}else{
				return return_value;	
			}
		};
		
		long long GetIdentity(const QByteArray& full_msg)
		{
			if(GetCommandType(full_msg)!= 0xbf
				|| GetPayloadSize(full_msg)!= 8 )
				{
					return -1;
				}
			return 0;
		};
		
		 bool GetPayload(const QByteArray& full_msg,QByteArray& payload)
		{
			//TODO: check formedness of msg.
			short int payload_size = DG100::MSG::GetPayloadSize(full_msg);
			if(payload_size<1)
			{
				return false;
			}
			payload.reserve((int)payload_size);
			for(int i =0;i<payload_size;i++)
			{
				payload[i]=full_msg[i+4];
			}
			return true;
		};
		
		short CalculateChecksum(const QByteArray& d, int payload_start,int payload_size)
		{
			unsigned short sum = 0;
			for(int i = payload_start;i< (payload_start+ payload_size);i++)
			{
				if(i>= d.size())
				{
					return 0;
				}
				sum += (unsigned short)d[i]&0x00ff;
			}
			sum &= 0x7fff;
			
			return (short)sum;
		};
		
		void FormHeaderRequest(QByteArray& d, short int header_num)
		{
			d.clear();
			d = QByteArray(header_request,11);
			SetShortValue(d,5,header_num);
			SetShortValue(d,7,CalculateChecksum(d,4,3));
		};
		
		void FormTrackfileRequest(QByteArray& d, short int header_num)
		{
			d.clear();
			d = QByteArray(trackfile_request,sizeof(trackfile_request));
			SetShortValue(d,5,header_num);
			SetShortValue(d,7,CalculateChecksum(d,4,3));
		};

		bool CheckFormedness(const QByteArray& msg)
		{
			if(msg.size()<9)
			{
				return false;
			}
			if(msg[0]!= (char)0xa0 ||
				msg[1]!=(char)0xa2 )
			{
				return false;
			}
			/*int payload_size = (int)GetPayloadSize(msg);
			if(payload_size<0 || payload_size>1024)
			{
				return false;
			}*/
			/*if(payload_size+8 != msg.size())
			{
				return false;
			}*/
			/*if(msg[4+payload_size+2]!=(char)0xb0
				|| msg[4+payload_size+3]!=(char)0xb3)
			{
				return false;
			}*/
			//TODO: VALIDATE THE CHECKSUM
			return true; 
			
			
		}
		QString& Time2String(int time, QString& stime)
		{
			stime.sprintf("%6d",time);
			stime.insert(2,":");
			stime.insert(5,":");
			return stime;
		}
		QString& Date2String(int date, QString& sdate)
		{
			sdate.sprintf("%6d",date);
			sdate.insert(2,"/");
			sdate.insert(5,"/");
			return sdate;
		}
		
				
		float Lat2Float(int lat)
		{
			float value = 0.0f;
			char buffer[32];
			sprintf(buffer,"%d",lat);
			std::string slat(buffer);
			std::string min = slat.substr(slat.size()-6);
			min.insert(2,".");
			std::string deg = slat.substr(0,slat.size()-6);
			float degf = (float)atof(deg.c_str());
			if(degf < 0)
			{
				value = degf - (float)(atof(min.c_str())/60.0f);
			}else{
				value =  degf + (float)(atof(min.c_str())/60.0f);
			}
			
			return value;
		};
		float Lon2Float(int lon)
		{
			float value = 0.0f;
			char buffer[32];
			sprintf(buffer,"%d",lon);
			std::string slon(buffer);
			std::string min = slon.substr(slon.size()-6);
			min.insert(2,".");
			std::string deg = slon.substr(0,slon.size()-6);
			float degf = (float)atof(deg.c_str());
			if(degf < 0)
			{
				value =  degf - (float)(atof(min.c_str())/60.0f);
			}else{
				value =  degf + (float)(atof(min.c_str())/60.0f);
			}
			
			return value;
		};

	};
	
	
	Measurement::Measurement(){};
	Measurement::Measurement( const QByteArray& d, int start){};
	Measurement::~Measurement(){};
	int Measurement::GetLatitude(void)
	{
		return 0;
	};
	int Measurement::GetLongitude(void)
	{
		return 0;
	};
	int Measurement::GetTime(void)
	{
		return 0;
	};
	int Measurement::GetDate(void)
	{
		return 0;
	};
	int Measurement::GetSpeed(void)
	{
		return 0;
	};
	int Measurement::GetAltitude(void)
	{
		return 0;
	};
	DG100::Measurement::Style Measurement::GetStyle(void)
	{
		return DG100::Measurement::INVALID;
	};
	
	void  Measurement::WriteGPX(std::ofstream& outfile)
	{
		//do nothing.
	};

	MeasurementA::MeasurementA( const QByteArray& d, int start)
	{
		if(d.size() <= start+8)
		{
			return;
		}
		m_Lat = DG100::MSG::GetIntegerValue(d,start);
		m_Lon = DG100::MSG::GetIntegerValue(d,start+4);
	};
	MeasurementA::MeasurementA(){};
	MeasurementA:: ~MeasurementA(void){};
	int MeasurementA::GetLatitude(void)
	{
		return m_Lat;
	};
	int MeasurementA::GetLongitude(void)
	{
		return m_Lon;
	};
	DG100::Measurement::Style MeasurementA:: GetStyle(void)
	{
		return DG100::Measurement::A;
	};
	
	void  MeasurementA::WriteGPX(std::ofstream& outfile)
	{
		QString plat,plon;
		plat.sprintf("%6d",this->m_Lat);
		plon.sprintf("%7d",this->m_Lon);
		
		std::string slat(plat.toAscii());
		std::string slon(plon.toAscii());
		
		//<trkpt lat="45.4431641" lon="-121.7295456"></trkpt>
		outfile.precision(14);
		outfile<<"\t\t\t\t<trkpt lat=\""<<DG100::MSG::Lat2Float(m_Lat)<<"\" lon=\""<<DG100::MSG::Lon2Float(m_Lon)<<"\"></trkpt>"<<std::endl;
	};
	
	MeasurementB::MeasurementB( const QByteArray& d, int start)
		:MeasurementA(d,start)
	{
		if( d.size() <= start+20)
		{
			return;
		}
		m_Time = DG100::MSG::GetIntegerValue(d,start+8);
		m_Date = DG100::MSG::GetIntegerValue(d,start+12);
		m_Speed = DG100::MSG::GetIntegerValue(d,start+15);
	};
	MeasurementB::MeasurementB()
	{
		
	};
	MeasurementB::~MeasurementB(void){};
	int MeasurementB::GetTime(void)
	{
		return m_Time;
	};
	int MeasurementB::GetDate(void)
	{
		return m_Date;
	}
	int MeasurementB::GetSpeed(void)
	{
		return m_Speed;
	};
	DG100::Measurement::Style MeasurementB::GetStyle(void)
	{
		return DG100::Measurement::B;
	};

	MeasurementC::MeasurementC( const QByteArray& d, int start)
		:MeasurementB(d,start)
	{
		if(d.size() <= start+ 32)
		{
			return;
		}
		m_Altitude = DG100::MSG::GetIntegerValue(d,start+20);
		m_Style = (DG100::Measurement::Style)DG100::MSG::GetIntegerValue(d,start+28);
		if( m_Style != DG100::Measurement::A &&
			m_Style != DG100::Measurement::B &&
			m_Style != DG100::Measurement::C )
		{
			m_Style = DG100::Measurement::INVALID;
		}
	};
	
	MeasurementC::~MeasurementC(void){};
	int MeasurementC::GetAltitude(void)
	{
		return m_Altitude;
	};
	DG100::Measurement::Style MeasurementC::GetStyle(void)
	{
		return m_Style;
	};
	
	TrackFileHeader::TrackFileHeader(int header_num,int time,int date)
		:m_HeaderNum(header_num)
		,m_Time(time)
		,m_Date(date)
		,m_Included(true)
	{
		
	};
	TrackFileHeader::~TrackFileHeader()
	{
		std::vector< DG100::Measurement* >::iterator i;
		for(i=m_Measurements.begin();i!=m_Measurements.end();i++)
		{
			delete *i;
		}
		m_Measurements.clear();
	};
		
	int TrackFileHeader::HeaderNum(void)
	{
		return this->m_HeaderNum;
	};
	int TrackFileHeader::GetNumMeasurements(void)
	{
		return m_Measurements.size();
	};
	int TrackFileHeader::GetTime(void)
	{
		return m_Time;
	};
	int TrackFileHeader::GetDate(void)
	{
		return m_Date;
	};
	DG100::Measurement* TrackFileHeader::Measurement(int n)
	{
		if (n < 0 || n >= (int) this->m_Measurements.size() )
		{
			return NULL;
		}
		return m_Measurements[n];
	};
	int TrackFileHeader::Load(const QByteArray& msg)
	{
		int size = msg.size();
		if( size < 2048 )
		{
			return 0;
		}
		//First measurement at pos 0. It's always type "C" (32 bytes)
		DG100::Measurement* first_measurement = new DG100::MeasurementC(msg,0);
		if( first_measurement->GetStyle() != DG100::Measurement::INVALID)
		{
			m_Measurements.push_back(first_measurement);
		}else{
			return 0;
		}
		
		int i = 0+32;
		DG100::Measurement* next_measurement = NULL;
		while( i < 2048 )
		{
			switch( first_measurement->GetStyle())
			{
				case DG100::Measurement::A:
					next_measurement = new DG100::MeasurementA(msg,i);
					i+=8;
					break;
				case DG100::Measurement::B:
					next_measurement = new DG100::MeasurementB(msg,i);
					i+=20;
					break;
				case DG100::Measurement::C:
					next_measurement = new DG100::MeasurementC(msg,i);
					i+=32;
					break;
				default:
					return false;
					break;
			}
			
			if( next_measurement->GetStyle() != DG100::Measurement::INVALID)
			{
				m_Measurements.push_back(next_measurement);
			}else{
				return m_Measurements.size();
			}
		}
		
		return m_Measurements.size();
	};
	
	void TrackFileHeader::WriteGPX(std::ofstream& outfile)
	{
		std::vector< DG100::Measurement* >::iterator i;
		QString time,date;
		DG100::MSG::Time2String(this->m_Time,time);
		DG100::MSG::Date2String(this->m_Date,date);
		
		std::string stime(time.toAscii());
		std::string sdate(date.toAscii());
		
		outfile<<"\t<trk>"<<std::endl;
		outfile<<"\t\t<name> "<<sdate<<" "<<stime<<" </name>"<<std::endl;
		outfile<<"\t\t\t<trkseg>"<<std::endl;
		
		for( i = m_Measurements.begin();i!=m_Measurements.end();++i)
		{
			(*i)->WriteGPX(outfile);
			//m_Measurements[i]->WriteGPX(outfile);
		}
				
		outfile<<"\t\t\t</trkseg>"<<std::endl;
		outfile<<"\t</trk>"<<std::endl;
	}
	
	void TrackFileHeader::FormGoogleMapsQuery( QString& query )
	{
		//Google maps doesn't allow queries larger than 2048 chars.
		if( query.size() >1900 )
		{
			return;
		}
		std::vector< DG100::Measurement* >::iterator i;
		for( i = m_Measurements.begin();i!=m_Measurements.end();++i)
		{
			QString loc;
			loc.sprintf("|%0.7f,%0.7f",DG100::MSG::Lat2Float((*i)->GetLatitude()),
								DG100::MSG::Lon2Float((*i)->GetLongitude()));
			query+=loc;
			if(  query.size() >1900 )
			{
				return;
		 	}
		}
		
	}
	void TrackFileHeader::FormGoogleMapsQueryEncodedPolyline( QString& query)
	{
		//Google maps doesn't allow queries larger than 2048 chars.
		//We're also limiting the number of trackpoints written per file
		//to 64.
		if( query.size() >1900 )
		{
			return;
		}
		int trackpoints_written = 0;
		std::vector< DG100::Measurement* >::iterator i;
		for( i = m_Measurements.begin();i!=m_Measurements.end();++i)
		{
			QString loc;
			loc.sprintf("|%0.7f,%0.7f",DG100::MSG::Lat2Float((*i)->GetLatitude()),
								DG100::MSG::Lon2Float((*i)->GetLongitude()));
			query+=loc;
			trackpoints_written+=1;
			if(  query.size() >1900 || trackpoints_written >=64)
			{
				return;
		 	}
		}
	}
			
		void TrackFileHeader::GetBounds(float& min_lat,float& max_lat,
									float& min_lon, float& max_lon)
		{
			std::vector< DG100::Measurement* >::iterator i;
			if(m_Measurements.size() < 1)
			{
				min_lat=max_lat=min_lon=max_lon=0.0f;
				return;
			}
			bool first = true;
			float lat,lon;
			for( i = m_Measurements.begin();i!=m_Measurements.end();++i)
			{
				lat = DG100::MSG::Lat2Float((*i)->GetLatitude());
				lon = DG100::MSG::Lon2Float((*i)->GetLongitude());
				if(first)
				{
					first = false;
					min_lat = lat;
					max_lat = lat;
					min_lon = lon;
					max_lon = lon;
				}else{
					if(lat < min_lat)min_lat = lat;
					if(lat > max_lat)max_lat = lat;
					if(lon < min_lon)min_lon = lon;
					if(lon > max_lon)max_lon = lon;
				}
			}
		}
		
		bool TrackFileHeader::IsIncluded(void)
		{
			return m_Included;
		}
		bool TrackFileHeader::SetIncluded(bool value)
		{
			m_Included = value;
			return m_Included;
		}
	
	void TrackFileDatabase::Clear(void)
	{
		std::map< int ,DG100::TrackFileHeader* >::iterator i;
		for(i = m_Headers.begin();i!=m_Headers.end();++i)
		{
			delete i->second;
		}
		m_Headers.clear();
	}
	TrackFileDatabase::TrackFileDatabase()
		:m_NextHeaderNum(0)
		,m_NumHeaders(0)
	{
		
	};
	TrackFileDatabase::~TrackFileDatabase()
	{
		Clear();
	};
	
	void TrackFileDatabase::FlushMsgBuffer(void)
	{
		m_MsgBuffer.clear();
	};
	short TrackFileDatabase::PushHeaderMsg( const QByteArray& msg)
	{
		if( msg.size() < 7)
		{
			return -1;
		}
		short num_headers = DG100::MSG::GetShortValue(msg,4+1);
		short next_track_index =DG100::MSG::GetShortValue(msg,4+3);
	
		if(num_headers == next_track_index && next_track_index != 0)
		{
			//First track header to arrive. We can clean up the database
			this->Clear();
		}
	
		for(int n = 0; n <(int)num_headers;n++)
		{
			int time =  DG100::MSG::GetIntegerValue(msg,4+1+4+n*12);
			int date = DG100::MSG::GetIntegerValue(msg,4+1+4+4+n*12);
			TrackFileHeader* new_header = new TrackFileHeader(n,time,date);
			
			m_Headers[n]= new_header;
		}
		if( next_track_index > 0 )
		{
			return next_track_index;
		}else{
			return -1;
		}
	}
	bool TrackFileDatabase::PushTrackfileMsg(const QByteArray& msg)
	{
		QByteArray array = msg;
		array.chop(8);
		array.remove(0,5);
		
		m_MsgBuffer+=array;
		if(m_MsgBuffer.size() == (1024*2) )
		{
			//We've pushed two messages
			//so we can load a new header
			std::map< int, TrackFileHeader* >::iterator header;
			header = m_Headers.find(m_NextHeaderNum);
			if( header != m_Headers.end())
			{
				header->second->Load(m_MsgBuffer);
			}
			//Finished processing, so clear the buffer
			FlushMsgBuffer();
			
			return true;
		}
		return false;
	};
	short TrackFileDatabase::GetNextHeaderNum( NextHeaderFromStart header)
	{
		if( header == Next)
		{
			m_NextHeaderNum++;
			
		}else if( header == FromStart)
		{
			m_NextHeaderNum = 0;
			
		}
		if( m_Headers.find(m_NextHeaderNum)==m_Headers.end())
		{
			m_NextHeaderNum = 0;
			return -1;
		}else{
			return m_NextHeaderNum;
		}
	}

	int TrackFileDatabase::GetNumHeaders(void)
	{
		return m_Headers.size();
	};
	DG100::TrackFileHeader* TrackFileDatabase::GetHeaderNum(int n)
	{
		if(m_Headers.find(n) == m_Headers.end())
		{
			return NULL;
		}
		return m_Headers[n];
	};
	
	void TrackFileDatabase::WriteGPX(std::ofstream& outfile)
	{
		std::map< int, DG100::TrackFileHeader* >::iterator i;
		outfile<<"<gpx creator=\"GlobDoG-DG100\" version=\"0.5\">"<<std::endl;
		for(i = m_Headers.begin();i!=m_Headers.end();++i)
		{
			if(i->second->IsIncluded())
			{
				i->second->WriteGPX(outfile);
			}
			
		}
		outfile<<"</gpx>"<<std::endl;
	}
	
	void TrackFileDatabase::GetBounds(float& min_lat,float& max_lat,
						float& min_lon, float& max_lon)
	{
		std::map< int, DG100::TrackFileHeader* >::iterator i;
		if(m_Headers.size() < 1)
		{
			min_lat = max_lat = min_lon = max_lon = 0.0f;
			return;
		}
		
		bool first = true;
		float loc_min_lat,loc_max_lat,loc_min_lon,loc_max_lon;
		for(i = m_Headers.begin();i!=m_Headers.end();++i)
		{
			if(!i->second->IsIncluded())
			{
				continue;
			}
			i->second->GetBounds(loc_min_lat,loc_max_lat,loc_min_lon,loc_max_lon);
			if(first)
			{
				first = false;
				min_lat = loc_min_lat;
				max_lat = loc_max_lat;
				min_lon = loc_min_lon;
				max_lon = loc_max_lon;
			}else{
				if(loc_min_lat < min_lat)min_lat=loc_min_lat;
				if(loc_max_lat > max_lat)max_lat=loc_max_lat;
				if(loc_min_lon < min_lon)min_lon = loc_min_lon;
				if(loc_max_lon > max_lon)max_lon= loc_max_lon;
			}
		}
	}
	void TrackFileDatabase::GetBounds(int track,float& min_lat,float& max_lat,
						float& min_lon, float& max_lon)
	{
		if(track<0)
		{
			GetBounds(min_lat,max_lat,min_lon,max_lon);
			return;
		}
		std::map< int, DG100::TrackFileHeader* >::iterator i;
		i = m_Headers.find(track);
		if(i == m_Headers.end())
		{
			min_lat = max_lat = min_lon = max_lon = 0.0f;
			return;
		}
		
		i->second->GetBounds(min_lat,max_lat,min_lon,max_lon);
	}
	bool TrackFileDatabase::FormGoogleMapsQuery(QString& query,int width,int height,
												int color,int weight)
	{

		QString value;
		query.clear();
		query+= "http://maps.google.com/maps/api/staticmap?size=";
		value.sprintf("%dx%d",width,height);
		query+=value;
		
		if(this->m_Headers.size() > 0)
		{
			query+="&path=color:0xff0000ff|weight:5";
		
			std::map< int, DG100::TrackFileHeader* >::iterator i;
			for(i = m_Headers.begin();i!=m_Headers.end();++i)
			{
				if(i->second->IsIncluded())
				{
					i->second->FormGoogleMapsQuery(query);
					//For now we only show the first selected file
					//in google maps.
					break;
				}
			
			}
		}
		
		
		query+="&sensor=false\
&key=ABQIAAAAgFUqtzVrDEQKSBZOEIqBkRSNZEPBPLD8coecyb9PD94IKYduaBTMXaQLOAcKmtr3epEPRVf7FhpMDg";
		return true;
	}
	bool TrackFileDatabase::FormGoogleMapsQuery(int trackfile ,QString& query,int width,int height,
												int color,int weight)
	{

		QString value;
		query.clear();
		query+= "http://maps.google.com/maps/api/staticmap?size=";
		value.sprintf("%dx%d",width,height);
		query+=value;
		std::map< int, DG100::TrackFileHeader* >::iterator i = m_Headers.find(trackfile);
		if( i != m_Headers.end() )
		{
			query+="&path=color:0xff0000ff|weight:5";
			i->second->FormGoogleMapsQuery(query);
		}
		query+="&sensor=false\
&key=ABQIAAAAgFUqtzVrDEQKSBZOEIqBkRSNZEPBPLD8coecyb9PD94IKYduaBTMXaQLOAcKmtr3epEPRVf7FhpMDg";
		return true;
	}

	Configuration::Configuration()
		:style(DG100::Measurement::C)
		,SpeedThresholdFlag(false)
		,SpeedThreshold(0)
		,DistanceThresholdFlag(false)
		,DistanceThreshold(0)
		,TimeIntervalA(300)
		,TimeIntervalB(200)
		,TimeIntervalC(300)
		,DistanceThresholdA(0)
		,DistanceThresholdB(0)
		,DistanceThresholdC(0)
		,TimeAoverDistanceA(false)
		,TimeBoverDistanceB(false)
		,TimeCoverDistanceC(false)
		,MemoryUsage(1)
		{
			
		}

	void Configuration::FormMsg(QByteArray& d)
	{
		d.clear();
		d.reserve(50);
		for(int i =0; i<50;i++)
		{
			d[i]=0x0;
		}
		d[0] = 0xa0;
		d[1] = 0xa2;
		d[2] = 0x00;
		d[3] = 0x2a;
		
		d[4+0] = 0xB8;

		d[4+1] = (char)( style & 0x000000ff);
		DG100::MSG::SetBooleanValue(d,4+2,SpeedThresholdFlag);
		DG100::MSG::SetIntegerValue(d,4+3,SpeedThreshold);
		DG100::MSG::SetBooleanValue(d,4+7,DistanceThresholdFlag);
		DG100::MSG::SetIntegerValue(d,4+8,DistanceThreshold);
		DG100::MSG::SetIntegerValue(d,4+12,TimeIntervalA);
		DG100::MSG::SetIntegerValue(d,4+16,TimeIntervalB);
		DG100::MSG::SetIntegerValue(d,4+20,TimeIntervalC);
		d[4+24]=(char)0x01;
		d[4+25]=(char)0x0;
		DG100::MSG::SetBooleanValue(d,4+26,TimeAoverDistanceA);
		DG100::MSG::SetBooleanValue(d,4+27,TimeBoverDistanceB);
		DG100::MSG::SetBooleanValue(d,4+28,TimeCoverDistanceC);
		DG100::MSG::SetIntegerValue(d,4+29,DistanceThresholdA);
		DG100::MSG::SetIntegerValue(d,4+33,DistanceThresholdB);
		DG100::MSG::SetIntegerValue(d,4+37,DistanceThresholdC);
		d[4+41]=(char)0x0;
		short checksum = DG100::MSG::CalculateChecksum(d,4,42);
		DG100::MSG::SetShortValue(d,4+42,checksum);

		d[4+42+2] = 0xb0;
		d[4+42+3] = 0xb3;
	};
};
