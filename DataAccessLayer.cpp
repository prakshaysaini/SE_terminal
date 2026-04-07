#include "DataAccessLayer.h"
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QStringList>

DataAccessLayer::DataAccessLayer(const QString &dbPath)
    : databasePath(dbPath) {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(databasePath);
}

DataAccessLayer::~DataAccessLayer() {
    closeConnection();
}

bool DataAccessLayer::initializeDatabase() {
    if (!openConnection()) {
        lastError = "Failed to open database connection";
        return false;
    }

    QStringList createStatements = {
        "CREATE TABLE IF NOT EXISTS sessions ("
        "    session_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    session_name TEXT NOT NULL,"
        "    start_time DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "    end_time DATETIME,"
        "    working_directory TEXT DEFAULT '/home',"
        "    session_status TEXT DEFAULT 'ACTIVE' CHECK(session_status IN ('ACTIVE', 'CLOSED')),"
        "    total_commands INTEGER DEFAULT 0,"
        "    notes TEXT);",

        "CREATE TABLE IF NOT EXISTS command_history ("
        "    command_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    session_id INTEGER NOT NULL,"
        "    user_input TEXT NOT NULL,"
        "    generated_command TEXT NOT NULL,"
        "    execution_status TEXT DEFAULT 'EXECUTED' CHECK(execution_status IN ('EXECUTED', 'FAILED', 'BLOCKED')),"
        "    exit_code INTEGER,"
        "    command_output TEXT,"
        "    error_output TEXT,"
        "    execution_time_ms REAL,"
        "    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "    is_safe BOOLEAN DEFAULT 1,"
        "    safety_reason TEXT,"
        "    FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE);",

        "CREATE TABLE IF NOT EXISTS command_cache ("
        "    cache_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    command_hash TEXT UNIQUE NOT NULL,"
        "    original_command TEXT NOT NULL,"
        "    llm_generated_command TEXT NOT NULL,"
        "    generated_timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "    usage_count INTEGER DEFAULT 1,"
        "    last_used DATETIME DEFAULT CURRENT_TIMESTAMP);",

        "CREATE TABLE IF NOT EXISTS system_logs ("
        "    log_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    log_level TEXT CHECK(log_level IN ('INFO', 'WARNING', 'ERROR', 'DEBUG')),"
        "    log_message TEXT NOT NULL,"
        "    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "    component TEXT,"
        "    session_id INTEGER,"
        "    FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE SET NULL);",

        "CREATE TABLE IF NOT EXISTS performance_metrics ("
        "    metric_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    session_id INTEGER NOT NULL,"
        "    command_id INTEGER,"
        "    llm_response_time_ms REAL,"
        "    safety_check_time_ms REAL,"
        "    command_execution_time_ms REAL,"
        "    total_time_ms REAL,"
        "    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "    FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE,"
        "    FOREIGN KEY (command_id) REFERENCES command_history(command_id) ON DELETE CASCADE);"
    };

    for (const QString &statement : createStatements) {
        QSqlQuery query;
        if (!query.exec(statement)) {
            lastError = "Failed to create table: " + query.lastError().text();
            qWarning() << "SQL Error: " << lastError;
            return false;
        }
    }

    QStringList indexStatements = {
        "CREATE INDEX IF NOT EXISTS idx_sessions_start_time ON sessions(start_time);",
        "CREATE INDEX IF NOT EXISTS idx_sessions_status ON sessions(session_status);",
        "CREATE INDEX IF NOT EXISTS idx_command_history_session ON command_history(session_id);",
        "CREATE INDEX IF NOT EXISTS idx_command_history_timestamp ON command_history(timestamp);",
        "CREATE INDEX IF NOT EXISTS idx_command_history_status ON command_history(execution_status);",
        "CREATE INDEX IF NOT EXISTS idx_command_cache_hash ON command_cache(command_hash);",
        "CREATE INDEX IF NOT EXISTS idx_system_logs_timestamp ON system_logs(timestamp);",
        "CREATE INDEX IF NOT EXISTS idx_performance_metrics_session ON performance_metrics(session_id);"
    };

    for (const QString &statement : indexStatements) {
        QSqlQuery query;
        if (!query.exec(statement)) {
            qWarning() << "Index creation warning: " << query.lastError().text();
        }
    }

    return true;
}

bool DataAccessLayer::openConnection() {
    if (!db.open()) {
        lastError = "Cannot open database: " + db.lastError().text();
        return false;
    }
    return true;
}

void DataAccessLayer::closeConnection() {
    if (db.isOpen()) {
        db.close();
    }
}

bool DataAccessLayer::isConnected() const {
    return db.isOpen();
}

QString DataAccessLayer::getLastError() const {
    return lastError;
}

int DataAccessLayer::createSession(const QString &sessionName, const QString &workingDir) {
    QSqlQuery query;
    query.prepare("INSERT INTO sessions (session_name, working_directory, session_status) "
                  "VALUES (:name, :dir, 'ACTIVE')");
    query.addBindValue(sessionName);
    query.addBindValue(workingDir);

    if (!query.exec()) {
        lastError = "Failed to create session: " + query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toInt();
}

bool DataAccessLayer::updateSessionEndTime(int sessionId) {
    QSqlQuery query;
    query.prepare("UPDATE sessions SET end_time = CURRENT_TIMESTAMP, session_status = 'CLOSED' "
                  "WHERE session_id = :id");
    query.addBindValue(sessionId);
    return query.exec();
}

bool DataAccessLayer::updateSessionStatus(int sessionId, const QString &status) {
    QSqlQuery query;
    query.prepare("UPDATE sessions SET session_status = :status WHERE session_id = :id");
    query.addBindValue(status);
    query.addBindValue(sessionId);
    return query.exec();
}

bool DataAccessLayer::updateSessionTotalCommands(int sessionId, int count) {
    QSqlQuery query;
    query.prepare("UPDATE sessions SET total_commands = :count WHERE session_id = :id");
    query.addBindValue(count);
    query.addBindValue(sessionId);
    return query.exec();
}

Session DataAccessLayer::getSession(int sessionId) {
    Session session = {-1, "", QDateTime(), QDateTime(), "", "", 0, ""};
    QSqlQuery query;
    query.prepare("SELECT * FROM sessions WHERE session_id = :id");
    query.addBindValue(sessionId);

    if (query.exec() && query.next()) {
        session.sessionId = query.value("session_id").toInt();
        session.sessionName = query.value("session_name").toString();
        session.startTime = query.value("start_time").toDateTime();
        session.endTime = query.value("end_time").toDateTime();
        session.workingDirectory = query.value("working_directory").toString();
        session.sessionStatus = query.value("session_status").toString();
        session.totalCommands = query.value("total_commands").toInt();
        session.notes = query.value("notes").toString();
    }
    return session;
}

QList<Session> DataAccessLayer::getAllSessions() {
    QList<Session> sessions;
    QSqlQuery query("SELECT * FROM sessions ORDER BY start_time DESC");

    while (query.next()) {
        Session session;
        session.sessionId = query.value("session_id").toInt();
        session.sessionName = query.value("session_name").toString();
        session.startTime = query.value("start_time").toDateTime();
        session.endTime = query.value("end_time").toDateTime();
        session.workingDirectory = query.value("working_directory").toString();
        session.sessionStatus = query.value("session_status").toString();
        session.totalCommands = query.value("total_commands").toInt();
        session.notes = query.value("notes").toString();
        sessions.append(session);
    }
    return sessions;
}

QList<Session> DataAccessLayer::getActiveSessions() {
    QList<Session> sessions;
    QSqlQuery query;
    query.prepare("SELECT * FROM sessions WHERE session_status = 'ACTIVE' ORDER BY start_time DESC");

    if (query.exec()) {
        while (query.next()) {
            Session session;
            session.sessionId = query.value("session_id").toInt();
            session.sessionName = query.value("session_name").toString();
            session.startTime = query.value("start_time").toDateTime();
            session.endTime = query.value("end_time").toDateTime();
            session.workingDirectory = query.value("working_directory").toString();
            session.sessionStatus = query.value("session_status").toString();
            session.totalCommands = query.value("total_commands").toInt();
            session.notes = query.value("notes").toString();
            sessions.append(session);
        }
    }
    return sessions;
}

QList<Session> DataAccessLayer::getSessionsBetween(const QDateTime &start, const QDateTime &end) {
    QList<Session> sessions;
    QSqlQuery query;
    query.prepare("SELECT * FROM sessions WHERE start_time BETWEEN :start AND :end "
                  "ORDER BY start_time DESC");
    query.addBindValue(start);
    query.addBindValue(end);

    if (query.exec()) {
        while (query.next()) {
            Session session;
            session.sessionId = query.value("session_id").toInt();
            session.sessionName = query.value("session_name").toString();
            session.startTime = query.value("start_time").toDateTime();
            session.endTime = query.value("end_time").toDateTime();
            session.workingDirectory = query.value("working_directory").toString();
            session.sessionStatus = query.value("session_status").toString();
            session.totalCommands = query.value("total_commands").toInt();
            session.notes = query.value("notes").toString();
            sessions.append(session);
        }
    }
    return sessions;
}

bool DataAccessLayer::deleteSession(int sessionId) {
    QSqlQuery query;
    query.prepare("DELETE FROM sessions WHERE session_id = :id");
    query.addBindValue(sessionId);
    return query.exec();
}

int DataAccessLayer::recordCommand(int sessionId, const QString &userInput,
                                   const QString &generatedCommand, bool isSafe,
                                   const QString &safetyReason) {
    QSqlQuery query;
    query.prepare("INSERT INTO command_history "
                  "(session_id, user_input, generated_command, execution_status, is_safe, safety_reason) "
                  "VALUES (:session, :input, :cmd, 'EXECUTED', :safe, :reason)");
    query.addBindValue(sessionId);
    query.addBindValue(userInput);
    query.addBindValue(generatedCommand);
    query.addBindValue(isSafe ? 1 : 0);
    query.addBindValue(safetyReason);

    if (!query.exec()) {
        lastError = "Failed to record command: " + query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toInt();
}

bool DataAccessLayer::updateCommandExecution(int commandId, const QString &output,
                                            const QString &errorOutput, int exitCode,
                                            double executionTimeMs) {
    QSqlQuery query;
    query.prepare("UPDATE command_history SET command_output = :output, "
                  "error_output = :error, exit_code = :code, "
                  "execution_time_ms = :time WHERE command_id = :id");
    query.addBindValue(output);
    query.addBindValue(errorOutput);
    query.addBindValue(exitCode);
    query.addBindValue(executionTimeMs);
    query.addBindValue(commandId);
    return query.exec();
}

bool DataAccessLayer::updateCommandStatus(int commandId, const QString &status) {
    QSqlQuery query;
    query.prepare("UPDATE command_history SET execution_status = :status WHERE command_id = :id");
    query.addBindValue(status);
    query.addBindValue(commandId);
    return query.exec();
}

CommandRecord DataAccessLayer::getCommandRecord(int commandId) {
    CommandRecord record = {-1, -1, "", "", "", 0, "", "", 0.0, QDateTime(), false, ""};
    QSqlQuery query;
    query.prepare("SELECT * FROM command_history WHERE command_id = :id");
    query.addBindValue(commandId);

    if (query.exec() && query.next()) {
        record.commandId = query.value("command_id").toInt();
        record.sessionId = query.value("session_id").toInt();
        record.userInput = query.value("user_input").toString();
        record.generatedCommand = query.value("generated_command").toString();
        record.executionStatus = query.value("execution_status").toString();
        record.exitCode = query.value("exit_code").toInt();
        record.commandOutput = query.value("command_output").toString();
        record.errorOutput = query.value("error_output").toString();
        record.executionTimeMs = query.value("execution_time_ms").toDouble();
        record.timestamp = query.value("timestamp").toDateTime();
        record.isSafe = query.value("is_safe").toBool();
        record.safetyReason = query.value("safety_reason").toString();
    }
    return record;
}

QList<CommandRecord> DataAccessLayer::getCommandHistory(int sessionId) {
    QList<CommandRecord> records;
    QSqlQuery query;
    query.prepare("SELECT * FROM command_history WHERE session_id = :session_id "
                  "ORDER BY timestamp DESC");
    query.addBindValue(sessionId);

    if (query.exec()) {
        while (query.next()) {
            CommandRecord record;
            record.commandId = query.value("command_id").toInt();
            record.sessionId = query.value("session_id").toInt();
            record.userInput = query.value("user_input").toString();
            record.generatedCommand = query.value("generated_command").toString();
            record.executionStatus = query.value("execution_status").toString();
            record.exitCode = query.value("exit_code").toInt();
            record.commandOutput = query.value("command_output").toString();
            record.errorOutput = query.value("error_output").toString();
            record.executionTimeMs = query.value("execution_time_ms").toDouble();
            record.timestamp = query.value("timestamp").toDateTime();
            record.isSafe = query.value("is_safe").toBool();
            record.safetyReason = query.value("safety_reason").toString();
            records.append(record);
        }
    }
    return records;
}

QList<CommandRecord> DataAccessLayer::getFailedCommands(int sessionId) {
    QList<CommandRecord> records;
    QSqlQuery query;
    query.prepare("SELECT * FROM command_history WHERE session_id = :session_id "
                  "AND execution_status = 'FAILED' ORDER BY timestamp DESC");
    query.addBindValue(sessionId);

    if (query.exec()) {
        while (query.next()) {
            CommandRecord record;
            record.commandId = query.value("command_id").toInt();
            record.sessionId = query.value("session_id").toInt();
            record.userInput = query.value("user_input").toString();
            record.generatedCommand = query.value("generated_command").toString();
            record.executionStatus = query.value("execution_status").toString();
            record.exitCode = query.value("exit_code").toInt();
            record.commandOutput = query.value("command_output").toString();
            record.errorOutput = query.value("error_output").toString();
            record.executionTimeMs = query.value("execution_time_ms").toDouble();
            record.timestamp = query.value("timestamp").toDateTime();
            record.isSafe = query.value("is_safe").toBool();
            record.safetyReason = query.value("safety_reason").toString();
            records.append(record);
        }
    }
    return records;
}

QList<CommandRecord> DataAccessLayer::getBlockedCommands(int sessionId) {
    QList<CommandRecord> records;
    QSqlQuery query;
    query.prepare("SELECT * FROM command_history WHERE session_id = :session_id "
                  "AND (execution_status = 'BLOCKED' OR is_safe = 0) ORDER BY timestamp DESC");
    query.addBindValue(sessionId);

    if (query.exec()) {
        while (query.next()) {
            CommandRecord record;
            record.commandId = query.value("command_id").toInt();
            record.sessionId = query.value("session_id").toInt();
            record.userInput = query.value("user_input").toString();
            record.generatedCommand = query.value("generated_command").toString();
            record.executionStatus = query.value("execution_status").toString();
            record.exitCode = query.value("exit_code").toInt();
            record.commandOutput = query.value("command_output").toString();
            record.errorOutput = query.value("error_output").toString();
            record.executionTimeMs = query.value("execution_time_ms").toDouble();
            record.timestamp = query.value("timestamp").toDateTime();
            record.isSafe = query.value("is_safe").toBool();
            record.safetyReason = query.value("safety_reason").toString();
            records.append(record);
        }
    }
    return records;
}

int DataAccessLayer::getTotalCommandsExecuted(int sessionId) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) as count FROM command_history WHERE session_id = :session_id");
    query.addBindValue(sessionId);

    if (query.exec() && query.next()) {
        return query.value("count").toInt();
    }
    return 0;
}

bool DataAccessLayer::deleteCommandRecord(int commandId) {
    QSqlQuery query;
    query.prepare("DELETE FROM command_history WHERE command_id = :id");
    query.addBindValue(commandId);
    return query.exec();
}

bool DataAccessLayer::cacheCommand(const QString &commandHash, const QString &originalCommand,
                                  const QString &llmGeneratedCommand) {
    QSqlQuery query;
    query.prepare("INSERT OR IGNORE INTO command_cache "
                  "(command_hash, original_command, llm_generated_command) "
                  "VALUES (:hash, :original, :generated)");
    query.addBindValue(commandHash);
    query.addBindValue(originalCommand);
    query.addBindValue(llmGeneratedCommand);
    return query.exec();
}

QString DataAccessLayer::getCachedCommand(const QString &commandHash) {
    QSqlQuery query;
    query.prepare("SELECT llm_generated_command FROM command_cache WHERE command_hash = :hash");
    query.addBindValue(commandHash);

    if (query.exec() && query.next()) {
        updateCacheUsage(commandHash);
        return query.value("llm_generated_command").toString();
    }
    return "";
}

bool DataAccessLayer::updateCacheUsage(const QString &commandHash) {
    QSqlQuery query;
    query.prepare("UPDATE command_cache SET usage_count = usage_count + 1, "
                  "last_used = CURRENT_TIMESTAMP WHERE command_hash = :hash");
    query.addBindValue(commandHash);
    return query.exec();
}

QList<CommandCacheRecord> DataAccessLayer::getAllCachedCommands() {
    QList<CommandCacheRecord> records;
    QSqlQuery query("SELECT * FROM command_cache ORDER BY usage_count DESC");

    while (query.next()) {
        CommandCacheRecord record;
        record.cacheId = query.value("cache_id").toInt();
        record.commandHash = query.value("command_hash").toString();
        record.originalCommand = query.value("original_command").toString();
        record.llmGeneratedCommand = query.value("llm_generated_command").toString();
        record.generatedTimestamp = query.value("generated_timestamp").toDateTime();
        record.usageCount = query.value("usage_count").toInt();
        record.lastUsed = query.value("last_used").toDateTime();
        records.append(record);
    }
    return records;
}

bool DataAccessLayer::clearOldCacheEntries(int daysOld) {
    QSqlQuery query;
    query.prepare("DELETE FROM command_cache WHERE "
                  "datetime(last_used) < datetime('now', '-' || :days || ' days')");
    query.addBindValue(daysOld);
    return query.exec();
}

int DataAccessLayer::addSystemLog(const QString &level, const QString &message,
                                 const QString &component, int sessionId) {
    QSqlQuery query;
    query.prepare("INSERT INTO system_logs (log_level, log_message, component, session_id) "
                  "VALUES (:level, :message, :component, :session)");
    query.addBindValue(level);
    query.addBindValue(message);
    query.addBindValue(component);
    query.addBindValue(sessionId == -1 ? QVariant(QVariant::Int) : sessionId);

    if (!query.exec()) {
        lastError = "Failed to add system log: " + query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toInt();
}

QList<SystemLog> DataAccessLayer::getSystemLogs(const QString &level) {
    QList<SystemLog> logs;
    QSqlQuery query;
    if (level.isEmpty()) {
        query.prepare("SELECT * FROM system_logs ORDER BY timestamp DESC LIMIT 1000");
    } else {
        query.prepare("SELECT * FROM system_logs WHERE log_level = :level "
                      "ORDER BY timestamp DESC LIMIT 1000");
        query.addBindValue(level);
    }

    if (query.exec()) {
        while (query.next()) {
            SystemLog log;
            log.logId = query.value("log_id").toInt();
            log.logLevel = query.value("log_level").toString();
            log.logMessage = query.value("log_message").toString();
            log.timestamp = query.value("timestamp").toDateTime();
            log.component = query.value("component").toString();
            log.sessionId = query.value("session_id").toInt();
            logs.append(log);
        }
    }
    return logs;
}

QList<SystemLog> DataAccessLayer::getSystemLogsByComponent(const QString &component) {
    QList<SystemLog> logs;
    QSqlQuery query;
    query.prepare("SELECT * FROM system_logs WHERE component = :component "
                  "ORDER BY timestamp DESC LIMIT 1000");
    query.addBindValue(component);

    if (query.exec()) {
        while (query.next()) {
            SystemLog log;
            log.logId = query.value("log_id").toInt();
            log.logLevel = query.value("log_level").toString();
            log.logMessage = query.value("log_message").toString();
            log.timestamp = query.value("timestamp").toDateTime();
            log.component = query.value("component").toString();
            log.sessionId = query.value("session_id").toInt();
            logs.append(log);
        }
    }
    return logs;
}

QList<SystemLog> DataAccessLayer::getSystemLogsBetween(const QDateTime &start, const QDateTime &end) {
    QList<SystemLog> logs;
    QSqlQuery query;
    query.prepare("SELECT * FROM system_logs WHERE timestamp BETWEEN :start AND :end "
                  "ORDER BY timestamp DESC");
    query.addBindValue(start);
    query.addBindValue(end);

    if (query.exec()) {
        while (query.next()) {
            SystemLog log;
            log.logId = query.value("log_id").toInt();
            log.logLevel = query.value("log_level").toString();
            log.logMessage = query.value("log_message").toString();
            log.timestamp = query.value("timestamp").toDateTime();
            log.component = query.value("component").toString();
            log.sessionId = query.value("session_id").toInt();
            logs.append(log);
        }
    }
    return logs;
}

int DataAccessLayer::recordPerformanceMetric(int sessionId, int commandId,
                                            double llmResponseTime, double safetyCheckTime,
                                            double commandExecutionTime) {
    QSqlQuery query;
    query.prepare("INSERT INTO performance_metrics "
                  "(session_id, command_id, llm_response_time_ms, safety_check_time_ms, "
                  "command_execution_time_ms, total_time_ms) "
                  "VALUES (:session, :command, :llm, :safety, :exec, :total)");
    query.addBindValue(sessionId);
    query.addBindValue(commandId);
    query.addBindValue(llmResponseTime);
    query.addBindValue(safetyCheckTime);
    query.addBindValue(commandExecutionTime);
    query.addBindValue(llmResponseTime + safetyCheckTime + commandExecutionTime);

    if (!query.exec()) {
        lastError = "Failed to record performance metric: " + query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toInt();
}

QList<PerformanceMetric> DataAccessLayer::getMetricsForSession(int sessionId) {
    QList<PerformanceMetric> metrics;
    QSqlQuery query;
    query.prepare("SELECT * FROM performance_metrics WHERE session_id = :session_id "
                  "ORDER BY timestamp DESC");
    query.addBindValue(sessionId);

    if (query.exec()) {
        while (query.next()) {
            PerformanceMetric metric;
            metric.metricId = query.value("metric_id").toInt();
            metric.sessionId = query.value("session_id").toInt();
            metric.commandId = query.value("command_id").toInt();
            metric.llmResponseTimeMs = query.value("llm_response_time_ms").toDouble();
            metric.safetyCheckTimeMs = query.value("safety_check_time_ms").toDouble();
            metric.commandExecutionTimeMs = query.value("command_execution_time_ms").toDouble();
            metric.totalTimeMs = query.value("total_time_ms").toDouble();
            metric.timestamp = query.value("timestamp").toDateTime();
            metrics.append(metric);
        }
    }
    return metrics;
}

QList<PerformanceMetric> DataAccessLayer::getMetricsForCommand(int commandId) {
    QList<PerformanceMetric> metrics;
    QSqlQuery query;
    query.prepare("SELECT * FROM performance_metrics WHERE command_id = :command_id "
                  "ORDER BY timestamp DESC");
    query.addBindValue(commandId);

    if (query.exec()) {
        while (query.next()) {
            PerformanceMetric metric;
            metric.metricId = query.value("metric_id").toInt();
            metric.sessionId = query.value("session_id").toInt();
            metric.commandId = query.value("command_id").toInt();
            metric.llmResponseTimeMs = query.value("llm_response_time_ms").toDouble();
            metric.safetyCheckTimeMs = query.value("safety_check_time_ms").toDouble();
            metric.commandExecutionTimeMs = query.value("command_execution_time_ms").toDouble();
            metric.totalTimeMs = query.value("total_time_ms").toDouble();
            metric.timestamp = query.value("timestamp").toDateTime();
            metrics.append(metric);
        }
    }
    return metrics;
}

double DataAccessLayer::getAverageLLMResponseTime(int sessionId) {
    QSqlQuery query;
    query.prepare("SELECT AVG(llm_response_time_ms) as avg FROM performance_metrics "
                  "WHERE session_id = :session_id");
    query.addBindValue(sessionId);

    if (query.exec() && query.next()) {
        return query.value("avg").toDouble();
    }
    return 0.0;
}

double DataAccessLayer::getAverageSafetyCheckTime(int sessionId) {
    QSqlQuery query;
    query.prepare("SELECT AVG(safety_check_time_ms) as avg FROM performance_metrics "
                  "WHERE session_id = :session_id");
    query.addBindValue(sessionId);

    if (query.exec() && query.next()) {
        return query.value("avg").toDouble();
    }
    return 0.0;
}

int DataAccessLayer::getCommandCountByStatus(int sessionId, const QString &status) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) as count FROM command_history "
                  "WHERE session_id = :session_id AND execution_status = :status");
    query.addBindValue(sessionId);
    query.addBindValue(status);

    if (query.exec() && query.next()) {
        return query.value("count").toInt();
    }
    return 0;
}

int DataAccessLayer::getSafeCommandCount(int sessionId) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) as count FROM command_history "
                  "WHERE session_id = :session_id AND is_safe = 1");
    query.addBindValue(sessionId);

    if (query.exec() && query.next()) {
        return query.value("count").toInt();
    }
    return 0;
}

int DataAccessLayer::getBlockedCommandCount(int sessionId) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) as count FROM command_history "
                  "WHERE session_id = :session_id AND is_safe = 0");
    query.addBindValue(sessionId);

    if (query.exec() && query.next()) {
        return query.value("count").toInt();
    }
    return 0;
}

QList<CommandRecord> DataAccessLayer::getCommandsSince(int sessionId, const QDateTime &since) {
    QList<CommandRecord> records;
    QSqlQuery query;
    query.prepare("SELECT * FROM command_history WHERE session_id = :session_id "
                  "AND timestamp >= :since ORDER BY timestamp DESC");
    query.addBindValue(sessionId);
    query.addBindValue(since);

    if (query.exec()) {
        while (query.next()) {
            CommandRecord record;
            record.commandId = query.value("command_id").toInt();
            record.sessionId = query.value("session_id").toInt();
            record.userInput = query.value("user_input").toString();
            record.generatedCommand = query.value("generated_command").toString();
            record.executionStatus = query.value("execution_status").toString();
            record.exitCode = query.value("exit_code").toInt();
            record.commandOutput = query.value("command_output").toString();
            record.errorOutput = query.value("error_output").toString();
            record.executionTimeMs = query.value("execution_time_ms").toDouble();
            record.timestamp = query.value("timestamp").toDateTime();
            record.isSafe = query.value("is_safe").toBool();
            record.safetyReason = query.value("safety_reason").toString();
            records.append(record);
        }
    }
    return records;
}

bool DataAccessLayer::clearAllData() {
    QStringList tables = {"command_history", "performance_metrics", "system_logs", 
                         "command_cache", "sessions"};
    for (const QString &table : tables) {
        QSqlQuery query;
        query.prepare("DELETE FROM " + table);
        if (!query.exec()) {
            lastError = "Failed to clear table " + table;
            return false;
        }
    }
    return true;
}

bool DataAccessLayer::backupDatabase(const QString &backupPath) {
    if (!isConnected()) {
        lastError = "Database not connected";
        return false;
    }
    return QFile::copy(databasePath, backupPath);
}

QVariant DataAccessLayer::executeRawQuery(const QString &query) {
    QSqlQuery q;
    if (q.exec(query)) {
        return true;
    }
    lastError = q.lastError().text();
    return false;
}

QString DataAccessLayer::hashCommand(const QString &command) const {
    return QString(QCryptographicHash::hash(command.toUtf8(), 
                   QCryptographicHash::Sha256).toHex());
}

QDateTime DataAccessLayer::getCurrentTimestamp() const {
    return QDateTime::currentDateTime();
}
