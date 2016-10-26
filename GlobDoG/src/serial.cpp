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
#include "serial.h"

#include <sys/types.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <QtCore>

Serial::Serial( QObject *parent )
    : QObject( parent )
{
  port = -1;
  baud = B115200;
  ErrorStr = QString(tr("All fine."));
  timer = new QTimer(this);
  timer->stop();
  connect(timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
}

Serial::~Serial()
{
}

QString Serial::errorStr() const
{
  return ErrorStr;
}

void Serial::setPort(const QString & pname)
{
  if (port >= 0)
    closePort();
  portName = pname;
}

bool Serial::setBaud(const int b)
{
  switch (b) {
    case 9600:
      baud = B9600;
      break;
    case 19200:
      baud = B19200;
      break;
    case 38400:
      baud = B38400;
      break;
    case 57600:
      baud = B57600;
      break;
    case 115200:
      baud = B115200;
      break;
    case 230400:
      baud = B230400;
      break;
    case 460800:
      baud = B460800;
      break;
    case 921600:
      baud = B921600;
      break;
    default:
      ErrorStr = QString(tr("Unsupported baudrate."));
      return false;
  }
  if (port < 0)
  {
    ErrorStr = QString(tr("All fine."));
    return true;
  }
  cfsetispeed(&ios, baud);
  cfsetospeed(&ios, baud);
  if (tcsetattr(port, TCSANOW, &ios) < 0)
  {
    ErrorStr = QString(tr("Can not set TTY attributes: %1")).arg(tr(strerror(errno)));
    return false;
  }
  return true;
}

bool Serial::openPort()
{
  int mc;

  if (port >= 0)
    return true;
  if (portName.isEmpty())
  {
    ErrorStr = QString(tr("Port name not specified."));
    return false;
  }

  cfmakeraw(&ios);
  /* Input flags */
  ios.c_iflag |= IGNBRK | IGNPAR;
  /* Control flags */
  ios.c_cflag |= CREAD | CLOCAL;
  ios.c_cflag &= ~CRTSCTS;
  /* Control char's */
  ios.c_cc[VINTR] = 0;
  ios.c_cc[VQUIT] = 0;
  ios.c_cc[VERASE] = 0;
  ios.c_cc[VKILL] = 0;
  ios.c_cc[VEOF] = 0;
  ios.c_cc[VMIN] = 0;
  ios.c_cc[VEOL] = 0;
  ios.c_cc[VTIME] = 0;
  ios.c_cc[VEOL2] = 0;
  ios.c_cc[VSTART] = 0;
  ios.c_cc[VSTOP] = 0;
  ios.c_cc[VSUSP] = 0;
  ios.c_cc[VLNEXT] = 0;
  ios.c_cc[VWERASE] = 0;
  ios.c_cc[VREPRINT] = 0;
  ios.c_cc[VDISCARD] = 0;

  cfsetospeed(&ios, baud);
  cfsetispeed(&ios, baud);

  port = open(portName.toAscii().data(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (port < 0)
  {
    port = -1;
    ErrorStr = QString("%1").arg(tr(strerror(errno)));
    return false;
  }

  if (tcgetattr(port, &oldios) < 0)
  {
    ErrorStr = QString(tr("Can not get TTY attributes: %1")).arg(tr(strerror(errno)));
    close(port);
    return false;
  }

  if (tcsetattr(port, TCSANOW, &ios) < 0)
  {
    ErrorStr = QString(tr("Can not set TTY attributes: %1")).arg(tr(strerror(errno)));
    close(port);
    return false;
  }

  if (ioctl(port, TIOCMGET, &mc)) {
    ErrorStr = QString(tr("Can not get modem control lines: %1")).arg(tr(strerror(errno)));
    close(port);
    return false;
  }
  oldmc = mc;
  mc &= (~(TIOCM_RTS | TIOCM_DTR));
  if (ioctl(port, TIOCMSET, &mc) < 0) {
    ErrorStr = QString(tr("Can not set modem control lines: %1")).arg(tr(strerror(errno)));
    close(port);
    fprintf(stderr, "Error 2\n");
    return false;
  }

  timer->start(5);
  ErrorStr = QString(tr("All fine."));
  emit portOpened();
  return true;
}

void Serial::closePort()
{
  if (port < 0)
    return;
  timer->stop();
  ioctl(port, TIOCMSET, &oldmc);
  tcsetattr(port, TCSANOW, &oldios);
  close(port);
  port = -1;
  ErrorStr = QString(tr("All fine."));
}

bool Serial::send(const QByteArray & d)
{
  return send(d.data(), d.size());
}

bool Serial::send(const char * buf, int size)
{
  int count = size;
  int retval;

  if (!openPort())
    return false;

  while (count)
  {
    if ((retval = write(port, &buf[size - count], count)) < 0)
    {
      if (errno == EAGAIN)
      {
        retval = 0;
      }
      else
      {
        ErrorStr = QString(tr("Can not write data to port: %1").arg(strerror(errno)));
        return false;
      }
    }
    count -= retval;
  }
  return true;
}

void Serial::slotTimer()
{
  QByteArray ba;

  if (port < 0)
    return;

  int retval;
  char d[1024];
  retval = read(port, d, 1024);
  if (retval > 0)
  {
    for (int i = 0; i < retval; i++)
      ba.append(d[i]);
  }
  if (ba.isEmpty())
    return;

  emit receive(ba);
}

/* End of file */
