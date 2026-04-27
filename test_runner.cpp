#include "TestSuite.h"
#include <QCoreApplication>
#include <iostream>

// ===========================================================================
//  OLD ASSIGNMENT — White Box + Black Box Testing
//  Run: ./test_runner
//  Report: wb_bb_test_report.txt
// ===========================================================================

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    TestSuite suite;

    // --- White Box Tests (Internal Logic) ---
    suite.addTest("WB 1: Data Access Layer Core Functionality",
                 "White Box", &WhiteBoxTests::test_DAL_CoreFunctionality);
    
    suite.addTest("WB 2: Safety Filter Security Rules",
                 "White Box", &WhiteBoxTests::test_SafetyFilter_SecurityRules);
    
    suite.addTest("WB 3: Command Processing Internal Logic",
                 "White Box", &WhiteBoxTests::test_CommandProcessing_InternalLogic);

    // --- Black Box Tests (User Requirements) ---
    suite.addTest("BB 1: User Requirements End-to-End Flow",
                 "Black Box", &BlackBoxTests::test_UserReq_EndToEndFlow);
    
    suite.addTest("BB 2: User Safety Protection and Blocking",
                 "Black Box", &BlackBoxTests::test_Safety_UserProtection);
    
    suite.addTest("BB 3: System Persistence and Stability",
                 "Black Box", &BlackBoxTests::test_System_PersistenceAndStability);

    suite.runAll();
    suite.generateReport("wb_bb_test_report.txt");
    
    return 0;
}
