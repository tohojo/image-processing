#include <QtGui/QApplication>
#include <upstream>
#include "processing-gui.h"

/**
 * Helper function to check that the argument list contains one more
 * argument. Aborts with an error if it does not.
 *
 * Used to check arguments that need a value (e.g. --processor NAME).
 */
void checkNext(QString arg, QStringList args)
{
  if(args.empty() || args.first().startsWith("-")) {
    qFatal("Missing parameter for %s argument", arg.toLocal8Bit().data());
  }
}

/**
 * Parse command line arguments into a QMap.
 *
 * Parses arguments of the following form:
 *
 * -b / --batch: Batch mode. Currently not used.
 *
 * -o FILE / --output FILE: Place to write output (in batch mode).
 *                          Currently not used.
 *
 * -p NAME / --processor NAME: Load processor NAME on load.
 *
 * --property-NAME VALUE: Set property NAME to VALUE for selected processor.
 */
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
