#ifndef ABOUTBOXIMPL_H
#define ABOUTBOXIMPL_H
//
#include <QDialog>
#include "ui_aboutbox.h"
//
class AboutBoxImpl : public QDialog, public Ui::AboutBox
{
Q_OBJECT
public:
	AboutBoxImpl( QWidget * parent = 0, Qt::WFlags f = 0 );
private slots:
};
#endif





