/**
 * message-handler.h
 *
 * Toke Høiland-Jensen
 * 2012-04-05
 */

#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <QtCore/QObject>

class MessageHandler : public QObject
{
  Q_OBJECT

public:
  MessageHandler();
  ~MessageHandler();

  void msgHandler(QtMsgType type, const char * msg);

signals:
  void newMessage(QString msg);

private:
  QtMsgHandler old_handler;

};

#endif
