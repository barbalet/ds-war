/****************************************************************

 DSShared.swift

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

import Cocoa

@objc class DSShared: NSObject {
    
    @objc var identification: Int = 0
    private var returned_value: shared_cycle_state?
    
    @objc convenience init(frame frameRect: NSRect) {
        self.init()
        identification = 0
    }

    @objc func start() -> Bool {
        let shared_response = shared_init(identification, UInt.random(in: 0 ..< 4294967295))
        if Int(shared_response) == -1 {
            return false
        }
        identification = shared_response
        return true
    }

    @objc func about() {
        shared_about()
    }

    @objc func keyReceived(_ key: Int) {
        shared_keyReceived(key, identification)
    }

    @objc func mouseReceived(withXLocation xLocation: Double, yLocation: Double) {
        shared_mouseReceived(xLocation, yLocation, identification)
    }

    @objc func mouseOption(_ mouseOption: Bool) {
        if mouseOption {
            shared_mouseOption(1)
        } else {
            shared_mouseOption(0)
        }
    }

    @objc func keyUp() {
        shared_keyUp()
    }

    @objc func mouseUp() {
        shared_mouseUp()
    }

    @objc func rotation(_ rotationAmount: Double) {
        shared_rotate(rotationAmount, identification)
    }

    @objc func delta_x(_ delta_x: Double, delta_y: Double) {
        shared_delta(delta_x, delta_y, identification)
    }

    @objc func zoom(_ zoomAmount: Double) {
        shared_zoom(zoomAmount, identification)
    }

    @objc func timeInterval() -> TimeInterval {
        return 1.0 / (TimeInterval(shared_max_fps()))
    }

    @objc func close() {
        shared_close()
    }

    @objc func identificationBased(onName windowName: String?) {
        identification = Int(NUM_VIEW)
        if (windowName == "Terrain") {
            identification = Int(NUM_TERRAIN)
        }
        if (windowName == "Control") {
            identification = Int(NUM_CONTROL)
        }
    }

    @objc func newSimulation() {
        shared_new(UInt.random(in: 0 ..< 4294967295))
    }

    @objc func newAgents() {
        shared_new_agents(UInt.random(in: 0 ..< 4294967295))
    }

    @objc func cycleDebugOutput() -> Bool {
        return returned_value == SHARED_CYCLE_DEBUG_OUTPUT
    }

    @objc func cycleQuit() -> Bool {
        return returned_value == SHARED_CYCLE_QUIT
    }

    @objc func cycleNewApes() -> Bool {
        return returned_value == SHARED_CYCLE_NEW_APES
    }
        
    @objc func scriptDebugHandle(_ fileName: String) {
        fileName.withCString {
            shared_script_debug_handle($0)
        }
    }
    
    @objc func savedFileName(_ name: String) {
        name.withCString {
            shared_saveFileName($0)
        }
    }

    @objc func openFileName(_ name: String, isScript scriptFile: Bool) -> Bool {
    
        let return_val = name.withCString { (cstr) -> Bool in
             return shared_openFileName(cstr, scriptFile ? 1 : 0) != 0
        }
        return return_val
    }
    
    @objc func sharedId() -> Int {
        return identification
    }
}
