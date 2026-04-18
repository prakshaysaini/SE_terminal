#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName("SE Terminal");
  app.setApplicationVersion("2.0.0");

  MainWindow window;
  window.show();
  
  return app.exec();
}