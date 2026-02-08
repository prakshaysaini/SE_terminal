#include <QApplication>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <QKeyEvent>
#include <QProcess>
#include <QTextCursor>
#include <QTextBlock> 

class SimpleTerminal : public QPlainTextEdit {
public:
    SimpleTerminal(QWidget *parent = nullptr) : QPlainTextEdit(parent) {
        setStyleSheet("background-color: #1e1e1e; color: #00ff00; font-family: 'Monospace'; font-size: 12pt;");
        appendPlainText("SE Terminal NLP-Ready");
        newPrompt();
    }

private:
    void newPrompt() {
        moveCursor(QTextCursor::End);
        insertPlainText("\n$ ");
    }

protected:
    void keyPressEvent(QKeyEvent *e) override {
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
            QTextCursor cursor = textCursor();
            QString currentLine = cursor.block().text();
            
            QString command = currentLine.simplified();
            if (command.startsWith("$")) {
                command = command.mid(1).trimmed();
            }

            if (!command.isEmpty()) {
                QProcess process;
                process.start("bash", QStringList() << "-c" << command);
                process.waitForFinished();
                
                QString output = process.readAllStandardOutput();
                QString error = process.readAllStandardError();

                if (!output.isEmpty()) appendPlainText(output.trimmed());
                if (!error.isEmpty()) appendPlainText("Error: " + error.trimmed());
            }
            
            newPrompt();
            return;
        }
        
        if (e->key() == Qt::Key_Backspace && textCursor().columnNumber() <= 2) {
            return;
        }

        QPlainTextEdit::keyPressEvent(e);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QWidget window;
    window.setWindowTitle("SE Terminal");
    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->setContentsMargins(0, 0, 0, 0);

    SimpleTerminal *term = new SimpleTerminal();
    layout->addWidget(term);
    
    window.resize(900, 600);
    window.show();
    
    return app.exec();
}