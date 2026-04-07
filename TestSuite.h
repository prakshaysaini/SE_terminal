

#ifndef TEST_SUITE_H
#define TEST_SUITE_H

#include <QString>
#include <QList>
#include <QDateTime>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <functional>
#include <vector>


struct TestResult {
    QString testName;
    QString category;  
    bool passed;
    QString message;
    double executionTimeMs;
    
    QString getStatus() const {
        return passed ? "✓ PASSED" : "✗ FAILED";
    }
};


struct TestStatistics {
    int totalTests = 0;
    int passedTests = 0;
    int failedTests = 0;
    double totalTimeMs = 0.0;
    int whiteBoxTests = 0;
    int whiteBoxPassed = 0;
    int blackBoxTests = 0;
    int blackBoxPassed = 0;
    
    double getPassRate() const {
        return totalTests > 0 ? (100.0 * passedTests / totalTests) : 0.0;
    }
    
    double getWhiteBoxPassRate() const {
        return whiteBoxTests > 0 ? (100.0 * whiteBoxPassed / whiteBoxTests) : 0.0;
    }
    
    double getBlackBoxPassRate() const {
        return blackBoxTests > 0 ? (100.0 * blackBoxPassed / blackBoxTests) : 0.0;
    }
};


class TestSuite {
public:
    TestSuite() : stats() {}
    
    
    void addTest(const QString &name, const QString &category, 
                std::function<bool()> testFunc);
    
    
    void runAll();
    
    
    const QList<TestResult>& getResults() const;
    const TestStatistics& getStatistics() const;
    
    
    void generateReport(const QString &filePath = "test_report.txt");
    void printSummary();
    
private:
    QList<TestResult> results;
    TestStatistics stats;
    
    struct Test {
        QString name;
        QString category;
        std::function<bool()> func;
    };
    
    QList<Test> tests;
    
    qint64 getCurrentTimeMs() const;
};






namespace WhiteBoxTests_DAL {
    
    
    bool test_DAL_DatabaseInitialization();
    
    
    bool test_DAL_CreateSession_ValidInput();
    
    
    bool test_DAL_CreateSession_EmptyName();
    
    
    bool test_DAL_GetSession_NonExistent();
    
    
    bool test_DAL_UpdateSessionEndTime();
    
    
    bool test_DAL_UpdateSessionStatus();
    
    
    bool test_DAL_RecordCommand();
    
    
    bool test_DAL_UpdateCommandExecution();
    
    
    bool test_DAL_GetCommandHistory_Order();
    
    
    bool test_DAL_GetFailedCommands_Filtering();
    
    
    bool test_DAL_GetBlockedCommands_UnsafeFiltering();
    
    
    bool test_DAL_CommandCache_SetAndGet();
    
    
    bool test_DAL_CacheUsageCount();
    
    
    bool test_DAL_SystemLog_Creation();
    
    
    bool test_DAL_SystemLog_LevelFiltering();
    
    
    bool test_DAL_PerformanceMetrics_Recording();
    
    
    bool test_DAL_AverageLLMResponseTime();
    
    
    bool test_DAL_AverageSafetyCheckTime();
    
    
    bool test_DAL_CommandCountByStatus();
    
    
    bool test_DAL_GetSafeCommandCount();
}


namespace WhiteBoxTests_SafetyFilter {
    
    
    bool test_SafetyFilter_ForkBombDetection();
    
    
    bool test_SafetyFilter_RootFsDelete();
    
    
    bool test_SafetyFilter_HomeDirDelete();
    
    
    bool test_SafetyFilter_RecursiveDelete();
    
    
    bool test_SafetyFilter_FilesystemFormat();
    
    
    bool test_SafetyFilter_BlockDeviceWrite();
    
    
    bool test_SafetyFilter_ShutdownCommands();
    
    
    bool test_SafetyFilter_KillAllProcesses();
    
    
    bool test_SafetyFilter_PrivilegeEscalation();
    
    
    bool test_SafetyFilter_SystemFileModification();
    
    
    bool test_SafetyFilter_RemoteCodeExecution();
    
    
    bool test_SafetyFilter_SafeCommands();
    
    
    bool test_SafetyFilter_CaseInsensitivity();
    
    
    bool test_SafetyFilter_MultiplePatterns();
    
    
    bool test_SafetyFilter_PartialMatching();
}


namespace WhiteBoxTests_CommandProcessing {
    
    
    bool test_CommandParsing_ExtractFromPrompt();
    
    
    bool test_CommandParsing_EmptyInput();
    
    
    bool test_CommandDetection_ShellCommands();
    
    
    bool test_CommandDetection_NLvsSL();
    
    
    bool test_CommandHandling_CD_Absolute();
    
    
    bool test_CommandHandling_CD_Relative();
    
    
    bool test_CommandHandling_CD_HomeExpansion();
    
    
    bool test_CommandHandling_CD_NonExistent();
    
    
    bool test_CommandHandling_Clear();
    
    
    bool test_CommandHandling_Exit();
}


namespace WhiteBoxTests_StateManagement {
    
    
    bool test_State_CurrentDirectory();
    
    
    bool test_State_SessionConsistency();
    
    
    bool test_State_InputLocking();
    
    
    bool test_State_PendingInputBuffer();
    
    
    bool test_State_ErrorRecovery();
}






namespace BlackBoxTests_UserRequirements {
    
    
    bool test_UserReq_ApplicationLaunch();
    
    
    bool test_UserReq_DisplayBanner();
    
    
    bool test_UserReq_NLCommandInput();
    
    
    bool test_UserReq_AIConversion();
    
    
    bool test_UserReq_CommandExecution();
    
    
    bool test_UserReq_DisplayOutput();
    
    
    bool test_UserReq_DirectShellCommands();
    
    
    bool test_UserReq_ChangeDirectory();
    
    
    bool test_UserReq_ClearTerminal();
    
    
    bool test_UserReq_ExitApplication();
    
    
    bool test_UserReq_CommandHistory();
    
    
    bool test_UserReq_PromptShowsDirectory();
}


namespace BlackBoxTests_Safety {
    
    
    bool test_Safety_DangerousCommandsBlocked();
    
    
    bool test_Safety_BlockReasonProvided();
    
    
    bool test_Safety_SafeCommandsExecute();
    
    
    bool test_Safety_SystemStability();
    
    
    bool test_Safety_ContinueAfterBlock();
}


namespace BlackBoxTests_DataPersistence {
    
    
    bool test_DataPersistence_SessionSaved();
    
    
    bool test_DataPersistence_CommandHistoryPersisted();
    
    
    bool test_DataPersistence_RetrievePreviousSessions();
    
    
    bool test_DataPersistence_PerformanceMetricsRecorded();
    
    
    bool test_DataPersistence_SystemLogsRecorded();
}


namespace BlackBoxTests_ErrorHandling {
    
    
    bool test_ErrorHandling_InvalidCommand();
    
    
    bool test_ErrorHandling_NetworkError();
    
    
    bool test_ErrorHandling_DatabaseError();
    
    
    bool test_ErrorHandling_Recovery();
    
    
    bool test_ErrorHandling_Retry();
}


namespace BlackBoxTests_Performance {
    
    
    bool test_Performance_CommandTimeout();
    
    
    bool test_Performance_LLMResponseTime();
    
    
    bool test_Performance_SafetyCheckSpeed();
    
    
    bool test_Performance_LongCommandHistory();
    
    
    bool test_Performance_DatabaseQuerySpeed();
}


namespace BlackBoxTests_Integration {
    
    
    bool test_Integration_AllComponents();
    
    
    bool test_Integration_MultiCommandSession();
    
    
    bool test_Integration_StateConsistency();
    
    
    bool test_Integration_DatabaseIntegration();
    
    
    bool test_Integration_LLMExecution();
}

#endif 
