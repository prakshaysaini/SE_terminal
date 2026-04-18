#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

private slots:
  void addNewTab();
  void closeTab(int index);

private:
  QTabWidget *tabWidget;
  int tabCounter;
};

#endif // MAINWINDOW_H
