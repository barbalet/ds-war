//
//  Simulated_WarUITests.swift
//  Simulated WarUITests
//
//  Created by Thomas Barbalet on 2/1/25.
//  Copyright © 2025 Thomas Barbalet. All rights reserved.
//

import XCTest

final class Simulated_WarUITests: XCTestCase {

    override func setUpWithError() throws {
        // Put setup code here. This method is called before the invocation of each test method in the class.

        // In UI tests it is usually best to stop immediately when a failure occurs.
        continueAfterFailure = false

        // In UI tests it’s important to set the initial state - such as interface orientation - required for your tests before they run. The setUp method is a good place to do this.
    }

    override func tearDownWithError() throws {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }

    @MainActor
    func testExample() throws {
        // UI tests must launch the application that they test.
        let app = XCUIApplication()
        app.launch()

        // Use XCTAssert and related functions to verify your tests produce the correct results.
    }

    @MainActor
    func testLaunchPerformance() throws {
        if #available(macOS 10.15, iOS 13.0, tvOS 13.0, watchOS 7.0, *) {
            // This measures how long it takes to launch your application.
            measure(metrics: [XCTApplicationLaunchMetric()]) {
                XCUIApplication().launch()
            }
        }
    }
    
    func testAppRunDuration() {
        // Record the start time
        let startTime = Date()

        // Launch the app
        let app = XCUIApplication()
        app.launch()

        //app.launchArguments = ""
        
        // Wait for the app to terminate
        // This assumes the app will quit after completing its task
        let appQuitExpectation = expectation(
            for: NSPredicate(format: "exists == false"),
            evaluatedWith: app,
            handler: nil
        )

        // Wait for the app to quit
        wait(for: [appQuitExpectation], timeout: 90)
        // Record the end time
        let endTime = Date()

        // Calculate the duration
        let duration = endTime.timeIntervalSince(startTime)

        // Print the duration to the console
        print("App ran and quit in \(duration) seconds")

        // Optionally, assert that the duration is within an expected range
        let expectedMaxDuration: TimeInterval = 120.0 // Set your expected maximum duration
        XCTAssertLessThanOrEqual(duration, expectedMaxDuration, "App took longer than expected to run and quit")
    }
}
