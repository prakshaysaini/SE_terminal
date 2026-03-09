#include "TerminalUI.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName("SE Terminal");
  app.setApplicationVersion("2.0.0");

  QWidget window;
  window.setWindowTitle("SE Terminal — AI-Powered Linux Shell ");
  window.setMinimumSize(900, 600);

  QVBoxLayout *layout = new QVBoxLayout(&window);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  TerminalUI *terminal = new TerminalUI();
  layout->addWidget(terminal);

  window.resize(1050, 680);
  window.show();
  return app.exec();
}