#include "TestSuite.h"
#include <QCoreApplication>
#include <iostream>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    TestSuite suite;

    // --- DAL Module: 8 Test Cases ---
    suite.addTest("TC-01: Database Initialization and Connection",
                 "White Box", &DALTests::test_DatabaseInitialization);
    
    suite.addTest("TC-02: Session Creation with Valid Data",
                 "White Box", &DALTests::test_SessionCreation);
    
    suite.addTest("TC-03: Session Retrieval by ID",
                 "White Box", &DALTests::test_SessionRetrieval);
    
    suite.addTest("TC-04: Command Recording and History Retrieval",
                 "White Box", &DALTests::test_CommandRecording);
    
    suite.addTest("TC-05: Safety Filter Blocks Dangerous Commands",
                 "Black Box", &DALTests::test_SafetyFilterBlocking);
    
    suite.addTest("TC-06: Safety Filter Allows Safe Commands",
                 "Black Box", &DALTests::test_SafetyFilterAllowing);
    
    suite.addTest("TC-07: Command Cache Insertion and Lookup",
                 "White Box", &DALTests::test_CommandCaching);
    
    suite.addTest("TC-08: Blocked Command Count Accuracy",
                 "White Box", &DALTests::test_BlockedCommandCount);
    
    suite.runAll();
    suite.generateReport("assignment9.txt");
    
    return 0;
}
