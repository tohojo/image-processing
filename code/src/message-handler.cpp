/**
 * message-handler.cpp
 *
 * Toke Høiland-Jørgensen
 * 2012-04-05
 */

#include "message-handler.h"
#include <stdio.h>

MessageHandler::MessageHandler() : QObject()
{
}

MessageHandler::~MessageHandler()
{
}

void MessageHandler::msgHandler(QtMsgType type, const char * msg)
{
  QString message;
  switch(type) {
  case QtDebugMsg:
    message = QString("Debug: %1").arg(msg);
    break;
  case QtWarningMsg:
    message = QString("Warning: %1").arg(msg);
    break;
  case QtCriticalMsg:
    message = QString("Critical: %1").arg(msg);
    break;
  case QtFatalMsg:
    message = QString("Fatal: %1").arg(msg);
    break;
  }
  fprintf(stderr, "%s\n", message.toLocal8Bit().data());
  emit newMessage(message);
}
