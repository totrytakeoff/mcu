/**
 * @file    test_drive_train.cpp
 * @brief   Unit tests for DriveTrain differential steering system
 * @author  AI Assistant
 * @date    2024
 * 
 * Compile and run on PC for algorithm verification before deploying to STM32
 */

#include <iostream>
#include <cmath>
#include <cassert>

// Mock Motor class for testing
class Motor {
public:
    Motor() = default;
    void setSpeed(int speed) { currentSpeed_ = speed; }
    void stop() { currentSpeed_ = 0; }
    int getSpeed() const { return currentSpeed_; }
private:
    int currentSpeed_ = 0;
};

// Include DriveTrain implementation (for PC testing, create simplified version)
#include "../include/drive_train.hpp"

// Test framework
class TestSuite {
public:
    void runAllTests() {
        std::cout << "========================================\n";
        std::cout << "DriveTrain Unit Tests\n";
        std::cout << "========================================\n\n";
        
        testBasicMovement();
        testTurning();
        testNormalization();
        testDeadband();
        testStop();
        testBoundaryConditions();
        testEdgeCases();
        
        std::cout << "\n========================================\n";
        std::cout << "All tests passed! ✓\n";
        std::cout << "========================================\n";
    }
    
private:
    void assertEqual(int actual, int expected, const char* testName) {
        if (actual != expected) {
            std::cout << "❌ FAIL: " << testName << "\n";
            std::cout << "   Expected: " << expected << ", Got: " << actual << "\n";
            assert(false);
        } else {
            std::cout << "✓ " << testName << "\n";
        }
    }
    
    void assertRange(int value, int min, int max, const char* testName) {
        if (value < min || value > max) {
            std::cout << "❌ FAIL: " << testName << "\n";
            std::cout << "   Value " << value << " out of range [" << min << ", " << max << "]\n";
            assert(false);
        } else {
            std::cout << "✓ " << testName << "\n";
        }
    }
    
    // Test 1: Basic movement
    void testBasicMovement() {
        std::cout << "\nTest 1: Basic Movement\n";
        std::cout << "----------------------\n";
        
        Motor lf, lb, rf, rb;
        DriveTrain robot(lf, lb, rf, rb);
        
        // Forward
        robot.drive(50, 0);
        assertEqual(lf.getSpeed(), 50, "Forward - Left motor");
        assertEqual(rf.getSpeed(), -50, "Forward - Right motor (inverted)");
        
        // Backward
        robot.drive(-50, 0);
        assertEqual(lf.getSpeed(), -50, "Backward - Left motor");
        assertEqual(rf.getSpeed(), 50, "Backward - Right motor (inverted)");
    }
    
    // Test 2: Turning
    void testTurning() {
        std::cout << "\nTest 2: Turning\n";
        std::cout << "---------------\n";
        
        Motor lf, lb, rf, rb;
        DriveTrain robot(lf, lb, rf, rb);
        
        // Left turn (turnSpeed > 0 means left)
        robot.drive(50, 40);  // Turn sensitivity = 0.8, so effective turn = 32
        // Left: 50 + 32 = 82
        // Right: 50 - 32 = 18 (inverted to -18)
        assertEqual(lf.getSpeed(), 82, "Left turn - Left motor faster");
        assertEqual(rf.getSpeed(), -18, "Left turn - Right motor slower");
        
        // Right turn (turnSpeed < 0 means right)
        robot.drive(50, -40);
        // Left: 50 - 32 = 18
        // Right: 50 + 32 = 82 (inverted to -82)
        assertEqual(lf.getSpeed(), 18, "Right turn - Left motor slower");
        assertEqual(rf.getSpeed(), -82, "Right turn - Right motor faster");
    }
    
    // Test 3: Speed normalization
    void testNormalization() {
        std::cout << "\nTest 3: Speed Normalization\n";
        std::cout << "----------------------------\n";
        
        Motor lf, lb, rf, rb;
        DriveTrain robot(lf, lb, rf, rb);
        
        // Case that would overflow without normalization
        // straight=80, turn=60 -> effective turn = 48
        // Left: 80 + 48 = 128 (overflow!)
        // Right: 80 - 48 = 32
        // After normalization: 128:32 = 4:1 ratio
        // Scaled: 100, 25
        robot.drive(80, 60);
        
        // Check speeds are within bounds
        assertRange(lf.getSpeed(), -100, 100, "Normalized left speed in range");
        assertRange(rf.getSpeed(), -100, 100, "Normalized right speed in range");
        
        // Check ratio is maintained (approximately)
        int leftSpeed = lf.getSpeed();
        int rightSpeed = std::abs(rf.getSpeed());
        float ratio = (float)leftSpeed / (float)rightSpeed;
        
        if (ratio > 3.5 && ratio < 4.5) {
            std::cout << "✓ Speed ratio maintained after normalization\n";
        } else {
            std::cout << "❌ Speed ratio not maintained: " << ratio << "\n";
            assert(false);
        }
    }
    
    // Test 4: Deadband filtering
    void testDeadband() {
        std::cout << "\nTest 4: Deadband Filtering\n";
        std::cout << "---------------------------\n";
        
        Motor lf, lb, rf, rb;
        DriveTrain robot(lf, lb, rf, rb);
        
        // Small inputs should be filtered out (deadband = 5)
        robot.drive(3, 0);
        assertEqual(lf.getSpeed(), 0, "Small straight input filtered");
        
        robot.drive(0, 4);
        assertEqual(lf.getSpeed(), 0, "Small turn input filtered");
        
        // Just above threshold should work
        robot.drive(10, 0);
        assertEqual(lf.getSpeed(), 10, "Input above deadband works");
    }
    
    // Test 5: Stop function
    void testStop() {
        std::cout << "\nTest 5: Stop Function\n";
        std::cout << "---------------------\n";
        
        Motor lf, lb, rf, rb;
        DriveTrain robot(lf, lb, rf, rb);
        
        robot.drive(50, 30);  // Move
        robot.stop();         // Stop
        
        assertEqual(lf.getSpeed(), 0, "Left motor stopped");
        assertEqual(lb.getSpeed(), 0, "Left back motor stopped");
        assertEqual(rf.getSpeed(), 0, "Right motor stopped");
        assertEqual(rb.getSpeed(), 0, "Right back motor stopped");
        assertEqual(robot.getStraightSpeed(), 0, "Straight speed reset");
        assertEqual(robot.getTurnSpeed(), 0, "Turn speed reset");
    }
    
    // Test 6: Boundary conditions
    void testBoundaryConditions() {
        std::cout << "\nTest 6: Boundary Conditions\n";
        std::cout << "----------------------------\n";
        
        Motor lf, lb, rf, rb;
        DriveTrain robot(lf, lb, rf, rb);
        
        // Maximum speed
        robot.drive(100, 0);
        assertEqual(lf.getSpeed(), 100, "Max forward speed");
        
        // Minimum speed
        robot.drive(-100, 0);
        assertEqual(lf.getSpeed(), -100, "Max backward speed");
        
        // Maximum turn
        robot.drive(0, 100);  // Effective turn = 80
        assertRange(lf.getSpeed(), -100, 100, "Max left turn in range");
        assertRange(rf.getSpeed(), -100, 100, "Max left turn right wheel in range");
        
        // Combined maximum
        robot.drive(100, 100);
        assertRange(lf.getSpeed(), -100, 100, "Combined max left in range");
        assertRange(rf.getSpeed(), -100, 100, "Combined max right in range");
    }
    
    // Test 7: Edge cases
    void testEdgeCases() {
        std::cout << "\nTest 7: Edge Cases\n";
        std::cout << "-------------------\n";
        
        Motor lf, lb, rf, rb;
        DriveTrain robot(lf, lb, rf, rb);
        
        // Zero input
        robot.drive(0, 0);
        assertEqual(lf.getSpeed(), 0, "Zero input - left");
        assertEqual(rf.getSpeed(), 0, "Zero input - right");
        
        // Negative both
        robot.drive(-50, -40);  // Backward right turn
        // Left: -50 - 32 = -82
        // Right: -50 + 32 = -18 (inverted to 18)
        assertEqual(lf.getSpeed(), -82, "Backward right turn - left");
        assertEqual(rf.getSpeed(), 18, "Backward right turn - right");
        
        // Spin in place left
        robot.drive(0, 60);  // Effective turn = 48
        assertEqual(lf.getSpeed(), 48, "Spin left - left wheel");
        assertEqual(rf.getSpeed(), -48, "Spin left - right wheel");
        
        // Spin in place right
        robot.drive(0, -60);
        assertEqual(lf.getSpeed(), -48, "Spin right - left wheel");
        assertEqual(rf.getSpeed(), 48, "Spin right - right wheel");
    }
};

// Main test runner
int main() {
    try {
        TestSuite tests;
        tests.runAllTests();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << "\n";
        return 1;
    }
}

/**
 * Expected Output:
 * ========================================
 * DriveTrain Unit Tests
 * ========================================
 * 
 * Test 1: Basic Movement
 * ----------------------
 * ✓ Forward - Left motor
 * ✓ Forward - Right motor (inverted)
 * ✓ Backward - Left motor
 * ✓ Backward - Right motor (inverted)
 * 
 * Test 2: Turning
 * ---------------
 * ✓ Left turn - Left motor faster
 * ✓ Left turn - Right motor slower
 * ✓ Right turn - Left motor slower
 * ✓ Right turn - Right motor faster
 * 
 * [... more tests ...]
 * 
 * ========================================
 * All tests passed! ✓
 * ========================================
 */
