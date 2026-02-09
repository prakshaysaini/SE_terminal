#include <QApplication>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <QKeyEvent>
#include <QProcess>
#include <QTextCursor>
#include <QTextBlock>
#include <QDateTime>
#include <vector>
#include <memory>

struct ProcessResult {
    QString output;
    QString error;
    int exitCode;
};

class CommandExecutor {
public:
    ProcessResult execute(QString command) {
        QProcess process;
        process.start("bash", QStringList() << "-c" << command);
        process.waitForFinished();
        
        return {
            QString::fromLocal8Bit(process.readAllStandardOutput()).trimmed(),
            QString::fromLocal8Bit(process.readAllStandardError()).trimmed(),
            process.exitCode()
        };
    }
};

class CommandListener {
public:
    virtual ~CommandListener() = default;
    virtual void onCommandSubmitted(QString cmd) = 0;
};

class TerminalUI : public QPlainTextEdit {
private:
    CommandListener* listener = nullptr;

public:
    TerminalUI(QWidget *parent = nullptr) : QPlainTextEdit(parent) {
        setStyleSheet("background-color: #1e1e1e; color: #00ff00; font-family: 'Monospace'; font-size: 12pt;");
        appendPlainText("SE Terminal NLP-Ready");
        newPrompt();
    }

    void setListener(CommandListener* l) { listener = l; }

    void displayOutput(QString text) {
        if (!text.isEmpty()) appendPlainText(text);
        newPrompt();
    }

    void newPrompt() {
        moveCursor(QTextCursor::End);
        insertPlainText("\n$ ");
    }

protected:
    void keyPressEvent(QKeyEvent *e) override {
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
            QString command = textCursor().block().text().simplified();
            if (command.startsWith("$")) command = command.mid(1).trimmed();

            if (listener) listener->onCommandSubmitted(command);
            return;
        }
        
        if (e->key() == Qt::Key_Backspace && textCursor().columnNumber() <= 2) return;
        QPlainTextEdit::keyPressEvent(e);
    }
};

class CommandController : public CommandListener {
private:
    TerminalUI* ui;
    std::unique_ptr<CommandExecutor> executor;

public:
    CommandController(TerminalUI* terminal) 
        : ui(terminal), executor(std::make_unique<CommandExecutor>()) {}

    void onCommandSubmitted(QString cmd) override {
        if (cmd.isEmpty()) {
            ui->newPrompt();
            return;
        }

        ProcessResult result = executor->execute(cmd);

        if (!result.error.isEmpty()) {
            ui->displayOutput("Error: " + result.error);
        } else {
            ui->displayOutput(result.output);
        }
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QWidget window;
    window.setWindowTitle("SE Terminal - Refactored");
    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->setContentsMargins(0, 0, 0, 0);

    TerminalUI *ui = new TerminalUI();
    CommandController controller(ui);
    ui->setListener(&controller);

    layout->addWidget(ui);
    window.resize(900, 600);
    window.show();
    
    return app.exec();
}