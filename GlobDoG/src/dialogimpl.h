#ifndef DIALOGIMPL_H
#define DIALOGIMPL_H
//
#include <QDialog>
#include <QTimer>
#include <QStandardItemModel>
#include <QIcon>
#include "ui_dialog.h"
#include <QFile>
#include "serial.h"
#include "../common/Queue.h"
#include "../common/nmea.h"
#include "../common/dg100.h"
//
class DialogImpl : public QDialog, public Ui::Dialog
{
Q_OBJECT
public:
	DialogImpl( QWidget * parent = 0, Qt::WFlags f = 0 );

	Serial* serial_port;
	DataQueue data_queue;
	DataQueue nmea_data;
	NMEA::SatelliteCoverage GPS;
	DG100::TrackFileDatabase TrackFileDatabase;
	QTimer serial_timeout;
	QStandardItemModel m_SatelliteModel;
	QIcon m_RedSat,m_GreenSat;
	
	void PopulateSerialDeviceCombo(void);
	void UpdateSelectedFileBounds(void);
	
public:
	void HandleDG100Msg(const QByteArray& d);
	void HandleNMEAMsg(const QByteArray& d);
	void OnTrackHeader(const QByteArray& d);
	void OnTrackFile(const QByteArray& d);
	void resizeEvent ( QResizeEvent * event );
	void UpdateMap(void);
public slots:
	void SerialInput(const QByteArray& d);
	void Send(void);
	void GetConfig(void);
	void WriteConfig(void);
	void ConfigRX(const QByteArray& d);
	void SetMouseMode(void);
	void UnsetMouseMode(void);
	void MNEA_GGA_Update(const QByteArray& sentence);
	void MNEA_GSV_Update(const QByteArray& sentence);
	void OnDG100Connect(void);
	void OnDG100Disconnect(void);
	void GetTrackFiles(void);
	void EraseTrackFiles(void);
	void OnTrackFileSelected(void);
	void ASetDistance(void);
	void ASetTime(void);
	void BSetDistance(void);
	void BSetTime(void);
	void CSetDistance(void);
	void CSetTime(void);
	void OnSerialTimeout(void);
	void ExportGPX(void);
	void OnMapVisible(bool visible);
	void OnHelp(void);
};
#endif




