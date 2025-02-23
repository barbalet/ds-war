/****************************************************************
 
 DSMacView.swift
 
 =============================================================
 
 Copyright 1996-2025 Tom Barbalet. All rights reserved.
 
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

class DSMacView: NSView {
    
    var localWindow: NSWindow?
    var returned_value: shared_cycle_state
    var indentification: Int = 0
    
    func start() -> Bool {
        let shared_response = shared_init(0, UInt.random(in: 0 ..< 4294967295))
        if Int(shared_response) == -1 {
            return false
        }
        return true
    }
    
    func startup() {
        NSLog("Starting up")
                
        let increments: NSSize = NSSize(width: 4, height: 4)
        
        window?.resizeIncrements = increments
        
        if start() == false {
            quitProcedure()
            return
        }
        window?.makeKeyAndOrderFront(nil)
        NSApp.activate(ignoringOtherApps: true)
        
        DispatchQueue.main.async { [weak self] in
            self?.needsDisplay = true
        }
    }
    
    public override init(frame frameRect: NSRect) {
        returned_value = SHARED_CYCLE_OK
        super.init(frame: frameRect)

        startup()
    }

    public required init?(coder: NSCoder) {
        returned_value = SHARED_CYCLE_OK
        super.init(coder: coder)
                
        startup()
    }
       
    func blitCode(dim_x: size_t, dim_y: size_t) {
        let optionalContext = NSGraphicsContext.current?.cgContext
        if let context = optionalContext {
            context.saveGState()
                        
            let  colorSpace: CGColorSpace = CGColorSpaceCreateDeviceRGB();
            
            let optionalDrawRef: CGContext? = CGContext.init(data: shared_draw(0, dim_x, dim_y, 0), width: dim_x, height: dim_y, bitsPerComponent: 8, bytesPerRow: dim_x * 4, space: colorSpace, bitmapInfo: UInt32(CGBitmapInfo.byteOrder32Big.rawValue | CGImageAlphaInfo.noneSkipFirst.rawValue))
            
            if let drawRef = optionalDrawRef {
                
                context.setBlendMode(.normal)
                
                context.setShouldAntialias(false)
                context.setAllowsAntialiasing(false)
                
                let optionalImage: CGImage? = drawRef.makeImage()
                
                if let image = optionalImage {
                    let newRect = NSRect(x:0, y:0, width:CGFloat(dim_x), height:CGFloat(dim_y))
                    context.draw(image, in: newRect)
                }
            }
            context.restoreGState()
        }
    }
    
    func cycle() {
        let time_info : UInt = UInt(CFAbsoluteTimeGetCurrent())
        returned_value = shared_cycle(time_info, 0)
    }
    
    func quitProcedure(){
        shared_close()
        exit(0)
    }
    
    func cycleQuit() -> Bool {
        return returned_value == SHARED_CYCLE_QUIT
    }

    func cycleNewApes() -> Bool {
        return returned_value == SHARED_CYCLE_NEW_APES
    }
    
    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)
        
        var dimY = Int(dirtyRect.height)
        let dimX = Int(dirtyRect.width)
        
        cycle()
        
        if cycleQuit() {
            quitProcedure()
        }
        
        if cycleNewApes() {
            shared_new_agents(UInt.random(in: 0 ..< 4294967295))
        }

        if #available(macOS 14, *) {
            dimY -= 28
        }
        
        blitCode(dim_x: dimX, dim_y: dimY)
        
        DispatchQueue.main.async { [weak self] in
            self?.needsDisplay = true
        }
    }

    
    func awakeFromNib() async {
        super.awakeFromNib()
    }

    
    @IBAction func aboutMenu(_ sender: Any) {
        shared_about()
    }
    
    
    @IBAction func quitMenu(_ sender: Any) {
        NSLog("Quit from menu")
        quitProcedure()
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
        shared_keyUp()
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
                shared_keyReceived(Int(localKey), 0)
            }
        }
        
        // Handle general characters
        if let characters = event.characters {
            let firstIndex = characters.startIndex
            let firstCharacter = characters[firstIndex]
            
            // Check if the first character is a letter
            if firstCharacter.isLetter {
                if let scalarValue = firstCharacter.unicodeScalars.first?.value {
                    shared_keyReceived(Int(UInt(scalarValue)), 0)
                }
            }
        }
    }

    override func acceptsFirstMouse(for event: NSEvent?) -> Bool {
        return true
    }
    
    override func mouseDown(with event: NSEvent) {
        let location = convert(event.locationInWindow, from: nil)
        let boolEvent = event.modifierFlags.contains(.control) || event.modifierFlags.contains(.option);
        
        shared_mouseOption(boolEvent ? 1 : 0)
        shared_mouseReceived(Double(Float(location.x)), Double(Float(bounds.size.height - location.y)), 0)
    }
    
    override func rightMouseDown(with event: NSEvent) {
        mouseDown(with: event)
        shared_mouseOption(1)
    }
    
    override func otherMouseDown(with event: NSEvent) {
        rightMouseDown(with: event)
    }
    
    override func mouseUp(with event: NSEvent) {
        shared_mouseUp()
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
        shared_delta(event.deltaX, event.deltaY, 0)
    }
    
    override func magnify(with event: NSEvent) {
        shared_zoom(Double(Float(event.magnification)), 0)
    }
    
    override func rotate(with event: NSEvent) {
        shared_rotate(Double(Float(event.rotation)), 0)
    }
}
