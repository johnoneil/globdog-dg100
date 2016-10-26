#ifndef __QUEUE_H__
#define __QUEUE_H__

/*	JOHN O'NEIL
	WED, SEPTEMBER 30TH 2009
	LIMASSOL,CYUPRUS
	
	A SIMPLE DATA QUEUE TO BUFFER SERIAL
	DATA AS IT ARRIVES.
	WE WILL SEARCH THROUGH THE QUEUE FOR
	FULLY FORMED MESSAGES, AND WAIT FOR THE
	FULL MESSAGE WHEN WE RECEIVE ONLY A PARTIAL.
	*/
	#include <QByteArray>
	#include <QString>
	
	//const static int MAX_QUEUE_SIZE = 4096;
	//const static int MAX_INPUT_SIZE = 1024;
	
	class DataQueue 
	{
		public:
		QByteArray m_InternalBuffer;
		
		public:
		DataQueue()
		{
			m_InternalBuffer.clear();
		};
		~DataQueue(){};
		
		public:
		
		bool Push(const QByteArray& input)
		{
			/*if(input.size() > MAX_INPUT_SIZE)
			{
				return false;
			}
			if(input.size() > (MAX_QUEUE_SIZE-unread_data))
			{
				return false;
			}*/
			/*for(int i=0;i<input.size();i++)
			{
				m_InternalBuffer[unread_data+i] = input[i];
				unread_data+=1;
			}*/
			m_InternalBuffer+=input;
			//unread_data+=input.size();
			return true;
		};
		void ShiftData(int start)
		{
			int unread_data = m_InternalBuffer.size();
			int new_unread_data = 0;
			for(int i=start,j=0;i<unread_data;i++,j++)
			{
				m_InternalBuffer[j]=m_InternalBuffer[i];
				new_unread_data+=1;
			}
			unread_data = new_unread_data;
		};
		bool PopMsg(QByteArray& msg)
		{
			/*find the first well formed message in the
			current internal buffer, and return it
			*/
			//int unread_data = m_InternalBuffer.size();
			if(m_InternalBuffer.size()<9)
			{
				return false;
			}
			QByteArray header;
			header.reserve(2);
			header[0]=(char)0xa0;
			header[1]=(char)0xa2;
			QByteArray footer;
			footer.reserve(2);
			footer[0]=(char)0xb0;
			footer[1]=(char)0xb3;
			
			int header_start = m_InternalBuffer.indexOf(header,0);
			if(header_start<0)
			{
				m_InternalBuffer.clear();
				return false;
			}
			//remove any "leading" garbage
			m_InternalBuffer.remove(0,header_start);
			
			int footer_start = m_InternalBuffer.indexOf(footer,0);
			if(footer_start <0)
			{
				return false;//Wait for the rest of the message to arrive.
			}
			int footer_end = footer_start+1;
			msg=m_InternalBuffer;
			msg.chop(m_InternalBuffer.size()-(footer_end-header_start+1));
			m_InternalBuffer.remove(0,msg.size());
			return true;
			//TODO: Is there a new message start BEFORE the footer?
			
/*			for(int i=0; i< (unread_data-4) ;i++)
			{
				if(m_InternalBuffer[i] == (char)0xa0
					&& m_InternalBuffer[i+1] == (char)0xa2 )
					{
						
						m_InternalBuffer.remove(0,i);
						
						short int lw = m_InternalBuffer[3];
						short int uw = m_InternalBuffer[2];
						short int payload_size = ( ( uw <<16 ) | (lw & 0x00ff) );
						int full_size = 8+(int)payload_size;
						if( (i + full_size) > unread_data)
						{
							return false;//msg not complete yet.
						}
						msg.resize(full_size);
						
						msg=m_InternalBuffer;
						msg.chop(m_InternalBuffer.size()-full_size);
						//AND take the msg OUT of the internal buffer
						//ShiftData(full_size);
						m_InternalBuffer.remove(0,full_size);
						return true;
					}
					
			}*/
			//if(unread_data>4)
			//unread_data=0;//Nothing found, so clear the buffer.
			//m_InternalBuffer.clear();
			//return false;
		};

		bool PopMsg(const QByteArray& msg_start,
					const QByteArray& msg_end,
					QByteArray& msg)
		{
			
			int header_start = m_InternalBuffer.indexOf(msg_start,0);
			if(header_start<0)
			{
				m_InternalBuffer.clear();
				return false;
			}
			//remove any "leading" garbage
			m_InternalBuffer.remove(0,header_start);
			
			int footer_start = m_InternalBuffer.indexOf(msg_end,0);
			if(footer_start <0)
			{
				return false;//Wait for the rest of the message to arrive.
			}
			int footer_end = footer_start+(msg_end.size()-1);
			msg=m_InternalBuffer;
			msg.chop(m_InternalBuffer.size()-(footer_end-header_start+1));
			m_InternalBuffer.remove(0,msg.size());
			return true;
		};
	};

#endif // __QUEUE_H__
