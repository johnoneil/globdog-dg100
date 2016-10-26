#ifndef __NMEA_H__
#define __NMEA_H__

#include <QByteArray>
#include <QString>

// place your code here
namespace NMEA
{
	static const char SENTENCE_START[] = {0x24};
	static const char SENTENCE_END[] = {0x0d,0x0a};
	
	static bool NextToken(QByteArray& sentence,QByteArray& token)
	{
		if(sentence.isEmpty() )
		{
			return false;
		}
		int comma = sentence.indexOf(',',0);
		if(comma<0)
		{
			return false;
		}
		token = sentence;
		token.chop(sentence.size()-comma);
		sentence.remove(0,token.size()+1);//plus one to include the comma
		return true;
	}
	static QString PosFixIndicator2String(const QString& indicator)
	{
		QString r;
		if(indicator == "0")
		{
			r="Fix Unavailable or Invalid";
		}else if(indicator=="1")
		{
			r="GPS SPS Mode, Fix Valid";
		}else if(indicator == "2")
		{
			r="Differential GPS, SPS Mode, Fix Valid";
		}else if(indicator == "3")
		{
			r="GPS PPS Mode, Fix Valid";
		}else{
			r="UNKNOWN POS FIX";
		}
		return r;
	};
	struct Satellites
	{
		QByteArray ID;
		QByteArray Azimuth;
		QByteArray Elevation;
		QByteArray SNR;
		bool valid;
		/*Satellites(const QByteArrayQByteArrayQByteArray& id,const QString& az,const QString& el,
					const QString& snr)
		:ID(id)
		,Azimuth(az)
		,Elevation(el)
		,SNR(snr)
		,valid(true){};*/
	};
	class SatelliteCoverage
	{
		//protected:
		public:
		SatelliteCoverage()
		:NumberofSatellitesInView("0")
		{
			ClearAllSatellites();
		};
		Satellites Satellite[32];//max 32 satellites
		QByteArray NumberofSatellitesInView;
		
		public:
		void ClearAllSatellites(void)
		{
			for(int i =0;i<32;i++)
			{
				Satellite[i].valid = false;
			}
		};
		bool LoadNextSatellite(QByteArray& msg)
		{
			QByteArray id;
			//satellite id
			if(NextToken(msg,id)==false)
			{
				return false;
			}
			int ID = id.toInt();
			if(ID<0 || ID >=32)
			{
				return false;
			}
			Satellite[ID].ID=id;
			
			//elevation
			if(NextToken(msg,Satellite[ID].Elevation)==false)
			{
				return false;
			}
			//azimuth
			if(NextToken(msg,Satellite[ID].Azimuth)==false)
			{
				return false;
			}
			//SNR
			if(NextToken(msg,Satellite[ID].SNR)==false)
			{
				return false;
			}
			if(Satellite[ID].SNR.isEmpty())
			{
				Satellite[ID].SNR = "UNAVAILABLE";
			}
			Satellite[ID].valid=true;
			return true;
		}
		
		bool Update(const QByteArray& gsv_msg)
		{
			if(gsv_msg.isEmpty())
			{
				return false;
			}
			QByteArray msg = gsv_msg;
			QByteArray token;
			//Strip off the checksum
			int checksum = msg.indexOf("*",0);
			if(checksum<0)
			{
				return false;//no checksum-- we can't deal with it.
			}
			msg.chop(msg.size()-checksum);
			msg.append(",");//helps to parse the last member
			//msg+=",,";
			
			//number of messages (in sequence)
			QByteArray number_of_messages;
			if(NextToken(msg,number_of_messages)==false)
			{
				return false;
			}
			QByteArray current_msg_num;
			if(NextToken(msg,current_msg_num)==false)
			{
				return false;
			}
			//Current message number (1 to NUM_MSGS)
			if(current_msg_num == "1")
			{
				//First message in a new group-- clear all satellites
				ClearAllSatellites();
			}
			
			//Number of satellites in view
			if(NextToken(msg,NumberofSatellitesInView)==false)
			{
				return false;//can't deal with an error here.
			}
			
			//n groups of 4 items
				//>>satellite Id
				//>>elevation
				//>>azimuth
				//>>snr
			while(LoadNextSatellite(msg)==true)
			{
				
			}
				
			//LAST GSV MESSAGE IN SEQUENCE?
			if(current_msg_num == number_of_messages)
			{
				return true;//"true" meaning redraw the gui
			}else{
				return false;
			}
		};
	};
/*	static bool GetSentenceType(const QString& sentence,QString& type_out)
	{
		if(sentence.isEmpty())
		{
			return false;
		}
		if(sentence[0]!='$')
		{
			return false;
		}
		QString header;
		int comma = sentence.indexOf(',',0);
		if(comma<0)
		{
			return false;
		}
		type_out=sentence;
		type_out.chop(comma);
		type_out.remove(3);
		return true;	
	}*/
};

#endif // __NMEA_H__
