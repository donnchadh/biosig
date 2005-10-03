// sigviewer.cpp

#include "base/user_types.h"
#include "main_window.h"
#include "main_window_model.h"
#include "base/event_table_file_reader.h"

#include <stdlib.h>

#include <QApplication>
#include <QTranslator>
#include <QTextCodec>

using BioSig_::MainWindow;
using BioSig_::MainWindowModel;

// main
int main(int32 argc, char* argv[])
{
    QApplication application(argc,argv);

    QTranslator qt_translator(0);
    qt_translator.load(QString("qt_") + QTextCodec::locale(),
                       QString(getenv("QTDIR")) + "/translations");
    application.installTranslator(&qt_translator);

    QTranslator sigviewer_translator(0);
    sigviewer_translator.load(QString("sigviewer_") + QTextCodec::locale(),
                              application.applicationDirPath());
    application.installTranslator(&sigviewer_translator);

    MainWindowModel main_window_model;
    MainWindow main_window(main_window_model);
    main_window_model.setMainWindow(&main_window);
    main_window_model.getEventTableFileReader()
        .load((application.applicationDirPath() + "/eventcodes.txt").ascii());
    main_window_model.loadSettings();
    main_window.show();
    int result = application.exec();
    main_window_model.saveSettings();

    return result;
}
