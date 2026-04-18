#include "MainWindow.h"
#include "TerminalUI.h"
#include <QTabBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QShortcut>
#include <QKeySequence>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), tabCounter(0) {
    
    setWindowTitle("SE Terminal — AI-Powered Linux Shell");
    setMinimumSize(900, 600);
    resize(1050, 680);

    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    setCentralWidget(tabWidget);

    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    // Add a '+' button to the tab bar
    QToolButton *newTabButton = new QToolButton(this);
    newTabButton->setText("+");
    newTabButton->setToolTip("Open a new terminal tab");
    connect(newTabButton, &QToolButton::clicked, this, &MainWindow::addNewTab);
    tabWidget->setCornerWidget(newTabButton, Qt::TopRightCorner);

    // Add shortcuts
    QShortcut *newTabShortcut = new QShortcut(QKeySequence("Ctrl+T"), this);
    connect(newTabShortcut, &QShortcut::activated, this, &MainWindow::addNewTab);

    QShortcut *closeTabShortcut = new QShortcut(QKeySequence("Ctrl+W"), this);
    connect(closeTabShortcut, &QShortcut::activated, this, [this]() {
        if (tabWidget->currentIndex() != -1) {
            closeTab(tabWidget->currentIndex());
        }
    });

    // Create the first tab
    addNewTab();
}

void MainWindow::addNewTab() {
    tabCounter++;
    TerminalUI *terminal = new TerminalUI(this);
    int index = tabWidget->addTab(terminal, QString("Terminal %1").arg(tabCounter));
    tabWidget->setCurrentIndex(index);
    terminal->setFocus();
}

void MainWindow::closeTab(int index) {
    QWidget *widget = tabWidget->widget(index);
    tabWidget->removeTab(index);
    widget->deleteLater();

    if (tabWidget->count() == 0) {
        close(); // Close the application if all tabs are closed
    }
}
