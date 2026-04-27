#include "TerminalUI.h"
#include "DataAccessLayer.h"
#include "LocalLLMService.h"
#include "SafetyFilter.h"

#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QDir>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMenu>
#include <QProcess>
#include <QSysInfo>
#include <QTextBlock>
#include <QTextCursor>

TerminalUI::TerminalUI(QWidget *parent) : QPlainTextEdit(parent) {
  highlighter = new TerminalHighlighter(document());
  applyTheme();
  setLineWrapMode(QPlainTextEdit::WidgetWidth);
  setUndoRedoEnabled(false);

  currentDir = QDir::homePath();

  // Initialize Data Access Layer
  dal = new DataAccessLayer("terminal_app.db");
  if (!dal->initializeDatabase()) {
    appendPlainText("[WARNING] Failed to initialize database: " +
                    dal->getLastError());
  }

  // Create a new session
  currentSessionId = dal->createSession("Terminal Session", currentDir);
  if (currentSessionId != -1) {
    dal->addSystemLog("INFO", "Session started", "TerminalUI",
                      currentSessionId);
  }

  llm = new LocalLLMService(this);
  connect(llm, &LocalLLMService::commandReady, this,
          &TerminalUI::onCommandReady);
  connect(llm, &LocalLLMService::errorOccurred, this,
          &TerminalUI::onOllamaError);
  connect(llm, &LocalLLMService::availabilityChecked, this,
          &TerminalUI::onAvailabilityChecked);

  // printBanner();
  llm->checkAvailability();
}

void TerminalUI::applyTheme() {
  if (isDarkTheme) {
    setStyleSheet(
        "background-color:#0d1117; color:#c9d1d9;"
        "font-family:'JetBrains Mono','Fira Code','Cascadia Code','Monospace';"
        "font-size:12pt; border:none; padding:8px;");
  } else {
    setStyleSheet(
        "background-color:#ffffff; color:#24292e;"
        "font-family:'JetBrains Mono','Fira Code','Cascadia Code','Monospace';"
        "font-size:12pt; border:none; padding:8px;");
  }
  highlighter->setTheme(isDarkTheme);
}

void TerminalUI::toggleTheme() {
  isDarkTheme = !isDarkTheme;
  applyTheme();
}

void TerminalUI::printBanner() {
  appendPlainText("       SE TERMINAL  ·  AI-POWERED LINUX TERMINAL        ");
  appendPlainText("       Model: Llama 3.1 8B                            ");
}

void TerminalUI::newPrompt() {
  QString user = qEnvironmentVariable("USER");
  QString host = QSysInfo::machineHostName();
  if (user.isEmpty())
    user = "user";
  if (host.isEmpty())
    host = "localhost";

  QString prompt = user + "@" + host + ":" + shortCwd() + "$ ";

  moveCursor(QTextCursor::End);
  insertPlainText("\n" + prompt);
  moveCursor(QTextCursor::End);
  ensureCursorVisible();
}

void TerminalUI::appendLine(const QString &text) {
  moveCursor(QTextCursor::End);
  appendPlainText(text);
}

void TerminalUI::replaceCurrentLine(const QString &text) {
  QTextCursor cur = textCursor();
  cur.movePosition(QTextCursor::StartOfBlock);

  QString lineText = cur.block().text();
  int dol = lineText.indexOf("$ ");
  int promptLen = (dol != -1) ? dol + 2 : 2;

  cur.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, promptLen);
  cur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
  cur.removeSelectedText();

  cur.insertText(text);
  setTextCursor(cur);
}

void TerminalUI::keyPressEvent(QKeyEvent *event) {
  setTextCursor(textCursor());

  if (inputLocked) {
    if (event->key() == Qt::Key_C &&
        (event->modifiers() & Qt::ControlModifier)) {
      inputLocked = false;
      appendLine("\n[Cancelled]");
      newPrompt();
    }
    return;
  }

  if (event->key() == Qt::Key_Tab) {
    event->accept();
    handleAutocomplete();
    return;
  } else {
    lastKeyWasTab = false;
  }

  //  HANDLE HISTORY FIRST (CRITICAL)

  if (event->key() == Qt::Key_Up) {
    event->accept();
    setTextCursor(textCursor());

    // Always move cursor to last line
    if (!textCursor().atEnd()) {
      moveCursor(QTextCursor::End);
    }

    if (history.empty())
      return;

    // Save current input before browsing history
    if (historyIndex == static_cast<int>(history.size())) {
      QString lineText = textCursor().block().text();
      int dol = lineText.indexOf("$ ");
      currentBuffer = (dol != -1) ? lineText.mid(dol + 2) : "";
    }

    if (historyIndex > 0) {
      historyIndex--;
      replaceCurrentLine(history[historyIndex]);
    }

    return;
  }

  if (event->key() == Qt::Key_Down) {
    event->accept();
    setTextCursor(textCursor());

    //  Correct fix
    if (!textCursor().atEnd()) {
      moveCursor(QTextCursor::End);
    }

    if (history.empty())
      return;

    if (historyIndex < static_cast<int>(history.size()) - 1) {
      historyIndex++;
      replaceCurrentLine(history[historyIndex]);
    } else {
      historyIndex = static_cast<int>(history.size());
      replaceCurrentLine(currentBuffer);
    }

    return;
  }

  // ENTER KEY
  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
    moveCursor(QTextCursor::End);
    QString line = textCursor().block().text();
    int dol = line.indexOf("$ ");
    if (dol != -1)
      line = line.mid(dol + 2);
    onCommandSubmitted(line.trimmed());
    return;
  }

  QTextCursor cur = textCursor();
  bool isLastBlock = (cur.block() == document()->lastBlock());

  QString lastLineText = document()->lastBlock().text();
  int dol = lastLineText.indexOf("$ ");
  int promptLen = (dol != -1) ? dol + 2 : 2;

  // Identify if it's a modification action
  bool isModification = false;
  if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)
    isModification = true;
  else if (event->matches(QKeySequence::Paste) ||
           event->matches(QKeySequence::Cut))
    isModification = true;
  else if (!event->text().isEmpty() &&
           !(event->modifiers() &
             (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)))
    isModification = true;

  if (!isLastBlock) {
    if (event->matches(QKeySequence::Copy)) {
      QPlainTextEdit::keyPressEvent(event);
      return;
    }
    if (event->matches(QKeySequence::Cut)) {
      copy(); // Treat cut as copy in read-only areas
      return;
    }
    if (isModification) {
      if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)
        return; // ignore
      moveCursor(QTextCursor::End);
      cur = textCursor();
      isLastBlock = true; // We moved to the last block
    } else {
      QPlainTextEdit::keyPressEvent(event);
      return;
    }
  }

  int col = cur.columnNumber();

  if (isModification) {
    if (cur.hasSelection()) {
      int selStart = cur.selectionStart();
      QTextCursor promptCur = textCursor();
      promptCur.movePosition(QTextCursor::StartOfBlock);
      promptCur.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor,
                             promptLen);
      if (selStart < promptCur.position()) {
        cur.clearSelection();
        setTextCursor(cur);
        return; // ignore modification if selection includes prompt
      }
    } else {
      if (col < promptLen) {
        if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)
          return;
        moveCursor(QTextCursor::End);
        cur = textCursor();
      } else if (col == promptLen) {
        if (event->key() == Qt::Key_Backspace)
          return;
      }
    }
  }

  // Prevent left arrow into the prompt area
  if (event->key() == Qt::Key_Left && !cur.hasSelection() &&
      cur.columnNumber() <= promptLen) {
    return;
  }

  // HOME key behavior
  if (event->key() == Qt::Key_Home) {
    cur.movePosition(QTextCursor::StartOfLine);
    cur.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, promptLen);
    setTextCursor(cur);
    return;
  }

  // Default behavior
  QPlainTextEdit::keyPressEvent(event);
}

void TerminalUI::onCommandSubmitted(const QString &raw) {
  if (raw.isEmpty()) {
    newPrompt();
    return;
  }

  // appendLine("DEBUG: " + raw);

  if (!raw.isEmpty()) {
    // Store history (avoid duplicates)
    if (history.empty() || history.back() != raw) {
      history.push_back(raw);
    }
    historyIndex = history.size();
  }

  if (raw == "clear") {
    clear();
    newPrompt();
    return;
  }
  if (raw == "exit" || raw == "quit") {
    QApplication::quit();
    return;
  }
  if (raw == "help") {
    appendLine(helpText());
    newPrompt();
    return;
  }
  if (raw == "pwd") {
    appendLine(currentDir);
    newPrompt();
    return;
  }
  if (raw == "cd" || raw.startsWith("cd ")) {
    handleCd(raw);
    return;
  }

  if (raw.startsWith("!")) {
    execute(raw.mid(1).trimmed());
    return;
  }

  if (isShellCommand(raw)) {
    execute(raw);
    return;
  }

  pendingInput = raw;
  inputLocked = true;
  appendLine("[AI]    Thinking...");
  llm->generateCommand(raw, currentDir);
}

void TerminalUI::handleCd(const QString &raw) {
  QString arg = raw.mid(2).trimmed();

  if (arg.isEmpty() || arg == "~")
    arg = QDir::homePath();
  else if (arg.startsWith("~/"))
    arg = QDir::homePath() + arg.mid(1);

  QDir target(arg.startsWith('/') ? arg : currentDir + "/" + arg);
  QString resolved = QDir::cleanPath(target.absolutePath());

  QFileInfo fi(resolved);
  if (!fi.exists()) {
    appendLine("[ERROR] cd: " + arg + ": No such file or directory");
    newPrompt();
    return;
  }
  if (!fi.isDir()) {
    appendLine("[ERROR] cd: " + arg + ": Not a directory");
    newPrompt();
    return;
  }
  if (!fi.isReadable()) {
    appendLine("[ERROR] cd: " + arg + ": Permission denied");
    newPrompt();
    return;
  }

  currentDir = resolved;
  newPrompt();
}

void TerminalUI::execute(const QString &cmd) {
  SafetyResult check = isSafeCommand(cmd);
  if (!check.isSafe) {
    appendLine("[BLOCKED] " + check.reason + "\nCommand: " + cmd);
    // Record blocked command in database
    if (currentSessionId != -1) {
      int cmdId = dal->recordCommand(currentSessionId, pendingInput.isEmpty() ? cmd : pendingInput, cmd, false, check.reason);
      if (cmdId != -1) {
        dal->updateCommandStatus(cmdId, "BLOCKED");
      }
    }
    newPrompt();
    return;
  }

  QProcess proc;
  proc.setWorkingDirectory(currentDir);
  proc.start("bash", {"-c", cmd});
  if (!proc.waitForFinished(15000)) {
    proc.kill();
    appendLine("[ERROR] Command timed out after 15 seconds.");
    // Record timed-out command in database
    if (currentSessionId != -1) {
      int cmdId = dal->recordCommand(currentSessionId, pendingInput.isEmpty() ? cmd : pendingInput, cmd, true);
      if (cmdId != -1) {
        dal->updateCommandStatus(cmdId, "FAILED");
        dal->updateCommandExecution(cmdId, "", "Command timed out", -1, 15000);
      }
    }
    newPrompt();
    return;
  }

  QString out = QString::fromLocal8Bit(proc.readAllStandardOutput()).trimmed();
  QString err = QString::fromLocal8Bit(proc.readAllStandardError()).trimmed();

  // Record successful command in database
  if (currentSessionId != -1) {
    int cmdId = dal->recordCommand(currentSessionId, pendingInput.isEmpty() ? cmd : pendingInput, cmd, true);
    if (cmdId != -1) {
      QString status = (proc.exitCode() == 0) ? "EXECUTED" : "FAILED";
      dal->updateCommandStatus(cmdId, status);
      dal->updateCommandExecution(cmdId, out, err, proc.exitCode(), 0);
    }
  }

  if (!out.isEmpty())
    appendLine(out);
  if (!err.isEmpty())
    appendLine(err);
  newPrompt();
}

QString TerminalUI::shortCwd() const {
  QString home = QDir::homePath();
  if (currentDir == home)
    return "~";
  if (currentDir.startsWith(home + "/"))
    return "~" + currentDir.mid(home.length());
  return currentDir;
}

void TerminalUI::onCommandReady(const QString &cmd) {
  inputLocked = false;
  appendLine("[AI]    Generated: " + cmd);
  execute(cmd);
}

void TerminalUI::onOllamaError(const QString &msg) {
  inputLocked = false;
  appendLine("[ERROR] " + msg);
  newPrompt();
}

void TerminalUI::onAvailabilityChecked(bool ok, const QString &modelName) {
  if (ok)
    appendLine("[INFO]  Ollama ready — model: " + modelName);
  else
    appendLine("[INFO]  Ollama not found.\n"
               "    Run: ollama serve\n"
               "    Then: ollama pull deepseek-coder:1.3b");
  newPrompt();
}

bool TerminalUI::isShellCommand(const QString &input) {
  if (input.startsWith('/') || input.startsWith("./") || input.startsWith("~/"))
    return true;
  if (!input.contains(' ') && input.contains('='))
    return true;
  QString first = input.split(' ', Qt::SkipEmptyParts).first().toLower();
  return SHELL_COMMANDS.contains(first);
}

QString TerminalUI::helpText() {
  return QString(
    "\n╔══════════════════════════════════════════════════════════════╗\n"
    "║            SE TERMINAL — HELP & COMMANDS                   ║\n"
    "╠══════════════════════════════════════════════════════════════╣\n"
    "║  BUILT-IN COMMANDS:                                        ║\n"
    "║    help      — Show this help message                      ║\n"
    "║    clear     — Clear the terminal screen                   ║\n"
    "║    exit/quit — Close the terminal                          ║\n"
    "║    pwd       — Print current working directory              ║\n"
    "║    cd <dir>  — Change directory                            ║\n"
    "║                                                            ║\n"
    "║  AI MODE:                                                  ║\n"
    "║    Type any natural language instruction and the AI will   ║\n"
    "║    translate it to a bash command automatically.           ║\n"
    "║    Example: 'show all files' → ls -la                     ║\n"
    "║                                                            ║\n"
    "║  SHELL COMMANDS:                                           ║\n"
    "║    All standard Linux commands (ls, cat, grep, etc.)       ║\n"
    "║    are executed directly in the shell.                     ║\n"
    "║                                                            ║\n"
    "║  FORCE SHELL:                                              ║\n"
    "║    Prefix with ! to force shell execution.                 ║\n"
    "║    Example: !echo Hello World                              ║\n"
    "║                                                            ║\n"
    "║  SHORTCUTS:                                                ║\n"
    "║    Ctrl+T    — New tab                                     ║\n"
    "║    Ctrl+W    — Close tab                                   ║\n"
    "║    Ctrl+C    — Cancel AI generation                        ║\n"
    "║    Tab       — Autocomplete commands/files                 ║\n"
    "║    Up/Down   — Browse command history                      ║\n"
    "║    Right-click — Context menu (copy, paste, theme)         ║\n"
    "╚══════════════════════════════════════════════════════════════╝"
  );
}

const QSet<QString> TerminalUI::SHELL_COMMANDS = {
    "ls",      "ll",         "la",       "dir",       "pwd",      "mkdir",
    "touch",   "rm",         "rmdir",    "mv",        "cp",       "ln",
    "cat",     "tac",        "bat",      "more",      "less",     "head",
    "tail",    "file",       "stat",     "tree",      "grep",     "egrep",
    "fgrep",   "rg",         "sed",      "awk",       "cut",      "sort",
    "uniq",    "wc",         "tr",       "diff",      "patch",    "strings",
    "xxd",     "hexdump",    "find",     "locate",    "which",    "whereis",
    "type",    "man",        "info",     "echo",      "printf",   "export",
    "env",     "printenv",   "set",      "unset",     "alias",    "eval",
    "exec",    "true",       "false",    "test",      "bash",     "sh",
    "zsh",     "fish",       "python",   "python3",   "node",     "ruby",
    "perl",    "php",        "ps",       "top",       "htop",     "kill",
    "killall", "pkill",      "pgrep",    "jobs",      "fg",       "bg",
    "nohup",   "sleep",      "wait",     "nice",      "time",     "timeout",
    "watch",   "uname",      "hostname", "whoami",    "id",       "groups",
    "uptime",  "date",       "cal",      "lsof",      "dmesg",    "lscpu",
    "lsblk",   "lspci",      "lsusb",    "free",      "df",       "du",
    "mount",   "umount",     "fdisk",    "blkid",     "fsck",     "sync",
    "curl",    "wget",       "ssh",      "scp",       "sftp",     "rsync",
    "ping",    "traceroute", "ifconfig", "ip",        "route",    "netstat",
    "ss",      "nmap",       "nc",       "dig",       "nslookup", "tar",
    "zip",     "unzip",      "gzip",     "gunzip",    "bzip2",    "xz",
    "7z",      "apt",        "apt-get",  "dpkg",      "pip",      "pip3",
    "npm",     "yarn",       "cargo",    "gem",       "git",      "svn",
    "make",    "cmake",      "gcc",      "g++",       "clang",    "java",
    "javac",   "docker",     "kubectl",  "gdb",       "valgrind", "vim",
    "vi",      "nvim",       "nano",     "emacs",     "code",     "sudo",
    "su",      "chmod",      "chown",    "chgrp",     "umask",    "systemctl",
    "service", "journalctl", "crontab",  "bc",        "expr",     "tee",
    "xargs",   "base64",     "md5sum",   "sha256sum", "openssl",  "screen",
    "tmux",    "ffmpeg",     "strace",   "source",
};

void TerminalUI::handleAutocomplete() {
  moveCursor(QTextCursor::End);
  QString lineText = textCursor().block().text();
  int dol = lineText.indexOf("$ ");
  if (dol == -1)
    return;

  QString buffer = lineText.mid(dol + 2);
  if (buffer.isEmpty())
    return;

  QStringList parts = buffer.split(" ", Qt::SkipEmptyParts);
  bool isCommand = !buffer.contains(" ");
  QString currentWord = parts.isEmpty() ? "" : parts.last();

  if (buffer.endsWith(" ")) {
    isCommand = false;
    currentWord = "";
  }

  QSet<QString> matchSet;

  if (isCommand) {
    for (const QString &cmd : SHELL_COMMANDS) {
      if (cmd.startsWith(currentWord)) {
        matchSet.insert(cmd);
      }
    }

    QString pathEnv = qEnvironmentVariable("PATH");
    QStringList paths = pathEnv.split(":");
    for (const QString &dir : paths) {
      QDir d(dir);
      if (!d.exists())
        continue;
      QStringList filters;
      filters << currentWord + "*";
      QStringList files = d.entryList(filters, QDir::Files | QDir::Executable);
      for (const QString &file : files) {
        matchSet.insert(file);
      }
    }
  } else {
    QString searchPath = currentDir;
    QString prefix = currentWord;

    int lastSlash = currentWord.lastIndexOf('/');
    if (lastSlash != -1) {
      QString dirPart = currentWord.left(lastSlash);
      prefix = currentWord.mid(lastSlash + 1);

      if (dirPart.startsWith("~")) {
        dirPart.replace(0, 1, QDir::homePath());
      }
      if (dirPart.startsWith("/")) {
        searchPath = dirPart;
      } else {
        searchPath = currentDir + "/" + dirPart;
      }
    }

    QDir d(searchPath);
    if (d.exists()) {
      QStringList filters;
      filters << prefix + "*";
      QStringList files =
          d.entryList(filters, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
      for (const QString &file : files) {
        matchSet.insert(file);
      }
    }
  }

  if (matchSet.isEmpty())
    return;

  QStringList matches = matchSet.values();
  matches.sort();

  if (matches.size() == 1) {
    QString match = matches.first();
    QString prefixStr = isCommand
                            ? currentWord
                            : currentWord.mid(currentWord.lastIndexOf('/') + 1);
    QString remainder = match.mid(prefixStr.length());

    if (!isCommand) {
      QString dirPart = currentWord.left(currentWord.lastIndexOf('/') + 1);
      QString fullPath = dirPart + match;
      QString searchPath = currentDir;
      if (fullPath.startsWith("~"))
        fullPath.replace(0, 1, QDir::homePath());

      if (fullPath.startsWith("/"))
        searchPath = fullPath;
      else
        searchPath = currentDir + "/" + fullPath;

      if (QFileInfo(searchPath).isDir())
        remainder += "/";
      else
        remainder += " ";
    } else {
      remainder += " ";
    }

    insertPlainText(remainder);
    lastKeyWasTab = false;
  } else {
    QString commonPrefix = matches.first();
    for (const QString &m : matches) {
      int i = 0;
      while (i < commonPrefix.length() && i < m.length() &&
             commonPrefix[i] == m[i]) {
        i++;
      }
      commonPrefix = commonPrefix.left(i);
    }

    QString prefixStr = isCommand
                            ? currentWord
                            : currentWord.mid(currentWord.lastIndexOf('/') + 1);

    if (commonPrefix.length() > prefixStr.length()) {
      QString remainder = commonPrefix.mid(prefixStr.length());
      insertPlainText(remainder);
      lastKeyWasTab = false;
    } else {
      if (lastKeyWasTab) {
        appendLine("\n" + matches.join("  "));
        newPrompt();
        insertPlainText(buffer);
        lastKeyWasTab = false;
      } else {
        lastKeyWasTab = true;
      }
    }
  }
}

void TerminalUI::contextMenuEvent(QContextMenuEvent *event) {
  QMenu *menu = new QMenu(this);

  QAction *copyAction = menu->addAction("Copy");
  connect(copyAction, &QAction::triggered, this, &TerminalUI::copy);
  copyAction->setEnabled(textCursor().hasSelection());

  QAction *pasteAction = menu->addAction("Paste");
  connect(pasteAction, &QAction::triggered, this, [this]() {
    moveCursor(QTextCursor::End);
    this->paste();
  });
  pasteAction->setEnabled(canPaste());

  menu->addSeparator();

  QAction *themeAction = menu->addAction(isDarkTheme ? "Switch to Light Theme"
                                                     : "Switch to Dark Theme");
  connect(themeAction, &QAction::triggered, this, &TerminalUI::toggleTheme);

  QAction *clearAction = menu->addAction("Clear");
  connect(clearAction, &QAction::triggered, this, [this]() {
    this->clear();
    this->newPrompt();
  });

  menu->exec(event->globalPos());
  delete menu;
}
