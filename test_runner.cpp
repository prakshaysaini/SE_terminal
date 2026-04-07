#include "TestSuite.h"
#include <QCoreApplication>
#include <iostream>
#include <iomanip>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    TestSuite suite;

    suite.addTest("WB 1.1: Database Initialization",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_DatabaseInitialization);
    suite.addTest("WB 1.2: Create Session (Valid Input)",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_CreateSession_ValidInput);
    suite.addTest("WB 1.3: Create Session (Empty Name)",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_CreateSession_EmptyName);
    suite.addTest("WB 1.4: Get Non-existent Session",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_GetSession_NonExistent);
    suite.addTest("WB 1.5: Update Session End Time",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_UpdateSessionEndTime);
    suite.addTest("WB 1.6: Update Session Status",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_UpdateSessionStatus);
    suite.addTest("WB 1.7: Record Command",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_RecordCommand);
    suite.addTest("WB 1.8: Update Command Execution",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_UpdateCommandExecution);
    suite.addTest("WB 1.9: Get Command History (Order)",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_GetCommandHistory_Order);
    suite.addTest("WB 1.10: Get Failed Commands",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_GetFailedCommands_Filtering);
    suite.addTest("WB 1.11: Get Blocked Commands",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_GetBlockedCommands_UnsafeFiltering);
    suite.addTest("WB 1.12: Command Cache Set and Get",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_CommandCache_SetAndGet);
    suite.addTest("WB 1.13: Cache Usage Count",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_CacheUsageCount);
    suite.addTest("WB 1.14: System Log Creation",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_SystemLog_Creation);
    suite.addTest("WB 1.15: System Log Level Filtering",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_SystemLog_LevelFiltering);
    suite.addTest("WB 1.16: Performance Metrics Recording",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_PerformanceMetrics_Recording);
    suite.addTest("WB 1.17: Average LLM Response Time",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_AverageLLMResponseTime);
    suite.addTest("WB 1.18: Average Safety Check Time",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_AverageSafetyCheckTime);
    suite.addTest("WB 1.19: Command Count by Status",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_CommandCountByStatus);
    suite.addTest("WB 1.20: Get Safe Command Count",
                 "White Box", &WhiteBoxTests_DAL::test_DAL_GetSafeCommandCount);

    suite.addTest("WB 2.1: Fork Bomb Detection",
                 "White Box", &WhiteBoxTests_SafetyFilter::test_SafetyFilter_ForkBombDetection);
    suite.addTest("WB 2.2: Root Filesystem Delete",
                 "White Box", &WhiteBoxTests_SafetyFilter::test_SafetyFilter_RootFsDelete);
    suite.addTest("WB 2.3: Home Directory Delete",
                 "White Box", &WhiteBoxTests_SafetyFilter::test_SafetyFilter_HomeDirDelete);
    suite.addTest("WB 2.4: Recursive Delete",
                 "White Box", &WhiteBoxTests_SafetyFilter::test_SafetyFilter_RecursiveDelete);
    suite.addTest("WB 2.5: Filesystem Formatting",
                 "White Box", &WhiteBoxTests_SafetyFilter::test_SafetyFilter_FilesystemFormat);
    suite.addTest("WB 2.6: Block Device Write",
                 "White Box", &WhiteBoxTests_SafetyFilter::test_SafetyFilter_BlockDeviceWrite);
    suite.addTest("WB 2.7: Shutdown Commands",
                 "White Box", &WhiteBoxTests_SafetyFilter::test_SafetyFilter_ShutdownCommands);
    suite.addTest("WB 2.8: Kill All Processes",
                 "White Box", &WhiteBoxTests_SafetyFilter::test_SafetyFilter_KillAllProcesses);
    suite.addTest("WB 2.9: Privilege Escalation",
                 "White Box", &WhiteBoxTests_SafetyFilter::test_SafetyFilter_PrivilegeEscalation);
    suite.addTest("WB 2.10: System File Modification",
                 "White Box", &WhiteBoxTests_SafetyFilter::test_SafetyFilter_SystemFileModification);
    suite.addTest("WB 2.11: Remote Code Execution",
                 "White Box", &WhiteBoxTests_SafetyFilter::test_SafetyFilter_RemoteCodeExecution);
    suite.addTest("WB 2.12: Safe Commands Pass",
                 "White Box", &WhiteBoxTests_SafetyFilter::test_SafetyFilter_SafeCommands);
    suite.addTest("WB 2.13: Case Insensitivity",
                 "White Box", &WhiteBoxTests_SafetyFilter::test_SafetyFilter_CaseInsensitivity);
    suite.addTest("WB 2.14: Multiple Patterns",
                 "White Box", &WhiteBoxTests_SafetyFilter::test_SafetyFilter_MultiplePatterns);
    suite.addTest("WB 2.15: Partial Pattern Matching",
                 "White Box", &WhiteBoxTests_SafetyFilter::test_SafetyFilter_PartialMatching);

    suite.addTest("WB 3.1: Command Parsing",
                 "White Box", &WhiteBoxTests_CommandProcessing::test_CommandParsing_ExtractFromPrompt);
    suite.addTest("WB 3.2: Empty Input Handling",
                 "White Box", &WhiteBoxTests_CommandProcessing::test_CommandParsing_EmptyInput);
    suite.addTest("WB 3.3: Shell Command Detection",
                 "White Box", &WhiteBoxTests_CommandProcessing::test_CommandDetection_ShellCommands);
    suite.addTest("WB 3.4: Natural Language vs Shell",
                 "White Box", &WhiteBoxTests_CommandProcessing::test_CommandDetection_NLvsSL);
    suite.addTest("WB 3.5: CD Absolute Path",
                 "White Box", &WhiteBoxTests_CommandProcessing::test_CommandHandling_CD_Absolute);
    suite.addTest("WB 3.6: CD Relative Path",
                 "White Box", &WhiteBoxTests_CommandProcessing::test_CommandHandling_CD_Relative);
    suite.addTest("WB 3.7: CD Home Expansion",
                 "White Box", &WhiteBoxTests_CommandProcessing::test_CommandHandling_CD_HomeExpansion);
    suite.addTest("WB 3.8: CD Non-existent",
                 "White Box", &WhiteBoxTests_CommandProcessing::test_CommandHandling_CD_NonExistent);
    suite.addTest("WB 3.9: Clear Command",
                 "White Box", &WhiteBoxTests_CommandProcessing::test_CommandHandling_Clear);
    suite.addTest("WB 3.10: Exit Command",
                 "White Box", &WhiteBoxTests_CommandProcessing::test_CommandHandling_Exit);

    suite.addTest("WB 4.1: Current Directory Tracking",
                 "White Box", &WhiteBoxTests_StateManagement::test_State_CurrentDirectory);
    suite.addTest("WB 4.2: Session State Consistency",
                 "White Box", &WhiteBoxTests_StateManagement::test_State_SessionConsistency);
    suite.addTest("WB 4.3: Input Lock Mechanism",
                 "White Box", &WhiteBoxTests_StateManagement::test_State_InputLocking);
    suite.addTest("WB 4.4: Pending Input Buffer",
                 "White Box", &WhiteBoxTests_StateManagement::test_State_PendingInputBuffer);
    suite.addTest("WB 4.5: Error Recovery",
                 "White Box", &WhiteBoxTests_StateManagement::test_State_ErrorRecovery);
    

    suite.addTest("BB 1.1: Application Launch",
                 "Black Box", &BlackBoxTests_UserRequirements::test_UserReq_ApplicationLaunch);
    suite.addTest("BB 1.2: Display Banner",
                 "Black Box", &BlackBoxTests_UserRequirements::test_UserReq_DisplayBanner);
    suite.addTest("BB 1.3: Natural Language Input",
                 "Black Box", &BlackBoxTests_UserRequirements::test_UserReq_NLCommandInput);
    suite.addTest("BB 1.4: AI Command Conversion",
                 "Black Box", &BlackBoxTests_UserRequirements::test_UserReq_AIConversion);
    suite.addTest("BB 1.5: Command Execution",
                 "Black Box", &BlackBoxTests_UserRequirements::test_UserReq_CommandExecution);
    suite.addTest("BB 1.6: Display Output",
                 "Black Box", &BlackBoxTests_UserRequirements::test_UserReq_DisplayOutput);
    suite.addTest("BB 1.7: Direct Shell Commands",
                 "Black Box", &BlackBoxTests_UserRequirements::test_UserReq_DirectShellCommands);
    suite.addTest("BB 1.8: Change Directory",
                 "Black Box", &BlackBoxTests_UserRequirements::test_UserReq_ChangeDirectory);
    suite.addTest("BB 1.9: Clear Terminal",
                 "Black Box", &BlackBoxTests_UserRequirements::test_UserReq_ClearTerminal);
    suite.addTest("BB 1.10: Exit Application",
                 "Black Box", &BlackBoxTests_UserRequirements::test_UserReq_ExitApplication);
    suite.addTest("BB 1.11: Command History",
                 "Black Box", &BlackBoxTests_UserRequirements::test_UserReq_CommandHistory);
    suite.addTest("BB 1.12: Prompt Shows Directory",
                 "Black Box", &BlackBoxTests_UserRequirements::test_UserReq_PromptShowsDirectory);

    suite.addTest("BB 2.1: Dangerous Commands Blocked",
                 "Black Box", &BlackBoxTests_Safety::test_Safety_DangerousCommandsBlocked);
    suite.addTest("BB 2.2: Block Reason Provided",
                 "Black Box", &BlackBoxTests_Safety::test_Safety_BlockReasonProvided);
    suite.addTest("BB 2.3: Safe Commands Execute",
                 "Black Box", &BlackBoxTests_Safety::test_Safety_SafeCommandsExecute);
    suite.addTest("BB 2.4: System Stability",
                 "Black Box", &BlackBoxTests_Safety::test_Safety_SystemStability);
    suite.addTest("BB 2.5: Continue After Block",
                 "Black Box", &BlackBoxTests_Safety::test_Safety_ContinueAfterBlock);

    suite.addTest("BB 3.1: Sessions Saved",
                 "Black Box", &BlackBoxTests_DataPersistence::test_DataPersistence_SessionSaved);
    suite.addTest("BB 3.2: Command History Persisted",
                 "Black Box", &BlackBoxTests_DataPersistence::test_DataPersistence_CommandHistoryPersisted);
    suite.addTest("BB 3.3: Retrieve Previous Sessions",
                 "Black Box", &BlackBoxTests_DataPersistence::test_DataPersistence_RetrievePreviousSessions);
    suite.addTest("BB 3.4: Performance Metrics",
                 "Black Box", &BlackBoxTests_DataPersistence::test_DataPersistence_PerformanceMetricsRecorded);
    suite.addTest("BB 3.5: System Logs Recorded",
                 "Black Box", &BlackBoxTests_DataPersistence::test_DataPersistence_SystemLogsRecorded);

    suite.addTest("BB 4.1: Invalid Command Error",
                 "Black Box", &BlackBoxTests_ErrorHandling::test_ErrorHandling_InvalidCommand);
    suite.addTest("BB 4.2: Network Error Handling",
                 "Black Box", &BlackBoxTests_ErrorHandling::test_ErrorHandling_NetworkError);
    suite.addTest("BB 4.3: Database Error Handling",
                 "Black Box", &BlackBoxTests_ErrorHandling::test_ErrorHandling_DatabaseError);
    suite.addTest("BB 4.4: Recovery from Errors",
                 "Black Box", &BlackBoxTests_ErrorHandling::test_ErrorHandling_Recovery);
    suite.addTest("BB 4.5: Retry Failed Commands",
                 "Black Box", &BlackBoxTests_ErrorHandling::test_ErrorHandling_Retry);

    suite.addTest("BB 5.1: Command Timeout",
                 "Black Box", &BlackBoxTests_Performance::test_Performance_CommandTimeout);
    suite.addTest("BB 5.2: LLM Response Time",
                 "Black Box", &BlackBoxTests_Performance::test_Performance_LLMResponseTime);
    suite.addTest("BB 5.3: Safety Check Speed",
                 "Black Box", &BlackBoxTests_Performance::test_Performance_SafetyCheckSpeed);
    suite.addTest("BB 5.4: Long Command History",
                 "Black Box", &BlackBoxTests_Performance::test_Performance_LongCommandHistory);
    suite.addTest("BB 5.5: Database Query Speed",
                 "Black Box", &BlackBoxTests_Performance::test_Performance_DatabaseQuerySpeed);

    suite.addTest("BB 6.1: All Components Integration",
                 "Black Box", &BlackBoxTests_Integration::test_Integration_AllComponents);
    suite.addTest("BB 6.2: Multi-command Session",
                 "Black Box", &BlackBoxTests_Integration::test_Integration_MultiCommandSession);
    suite.addTest("BB 6.3: State Consistency",
                 "Black Box", &BlackBoxTests_Integration::test_Integration_StateConsistency);
    suite.addTest("BB 6.4: Database Integration",
                 "Black Box", &BlackBoxTests_Integration::test_Integration_DatabaseIntegration);
    suite.addTest("BB 6.5: LLM Execution Integration",
                 "Black Box", &BlackBoxTests_Integration::test_Integration_LLMExecution);

    suite.runAll();

    suite.generateReport("test_report.txt");
    
    return 0;
}
