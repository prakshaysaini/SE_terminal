#ifndef SAFETY_FILTER_H
#define SAFETY_FILTER_H

#include <QList>
#include <QPair>
#include <QString>

struct SafetyResult {
  bool isSafe;
  QString reason;
};

inline SafetyResult isSafeCommand(const QString &cmd) {
  QString c = cmd.trimmed().toLower();

  static const QList<QPair<QString, QString>> PATTERNS = {
      {":(){:|:&};:", "Fork bomb detected"},
      {"rm -rf /", "Recursive deletion of root filesystem"},
      {"rm -rf ~", "Recursive deletion of home directory"},
      {"rm -rf /*", "Recursive deletion of root filesystem"},
      {"rm -rf *", "Recursive deletion of all files"},
      {"rm --no-preserve-root", "Root protection bypass"},
      {"mkfs", "Filesystem formatting (data destruction)"},
      {"dd if=", "Raw disk write"},
      {"> /dev/sd", "Direct write to block device"},
      {"> /dev/nvme", "Direct write to NVMe device"},
      {"mv /* ", "Moving root filesystem contents"},
      {"chmod 000 /", "Removing all permissions on root"},
      {"shutdown", "System shutdown command"},
      {"reboot", "System reboot command"},
      {"halt", "System halt command"},
      {"poweroff", "System poweroff command"},
      {"init 0", "Runlevel 0 (shutdown)"},
      {"init 6", "Runlevel 6 (reboot)"},
      {"kill -9 -1", "Kill all processes"},
      {"sudo su", "Escalating to root shell"},
      {"sudo bash", "Escalating to root bash"},
      {"sudo sh", "Escalating to root sh"},
      {"passwd root", "Changing root password"},
      {"> /etc/passwd", "Overwriting system user database"},
      {"> /etc/shadow", "Overwriting system password file"},
      {"> /etc/sudoers", "Overwriting sudoers configuration"},
      {"wget.*| bash", "Remote code execution via wget|bash"},
      {"curl.*| bash", "Remote code execution via curl|bash"},
      {"wget.*| sh", "Remote code execution via wget|sh"},
      {"curl.*| sh", "Remote code execution via curl|sh"},
  };

  for (const auto &[pattern, reason] : PATTERNS)
    if (c.contains(pattern))
      return {false, reason};

  if (c.startsWith("sudo") && (c.contains("rm -rf") || c.contains("mkfs") ||
                               c.contains("dd if=") || c.contains("> /dev/")))
    return {false, "Dangerous privileged command blocked"};

  return {true, ""};
}

#endif 
