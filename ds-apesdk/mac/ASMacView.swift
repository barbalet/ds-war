/****************************************************************
 
 ASMacView.swift
 
 =============================================================
 
 Copyright 1996-2021 Tom Barbalet. All rights reserved.
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 
 This software is a continuing work of Tom Barbalet, begun on
 13 June 1996. No apes or cats were harmed in the writing of
 this software.
 
 ****************************************************************/

import Foundation

import Cocoa

class ASMacView: NSView {
    
    var shared: ASShared?
    var localWindow: NSWindow?
    
    public override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)

        let localWindow:NSWindow? = self.window
        guard (self.window != nil) else {
            return
        }
        
        self.localWindow = localWindow
        shared = ASShared(frame: frameRect)
    }
    
    public required init?(coder: NSCoder) {
        super.init(coder: coder)
        
        shared = ASShared(frame: self.bounds)
        
        NSLog("Starting up")
                
        let increments: NSSize = NSSize(width: 4, height: 4)
        
        window?.resizeIncrements = increments
        
        if shared?.start() == false {
            shared?.quitProcedure()
            return
        }
        window?.makeKeyAndOrderFront(nil)
        NSApp.activate(ignoringOtherApps: true)
        
        DispatchQueue.main.async { [weak self] in
            self?.needsDisplay = true
        }
    }
    
//    func startView() {
//        shared = ASShared(frame: self.bounds)
//    }
    
    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)
        
        var dimY = Int(dirtyRect.height)
        let dimX = Int(dirtyRect.width)
        
        shared?.cycledo()
        
        if #available(macOS 14, *) {
            dimY -= 28
        }
        
        shared?.blitCode(dim_x: dimX, dim_y: dimY)
        
        DispatchQueue.main.async { [weak self] in
            self?.needsDisplay = true
        }
    }
    
    func processFile(_ urlString: String) {
        shared?.processFile(urlString)
    }
    
    func awakeFromNib() async {
        super.awakeFromNib()

        
        
//        if let appDelegate = NSApplication.shared.delegate as? AppDelegate {
//            
//            var localShared: ASShared
//            
//            guard localShared == shared else {
//                return
//            }
//            appDelegate.add(shared: localShared)
//            
//        }
    }
    
//    @IBAction func openFileManually(_ sender: Any) {
//        
//        let panel = NSOpenPanel()
//        panel.canChooseFiles = true
//        panel.allowedContentTypes = [.png] // Customize file types
//        panel.canChooseDirectories = false
//        panel.allowsMultipleSelection = false
//
//        panel.begin { result in
//            if result == .OK, let url = panel.url {
//                self.processFile(url.path)
//            }
//        }
//    }
    
    @IBAction func aboutMenu(_ sender: Any) {
        shared?.about()
    }
    
    
    @IBAction func quitMenu(_ sender: Any) {
        NSLog("Quit from menu")
        shared?.quitProcedure()
    }

    override var acceptsFirstResponder: Bool {
        return true
    }
    
    override func becomeFirstResponder() -> Bool {
        return true
    }
    
    override func resignFirstResponder() -> Bool {
        return true
    }
    
    // MARK: - Method Overrides
    
    override func keyUp(with event: NSEvent) {
        shared?.keyUp()
    }
    
    override func keyDown(with event: NSEvent) {
        var localKey: UInt = 0
        
        // Check for Control or Option modifier flags
        if event.modifierFlags.contains(.control) || event.modifierFlags.contains(.option) {
            localKey = 2048
        }
        
        // Check for NumericPad modifier flag (arrow keys)
        if event.modifierFlags.contains(.numericPad) {
            if let theArrow = event.charactersIgnoringModifiers, !theArrow.isEmpty {
                let keyChar = theArrow.unicodeScalars.first!
                
                switch keyChar {
                case UnicodeScalar(NSLeftArrowFunctionKey):
                    localKey += 28
                case UnicodeScalar(NSRightArrowFunctionKey):
                    localKey += 29
                case UnicodeScalar(NSUpArrowFunctionKey):
                    localKey += 30
                case UnicodeScalar(NSDownArrowFunctionKey):
                    localKey += 31
                default:
                    break
                }
                
                // Call your shared keyReceived function
                shared?.keyReceived(Int(localKey))
            }
        }
        
        // Handle general characters
        if let characters = event.characters {
            let firstIndex = characters.startIndex
            let firstCharacter = characters[firstIndex]
            
            // Check if the first character is a letter
            if firstCharacter.isLetter {
                if let scalarValue = firstCharacter.unicodeScalars.first?.value {
                    shared?.keyReceived(Int(UInt(scalarValue)))
                }
            }
        }
    }

    
    override func acceptsFirstMouse(for event: NSEvent?) -> Bool {
        return true
    }
    
    override func mouseDown(with event: NSEvent) {
        let location = convert(event.locationInWindow, from: nil)
        shared?.mouseOption(event.modifierFlags.contains(.control) || event.modifierFlags.contains(.option))
        shared?.mouseReceived(withXLocation: Double(Float(location.x)), yLocation: Double(Float(bounds.size.height - location.y)))
    }
    
    override func rightMouseDown(with event: NSEvent) {
        mouseDown(with: event)
        shared?.mouseOption(true)
    }
    
    override func otherMouseDown(with event: NSEvent) {
        rightMouseDown(with: event)
    }
    
    override func mouseUp(with event: NSEvent) {
        shared?.mouseUp()
    }
    
    override func rightMouseUp(with event: NSEvent) {
        mouseUp(with: event)
    }
    
    override func otherMouseUp(with event: NSEvent) {
        mouseUp(with: event)
    }
    
    override func mouseDragged(with event: NSEvent) {
        mouseDown(with: event)
    }
    
    override func rightMouseDragged(with event: NSEvent) {
        rightMouseDown(with: event)
    }
    
    override func otherMouseDragged(with event: NSEvent) {
        rightMouseDown(with: event)
    }
    
    override func scrollWheel(with event: NSEvent) {
        shared?.delta_x(event.deltaX, delta_y: event.deltaY)
    }
    
    override func magnify(with event: NSEvent) {
        shared?.zoom(Double(Float(event.magnification)))
    }
    
    override func rotate(with event: NSEvent) {
        shared?.rotation(Double(Float(event.rotation)))
    }
}
