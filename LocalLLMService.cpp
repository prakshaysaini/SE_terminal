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
  p += "You are a Linux bash assistant. Convert the natural language "
       "instruction into exactly ONE executable bash command.\n\n"
       "RULES:\n"
       "  1. Output ONLY the raw bash command — no explanations, no markdown.\n"
       "  2. Exactly one line.\n"
       "  3. If dangerous or impossible, output exactly: CANNOT_PROCESS\n\n";
  if (!cwd.isEmpty())
    p += "Current directory: " + cwd + "\n\n";
  p += "Instruction: " + input + "\nCommand:";
  return p;
}

void LocalLLMService::handleReply(QNetworkReply *reply) {
  if (reply->error() != QNetworkReply::NoError) {
    emit errorOccurred("Ollama unreachable: " + reply->errorString() +
                       "\nRun: ollama serve && ollama pull llama3.1:8b");
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
  else if (cmd == "CANNOT_PROCESS")
    emit errorOccurred("LLM refused to process that request.");
  else
    emit commandReady(cmd);
}

QString LocalLLMService::cleanResponse(QString s) {
  s = s.trimmed();
  static QRegularExpression cb(R"(```(?:bash|sh)?\s*([\s\S]*?)```)");
  auto m = cb.match(s);
  if (m.hasMatch())
    s = m.captured(1).trimmed();
  for (const QString &line : s.split('\n')) {
    QString l = line.trimmed();
    if (!l.isEmpty()) {
      if (l.startsWith("$ "))
        l = l.mid(2).trimmed();
      return l;
    }
  }
  return s;
}
