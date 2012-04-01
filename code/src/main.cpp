#include <QtGui/QApplication>
#include <iostream>
#include "processing-gui.h"

void checkNext(QString arg, QStringList args)
{
  if(args.empty() || args.first().startsWith("-")) {
    qFatal("Missing parameter for %s argument", arg.toLocal8Bit().data());
  }
}

QMap<QString, QVariant> parseArgs(QStringList args)
{
  QMap<QString, QVariant> parsed;
  parsed.insert("properties", QVariant(QMap<QString, QVariant>()));
  args.removeFirst(); // Program name
  while(!args.empty()) {
    QString arg = args.takeFirst();
    if(arg == "-b" || arg == "--batch") {
      parsed.insert("batch", QVariant(true));
    } else if(arg == "-o" || arg == "--output") {
      checkNext(arg, args);
      parsed.insert("output", QVariant(args.takeFirst()));
    } else if(arg == "-p" || arg == "--processor") {
      checkNext(arg, args);
      parsed.insert("processor", QVariant(args.takeFirst()));
    } else if(arg.startsWith("--property-")) {
      checkNext(arg, args);
      QString prop = arg.replace("--property-", "");
      QMap<QString, QVariant> properties = parsed.value("properties").toMap();
      properties.insert(prop, QVariant(args.takeFirst()));
      parsed.insert("properties", properties);
    } else if(arg.startsWith("-")) {
        qFatal("Unrecognised command line argument: %s", arg.toLocal8Bit().data());
    } else {
      parsed.insert("input", QVariant(arg));
    }
  }
  return parsed;
}


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QMap<QString, QVariant> args = parseArgs(app.arguments());

  ProcessingGUI ui;
  ui.show();
  ui.set_args(args);

  return app.exec();
}
