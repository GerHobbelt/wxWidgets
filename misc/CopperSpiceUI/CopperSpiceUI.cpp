// CopperSpiceUI.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "CopperSpiceUI.h"
#include "MainWindow.h"

int WinMain(int argc, char* argv[])
{
   QApplication app(argc, argv);
   cMainWindow frame;

   QCommandLineParser parser;
   parser.process(app);

   const QStringList posArgs = parser.positionalArguments();
   for (const QString& fileName : posArgs) {
      frame.openFile(fileName);
   }

   frame.show();

   return app.exec();
}
