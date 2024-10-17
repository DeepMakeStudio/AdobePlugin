import Cocoa
import Foundation

class DialogController: NSViewController {
    
    @IBOutlet weak var promptTextField: NSTextField!
    var defaultPrompt: String = ""
    
    override func viewDidAppear() {
        if let closeButton = view.window?.standardWindowButton(.closeButton) {
            closeButton.isEnabled = false
        }
    }
    @IBAction func okButtonClicked(_ sender: NSButton) {
        writeTextToStdOut(promptText: promptTextField.stringValue)
        view.window?.close()
        NSApp.stopModal()
        NSApplication.shared.terminate(nil)
    }

    @IBAction func cancelButtonClicked(_ sender: NSButton) {
        writeTextToStdOut(promptText: defaultPrompt)
        view.window?.close()
        NSApp.stopModal()
        NSApplication.shared.terminate(nil)
    }
    
    func writeTextToStdOut(promptText: String){
        let cleanedText = promptText.replacingOccurrences(of: "\n", with: "")
        print(cleanedText, separator:"")
    }
}

func showDialog(withPrompt prompt: String) -> String {
    let storyboard = NSStoryboard(name: NSStoryboard.Name("Main"), bundle: nil)
    let viewController = storyboard.instantiateController(withIdentifier: NSStoryboard.SceneIdentifier("DialogController")) as! DialogController
    viewController.defaultPrompt = prompt
    
    let window = NSWindow(contentViewController: viewController)
    window.level = NSWindow.Level.modalPanel
    window.makeKeyAndOrderFront(nil)
    window.orderFrontRegardless()
    window.makeMain()

    DispatchQueue.main.async {
        viewController.promptTextField.stringValue = viewController.defaultPrompt
    }
    
    NSApplication.shared.activate(ignoringOtherApps: true)
    NSApplication.shared.runModal(for: window)


    return viewController.defaultPrompt
}
