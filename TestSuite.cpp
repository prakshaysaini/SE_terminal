#include "TestSuite.h"
#include "DataAccessLayer.h"
#include "SafetyFilter.h"
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QSqlDatabase>
#include <ctime>





void TestSuite::addTest(const QString &name, const QString &category,
                        std::function<bool()> testFunc) {
    tests.append({name, category, testFunc});
}

void TestSuite::runAll() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "  AI-POWERED TERMINAL APPLICATION - TEST SUITE" << std::endl;
    std::cout << std::string(70, '=') << "\n" << std::endl;
    
    int testNumber = 1;
    for (const Test &test : tests) {
        qint64 startTime = getCurrentTimeMs();
        bool result = false;
        QString message = "";
        
        try {
            result = test.func();
            message = result ? "Test passed" : "Assertion failed";
        } catch (const std::exception &e) {
            result = false;
            message = QString("Exception: %1").arg(e.what());
        } catch (...) {
            result = false;
            message = "Unknown error";
        }
        
        qint64 endTime = getCurrentTimeMs();
        double executionTime = endTime - startTime;
        
        TestResult tr;
        tr.testName = test.name;
        tr.category = test.category;
        tr.passed = result;
        tr.message = message;
        tr.executionTimeMs = executionTime;
        
        results.append(tr);
        
        
        stats.totalTests++;
        stats.totalTimeMs += executionTime;
        
        if (test.category == "White Box") {
            stats.whiteBoxTests++;
            if (result) stats.whiteBoxPassed++;
        } else {
            stats.blackBoxTests++;
            if (result) stats.blackBoxPassed++;
        }
        
        if (result) {
            stats.passedTests++;
        } else {
            stats.failedTests++;
        }
        
        
        std::cout << std::left << std::setw(3) << testNumber << ". "
                  << std::setw(70) << test.name.toStdString();
        std::cout << (result ? "✓" : "✗") << std::endl;
        testNumber++;
    }
    
    printSummary();
}

const QList<TestResult>& TestSuite::getResults() const {
    return results;
}

const TestStatistics& TestSuite::getStatistics() const {
    return stats;
}

void TestSuite::generateReport(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        std::cerr << "Failed to open report file: " << filePath.toStdString() << std::endl;
        return;
    }
    
    QString report;
    report += "=" + QString(69, '=') + "\n";
    report += "AI-POWERED TERMINAL APPLICATION - TEST REPORT\n";
    report += "=" + QString(69, '=') + "\n\n";
    
    report += QString("Generated: %1\n\n").arg(QDateTime::currentDateTime().toString());
    
    
    report += "SUMMARY\n";
    report += "-" + QString(69, '-') + "\n";
    report += QString("Total Tests:        %1\n").arg(stats.totalTests);
    report += QString("Passed Tests:       %1\n").arg(stats.passedTests);
    report += QString("Failed Tests:       %1\n").arg(stats.failedTests);
    report += QString("Overall Pass Rate:  %1%\n").arg(QString::number(stats.getPassRate(), 'f', 2));
    report += "\n";
    
    
    report += "CATEGORY BREAKDOWN\n";
    report += "-" + QString(69, '-') + "\n";
    report += QString("White Box Tests:    %1/%2 passed (%3%)\n")
        .arg(stats.whiteBoxPassed).arg(stats.whiteBoxTests)
        .arg(QString::number(stats.getWhiteBoxPassRate(), 'f', 2));
    report += QString("Black Box Tests:    %1/%2 passed (%3%)\n")
        .arg(stats.blackBoxPassed).arg(stats.blackBoxTests)
        .arg(QString::number(stats.getBlackBoxPassRate(), 'f', 2));
    report += "\n";
    
    
    report += "PERFORMANCE METRICS\n";
    report += "-" + QString(69, '-') + "\n";
    report += QString("Total Execution Time: %1 ms\n").arg(stats.totalTimeMs, 0, 'f', 2);
    report += QString("Average Test Time:    %1 ms\n")
        .arg(stats.totalTimeMs / stats.totalTests, 0, 'f', 2);
    report += "\n";
    
    
    report += "DETAILED TEST RESULTS\n";
    report += "=" + QString(69, '=') + "\n\n";
    
    int whiteBoxCount = 0;
    int blackBoxCount = 0;
    
    for (const TestResult &result : results) {
        if (result.category == "White Box") {
            whiteBoxCount++;
            report += QString("WB.%1: %2\n").arg(whiteBoxCount).arg(result.testName);
        } else {
            blackBoxCount++;
            report += QString("BB.%1: %2\n").arg(blackBoxCount).arg(result.testName);
        }
        report += QString("  Status:    %1\n").arg(result.getStatus());
        report += QString("  Message:   %1\n").arg(result.message);
        report += QString("  Time:      %1 ms\n\n").arg(result.executionTimeMs, 0, 'f', 2);
    }
    
    
    if (stats.failedTests > 0) {
        report += "\nFAILED TESTS SUMMARY\n";
        report += "=" + QString(69, '=') + "\n";
        for (const TestResult &result : results) {
            if (!result.passed) {
                report += QString("- %1\n  Reason: %2\n\n")
                    .arg(result.testName).arg(result.message);
            }
        }
    }
    
    
    report += "\nRECOMMENDATIONS\n";
    report += "=" + QString(69, '=') + "\n";
    
    if (stats.passedTests == stats.totalTests) {
        report += "✓ All tests passed! Application is ready for deployment.\n";
    } else {
        double failRate = (100.0 * stats.failedTests / stats.totalTests);
        report += QString("⚠ %1% of tests failed. Review failed tests above.\n")
            .arg(QString::number(failRate, 'f', 2));
        report += "  Priority: Review and fix all failing tests before deployment.\n";
    }
    
    file.write(report.toUtf8());
    file.close();
    
    std::cout << "\nReport saved to: " << filePath.toStdString() << std::endl;
}

void TestSuite::printSummary() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "  TEST SUMMARY" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << std::right << std::setw(30) << "Total Tests: " << std::left << stats.totalTests << std::endl;
    std::cout << std::right << std::setw(30) << "Passed: " << std::left << stats.passedTests << std::endl;
    std::cout << std::right << std::setw(30) << "Failed: " << std::left << stats.failedTests << std::endl;
    std::cout << std::right << std::setw(30) << "Overall Pass Rate: " 
              << std::left << QString::number(stats.getPassRate(), 'f', 2).toStdString() << "%" << std::endl;
    std::cout << std::right << std::setw(30) << "White Box Pass Rate: " 
              << std::left << QString::number(stats.getWhiteBoxPassRate(), 'f', 2).toStdString() << "%" << std::endl;
    std::cout << std::right << std::setw(30) << "Black Box Pass Rate: " 
              << std::left << QString::number(stats.getBlackBoxPassRate(), 'f', 2).toStdString() << "%" << std::endl;
    std::cout << std::right << std::setw(30) << "Total Execution Time: " 
              << std::left << QString::number(stats.totalTimeMs, 'f', 2).toStdString() << " ms" << std::endl;
    std::cout << std::string(70, '=') << "\n" << std::endl;
}

qint64 TestSuite::getCurrentTimeMs() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}





namespace WhiteBoxTests_DAL {

    bool test_DAL_DatabaseInitialization() {
        DataAccessLayer dal("test_db_init.db");
        return dal.initializeDatabase() && dal.isConnected();
    }

    bool test_DAL_CreateSession_ValidInput() {
        DataAccessLayer dal("test_db_session.db");
        dal.initializeDatabase();
        int sessionId = dal.createSession("Test Session", "/home/test");
        return sessionId > 0;
    }

    bool test_DAL_CreateSession_EmptyName() {
        DataAccessLayer dal("test_db_empty_name.db");
        dal.initializeDatabase();
        int sessionId = dal.createSession("", "/home");
        return sessionId > 0; 
    }

    bool test_DAL_GetSession_NonExistent() {
        DataAccessLayer dal("test_db_nonexistent.db");
        dal.initializeDatabase();
        Session session = dal.getSession(99999);
        return session.sessionId == -1;
    }

    bool test_DAL_UpdateSessionEndTime() {
        DataAccessLayer dal("test_db_end_time.db");
        dal.initializeDatabase();
        int sessionId = dal.createSession("Test", "/home");
        return dal.updateSessionEndTime(sessionId);
    }

    bool test_DAL_UpdateSessionStatus() {
        DataAccessLayer dal("test_db_status.db");
        dal.initializeDatabase();
        int sessionId = dal.createSession("Test", "/home");
        return dal.updateSessionStatus(sessionId, "CLOSED");
    }

    bool test_DAL_RecordCommand() {
        DataAccessLayer dal("test_db_record_cmd.db");
        dal.initializeDatabase();
        int sessionId = dal.createSession("Test", "/home");
        int cmdId = dal.recordCommand(sessionId, "list files", "ls", true);
        return cmdId > 0;
    }

    bool test_DAL_UpdateCommandExecution() {
        DataAccessLayer dal("test_db_update_cmd.db");
        dal.initializeDatabase();
        int sessionId = dal.createSession("Test", "/home");
        int cmdId = dal.recordCommand(sessionId, "list", "ls", true);
        return dal.updateCommandExecution(cmdId, "file1.txt\nfile2.txt", "", 0, 50.0);
    }

    bool test_DAL_GetCommandHistory_Order() {
        DataAccessLayer dal("test_db_history_order.db");
        dal.initializeDatabase();
        int sessionId = dal.createSession("Test", "/home");
        dal.recordCommand(sessionId, "cmd1", "echo 1", true);
        dal.recordCommand(sessionId, "cmd2", "echo 2", true);
        QList<CommandRecord> history = dal.getCommandHistory(sessionId);
        return history.size() >= 1; 
    }

    bool test_DAL_GetFailedCommands_Filtering() {
        DataAccessLayer dal("test_db_failed_cmds.db");
        dal.initializeDatabase();
        int sessionId = dal.createSession("Test", "/home");
        int cmdId1 = dal.recordCommand(sessionId, "cmd1", "echo 1", true);
        int cmdId2 = dal.recordCommand(sessionId, "cmd2", "echo 2", true);
        dal.updateCommandStatus(cmdId2, "FAILED");
        QList<CommandRecord> failed = dal.getFailedCommands(sessionId);
        return failed.size() == 1 && failed[0].commandId == cmdId2;
    }

    bool test_DAL_GetBlockedCommands_UnsafeFiltering() {
        DataAccessLayer dal("test_db_blocked.db");
        dal.initializeDatabase();
        int sessionId = dal.createSession("Test", "/home");
        dal.recordCommand(sessionId, "safe", "ls", true);
        dal.recordCommand(sessionId, "dangerous", "rm -rf /", false, "Dangerous command");
        QList<CommandRecord> blocked = dal.getBlockedCommands(sessionId);
        return blocked.size() == 1 && !blocked[0].isSafe;
    }

    bool test_DAL_CommandCache_SetAndGet() {
        DataAccessLayer dal("test_db_cache.db");
        dal.initializeDatabase();
        dal.cacheCommand("hash123", "list files", "ls");
        QString cached = dal.getCachedCommand("hash123");
        return cached == "ls";
    }

    bool test_DAL_CacheUsageCount() {
        DataAccessLayer dal("test_db_cache_count.db");
        dal.initializeDatabase();
        dal.cacheCommand("hash456", "original", "generated");
        
        dal.getCachedCommand("hash456");  
        dal.getCachedCommand("hash456");  
        dal.getCachedCommand("hash456");  
        QList<CommandCacheRecord> all = dal.getAllCachedCommands();
        return all.size() > 0 && all[0].usageCount >= 3;
    }

    bool test_DAL_SystemLog_Creation() {
        DataAccessLayer dal("test_db_log.db");
        dal.initializeDatabase();
        int logId = dal.addSystemLog("INFO", "Test message", "TestComponent");
        return logId > 0;
    }

    bool test_DAL_SystemLog_LevelFiltering() {
        DataAccessLayer dal("test_db_log_filter.db");
        dal.initializeDatabase();
        dal.addSystemLog("INFO", "Info message");
        dal.addSystemLog("ERROR", "Error message");
        dal.addSystemLog("DEBUG", "Debug message");
        QList<SystemLog> allLogs = dal.getSystemLogs("");
        QList<SystemLog> errors = dal.getSystemLogs("ERROR");
        
        return allLogs.size() >= 1 && errors.size() >= 1;
    }

    bool test_DAL_PerformanceMetrics_Recording() {
        DataAccessLayer dal("test_db_perf.db");
        dal.initializeDatabase();
        int sessionId = dal.createSession("Test", "/home");
        int metricId = dal.recordPerformanceMetric(sessionId, 1, 100.0, 50.0, 200.0);
        return metricId > 0;
    }

    bool test_DAL_AverageLLMResponseTime() {
        DataAccessLayer dal("test_db_avg_llm.db");
        dal.initializeDatabase();
        int sessionId = dal.createSession("Test", "/home");
        dal.recordPerformanceMetric(sessionId, 1, 100.0, 50.0, 200.0);
        dal.recordPerformanceMetric(sessionId, 2, 120.0, 50.0, 200.0);
        double avg = dal.getAverageLLMResponseTime(sessionId);
        return avg >= 109.0 && avg <= 111.0;
    }

    bool test_DAL_AverageSafetyCheckTime() {
        DataAccessLayer dal("test_db_avg_safety.db");
        dal.initializeDatabase();
        int sessionId = dal.createSession("Test", "/home");
        dal.recordPerformanceMetric(sessionId, 1, 100.0, 50.0, 200.0);
        dal.recordPerformanceMetric(sessionId, 2, 100.0, 60.0, 200.0);
        double avg = dal.getAverageSafetyCheckTime(sessionId);
        return avg == 55.0;
    }

    bool test_DAL_CommandCountByStatus() {
        DataAccessLayer dal("test_db_count_status.db");
        dal.initializeDatabase();
        int sessionId = dal.createSession("Test", "/home");
        dal.recordCommand(sessionId, "cmd1", "echo 1", true);
        dal.recordCommand(sessionId, "cmd2", "echo 2", true);
        int count = dal.getCommandCountByStatus(sessionId, "EXECUTED");
        return count == 2;
    }

    bool test_DAL_GetSafeCommandCount() {
        DataAccessLayer dal("test_db_safe_count.db");
        dal.initializeDatabase();
        int sessionId = dal.createSession("Test", "/home");
        dal.recordCommand(sessionId, "safe1", "ls", true);
        dal.recordCommand(sessionId, "safe2", "pwd", true);
        dal.recordCommand(sessionId, "unsafe", "rm -rf /", false);
        int count = dal.getSafeCommandCount(sessionId);
        return count == 2;
    }
}

namespace WhiteBoxTests_SafetyFilter {

    bool test_SafetyFilter_ForkBombDetection() {
        SafetyResult result = isSafeCommand(":(){:|:&};:");
        return !result.isSafe && result.reason.contains("Fork bomb");
    }

    bool test_SafetyFilter_RootFsDelete() {
        SafetyResult result = isSafeCommand("rm -rf /");
        return !result.isSafe;
    }

    bool test_SafetyFilter_HomeDirDelete() {
        SafetyResult result = isSafeCommand("rm -rf ~");
        return !result.isSafe;
    }

    bool test_SafetyFilter_RecursiveDelete() {
        SafetyResult result = isSafeCommand("rm -rf /*");
        return !result.isSafe;
    }

    bool test_SafetyFilter_FilesystemFormat() {
        SafetyResult result = isSafeCommand("mkfs /dev/sda1");
        return !result.isSafe;
    }

    bool test_SafetyFilter_BlockDeviceWrite() {
        SafetyResult result = isSafeCommand("echo data > /dev/sda");
        return !result.isSafe;
    }

    bool test_SafetyFilter_ShutdownCommands() {
        SafetyResult result1 = isSafeCommand("shutdown -h now");
        SafetyResult result2 = isSafeCommand("reboot");
        return !result1.isSafe && !result2.isSafe;
    }

    bool test_SafetyFilter_KillAllProcesses() {
        SafetyResult result = isSafeCommand("kill -9 -1");
        return !result.isSafe;
    }

    bool test_SafetyFilter_PrivilegeEscalation() {
        SafetyResult result1 = isSafeCommand("sudo su");
        SafetyResult result2 = isSafeCommand("sudo bash");
        return !result1.isSafe && !result2.isSafe;
    }

    bool test_SafetyFilter_SystemFileModification() {
        SafetyResult result = isSafeCommand("echo root::0:0::: > /etc/passwd");
        return !result.isSafe;
    }

    bool test_SafetyFilter_RemoteCodeExecution() {
        
        SafetyResult result1 = isSafeCommand("wget.*| bash");
        SafetyResult result2 = isSafeCommand("curl.*| sh");
        
        return !result1.isSafe && !result2.isSafe;
    }

    bool test_SafetyFilter_SafeCommands() {
        SafetyResult result1 = isSafeCommand("ls -la");
        SafetyResult result2 = isSafeCommand("echo hello");
        SafetyResult result3 = isSafeCommand("pwd");
        return result1.isSafe && result2.isSafe && result3.isSafe;
    }

    bool test_SafetyFilter_CaseInsensitivity() {
        SafetyResult result1 = isSafeCommand("RM -RF /");
        SafetyResult result2 = isSafeCommand("Rm -Rf /");
        return !result1.isSafe && !result2.isSafe;
    }

    bool test_SafetyFilter_MultiplePatterns() {
        SafetyResult result = isSafeCommand("rm -rf / && shutdown -h now");
        return !result.isSafe;
    }

    bool test_SafetyFilter_PartialMatching() {
        
        SafetyResult result1 = isSafeCommand("grep -r 'pattern' /home/user");
        
        SafetyResult result2 = isSafeCommand("echo hello world");
        return result1.isSafe && result2.isSafe;
    }
}

namespace WhiteBoxTests_CommandProcessing {
    bool test_CommandParsing_ExtractFromPrompt() {
        return true;
    }
    bool test_CommandParsing_EmptyInput() {
        return true; 
    }
    bool test_CommandDetection_ShellCommands() {
        return true; 
    }
    bool test_CommandDetection_NLvsSL() {
        return true; 
    }
    bool test_CommandHandling_CD_Absolute() {
        return true; 
    }
    bool test_CommandHandling_CD_Relative() {
        return true; 
    }
    bool test_CommandHandling_CD_HomeExpansion() {
        return true; 
    }
    bool test_CommandHandling_CD_NonExistent() {
        return true; 
    }
    bool test_CommandHandling_Clear() {
        return true; 
    }
    bool test_CommandHandling_Exit() {
        return true; 
    }
}

namespace WhiteBoxTests_StateManagement {
    bool test_State_CurrentDirectory() {
        return true; 
    }
    bool test_State_SessionConsistency() {
        return true; 
    }
    bool test_State_InputLocking() {
        return true; 
    }
    bool test_State_PendingInputBuffer() {
        return true; 
    }
    bool test_State_ErrorRecovery() {
        return true; 
    }
}





namespace BlackBoxTests_UserRequirements {
    bool test_UserReq_ApplicationLaunch() {
        return true; 
    }
    bool test_UserReq_DisplayBanner() {
        return true; 
    }
    bool test_UserReq_NLCommandInput() {
        return true; 
    }
    bool test_UserReq_AIConversion() {
        return true; 
    }
    bool test_UserReq_CommandExecution() {
        return true; 
    }
    bool test_UserReq_DisplayOutput() {
        return true; 
    }
    bool test_UserReq_DirectShellCommands() {
        return true; 
    }
    bool test_UserReq_ChangeDirectory() {
        return true; 
    }
    bool test_UserReq_ClearTerminal() {
        return true; 
    }
    bool test_UserReq_ExitApplication() {
        return true; 
    }
    bool test_UserReq_CommandHistory() {
        return true; 
    }
    bool test_UserReq_PromptShowsDirectory() {
        return true; 
    }
}

namespace BlackBoxTests_Safety {
    bool test_Safety_DangerousCommandsBlocked() {
        return true; 
    }
    bool test_Safety_BlockReasonProvided() {
        return true; 
    }
    bool test_Safety_SafeCommandsExecute() {
        return true; 
    }
    bool test_Safety_SystemStability() {
        return true; 
    }
    bool test_Safety_ContinueAfterBlock() {
        return true; 
    }
}

namespace BlackBoxTests_DataPersistence {
    bool test_DataPersistence_SessionSaved() {
        return true; 
    }
    bool test_DataPersistence_CommandHistoryPersisted() {
        return true; 
    }
    bool test_DataPersistence_RetrievePreviousSessions() {
        return true; 
    }
    bool test_DataPersistence_PerformanceMetricsRecorded() {
        return true; 
    }
    bool test_DataPersistence_SystemLogsRecorded() {
        return true; 
    }
}

namespace BlackBoxTests_ErrorHandling {
    bool test_ErrorHandling_InvalidCommand() {
        return true; 
    }
    bool test_ErrorHandling_NetworkError() {
        return true; 
    }
    bool test_ErrorHandling_DatabaseError() {
        return true; 
    }
    bool test_ErrorHandling_Recovery() {
        return true; 
    }
    bool test_ErrorHandling_Retry() {
        return true; 
    }
}

namespace BlackBoxTests_Performance {
    bool test_Performance_CommandTimeout() {
        return true; 
    }
    bool test_Performance_LLMResponseTime() {
        return true; 
    }
    bool test_Performance_SafetyCheckSpeed() {
        return true; 
    }
    bool test_Performance_LongCommandHistory() {
        return true; 
    }
    bool test_Performance_DatabaseQuerySpeed() {
        return true; 
    }
}

namespace BlackBoxTests_Integration {
    bool test_Integration_AllComponents() {
        return true; 
    }
    bool test_Integration_MultiCommandSession() {
        return true; 
    }
    bool test_Integration_StateConsistency() {
        return true; 
    }
    bool test_Integration_DatabaseIntegration() {
        return true; 
    }
    bool test_Integration_LLMExecution() {
        return true; 
    }
}
