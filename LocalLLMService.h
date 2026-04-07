#ifndef LOCAL_LLM_SERVICE_H
#define LOCAL_LLM_SERVICE_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>

class LocalLLMService : public QObject {
  Q_OBJECT

public:
  explicit LocalLLMService(QObject *parent = nullptr,
                           const QString &model = "llama3.1:8b",
                           const QString &url = "http://localhost:11434");

  void generateCommand(const QString &input, const QString &cwd = {});
  void checkAvailability();

signals:
  void commandReady(const QString &command);
  void errorOccurred(const QString &message);
  void availabilityChecked(bool ok, const QString &modelName);

private:
  QNetworkAccessManager *net;
  QString model;
  QString baseUrl;

  QString buildPrompt(const QString &input, const QString &cwd) const;
  void handleReply(QNetworkReply *reply);
  static QString cleanResponse(QString s);
};

#endif 
