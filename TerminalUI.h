#ifndef TERMINAL_UI_H
#define TERMINAL_UI_H

#include <QPlainTextEdit>
#include <QSet>

class LocalLLMService;

class TerminalUI : public QPlainTextEdit {
  Q_OBJECT

public:
  explicit TerminalUI(QWidget *parent = nullptr);

private:
  LocalLLMService *llm;
  QString currentDir;
  QString pendingInput;
  bool inputLocked = false;

  static const QSet<QString> SHELL_COMMANDS;

  void printBanner();
  void newPrompt();
  void appendLine(const QString &text);

  void onCommandSubmitted(const QString &raw);
  void handleCd(const QString &raw);
  void execute(const QString &cmd);
  QString shortCwd() const;

  static bool isShellCommand(const QString &input);
  static QString helpText();

protected:
  void keyPressEvent(QKeyEvent *event) override;

private slots:
  void onCommandReady(const QString &cmd);
  void onOllamaError(const QString &msg);
  void onAvailabilityChecked(bool ok, const QString &modelName);
};

#endif // TERMINAL_UI_H
