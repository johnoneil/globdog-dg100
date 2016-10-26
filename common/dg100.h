#ifndef __DG100_H__
#define __DG100_H__

#include <QByteArray>
#include <QString>
#include <vector>
#include <map>
#include <fstream>
#include <string>

namespace DG100
{

	namespace MSG
	{ 
		static const char header[] = {0xA0,0xA2};
		static const char coda[]={0xB0,0xB3};
		static const char ident[] = {0xA0, 0xA2, 0x00, 0x01, 0xBF, 0x00, 0xBF, 0xB0, 0xB3};
		static const char config[]= {0xA0, 0xA2, 0x00, 0x01, 0xB7, 0x00, 0xB7, 0xB0, 0xB3};
		static const char start_mouse_mode[] = { 0xA0, 0xA2, 0x00, 0x02, 0xBC, 0x01, 0x00, 0xBD, 0xB0, 0xB3 };
		static const char stop_mouse_mode[] = { 0xA0, 0xA2, 0x00, 0x02, 0xBC, 0x00, 0x00, 0xBC, 0xB0, 0xB3 };
		static const char header_request[]={0xA0, 0xA2, 0x00, 0x03, 0xBB, 0x00, 0x00, 0x00, 0xBB, 0xB0, 0xB3};		
		static const char trackfile_request[]={0xA0, 0xA2, 0x00, 0x03, 0xB5, 0x00, 0x00, 0x00, 0xB5, 0xB0, 0xB3};
		static const char erase_trackfiles[]={0xA0, 0xA2, 0x00, 0x03, 0xBA, 0xFF, 0xFF, 0x02, 0xB8, 0xB0, 0xB3};
		
		static const char identity_response = (char)0xbf;
		static const char config_response =(char)0xb7;
		static const char trackfile_response = (char)0xb5;
		static const char header_response = (char)0xbb;
		
		int GetIntegerValue(const QByteArray& d,int start);
		void SetIntegerValue(QByteArray& d, int start,int value);
		short GetShortValue(const QByteArray& d,int start);
		void SetShortValue(QByteArray&d,int start,short value);
		char GetCharValue(const QByteArray& d, int pos);
		bool GetBooleanValue(const QByteArray& d,int pos);
		void SetBooleanValue(QByteArray& d, int pos, bool value);
		char GetCommandType(const QByteArray& full_msg);
		short int GetPayloadSize(const QByteArray& array);
		long long GetIdentity(const QByteArray& full_msg);
		bool GetPayload(const QByteArray& full_msg,QByteArray& payload);
		short CalculateChecksum(const QByteArray& d, 
									int payload_start,int payload_size);
		void FormHeaderRequest(QByteArray& d, short int header_num);
		void FormTrackfileRequest(QByteArray& d, short int header_num);
		bool CheckFormedness(const QByteArray& msg);
		
		QString& Time2String(int time, QString& stime);
		QString& Date2String(int date, QString& sdate);
		
		float Lat2Float(int lat);
		float Lon2Float(int lon);

	}
	
	class Measurement
	{
		public:
		enum Style
		{
			A = 0,
			B = 1,
			C = 2,
			INVALID = -1
		};
		Measurement();
		Measurement( const QByteArray& d, int start);
		virtual ~Measurement();
		virtual int GetLatitude(void);
		virtual int GetLongitude(void);
		virtual int GetTime(void);
		virtual int GetDate(void);
		virtual int GetSpeed(void);
		virtual int GetAltitude(void);
		virtual DG100::Measurement::Style GetStyle(void);
		
		virtual void WriteGPX(std::ofstream& outfile);
	};
	class MeasurementA : public Measurement
	{
		public:
		MeasurementA( const QByteArray& d, int start);
		MeasurementA();
		virtual ~MeasurementA(void);
		
		protected:
		int m_Lat;
		int m_Lon;
		
		public:
		virtual int GetLatitude(void);
		virtual int GetLongitude(void);
		virtual DG100::Measurement::Style GetStyle(void);
		
		virtual void WriteGPX(std::ofstream& outfile);
	};
	class MeasurementB : public MeasurementA
	{
		public:
		MeasurementB( const QByteArray& d, int start);
		MeasurementB();
		virtual ~MeasurementB(void);
		
		protected:
		int m_Time;
		int m_Date;
		int m_Speed;
		
		public:
		virtual int GetTime(void);
		virtual int GetDate(void);
		virtual int GetSpeed(void);
		virtual DG100::Measurement::Style GetStyle(void);
	};
	class MeasurementC : public MeasurementB
	{
		public:
		MeasurementC( const QByteArray& d, int start);
		virtual ~MeasurementC(void);
		
		protected:
		int m_Altitude;
		DG100::Measurement::Style m_Style;
		
		public:
		virtual int GetAltitude(void);
		virtual DG100::Measurement::Style GetStyle(void);
	};
	
	class TrackFileHeader
	{
		public:
		TrackFileHeader(int header_num,int time,int date);
		~TrackFileHeader();
		
		int HeaderNum(void);
		int GetNumMeasurements(void);
		DG100::Measurement* Measurement(int n);
		int Load(const QByteArray& msg);
		int GetTime(void);
		int GetDate(void);
		
		void WriteGPX(std::ofstream& outfile);
		void FormGoogleMapsQuery( QString& query );
		void FormGoogleMapsQueryEncodedPolyline( QString& query);
		
		void GetBounds(float& min_lat,float& max_lat,
						float& min_lon, float& max_lon);
		bool IsIncluded(void);
		bool SetIncluded(bool value);
		
		protected:
		std::vector< DG100::Measurement* > m_Measurements;
		int m_HeaderNum;
		int m_Time;
		int m_Date;
		bool m_Included;
	};
	
	class TrackFileDatabase
	{
		protected:
		std::map< int, DG100::TrackFileHeader* > m_Headers;
		QByteArray m_MsgBuffer;
		int m_NextHeaderNum;
		int m_NumHeaders;
		
		public:
		void Clear(void);
		TrackFileDatabase();
		~TrackFileDatabase();
		
		void FlushMsgBuffer(void);
		
		short PushHeaderMsg( const QByteArray& msg);
		bool PushTrackfileMsg(const QByteArray& msg);

		enum NextHeaderFromStart
		{
			FromStart = 0,
			Next = 1,
			Current = 2
		};
		short GetNextHeaderNum( NextHeaderFromStart header);
		
		int GetNumHeaders(void);
		DG100::TrackFileHeader* GetHeaderNum(int n);
		
		void WriteGPX(std::ofstream& outfile);
		
		void GetBounds(float& min_lat,float& max_lat,
						float& min_lon, float& max_lon);
		void GetBounds(int track,float& min_lat,float& max_lat,
						float& min_lon, float& max_lon);
		bool FormGoogleMapsQuery(QString& query,int width,int height,int color,int weight);
		bool FormGoogleMapsQuery(int trackfile,QString& query,int width,int height,int color,int weight);
		
	};
	struct Configuration
	{
		DG100::Measurement::Style style;
		bool SpeedThresholdFlag;
		int SpeedThreshold;
		bool DistanceThresholdFlag;
		int DistanceThreshold;
		int TimeIntervalA;
		int TimeIntervalB;
		int TimeIntervalC;
		int DistanceThresholdA;
		int DistanceThresholdB;
		int DistanceThresholdC;
		bool TimeAoverDistanceA;
		bool TimeBoverDistanceB;
		bool TimeCoverDistanceC;
		int MemoryUsage;
		
		Configuration();
		
		void FormMsg(QByteArray& d);
	};
};

#endif // __DG100_H__
