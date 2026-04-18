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
              << std::setw(70) << test.name.toStdString();
    std::cout << (result ? "✓" : "✗") << std::endl;
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
  report += "CONSOLIDATED TEST REPORT\n\n";
  report += QString("Total Tests: %1\n").arg(stats.totalTests);
  report += QString("Passed:      %1\n").arg(stats.passedTests);
  report += QString("Failed:      %1\n").arg(stats.failedTests);

  file.write(report.toUtf8());
  file.close();
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
  std::cout << std::string(70, '=') << "\n" << std::endl;
}

qint64 TestSuite::getCurrentTimeMs() const {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

namespace WhiteBoxTests {

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
  return history.size() == 1 && history[0].commandId == cmdId;
}

bool test_SafetyFilter_SecurityRules() {
  SafetyResult res1 = isSafeCommand(":(){:|:&};:");
  SafetyResult res2 = isSafeCommand("rm -rf /");
  SafetyResult res3 = isSafeCommand("ls -la");
  SafetyResult res4 = isSafeCommand("sudo su");

  return (!res1.isSafe && res1.reason.contains("Fork bomb")) &&
         (!res2.isSafe && res2.reason.contains("root filesystem")) &&
         (res3.isSafe) && (!res4.isSafe);
}

bool test_CommandProcessing_InternalLogic() {
  DataAccessLayer dal("test_processing.db");
  dal.initializeDatabase();
  int sid = dal.createSession("Test", "/");

  int id1 = dal.recordCommand(sid, "list files", "ls", true);
  int id2 = dal.recordCommand(sid, "delete all", "rm -rf /", false);

  QList<CommandRecord> blocked = dal.getBlockedCommands(sid);
  return blocked.size() == 1 && blocked[0].commandId == id2;
}
} // namespace WhiteBoxTests

namespace BlackBoxTests {

bool test_UserReq_EndToEndFlow() {
  DataAccessLayer dal("test_blackbox_flow.db");
  dal.initializeDatabase();
  int sessionId = dal.createSession("E2E Session", "/home/user");

  QString userPrompt = "show all files";
  QString generatedCmd = "ls -a";
  dal.recordCommand(sessionId, userPrompt, generatedCmd, true);

  QList<CommandRecord> history = dal.getCommandHistory(sessionId);
  return history.size() > 0 && history[0].userInput == userPrompt;
}

bool test_Safety_UserProtection() {
  QString dangerousInput = "rm -rf /";
  SafetyResult result = isSafeCommand(dangerousInput);

  return result.isSafe == false && !result.reason.isEmpty();
}

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
  return sessions.size() >= 1 &&
         sessions[0].sessionName == "Persistent Session";
}
} // namespace BlackBoxTests
