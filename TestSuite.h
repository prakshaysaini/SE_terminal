#ifndef TEST_SUITE_H
#define TEST_SUITE_H

#include <QDateTime>
#include <QList>
#include <QString>
#include <cassert>
#include <functional>
#include <iomanip>
#include <iostream>
#include <vector>

struct TestResult {
  QString testName;
  QString category;
  bool passed;
  QString message;
  double executionTimeMs;

  QString getStatus() const { return passed ? "✓ PASSED" : "✗ FAILED"; }
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

  const QList<TestResult> &getResults() const;
  const TestStatistics &getStatistics() const;

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

namespace WhiteBoxTests {

bool test_DAL_CoreFunctionality();

bool test_SafetyFilter_SecurityRules();

bool test_CommandProcessing_InternalLogic();
} // namespace WhiteBoxTests

namespace BlackBoxTests {

bool test_UserReq_EndToEndFlow();

bool test_Safety_UserProtection();

bool test_System_PersistenceAndStability();
} // namespace BlackBoxTests

#endif
