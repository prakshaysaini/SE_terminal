#include "TestSuite.h"
#include "DataAccessLayer.h"
#include "SafetyFilter.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include <chrono>
#include <ctime>

// ===========================================================================
//  TestSuite Framework Implementation
// ===========================================================================

void TestSuite::addTest(const QString &name, const QString &category,
                        std::function<bool()> testFunc) {
  tests.append({name, category, testFunc});
}

void TestSuite::runAll() {
  std::cout << "\n" << std::string(70, '=') << std::endl;
  std::cout << "  CONSOLIDATED TERMINAL APPLICATION - TEST SUITE" << std::endl;
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
      if (result)
        stats.whiteBoxPassed++;
    } else {
      stats.blackBoxTests++;
      if (result)
        stats.blackBoxPassed++;
    }

    if (result) {
      stats.passedTests++;
    } else {
      stats.failedTests++;
    }

    std::cout << std::left << std::setw(3) << testNumber << ". "
              << std::setw(58) << test.name.toStdString()
              << " [" << std::setw(5) << executionTime << " ms] "
              << (result ? "✓ PASS" : "✗ FAIL") << std::endl;
    testNumber++;
  }

  printSummary();
}

const QList<TestResult> &TestSuite::getResults() const { return results; }

const TestStatistics &TestSuite::getStatistics() const { return stats; }

void TestSuite::generateReport(const QString &filePath) {
  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    std::cerr << "Failed to open report file: " << filePath.toStdString()
              << std::endl;
    return;
  }

  QString report;
  report += "CONSOLIDATED TEST REPORT\n";
  report += QString("Date: %1\n\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
  report += QString("Total Tests: %1\n").arg(stats.totalTests);
  report += QString("Passed:      %1\n").arg(stats.passedTests);
  report += QString("Failed:      %1\n").arg(stats.failedTests);
  report += QString("Pass Rate:   %1%\n\n").arg(stats.getPassRate(), 0, 'f', 1);

  report += QString("White Box Tests: %1/%2 passed (%3%)\n")
    .arg(stats.whiteBoxPassed).arg(stats.whiteBoxTests).arg(stats.getWhiteBoxPassRate(), 0, 'f', 1);
  report += QString("Black Box Tests: %1/%2 passed (%3%)\n\n")
    .arg(stats.blackBoxPassed).arg(stats.blackBoxTests).arg(stats.getBlackBoxPassRate(), 0, 'f', 1);

  report += QString(70, '-') + "\n";
  report += "DETAILED RESULTS:\n";
  report += QString(70, '-') + "\n\n";

  for (const TestResult &r : results) {
    report += QString("[%1] %2\n").arg(r.getStatus(), r.testName);
    report += QString("   Category: %1 | Time: %2 ms\n").arg(r.category).arg(r.executionTimeMs);
    if (!r.passed) {
      report += QString("   Message: %1\n").arg(r.message);
    }
    report += "\n";
  }

  file.write(report.toUtf8());
  file.close();
  std::cout << "Report saved to: " << filePath.toStdString() << std::endl;
}

void TestSuite::printSummary() {
  std::cout << "\n" << std::string(70, '=') << std::endl;
  std::cout << "  TEST SUMMARY" << std::endl;
  std::cout << std::string(70, '=') << std::endl;
  std::cout << "Total Tests: " << stats.totalTests
            << " (White Box: " << stats.whiteBoxTests
            << ", Black Box: " << stats.blackBoxTests << ")\n";
  std::cout << "Passed:      " << stats.passedTests << "\n";
  std::cout << "Failed:      " << stats.failedTests << "\n";
  std::cout << "Pass Rate:   " << std::fixed << std::setprecision(1)
            << stats.getPassRate() << "%\n";
  std::cout << "Total Time:  " << std::fixed << std::setprecision(1)
            << stats.totalTimeMs << " ms\n";
  std::cout << std::string(70, '=') << "\n" << std::endl;
}

qint64 TestSuite::getCurrentTimeMs() const {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}


// ###########################################################################
//
//  PART B — WHITE BOX TESTS (3 tests)
//  Tests with full knowledge of internal code structure, logic, and paths.
//
// ###########################################################################

namespace WhiteBoxTests {

// WB-1: Data Access Layer Core Functionality
// Tests internal DAL workflow: init DB → create session → record command → retrieve history
bool test_DAL_CoreFunctionality() {
  DataAccessLayer dal("test_whitebox_dal.db");
  if (!dal.initializeDatabase() || !dal.isConnected())
    return false;

  int sessionId = dal.createSession("WhiteBox Session", "/home/test");
  if (sessionId <= 0)
    return false;

  int cmdId = dal.recordCommand(sessionId, "list files", "ls -la", true);
  if (cmdId <= 0)
    return false;

  QList<CommandRecord> history = dal.getCommandHistory(sessionId);
  bool result = history.size() == 1 && history[0].commandId == cmdId;

  QFile::remove("test_whitebox_dal.db");
  return result;
}

// WB-2: Safety Filter Security Rules
// Tests the internal pattern matching logic of the safety filter against
// known dangerous patterns and verifies correct reason strings.
bool test_SafetyFilter_SecurityRules() {
  SafetyResult res1 = isSafeCommand(":(){:|:&};:");
  SafetyResult res2 = isSafeCommand("rm -rf /");
  SafetyResult res3 = isSafeCommand("ls -la");
  SafetyResult res4 = isSafeCommand("sudo su");

  return (!res1.isSafe && res1.reason.contains("Fork bomb")) &&
         (!res2.isSafe && res2.reason.contains("root filesystem")) &&
         (res3.isSafe) && (!res4.isSafe);
}

// WB-3: Command Processing Internal Logic
// Tests internal branching: records a safe and unsafe command, then verifies
// getBlockedCommands() returns only the unsafe one.
bool test_CommandProcessing_InternalLogic() {
  DataAccessLayer dal("test_processing.db");
  dal.initializeDatabase();
  int sid = dal.createSession("Test", "/");

  int id1 = dal.recordCommand(sid, "list files", "ls", true);
  int id2 = dal.recordCommand(sid, "delete all", "rm -rf /", false);

  QList<CommandRecord> blocked = dal.getBlockedCommands(sid);
  bool result = blocked.size() == 1 && blocked[0].commandId == id2;

  QFile::remove("test_processing.db");
  return result;
}

} // namespace WhiteBoxTests


// ###########################################################################
//
//  PART B — BLACK BOX TESTS (3 tests)
//  Tests WITHOUT knowledge of internal code; only input → expected output.
//
// ###########################################################################

namespace BlackBoxTests {

// BB-1: User Requirements End-to-End Flow
// Simulates the user flow: provide a natural language prompt, expect it to be
// stored in command history with the correct user input.
bool test_UserReq_EndToEndFlow() {
  DataAccessLayer dal("test_blackbox_flow.db");
  dal.initializeDatabase();
  int sessionId = dal.createSession("E2E Session", "/home/user");

  QString userPrompt = "show all files";
  QString generatedCmd = "ls -a";
  dal.recordCommand(sessionId, userPrompt, generatedCmd, true);

  QList<CommandRecord> history = dal.getCommandHistory(sessionId);
  bool result = history.size() > 0 && history[0].userInput == userPrompt;

  QFile::remove("test_blackbox_flow.db");
  return result;
}

// BB-2: User Safety Protection and Blocking
// From the user's perspective: input a dangerous command, expect it to be
// blocked and a non-empty reason to be returned.
bool test_Safety_UserProtection() {
  QString dangerousInput = "rm -rf /";
  SafetyResult result = isSafeCommand(dangerousInput);

  return result.isSafe == false && !result.reason.isEmpty();
}

// BB-3: System Persistence and Stability
// Tests that data persists across DAL instance destruction and re-creation:
// create data, destroy DAL, create new DAL, verify data is still there.
bool test_System_PersistenceAndStability() {
  QString dbName = "test_persistence.db";
  {
    DataAccessLayer dal(dbName);
    dal.initializeDatabase();
    int sid = dal.createSession("Persistent Session", "/tmp");
    dal.recordCommand(sid, "test", "echo 1", true);
  }

  DataAccessLayer dal2(dbName);
  dal2.initializeDatabase();
  QList<Session> sessions = dal2.getAllSessions();
  bool result = sessions.size() >= 1 &&
         sessions[0].sessionName == "Persistent Session";

  QFile::remove(dbName);
  return result;
}

} // namespace BlackBoxTests


// ###########################################################################
//
//  PART A — DAL MODULE TEST CASES (8 comprehensive test cases)
//  Tests for the Data Access Layer component.
//
// ###########################################################################

namespace DALTests {

// Helper: clean up a test database file
static void cleanupTestDb(const QString &dbName) {
  QFile::remove(dbName);
}

// --------------------------------------------------------------------------
// TC-01: Database Initialization and Connection
// --------------------------------------------------------------------------
bool test_DatabaseInitialization() {
  QString dbName = "test_tc01_init.db";
  cleanupTestDb(dbName);

  DataAccessLayer dal(dbName);

  // Test: initializeDatabase() should succeed
  if (!dal.initializeDatabase()) {
    std::cout << "    DETAIL: initializeDatabase() returned false: "
              << dal.getLastError().toStdString() << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  // Test: isConnected() should return true after initialization
  if (!dal.isConnected()) {
    std::cout << "    DETAIL: isConnected() returned false after init" << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  // Test: Database file should exist on disk
  if (!QFile::exists(dbName)) {
    std::cout << "    DETAIL: Database file does not exist on disk" << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  cleanupTestDb(dbName);
  return true;
}

// --------------------------------------------------------------------------
// TC-02: Session Creation with Valid Data
// --------------------------------------------------------------------------
bool test_SessionCreation() {
  QString dbName = "test_tc02_session.db";
  cleanupTestDb(dbName);

  DataAccessLayer dal(dbName);
  dal.initializeDatabase();

  // Test: Creating session returns valid (positive) ID
  int sessionId = dal.createSession("Test Session Alpha", "/home/testuser");
  if (sessionId <= 0) {
    std::cout << "    DETAIL: createSession returned invalid ID: " << sessionId << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  // Test: Creating a second session returns a different, higher ID
  int sessionId2 = dal.createSession("Test Session Beta", "/tmp");
  if (sessionId2 <= sessionId) {
    std::cout << "    DETAIL: Second session ID not greater than first" << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  // Test: getAllSessions should return exactly 2 sessions
  QList<Session> sessions = dal.getAllSessions();
  if (sessions.size() != 2) {
    std::cout << "    DETAIL: Expected 2 sessions, got " << sessions.size() << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  cleanupTestDb(dbName);
  return true;
}

// --------------------------------------------------------------------------
// TC-03: Session Retrieval by ID
// --------------------------------------------------------------------------
bool test_SessionRetrieval() {
  QString dbName = "test_tc03_retrieval.db";
  cleanupTestDb(dbName);

  DataAccessLayer dal(dbName);
  dal.initializeDatabase();

  int sessionId = dal.createSession("Retrieval Test", "/home/user");
  Session session = dal.getSession(sessionId);

  // Test: Retrieved session has correct name
  if (session.sessionName != "Retrieval Test") {
    std::cout << "    DETAIL: Session name mismatch: " << session.sessionName.toStdString() << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  // Test: Retrieved session has correct working directory
  if (session.workingDirectory != "/home/user") {
    std::cout << "    DETAIL: Working dir mismatch: " << session.workingDirectory.toStdString() << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  // Test: Session status should be ACTIVE
  if (session.sessionStatus != "ACTIVE") {
    std::cout << "    DETAIL: Status mismatch: " << session.sessionStatus.toStdString() << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  // Test: Retrieving non-existent session returns invalid sessionId
  Session notFound = dal.getSession(99999);
  if (notFound.sessionId != -1) {
    std::cout << "    DETAIL: Non-existent session returned valid ID" << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  cleanupTestDb(dbName);
  return true;
}

// --------------------------------------------------------------------------
// TC-04: Command Recording and History Retrieval
// --------------------------------------------------------------------------
bool test_CommandRecording() {
  QString dbName = "test_tc04_commands.db";
  cleanupTestDb(dbName);

  DataAccessLayer dal(dbName);
  dal.initializeDatabase();
  int sid = dal.createSession("Command Test", "/home/user");

  // Record multiple commands
  int cmd1 = dal.recordCommand(sid, "list files", "ls -la", true);
  int cmd2 = dal.recordCommand(sid, "show disk usage", "df -h", true);
  int cmd3 = dal.recordCommand(sid, "delete everything", "rm -rf /", false, "Dangerous command");

  // Test: All command IDs should be valid
  if (cmd1 <= 0 || cmd2 <= 0 || cmd3 <= 0) {
    std::cout << "    DETAIL: Invalid command IDs: " << cmd1 << ", " << cmd2 << ", " << cmd3 << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  // Test: getCommandHistory returns all 3 commands
  QList<CommandRecord> history = dal.getCommandHistory(sid);
  if (history.size() != 3) {
    std::cout << "    DETAIL: Expected 3 commands, got " << history.size() << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  // Test: getCommandRecord retrieves correct userInput
  CommandRecord rec = dal.getCommandRecord(cmd1);
  if (rec.userInput != "list files") {
    std::cout << "    DETAIL: userInput mismatch: " << rec.userInput.toStdString() << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  // Test: getTotalCommandsExecuted returns 3
  int total = dal.getTotalCommandsExecuted(sid);
  if (total != 3) {
    std::cout << "    DETAIL: Total commands expected 3, got " << total << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  cleanupTestDb(dbName);
  return true;
}

// --------------------------------------------------------------------------
// TC-05: Safety Filter Blocks Dangerous Commands
// --------------------------------------------------------------------------
bool test_SafetyFilterBlocking() {
  struct TestCase {
    QString command;
    QString expectedReason;
  };

  QList<TestCase> dangerousCmds = {
    {":(){:|:&};:", "Fork bomb"},
    {"rm -rf /", "root filesystem"},
    {"rm -rf ~", "home directory"},
    {"sudo su", "root shell"},
    {"mkfs.ext4 /dev/sda", "Filesystem formatting"},
    {"dd if=/dev/zero of=/dev/sda", "Raw disk write"},
    {"shutdown -h now", "shutdown"},
    {"kill -9 -1", "Kill all"},
  };

  for (const TestCase &tc : dangerousCmds) {
    SafetyResult result = isSafeCommand(tc.command);
    if (result.isSafe) {
      std::cout << "    DETAIL: Command NOT blocked: " << tc.command.toStdString() << std::endl;
      return false;
    }
    if (!result.reason.contains(tc.expectedReason, Qt::CaseInsensitive)) {
      std::cout << "    DETAIL: Reason mismatch for '" << tc.command.toStdString()
                << "': got '" << result.reason.toStdString() << "'" << std::endl;
      return false;
    }
  }

  return true;
}

// --------------------------------------------------------------------------
// TC-06: Safety Filter Allows Safe Commands
// --------------------------------------------------------------------------
bool test_SafetyFilterAllowing() {
  QStringList safeCmds = {
    "ls -la",
    "pwd",
    "echo Hello World",
    "cat /etc/hostname",
    "grep -r pattern .",
    "find . -name '*.cpp'",
    "mkdir test_dir",
    "touch new_file.txt",
    "ps aux",
    "date",
  };

  for (const QString &cmd : safeCmds) {
    SafetyResult result = isSafeCommand(cmd);
    if (!result.isSafe) {
      std::cout << "    DETAIL: Safe command BLOCKED: " << cmd.toStdString()
                << " Reason: " << result.reason.toStdString() << std::endl;
      return false;
    }
  }

  return true;
}

// --------------------------------------------------------------------------
// TC-07: Command Cache Insertion and Lookup
// --------------------------------------------------------------------------
bool test_CommandCaching() {
  QString dbName = "test_tc07_cache.db";
  cleanupTestDb(dbName);

  DataAccessLayer dal(dbName);
  dal.initializeDatabase();

  // Insert a cache entry
  bool cached = dal.cacheCommand("hash_abc123", "show files", "ls -la");
  if (!cached) {
    std::cout << "    DETAIL: cacheCommand returned false" << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  // Retrieve the cached command
  QString result = dal.getCachedCommand("hash_abc123");
  if (result != "ls -la") {
    std::cout << "    DETAIL: Cached command mismatch: " << result.toStdString() << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  // Non-existent cache key returns empty string
  QString notFound = dal.getCachedCommand("nonexistent_hash");
  if (!notFound.isEmpty()) {
    std::cout << "    DETAIL: Non-existent key returned data" << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  // Duplicate insert should be ignored (INSERT OR IGNORE)
  dal.cacheCommand("hash_abc123", "show files", "ls -different");
  QString result2 = dal.getCachedCommand("hash_abc123");
  if (result2 != "ls -la") {
    std::cout << "    DETAIL: Duplicate insert overwrote existing data" << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  cleanupTestDb(dbName);
  return true;
}

// --------------------------------------------------------------------------
// TC-08: Blocked Command Count Accuracy
// --------------------------------------------------------------------------
bool test_BlockedCommandCount() {
  QString dbName = "test_tc08_blocked.db";
  cleanupTestDb(dbName);

  DataAccessLayer dal(dbName);
  dal.initializeDatabase();
  int sid = dal.createSession("Blocked Count Test", "/home");

  // Record 3 safe commands and 2 unsafe commands
  dal.recordCommand(sid, "list files", "ls", true);
  dal.recordCommand(sid, "show dir", "pwd", true);
  dal.recordCommand(sid, "disk space", "df -h", true);
  dal.recordCommand(sid, "destroy system", "rm -rf /", false, "dangerous");
  dal.recordCommand(sid, "fork bomb", ":(){:|:&};:", false, "fork bomb");

  int safeCount = dal.getSafeCommandCount(sid);
  int blockedCount = dal.getBlockedCommandCount(sid);

  if (safeCount != 3) {
    std::cout << "    DETAIL: Expected 3 safe, got " << safeCount << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  if (blockedCount != 2) {
    std::cout << "    DETAIL: Expected 2 blocked, got " << blockedCount << std::endl;
    cleanupTestDb(dbName);
    return false;
  }

  cleanupTestDb(dbName);
  return true;
}

} // namespace DALTests
