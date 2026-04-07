#include "TerminalUI.h"
#include "LocalLLMService.h"
#include "DataAccessLayer.h"
#include "SafetyFilter.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QKeyEvent>
#include <QProcess>
#include <QTextBlock>
#include <QTextCursor>

TerminalUI::TerminalUI(QWidget *parent) : QPlainTextEdit(parent) {
  setStyleSheet(
      "background-color:#0d1117; color:#00ff88;"
      "font-family:'JetBrains Mono','Fira Code','Cascadia Code','Monospace';"
      "font-size:12pt; border:none; padding:8px;");
  setLineWrapMode(QPlainTextEdit::WidgetWidth);

  currentDir = QDir::homePath();

  // Initialize Data Access Layer
  dal = new DataAccessLayer("terminal_app.db");
  if (!dal->initializeDatabase()) {
    appendPlainText("[WARNING] Failed to initialize database: " + dal->getLastError());
  }

  // Create a new session
  currentSessionId = dal->createSession("Terminal Session", currentDir);
  if (currentSessionId != -1) {
    dal->addSystemLog("INFO", "Session started", "TerminalUI", currentSessionId);
  }

  llm = new LocalLLMService(this);
  connect(llm, &LocalLLMService::commandReady, this,
          &TerminalUI::onCommandReady);
  connect(llm, &LocalLLMService::errorOccurred, this,
          &TerminalUI::onOllamaError);
  connect(llm, &LocalLLMService::availabilityChecked, this,
          &TerminalUI::onAvailabilityChecked);

  printBanner();
  llm->checkAvailability();
}

void TerminalUI::printBanner() {
  appendPlainText("       SE TERMINAL  ·  AI-POWERED LINUX TERMINAL        ");
  appendPlainText("       Model: Llama 3.1 8B                            ");
}

void TerminalUI::newPrompt() {
  moveCursor(QTextCursor::End);
  insertPlainText("\n[" + shortCwd() + "]$ ");
  moveCursor(QTextCursor::End);
  ensureCursorVisible();
}

void TerminalUI::appendLine(const QString &text) {
  moveCursor(QTextCursor::End);
  appendPlainText(text);
}

void TerminalUI::keyPressEvent(QKeyEvent *event) {
  if (inputLocked) {
    if (event->key() == Qt::Key_C &&
        (event->modifiers() & Qt::ControlModifier)) {
      inputLocked = false;
      appendLine("\n[Cancelled]");
      newPrompt();
    }
    return;
  }

  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
    QString line = textCursor().block().text();
    int dol = line.indexOf("$ ");
    if (dol != -1)
      line = line.mid(dol + 2);
    onCommandSubmitted(line.trimmed());
    return;
  }

  QTextCursor cur = textCursor();
  QString lineText = cur.block().text();
  int dol = lineText.indexOf("$ ");
  int promptLen = (dol != -1) ? dol + 2 : 2;

  if ((event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Left) &&
      cur.columnNumber() <= promptLen)
    return;

  if (event->key() == Qt::Key_Home) {
    cur.movePosition(QTextCursor::StartOfLine);
    cur.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, promptLen);
    setTextCursor(cur);
    return;
  }

  QPlainTextEdit::keyPressEvent(event);
}

void TerminalUI::onCommandSubmitted(const QString &raw) {
  if (raw.isEmpty()) {
    newPrompt();
    return;
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
    newPrompt();
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
    newPrompt();
    return;
  }

  QProcess proc;
  proc.setWorkingDirectory(currentDir);
  proc.start("bash", {"-c", cmd});
  if (!proc.waitForFinished(15000)) {
    proc.kill();
    appendLine("[ERROR] Command timed out after 15 seconds.");
    newPrompt();
    return;
  }

  QString out = QString::fromLocal8Bit(proc.readAllStandardOutput()).trimmed();
  QString err = QString::fromLocal8Bit(proc.readAllStandardError()).trimmed();

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
               "    Then: ollama pull llama3.1:8b");
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
