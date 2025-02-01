/****************************************************************

 ASShared.swift

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

@objc class ASShared: NSObject {
    
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

    @objc func cycle() {
        let time_info : UInt = UInt(CFAbsoluteTimeGetCurrent())
        returned_value = shared_cycle(time_info, identification)
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

    @objc func menuPause() -> Int {
        return shared_menu(NA_MENU_PAUSE)
    }
    
    @objc func menuFollow() -> Int {
        return shared_menu(NA_MENU_FOLLOW)
    }
    
    @objc func menuSocialWeb() -> Int {
        return shared_menu(NA_MENU_SOCIAL_WEB)
    }

    @objc func menuPreviousApe() {
        shared_menu(NA_MENU_PREVIOUS_APE)
    }

    @objc func menuNextApe() {
        shared_menu(NA_MENU_NEXT_APE)
    }

    @objc func menuClearErrors() {
        shared_menu(NA_MENU_CLEAR_ERRORS)
    }

    @objc func menuNoTerritory() -> Int {
        return shared_menu(NA_MENU_TERRITORY)
    }

    @objc func menuNoWeather() -> Int {
        return shared_menu(NA_MENU_WEATHER)
    }

    @objc func menuNoBrain() -> Int {
        return shared_menu(NA_MENU_BRAIN)
    }

    @objc func menuNoBrainCode() -> Int {
        return shared_menu(NA_MENU_BRAINCODE)
    }

    @objc func menuDaylightTide() -> Int {
        return shared_menu(NA_MENU_TIDEDAYLIGHT)
    }

    @objc func menuFlood() {
        shared_menu(NA_MENU_FLOOD)
    }

    @objc func menuHealthyCarrier() {
        shared_menu(NA_MENU_HEALTHY_CARRIER)
    }

    @objc func menuCommandLineExecute() {
//        io_command_line_execution_set()
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
    
    @objc func blitCode(dim_x: size_t, dim_y: size_t) {
        let optionalContext = NSGraphicsContext.current?.cgContext
        if let context = optionalContext {
            context.saveGState()
                        
            let  colorSpace: CGColorSpace = CGColorSpaceCreateDeviceRGB();
            let  sharedId: Int = sharedId()
            
            let optionalDrawRef: CGContext? = CGContext.init(data: shared_draw(sharedId, dim_x, dim_y, 0), width: dim_x, height: dim_y, bitsPerComponent: 8, bytesPerRow: dim_x * 4, space: colorSpace, bitmapInfo: UInt32(CGBitmapInfo.byteOrder32Big.rawValue | CGImageAlphaInfo.noneSkipFirst.rawValue))
            
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
    
    @objc func processFile(_ urlPath: String) {
    }
    
    @objc func quitProcedure(){
        close()
        exit(0);
    }
    
    @objc func startEveything(headyLifting: Bool, window: NSWindow?) async {

        
        if headyLifting {
            if start() == false {
                quitProcedure()
                return
            }
        }
        //window?.makeKeyAndOrderFront(nil)
        //NSApp.activate(ignoringOtherApps: true)
    }
    
    @objc func cycledo() {
        
        cycle()
        
        if cycleQuit() {
            quitProcedure()
        }
        
        if cycleNewApes() {
            newAgents()
        }
    }
    
    func startEverything(_ headyLifting: Bool) {
        if headyLifting {
            if !start() {
                self.quitProcedure()
                return
            }
        }
        //NSApp.activate(ignoringOtherApps: true)
    }

    func identificationBasedOnName(windowName: String) {
        identification = Int(NUM_VIEW)
        
        if windowName == "Terrain" {
            identification = Int(NUM_TERRAIN)
        }
        if windowName == "Control" {
            identification = Int(NUM_CONTROL)
        }
    }
}
