#include "LocalLLMService.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrl>

LocalLLMService::LocalLLMService(QObject *parent, const QString &model,
                                 const QString &url)
    : QObject(parent), net(new QNetworkAccessManager(this)), model(model),
      baseUrl(url) {}

void LocalLLMService::generateCommand(const QString &input,
                                      const QString &cwd) {
  QJsonObject body;
  body["model"] = model;
  body["prompt"] = buildPrompt(input, cwd);
  body["stream"] = false;
  body["options"] = QJsonObject{{"temperature", 0.05}, {"num_predict", 128}};

  QNetworkRequest req(QUrl(baseUrl + "/api/generate"));
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  req.setTransferTimeout(30000);

  QNetworkReply *reply = net->post(req, QJsonDocument(body).toJson());
  connect(reply, &QNetworkReply::finished, this,
          [this, reply] { handleReply(reply); });
}

void LocalLLMService::checkAvailability() {
  QNetworkRequest req(QUrl(baseUrl + "/api/tags"));
  req.setTransferTimeout(5000);
  QNetworkReply *reply = net->get(req);
  connect(reply, &QNetworkReply::finished, this, [this, reply] {
    bool ok = (reply->error() == QNetworkReply::NoError);
    emit availabilityChecked(ok, ok ? model : QString());
    reply->deleteLater();
  });
}

QString LocalLLMService::buildPrompt(const QString &input,
                                     const QString &cwd) const {
  QString p;
  p += "Convert the instruction to a single bash command. "
       "Reply with ONLY the command, nothing else.\n\n"
       "Examples:\n"
       "Instruction: create a file named hello.txt\n"
       "Command: touch hello.txt\n\n"
       "Instruction: list all files\n"
       "Command: ls -la\n\n"
       "Instruction: show disk usage\n"
       "Command: df -h\n\n"
       "Instruction: show current directory\n"
       "Command: pwd\n\n";
  if (!cwd.isEmpty())
    p += "Current directory: " + cwd + "\n\n";
  p += "Instruction: " + input + "\nCommand:";
  return p;
}

void LocalLLMService::handleReply(QNetworkReply *reply) {
  if (reply->error() != QNetworkReply::NoError) {
    emit errorOccurred("Ollama unreachable: " + reply->errorString() +
                       "\nRun: ollama serve && ollama pull deepseek-coder:1.3b");
    reply->deleteLater();
    return;
  }
  QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
  reply->deleteLater();
  if (doc.isNull()) {
    emit errorOccurred("Could not parse Ollama response.");
    return;
  }
  QString cmd = cleanResponse(doc.object()["response"].toString().trimmed());
  if (cmd.isEmpty())
    emit errorOccurred("LLM returned empty response.");
  else if (cmd.contains("CANNOT_PROCESS", Qt::CaseInsensitive) ||
           cmd.contains("cannot process", Qt::CaseInsensitive))
    emit errorOccurred("LLM refused to process that request.");
  else
    emit commandReady(cmd);
}

QString LocalLLMService::cleanResponse(QString s) {
  s = s.trimmed();

  // Strip markdown code blocks
  static QRegularExpression cb(R"(```(?:bash|sh)?\s*([\s\S]*?)```)");
  auto m = cb.match(s);
  if (m.hasMatch())
    s = m.captured(1).trimmed();

  // Take the first non-empty line
  QString firstLine;
  for (const QString &line : s.split('\n')) {
    QString l = line.trimmed();
    if (!l.isEmpty()) {
      if (l.startsWith("$ "))
        l = l.mid(2).trimmed();
      firstLine = l;
      break;
    }
  }

  if (firstLine.isEmpty())
    return s;

  // Remove common LLM pollution prefixes
  static QRegularExpression prefixRe(
      R"(^(?:Command:\s*|Answer:\s*|Output:\s*|Result:\s*|Here.*?:\s*))",
      QRegularExpression::CaseInsensitiveOption);
  firstLine.replace(prefixRe, "");
  firstLine = firstLine.trimmed();

  // If the LLM embedded CANNOT_PROCESS anywhere, flag it
  if (firstLine.contains("CANNOT_PROCESS", Qt::CaseInsensitive) ||
      firstLine.contains("Cannot process", Qt::CaseInsensitive))
    return "CANNOT_PROCESS";

  // If the LLM generated a multi-command with semicolons containing
  // error/exit patterns, try to extract the useful command
  if (firstLine.contains("exit") && firstLine.contains(";")) {
    QStringList parts = firstLine.split(";");
    for (const QString &part : parts) {
      QString p = part.trimmed();
      if (!p.isEmpty() && !p.contains("exit") &&
          !p.contains("echo") && !p.contains("CANNOT") &&
          !p.contains("/dev/stderr")) {
        firstLine = p;
        break;
      }
    }
  }

  // Strip trailing comments
  int commentIdx = firstLine.indexOf(" #");
  if (commentIdx > 0)
    firstLine = firstLine.left(commentIdx).trimmed();

  // Remove surrounding quotes if the entire line is quoted
  if ((firstLine.startsWith('"') && firstLine.endsWith('"')) ||
      (firstLine.startsWith('\'') && firstLine.endsWith('\'')))
    firstLine = firstLine.mid(1, firstLine.length() - 2).trimmed();

  return firstLine;
}

