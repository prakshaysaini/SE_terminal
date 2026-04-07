#ifndef DATA_ACCESS_LAYER_H
#define DATA_ACCESS_LAYER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QList>
#include <QDateTime>
#include <QVariant>

struct Session {
    int sessionId;
    QString sessionName;
    QDateTime startTime;
    QDateTime endTime;
    QString workingDirectory;
    QString sessionStatus;
    int totalCommands;
    QString notes;
};

struct CommandRecord {
    int commandId;
    int sessionId;
    QString userInput;
    QString generatedCommand;
    QString executionStatus;
    int exitCode;
    QString commandOutput;
    QString errorOutput;
    double executionTimeMs;
    QDateTime timestamp;
    bool isSafe;
    QString safetyReason;
};

struct CommandCacheRecord {
    int cacheId;
    QString commandHash;
    QString originalCommand;
    QString llmGeneratedCommand;
    QDateTime generatedTimestamp;
    int usageCount;
    QDateTime lastUsed;
};

struct SystemLog {
    int logId;
    QString logLevel;
    QString logMessage;
    QDateTime timestamp;
    QString component;
    int sessionId;
};

struct PerformanceMetric {
    int metricId;
    int sessionId;
    int commandId;
    double llmResponseTimeMs;
    double safetyCheckTimeMs;
    double commandExecutionTimeMs;
    double totalTimeMs;
    QDateTime timestamp;
};

class DataAccessLayer {
public:
    explicit DataAccessLayer(const QString &dbPath = "terminal_app.db");
    ~DataAccessLayer();

    bool initializeDatabase();
    bool openConnection();
    void closeConnection();
    bool isConnected() const;
    QString getLastError() const;

    int createSession(const QString &sessionName, const QString &workingDir = "/home");
    bool updateSessionEndTime(int sessionId);
    bool updateSessionStatus(int sessionId, const QString &status);
    bool updateSessionTotalCommands(int sessionId, int count);
    Session getSession(int sessionId);
    QList<Session> getAllSessions();
    QList<Session> getActiveSessions();
    QList<Session> getSessionsBetween(const QDateTime &start, const QDateTime &end);
    bool deleteSession(int sessionId);

    int recordCommand(int sessionId, const QString &userInput, 
                     const QString &generatedCommand, bool isSafe,
                     const QString &safetyReason = "");
    bool updateCommandExecution(int commandId, const QString &output,
                               const QString &errorOutput, int exitCode,
                               double executionTimeMs);
    bool updateCommandStatus(int commandId, const QString &status);
    CommandRecord getCommandRecord(int commandId);
    QList<CommandRecord> getCommandHistory(int sessionId);
    QList<CommandRecord> getFailedCommands(int sessionId);
    QList<CommandRecord> getBlockedCommands(int sessionId);
    int getTotalCommandsExecuted(int sessionId);
    bool deleteCommandRecord(int commandId);

    bool cacheCommand(const QString &commandHash, const QString &originalCommand,
                     const QString &llmGeneratedCommand);
    QString getCachedCommand(const QString &commandHash);
    bool updateCacheUsage(const QString &commandHash);
    QList<CommandCacheRecord> getAllCachedCommands();
    bool clearOldCacheEntries(int daysOld = 30);

    int addSystemLog(const QString &level, const QString &message,
                    const QString &component = "", int sessionId = -1);
    QList<SystemLog> getSystemLogs(const QString &level = "");
    QList<SystemLog> getSystemLogsByComponent(const QString &component);
    QList<SystemLog> getSystemLogsBetween(const QDateTime &start, const QDateTime &end);

    int recordPerformanceMetric(int sessionId, int commandId,
                               double llmResponseTime, double safetyCheckTime,
                               double commandExecutionTime);
    QList<PerformanceMetric> getMetricsForSession(int sessionId);
    QList<PerformanceMetric> getMetricsForCommand(int commandId);
    double getAverageLLMResponseTime(int sessionId);
    double getAverageSafetyCheckTime(int sessionId);

    int getCommandCountByStatus(int sessionId, const QString &status);
    int getSafeCommandCount(int sessionId);
    int getBlockedCommandCount(int sessionId);
    QList<CommandRecord> getCommandsSince(int sessionId, const QDateTime &since);

    bool clearAllData();
    bool backupDatabase(const QString &backupPath);
    QVariant executeRawQuery(const QString &query);
    
private:
    QSqlDatabase db;
    QString databasePath;
    QString lastError;

    bool executeSqlFile(const QString &filePath);
    bool executeQuery(QSqlQuery &query);
    QString hashCommand(const QString &command) const;
    QDateTime getCurrentTimestamp() const;
};

#endif
