#ifndef TERMINAL_UI_H
#define TERMINAL_UI_H

#include "TerminalHighlighter.h"
#include <QPlainTextEdit>
#include <QSet>
#include <vector>

class LocalLLMService;
class DataAccessLayer;

class TerminalUI : public QPlainTextEdit {
  Q_OBJECT

public:
  explicit TerminalUI(QWidget *parent = nullptr);

private:
  LocalLLMService *llm;
  DataAccessLayer *dal;
  QString currentDir;
  QString pendingInput;
  bool inputLocked = false;
  int currentSessionId = -1;

  static const QSet<QString> SHELL_COMMANDS;

  //  History
  std::vector<QString> history;
  int historyIndex = 0;
  QString currentBuffer;

  //  Theming & Syntax Highlighting
  TerminalHighlighter *highlighter;
  bool isDarkTheme = true;
  void applyTheme();

  //  UI helpers
  void printBanner();
  void newPrompt();
  void appendLine(const QString &text);
  void replaceCurrentLine(const QString &text);

  //  Autocomplete
  void handleAutocomplete();
  bool lastKeyWasTab = false;

  //  Core logic
  void onCommandSubmitted(const QString &raw);
  void handleCd(const QString &raw);
  void execute(const QString &cmd);
  QString shortCwd() const;

  static bool isShellCommand(const QString &input);
  static QString helpText();

protected:
  void keyPressEvent(QKeyEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
  void onCommandReady(const QString &cmd);
  void onOllamaError(const QString &msg);
  void onAvailabilityChecked(bool ok, const QString &modelName);
  void toggleTheme();
};

#endif