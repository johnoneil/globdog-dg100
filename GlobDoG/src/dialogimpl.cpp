#include "dialogimpl.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <QListWidgetItem>
#include <QFileDialog>
#include <dirent.h>
#include <QMessageBox>
#include <QChar>
#include <ostream>

#include "../common/dg100.h"
#include "../common/nmea.h"
#include "aboutboximpl.h"


DialogImpl::DialogImpl( QWidget * parent, Qt::WFlags f) 
	: QDialog(parent, f)
	,serial_port(NULL)
{
	setupUi(this);
	
	//Google maps not visible at first
	Map->setVisible(false);
	
	PopulateSerialDeviceCombo();
	
	//Connect satellite list and model
	SatellitesInView->setModel(&m_SatelliteModel);
	//m_RedSat;
	//m_GreenSat;
	
	connect(&serial_timeout,SIGNAL(timeout()),this,SLOT(OnSerialTimeout()));
}

void DialogImpl::PopulateSerialDeviceCombo(void)
{
	//Fill the device combo box control
	//with all possible serial devices
	DIR *dp;
	int items = 0;
	struct dirent *dirp;
	if( (dp  = opendir("/dev") ) == NULL)
	{
		SerialPorts->insertItem(0,"/ttyUSB0");
	}else{
		while ((dirp = readdir(dp)) != NULL)
		{
			QString devicename = dirp->d_name;
			if(devicename.indexOf("ttyUSB")!= (-1) )
			{
				//add serial device.
				SerialPorts->insertItem(items,devicename);
			}
		}
		closedir(dp);
	}


}
void DialogImpl::OnDG100Connect(void)
{
	//We've just connected, so enable our tabs
	SettingsTab->setDisabled(false);
	MouseModeTab->setDisabled(false);
	TrackFileTab->setDisabled(false);
	DG100ConnectionStatus->setText("Connected to "+SerialPorts->currentText());
}
void DialogImpl::OnDG100Disconnect(void)
{
	//No DG100 available anymore, disable the tabs
	SettingsTab->setDisabled(true);
	MouseModeTab->setDisabled(true);
	TrackFileTab->setDisabled(true);
	DG100ConnectionStatus->setText("Disconnected.");
}
void DialogImpl::SerialInput(const QByteArray& d)
{
	QByteArray snatched = d;

	HandleDG100Msg(snatched);
	HandleNMEAMsg(snatched);
}

void DialogImpl::HandleDG100Msg(const QByteArray& d)
{
	data_queue.Push(d);
	QByteArray dg100_msg;

	QByteArray header(DG100::MSG::header,2);
	QByteArray coda(DG100::MSG::coda,2);
	
	while( data_queue.PopMsg(header,
								coda,
								dg100_msg) )
	{
		switch( dg100_msg[4] )
		{
			case DG100::MSG::identity_response:
				//LOG->insertItem(0,"Received DG100 Ident msg.");
				break;
			case DG100::MSG::config_response:
				//LOG->insertItem(0,"Received DG100 configuration.");
				ConfigRX(dg100_msg);
				break;
			case DG100::MSG::header_response:
				//LOG->insertItem(0,"Received DG100 track header.");
				OnTrackHeader(dg100_msg);
				break;
			case DG100::MSG::trackfile_response:
				//LOG->insertItem(0,"Received DG100 trackfile.");
				OnTrackFile(dg100_msg);
				break;
			default:
				//LOG->insertItem(0,"Unknown DG100 msg received.");
				break;
		}
	}
}
void DialogImpl::HandleNMEAMsg(const QByteArray& d)
{
	//LOOK THROUGH FOR NMEA FORMAT DATA
	nmea_data.Push(d);
	QByteArray nmea_sentence;
	QByteArray nmea_start(NMEA::SENTENCE_START,1);
	QByteArray nmea_end(NMEA::SENTENCE_END,2);
	QByteArray token;
	QString sentence;
	while( nmea_data.PopMsg(nmea_start,
							nmea_end,
							nmea_sentence) )
	{
		if(NMEA::NextToken(nmea_sentence,token)==true)
		{
			if(token.size()>3)
			{
				token.remove(0,3);
				if(token=="GGA")
				{
					MNEA_GGA_Update(nmea_sentence);
				}else if(token =="GSA")
				{
					//Not currently supported.
				}else if (token == "GSV")
				{
					MNEA_GSV_Update(nmea_sentence);
				}else{
					//Must be currently unsupported NMEA sentence.
				}
			}
		}
	}
}

void DialogImpl::Send()
{
	if(serial_port)
	{
		serial_port->closePort();
		serial_port->deleteLater();
	}
	serial_port = new Serial(this);
	QString dev_name = SerialPorts->currentText();
	dev_name.insert(0,"/dev/");
	
	serial_port->setPort( dev_name );
	serial_port->setBaud(115200);
	
	connect(serial_port,SIGNAL(receive(const QByteArray &)),this,SLOT(SerialInput(const QByteArray &)));

	if(serial_port->openPort()==false)
	{
		LOG->insertItem(0,"ERROR: Could not open serial port.");
		OnDG100Disconnect();
	}else{
		LOG->insertItem(0,"Successfully opened port.");
		GetConfig();
	}
	
}
void DialogImpl::GetConfig(void)
{
	QByteArray d(DG100::MSG::config,sizeof(DG100::MSG::config));
	serial_port->send(d);
}
void DialogImpl::ConfigRX(const QByteArray& d)
{
	if(d.size()<42)
	{
		return;
	}
	//We've received a configuration. We MUST be connected.
	OnDG100Connect();
	switch(d[1+4])
	{
		case 0:
			RecordingMode->setCurrentIndex(0);
			break;
		case 1:
			RecordingMode->setCurrentIndex(1);
			break;
		case 2:
			RecordingMode->setCurrentIndex(2);
			break;
		default:
			RecordingMode->setCurrentIndex(0);
			break;
	}
	SpeedThresholdFlag->setChecked(DG100::MSG::GetBooleanValue(d,2+4));
	SpeedThreshold->setDisabled(!DG100::MSG::GetBooleanValue(d,2+4));
	SpeedThreshold->setValue(DG100::MSG::GetIntegerValue(d,3+4));
	DistanceThresholdFlag->setChecked(DG100::MSG::GetBooleanValue(d,7+4));
	DistanceThreshold->setDisabled(!DG100::MSG::GetBooleanValue(d,7+4));
	DistanceThreshold->setValue(DG100::MSG::GetBooleanValue(d,8+4));
	ATime->setValue((DG100::MSG::GetIntegerValue(d,12+4)/1000));
	BTime->setValue((DG100::MSG::GetIntegerValue(d,16+4)/1000));
	CTime->setValue((DG100::MSG::GetIntegerValue(d,20+4)/1000));
	ADistance->setValue(DG100::MSG::GetIntegerValue(d,29+4)/1000);
	BDistance->setValue(DG100::MSG::GetIntegerValue(d,33+4)/1000);
	CDistance->setValue(DG100::MSG::GetIntegerValue(d,37+4)/1000);
	if(DG100::MSG::GetBooleanValue(d,26+4))
	{
		//Mode A uses "by distance"
		ASetDistance();
	}else{
		//Mode A uses "by time"
		ASetTime();
	}
	if(DG100::MSG::GetBooleanValue(d,27+4))
	{
		BSetDistance();
	}else{
		BSetTime();
	}
	if(DG100::MSG::GetBooleanValue(d,28+4))
	{
		CSetDistance();
	}else{
		CSetTime();
	}
	
	QString usage;
	usage.sprintf("Current Memory Usage: %d percent",DG100::MSG::GetCharValue(d,42+4));
	MemoryUsage->setText(usage);

}
void DialogImpl::WriteConfig(void)
{
	LOG->insertItem(0,"Writing configuration to device.");
	DG100::Configuration config;
	switch( RecordingMode->currentIndex())
	{
		case 0:
			config.style = DG100::Measurement::A;
			break;
		case 1:
			config.style = DG100::Measurement::B;
			break;
		default:
			config.style = DG100::Measurement::C;
			break;
	}
	
	config.DistanceThresholdFlag = DistanceThresholdFlag->checkState();
	config.DistanceThreshold = DistanceThreshold->value();
	config.SpeedThresholdFlag = SpeedThresholdFlag->checkState();
	config.SpeedThreshold = SpeedThreshold->value();
	config.TimeIntervalA = ATime->value()*1000;
	config.TimeIntervalB = BTime->value()*1000;
	config.TimeIntervalC = CTime->value()*1000;
	config.TimeAoverDistanceA = AUseDistance->isChecked();
	config.TimeBoverDistanceB = BUseDistance->isChecked();
	config.TimeCoverDistanceC = CUseDistance->isChecked();
	config.DistanceThresholdA = ADistance->value()*1000;
	config.DistanceThresholdB = BDistance->value()*1000;
	config.DistanceThresholdC = CDistance->value()*1000;
	
	QByteArray message;
	config.FormMsg(message);
	serial_port->send(message);
}
void DialogImpl::SetMouseMode(void)
{
	LOG->insertItem(0,"Putting dg100 into GPS mouse mode");
	QByteArray d(DG100::MSG::start_mouse_mode,sizeof(DG100::MSG::start_mouse_mode));
	serial_port->send(d);
}
void DialogImpl::UnsetMouseMode(void)
{
	LOG->insertItem(0,"Stopping GPS mouse mode");
	QByteArray d(DG100::MSG::stop_mouse_mode,sizeof(DG100::MSG::stop_mouse_mode));
	serial_port->send(d);
}
void DialogImpl::MNEA_GGA_Update(const QByteArray& sentence)
{
	if(sentence.isEmpty())
	{
		return;
	}
	QByteArray local_sentence = sentence;
	QByteArray token;
	QString loc,lat_deg,lat_min,lon_deg,lon_min,ns,ew;
	QString hour,min,sec;
	bool no_fix = false;
	//UTC time in hhmmss.sss 
	if(NMEA::NextToken(local_sentence,token)==false)
	{
		return;
	}
	if(token.isEmpty() || token.size()< 10)
	{
		MouseModeUTCTime->setText("--:--:--");
	}else{
		token.insert(4,":");
		token.insert(2,":");
		MouseModeUTCTime->setText(token);
		
	}
	//Latitude in ddmm.mmmm 
	if(NMEA::NextToken(local_sentence,token)==false)
	{
		return;
	}
	if(token.isEmpty() || token.size()<9)
	{
		//lat_deg = lat_min = "-";
		no_fix = true;
	}else{
		
		lat_deg = lat_min = token;
		lat_deg.chop(7);
		lat_min.remove(0,2);
	}
	//loc.sprintf(")
	//N or S 
	if(NMEA::NextToken(local_sentence,token)==false)
	{
		return;
	}
	if(token.isEmpty() )
	{
		//ns = "-";
		no_fix = true;
	}else{
		
		ns = token;
	}
	//Longitude in dddmm.mmmm 
	if(NMEA::NextToken(local_sentence,token)==false)
	{
		return;
	}
	if(token.isEmpty() || token.size()<10)
	{
		//lon_deg= lon_min = "-";
		no_fix = true;
	}else{
		lon_deg = lon_min = token;
		lon_deg.chop(7);
		lon_min.remove(0,3);
	}
	//E or W 
	if(NMEA::NextToken(local_sentence,token)==false)
	{
		return;
	}
	if(token.isEmpty())
	{
		//ew = "-";
		no_fix = true;
	}else{
		ew=token;
	}
		//Fixed position indicator 
	if(NMEA::NextToken(local_sentence,token)==false)
	{
		return;
	}
	MouseModeFixStatus->setText(NMEA::PosFixIndicator2String(token));
	
	//satelites used (0 to 12) two digits 
	if(NMEA::NextToken(local_sentence,token)==false)
	{
		return;
	}
	//HDOP (float) 
	if(NMEA::NextToken(local_sentence,token)==false)
	{
		return;
	}
	if( no_fix )
	{
		HDOP->setText(tr("Unknown"));
	}else{
		HDOP->setText(token);
	}
	
	
	//MSL altitude (float 
	if(NMEA::NextToken(local_sentence,token)==false)
	{
		return;
	}
	QByteArray alt = token;
	
	//altitude units (e.g. m) 
	if(NMEA::NextToken(local_sentence,token)==false)
	{
		return;
	}
	if( no_fix )
	{
		Altitude->setText(tr("Unknown"));
	}else{
		Altitude->setText(alt+" "+token);
	}
	
	//Geoid separation (float) 
	if(NMEA::NextToken(local_sentence,token)==false)
	{
		return;
	}
	//Geoid units (e.g. m) 
	if(NMEA::NextToken(local_sentence,token)==false)
	{
		return;
	}
	//Age of Diff.Corr. (perhaps null if no DIFF GPS) 
	if(NMEA::NextToken(local_sentence,token)==false)
	{
		//return;
	}
	//Diff reference station ID 
	if(NMEA::NextToken(local_sentence,token)==false)
	{
		//return;
	}
	//Checksum 
	if(NMEA::NextToken(local_sentence,token)==false)
	{
		//return;
	}
	//CR LF
	
	//Put it up on the gui
	if( no_fix )
	{
		loc = tr("Location Unknown");
	}else{
		QChar deg( 0x00B0 );
		loc = lat_deg+deg+" "+lat_min+"' "+ns+" "+lon_deg+deg+" "+lon_min+"' "+ew;
	}
	
	MouseModeLoc->setText(loc);
}
void DialogImpl::MNEA_GSV_Update(const QByteArray& sentence)
{
	if(GPS.Update(sentence)==true)
	{
		//update our listbox
		//SatellitesInView->clear();
		m_SatelliteModel.clear();
		int row = 0;
		for(int i = 0;i<32;i++)
		{
			if(GPS.Satellite[i].valid==true)
			{
				QString sat = GPS.Satellite[i].ID+" Az:"+GPS.Satellite[i].Azimuth;
					sat+= " El:"+GPS.Satellite[i].Elevation+" SNR:"+GPS.Satellite[i].SNR;
				QStandardItem* newindex = new QStandardItem();
				newindex->setText(sat);
				if( GPS.Satellite[i].SNR.toInt() > 0 )
				{
					newindex->setIcon(QIcon(":/png_files/satellite_green.png"));
				}else{
					newindex->setIcon(QIcon(":/png_files/satellite_red.png"));
				}

				m_SatelliteModel.appendRow(newindex);
				row++;
			}
		}
		SatellitesInViewGroupbox->setText(GPS.NumberofSatellitesInView+" Satellites in View");
	}
}
void DialogImpl::GetTrackFiles(void)
{
	//LOG->insertItem(0,"Getting DG100 track headers.");
	QByteArray d;
	DG100::MSG::FormHeaderRequest(d,0);
	QString msg;
	//msg.sprintf("msg: %x",d.toHex());
	//LOG->insertItem(0,d.toHex());
	serial_port->send(d);
}
void DialogImpl::EraseTrackFiles(void)
{
	QMessageBox msgBox;
	msgBox.setText("Do you really wish to erase all recorded DG-100 trackfiles?");
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Cancel);
	msgBox.setIcon(QMessageBox::Question);
	if( msgBox.exec() == QMessageBox::Ok)
	{
		QByteArray message(DG100::MSG::erase_trackfiles,sizeof(DG100::MSG::erase_trackfiles));
		serial_port->send(message);
	}

}

void DialogImpl::OnTrackHeader(const QByteArray& d)
{
	int next_header = TrackFileDatabase.PushHeaderMsg(d);
	if( next_header > 0)
	{
		QByteArray request;
		DG100::MSG::FormHeaderRequest(request,(short)next_header);
		serial_port->send(request);
	}else{
		//Last header has arrived
		int num_headers = TrackFileDatabase.GetNumHeaders();
		TrackHeaders->clear();
		
		//And if we have headers, kick off requests for the file contents.
		if( num_headers > 0)
		{
			QByteArray request;
			short header_num = TrackFileDatabase.GetNextHeaderNum(
										DG100::TrackFileDatabase::FromStart);
			if( header_num >=0)
			{
				FileLoadProgress->setMinimum(0);
				FileLoadProgress->setMaximum(TrackFileDatabase.GetNumHeaders());
				FileLoadProgress->setValue(0);
				TrackFileDatabase.FlushMsgBuffer();
				DG100::MSG::FormTrackfileRequest(request,header_num);
				
				serial_timeout.setSingleShot(true);
				serial_timeout.start(500);
				serial_port->send(request);
			}
			
		}
	}
}
void DialogImpl::OnTrackFile(const QByteArray& d)
{
	//This can be either the first or second half
	//of the file.
	QString msg;
	msg.sprintf("Trackfile of size %d",d.size());
	LOG->insertItem(0,msg);
	if(TrackFileDatabase.PushTrackfileMsg(d)==true)
	{
		//Got both halves, so stop our timer
		serial_timeout.stop();
		
		//Update the loaded track file entry in the listbox
		short current_header = TrackFileDatabase.GetNextHeaderNum(
											DG100::TrackFileDatabase::Current);
		if(current_header>=0)
		{
			DG100::TrackFileHeader* header = TrackFileDatabase.GetHeaderNum(current_header);
			if(header!=NULL)
			{
				QString msg,t,d;
				msg.sprintf("File %d ",current_header);
				msg+=DG100::MSG::Time2String(header->GetTime(),t);
				msg+=" ";
				msg+=DG100::MSG::Date2String(header->GetDate(),d);
				QListWidgetItem* newitem = new QListWidgetItem(TrackHeaders);
				newitem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable 
								| Qt::ItemIsEnabled );
				newitem->setText(msg);
				newitem->setCheckState(Qt::Checked);
				TrackHeaders->addItem(newitem);
			}
		}
		
		short next_header = TrackFileDatabase.GetNextHeaderNum(
											DG100::TrackFileDatabase::Next);
		if( next_header < 0)
		{
			//No more headers. We're done loading the files.
			FileLoadProgress->setValue(TrackFileDatabase.GetNumHeaders());
			
		}else{
			//Request file contents for the next header.
			QByteArray request;
			DG100::MSG::FormTrackfileRequest(request,next_header);
			
			TrackFileDatabase.FlushMsgBuffer();
			serial_timeout.setSingleShot(true);
			serial_timeout.start(500);
			serial_port->send(request);
			
			QString msg;
			msg.sprintf("Requesting trackfile for header %d",next_header);
			LOG->insertItem(0,msg);

			FileLoadProgress->setValue(next_header);
		}
	}
	
	
}
void DialogImpl::OnTrackFileSelected(void)
{

	for(int i = 0; i< TrackFileDatabase.GetNumHeaders();i++)
	{
		QListWidgetItem* item = TrackHeaders->item(i);
		
		//Set the item as selected
		DG100::TrackFileHeader* header = TrackFileDatabase.GetHeaderNum(i);
		if( header != NULL)
		{
			if( item->checkState()== Qt::Checked )
			{
				header->SetIncluded(true);
			}else{
				header->SetIncluded(false);
			}
		}
	}
	UpdateMap();
	UpdateSelectedFileBounds();
}
void DialogImpl::ASetDistance(void)
{
	AUseDistance->setChecked(true);
	ATime->setDisabled(true);
}
void DialogImpl::ASetTime(void)
{
	AUseTime->setChecked(true);
	ADistance->setDisabled(true);
}
void DialogImpl::BSetDistance(void)
{
	BUseDistance->setChecked(true);
	BTime->setDisabled(true);
}
void DialogImpl::BSetTime(void)
{
	BUseTime->setChecked(true);
	BDistance->setDisabled(true);
}
void DialogImpl::CSetDistance(void)
{
	CUseDistance->setChecked(true);
	CTime->setDisabled(true);
}
void DialogImpl::CSetTime(void)
{
	CUseTime->setChecked(true);
	CDistance->setDisabled(true);
}

void DialogImpl::OnSerialTimeout(void)
{
	//If this timer has gone off, 
	//Our serial file request has gone
	//unanswered. So resend the request
	short header = TrackFileDatabase.GetNextHeaderNum(
											DG100::TrackFileDatabase::Current);
	if( header < 0)
	{
		//No more headers. We're done loading the files.
		//LOG->insertItem(0,"No more file contents. We're done loading files");
		//FileLoadProgress->setValue(TrackFileDatabase.GetNumHeaders());
	}else{
		//Request file contents for the next header.
		TrackFileDatabase.FlushMsgBuffer();
		QByteArray request;
		DG100::MSG::FormTrackfileRequest(request,header);
		
		TrackFileDatabase.FlushMsgBuffer();
		serial_timeout.setSingleShot(true);
		serial_timeout.start(500);
		serial_port->send(request);
		
		QString msg;
		msg.sprintf("RE-Requesting trackfile for header %d",header);
		LOG->insertItem(0,msg);
		FileLoadProgress->setValue(header);
	}
	
}

void DialogImpl::ExportGPX(void)
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export GPX"),
							"/home",
							tr("GPS Exchange File (*.gpx)"));
	if( !fileName.isNull() )
	{
		std::ofstream outfile(fileName.toAscii());
		if(!outfile.is_open())
		{
			return;
		}
		this->TrackFileDatabase.WriteGPX(outfile);
	}
}
void DialogImpl::UpdateSelectedFileBounds(void)
{
	if(Map->isVisible())
	{
		return;
	}
	for(int i = 0; i< TrackFileDatabase.GetNumHeaders();i++)
	{
		QListWidgetItem* item = TrackHeaders->item(i);
		if( item == NULL)
		{
			continue;
		}
		if(item->isSelected())
		{
			float min_lat,max_lat,min_lon,max_lon;
			this->TrackFileDatabase.GetBounds(i,min_lat,max_lat,min_lon,max_lon);
			QString msg;
			msg.sprintf("Selected Tracks:\nLat: %f to %f\nLon: %f to %f",
						min_lat,max_lat,min_lon,max_lon);
			TrackBoundsDisplay->setText(msg);
			return;
		}
	}
	//nothing selected, so just get the bounds of ALL (checked) tracks
	float min_lat,max_lat,min_lon,max_lon;
	this->TrackFileDatabase.GetBounds(min_lat,max_lat,min_lon,max_lon);
	QString msg;
	msg.sprintf("Selected Tracks:\nLat: %f to %f\nLon: %f to %f",
						min_lat,max_lat,min_lon,max_lon);
	TrackBoundsDisplay->setText(msg);
}
void DialogImpl::OnMapVisible(bool visible)
{
	UpdateSelectedFileBounds();
	UpdateMap();

}
void DialogImpl::resizeEvent( QResizeEvent * event )
{
	if(Map->isVisible())
	{
		UpdateMap();
	}
	QDialog::resizeEvent(event);
}
void  DialogImpl::UpdateMap(void)
{
	if(!Map->isVisible())
	{
		return;
	}
	QString req;
	for(int i = 0; i< TrackFileDatabase.GetNumHeaders();i++)
	{
		QListWidgetItem* item = TrackHeaders->item(i);
		if( item == NULL)
		{
			continue;
		}
		if(item->isSelected())
		{
			TrackFileDatabase.FormGoogleMapsQuery(i,req,Map->width(),Map->height(),100,100);
			//LOG->insertItem(0,req);
			if(req.size() >= 2048 )
			{
				LOG->insertItem(0,"Request too large");
				break;
			}else{
				Map->setUrl(req);
				return;
			}
		}
	}
	//If we get here, nothing has been selected-- update a "blank" map.
	TrackFileDatabase.FormGoogleMapsQuery(-1,req,Map->width(),Map->height(),100,100);
	Map->setUrl(req);
}

void DialogImpl::OnHelp(void)
{
	AboutBoxImpl about_box;
	about_box.exec();
}
//
