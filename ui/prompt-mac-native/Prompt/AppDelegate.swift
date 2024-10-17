//
//  AppDelegate.swift
//  Prompt
//
//  Created by Steven Davis on 11/24/23.
//

import Cocoa
import Foundation

@main
class AppDelegate: NSObject, NSApplicationDelegate {
    
    func applicationDidFinishLaunching(_ aNotification: Notification)
    {
        var promptText: String = ""

        if let index = CommandLine.arguments.firstIndex(of: "-p") {
            // Check if there is a value after the -p option
            if index + 1 < CommandLine.arguments.count {
                promptText = CommandLine.arguments[index + 1]
            }
        }

        let result = showDialog(withPrompt: promptText)
        if let outputData = result.data(using: .utf8) {
            let outputFileHandle = FileHandle.standardOutput
            outputFileHandle.write(outputData)
        }
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
    }

    func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool {
        return true
    }


}

