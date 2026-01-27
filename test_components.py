#!/usr/bin/env python3
"""
Test script for AI-Powered Terminal components
This script tests all modules without requiring GUI or API keys
"""

import sys
import os

def test_safety_filter():
    """Test the safety filter module"""
    print("\n" + "="*70)
    print("TESTING: Safety Filter Module")
    print("="*70)
    
    from safety_filter import SafetyFilter
    sf = SafetyFilter()
    
    # Test safe commands
    safe_tests = [
        ("ls -la", True),
        ("pwd", True),
        ("cat file.txt", True),
        ("echo 'hello'", True),
        ("find . -name '*.py'", True),
        ("df -h", True),
        ("whoami", True),
    ]
    
    # Test dangerous commands
    dangerous_tests = [
        ("rm -rf /", False),
        ("rm -rf /*", False),
        ("sudo rm file.txt", False),
        ("shutdown now", False),
        ("mkfs.ext4 /dev/sda", False),
        ("dd if=/dev/zero of=/dev/sda", False),
        ("chmod -R 777 /", False),
        ("reboot", False),
        (":(){:|:&};:", False),  # Fork bomb
    ]
    
    all_tests = safe_tests + dangerous_tests
    passed = 0
    failed = 0
    
    for cmd, should_be_safe in all_tests:
        is_safe, msg = sf.is_safe(cmd)
        if is_safe == should_be_safe:
            passed += 1
            status = "✓"
        else:
            failed += 1
            status = "✗"
        expected = "SAFE" if should_be_safe else "BLOCKED"
        actual = "SAFE" if is_safe else "BLOCKED"
        print(f"{status} {cmd:35s} Expected: {expected:10s} Got: {actual:10s}")
    
    print(f"\nResults: {passed} passed, {failed} failed")
    return failed == 0


def test_command_executor():
    """Test the command executor module"""
    print("\n" + "="*70)
    print("TESTING: Command Executor Module")
    print("="*70)
    
    from command_executor import CommandExecutor
    executor = CommandExecutor()
    
    tests = [
        ("echo 'Hello World'", True, "Hello World"),
        ("pwd", True, None),
        ("ls /tmp", True, None),
        ("date", True, None),
    ]
    
    passed = 0
    failed = 0
    
    for cmd, should_succeed, expected_output in tests:
        success, stdout, stderr = executor.execute(cmd, timeout=5)
        
        if success == should_succeed:
            if expected_output is None or expected_output in stdout:
                passed += 1
                print(f"✓ {cmd:30s} Success: {success}")
            else:
                failed += 1
                print(f"✗ {cmd:30s} Output mismatch")
        else:
            failed += 1
            print(f"✗ {cmd:30s} Expected success={should_succeed}, got {success}")
    
    print(f"\nResults: {passed} passed, {failed} failed")
    return failed == 0


def test_session_logger():
    """Test the session logger module"""
    print("\n" + "="*70)
    print("TESTING: Session Logger Module")
    print("="*70)
    
    from session_logger import SessionLogger
    logger = SessionLogger(log_to_file=False)
    
    # Log some interactions
    logger.log_interaction("list files", "ls -la", "output1", "")
    logger.log_interaction("show space", "df -h", "output2", "")
    logger.log_interaction("bad cmd", "rm -rf /", "", "blocked", blocked=True)
    
    # Test history
    history = logger.get_history()
    
    passed = 0
    failed = 0
    
    # Check history count
    if len(history) == 3:
        passed += 1
        print("✓ History count correct (3)")
    else:
        failed += 1
        print(f"✗ History count wrong: expected 3, got {len(history)}")
    
    # Check recent history
    recent = logger.get_recent_history(count=2)
    if len(recent) == 2:
        passed += 1
        print("✓ Recent history count correct (2)")
    else:
        failed += 1
        print(f"✗ Recent history count wrong: expected 2, got {len(recent)}")
    
    # Check summary
    summary = logger.get_session_summary()
    if "Total Interactions: 3" in summary:
        passed += 1
        print("✓ Session summary correct")
    else:
        failed += 1
        print("✗ Session summary incorrect")
    
    logger.close_session()
    
    print(f"\nResults: {passed} passed, {failed} failed")
    return failed == 0


def test_llm_handler():
    """Test the LLM handler initialization"""
    print("\n" + "="*70)
    print("TESTING: LLM Handler Module (initialization only)")
    print("="*70)
    
    # Create temporary .env for testing
    with open('.env', 'w') as f:
        f.write('OPENAI_API_KEY=sk-test-key-for-testing-only\n')
    
    try:
        from llm_handler import LLMHandler
        
        # Test initialization
        llm = LLMHandler()
        
        # Check if initialized
        if llm.client is not None:
            print("✓ LLMHandler initialized successfully")
            print("✓ API key loaded from environment")
            print(f"✓ Model configured: {llm.model}")
            return True
        else:
            print("✗ LLMHandler initialization failed")
            return False
            
    except Exception as e:
        print(f"✗ Error: {e}")
        return False
    finally:
        # Clean up
        if os.path.exists('.env'):
            os.remove('.env')


def main():
    """Run all tests"""
    print("\n" + "="*70)
    print(" AI-POWERED TERMINAL - COMPONENT TEST SUITE")
    print("="*70)
    print("\nThis script tests all core components without requiring:")
    print("  • Actual OpenAI API key (uses dummy key for init test)")
    print("  • GUI/tkinter (tests backend only)")
    print("  • Network connection")
    
    results = []
    
    # Run all tests
    results.append(("Safety Filter", test_safety_filter()))
    results.append(("Command Executor", test_command_executor()))
    results.append(("Session Logger", test_session_logger()))
    results.append(("LLM Handler", test_llm_handler()))
    
    # Print summary
    print("\n" + "="*70)
    print(" TEST SUMMARY")
    print("="*70)
    
    all_passed = True
    for test_name, passed in results:
        status = "✓ PASSED" if passed else "✗ FAILED"
        print(f"{status:10s} - {test_name}")
        if not passed:
            all_passed = False
    
    print("="*70)
    
    if all_passed:
        print("\n✓ ALL TESTS PASSED\n")
        return 0
    else:
        print("\n✗ SOME TESTS FAILED\n")
        return 1


if __name__ == "__main__":
    sys.exit(main())
