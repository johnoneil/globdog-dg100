/***************************************************************************
*   Copyright (C) 2006 by Yuri Ovcharenko                                 *
*   amwsoft@gmail.com                                                     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef SERIAL_H
#define SERIAL_H

#include <termios.h>

#include <QObject>

/**
 * @author Yuri Ovcharenko <amwsoft@gmail.com>
 */

class QTimer;

/**
 * @class Serial
 * Serial provides generic interface to serial port such as ttyS0.
 * Because of QT's QIODevice does not support serial ports as ususal
 * the \a Serial class uses standart libc functions to open(), close(),
 * write(), read() for manipulate serial port and write/read data.
 * It is also uses a termios interface to do serial port specific operations
 * such as setup baudrate or so on.
 */
class Serial : public QObject
{
    Q_OBJECT
  public:
    /**
     * @Serial
     * Constructor. Constructs the Serial object with parent \a parent.
     */
    Serial( QObject *parent = 0 );
    /**
     * @~Serial
     * Destructor. Destructs the Serial object.
     */
    ~Serial();

  public:
    /**
     * @errorStr
     * Returns the string which describes error.
     */
    QString errorStr() const;

  public slots:
    /**
     * @setPort
     * Set serial port name to \a pname.
     * \a pname must be the full path name of valid tty device, "/dev/ttyS0" for example.
     *
     * \sa setBaud \sa openPort
     */
    void setPort(const QString & pname);
    /**
     * @setBaud
     * Set baudrate for tty device to \a b.
     * \a b is an integer value, represents a numeric value for baudrate.
     * For example to set baudrate to 115200 bps call this function as setBaud(115200).
     *
     * Supported baudrates are: 9600, 19200, 38400, 57600, 115200, 230400, 460800 and 921600.
     *
     * Retuns true if success, otherwise return false. Use \a errorStr() to obtain human readable
     * string describes error.
     * \sa setPort \sa openPort
     */
    bool setBaud(const int b);
    /**
     * @openPort
     * Open tty device.
     * The name of tty device must be set before using \a setPort().
     *
     * Retuns true if success, otherwise return false. Use \a errorStr() to obtain human readable
     * string describes error.
     * \sa closePort \sa setPort \sa setBaud \sa errorStr
     */
    bool openPort();
    /**
     * @closePort
     * Close tty device.
     * If device is not opened do noting.
     * \sa openPort
     */
    void closePort();
    /**
     * @send
     * Sends data \a d to serial port.
     * If port is not opened first open port.
     *
     * Return true on success, otherwise return false. Use \a errorStr() to obtain human readable
     * string describes error.
     *
     * \sa setPort \sa setBaud \sa openPort
     */
    bool send(const QByteArray & d);
    /**
     * @send
     * Sends data \a d to serial port.
     * If port is not opened first open port.
     *
     * Return true on success, otherwise return false. Use \a errorStr() to obtain human readable
     * string describes error.
     *
     * \sa setPort \sa setBaud \sa openPort
     */
    bool send(const char * buf, int size);

  private slots:
    void slotTimer();

  private:
    QString ErrorStr;
    struct termios ios, oldios;
    int oldmc;
    QString portName;
    int port;
    int baud;
    QTimer *timer;

  signals:
    /**
     * @receive
     * This signal is emited whenever any data from serial port received.
     * \a d contains received data.
     */
    void receive(const QByteArray & d);

    /**
     * @portOpened
     * This signal is emited whenever serial port is successfully opened and configured.
     */
    void portOpened();
};

#endif
