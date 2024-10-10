import {IWasmModule, IWasmModuleAsync, twrLibrary, TLibImports, twrLibraryInstanceRegistry} from "twr-wasm"

// Libraries use default export
export default class clearIODivLib extends twrLibrary {
   id: number;

   imports:TLibImports = {
      sendGlobalKeyboardEvent: {},
      sendLocalKeyboardEvent: {},
      sendGlobalMouseEvent: {},
      sendLocalMouseEvent: {},
      sendGlobalWheelEvent: {},
      sendLocalWheelEvent: {},
   };

   // every library should have this line
   libSourcePath = new URL(import.meta.url).pathname;

   constructor() {
      // all library constructors should start with these two lines
      super();
      this.id=twrLibraryInstanceRegistry.register(this);
   }

   internalSendGlobalEvent(event: Event) {
      document.dispatchEvent(event);
   }
   internalSendLocalEvent(mod:IWasmModule|IWasmModuleAsync, event: Event, elementIDPtr: number, rootFuncName: string) {
      const elementID = mod.wasmMem.getString(elementIDPtr);
      const elem = document.getElementById(elementID);
      if (!elem) throw new Error(`${rootFuncName} was given an invalid element ID (${elementID})!`);

      elem.dispatchEvent(event);
   }

   sendGlobalKeyboardEvent(mod:IWasmModule|IWasmModuleAsync, eventNamePtr: number, keyPtr: number) {
      this.internalSendGlobalEvent(
         new KeyboardEvent(
            mod.wasmMem.getString(eventNamePtr),
            {
               key: mod.wasmMem.getString(keyPtr)
            }
         )
      )
   }

   sendLocalKeyboardEvent(mod:IWasmModule|IWasmModuleAsync, eventNamePtr: number, keyPtr: number, elementIDPtr: number) {
      this.internalSendLocalEvent(
         mod,
         new KeyboardEvent(
            mod.wasmMem.getString(eventNamePtr),
            {
               key: mod.wasmMem.getString(keyPtr)
            }
         ),
         elementIDPtr,
         "sendLocalKeyboardEvent"
      )
   }

   sendGlobalMouseEvent(mod:IWasmModule|IWasmModuleAsync, eventNamePtr: number, pageX: number, pageY: number) {
      this.internalSendGlobalEvent(
         new MouseEvent(
            mod.wasmMem.getString(eventNamePtr),
            {
               screenX: pageX - window.scrollX,
               screenY: pageY - window.scrollY
            }
         )
      )
   }
   sendLocalMouseEvent(mod:IWasmModule|IWasmModuleAsync, eventNamePtr: number, pageX: number, pageY: number, elementIDPtr: number) {
      this.internalSendLocalEvent(
         mod,
         new MouseEvent(
            mod.wasmMem.getString(eventNamePtr),
            {
               screenX: pageX - window.scrollX,
               screenY: pageY - window.scrollY
            }
         ),
         elementIDPtr,
         "sendLocalMouseEvent"
      )
   }

   sendGlobalWheelEvent(mod:IWasmModule|IWasmModuleAsync, eventNamePtr: number, deltaX: number, deltaY: number, deltaZ: number, deltaMode: number) {
      this.internalSendGlobalEvent(
         new WheelEvent(
            mod.wasmMem.getString(eventNamePtr),
            {
               deltaX: deltaX,
               deltaY: deltaY,
               deltaZ: deltaZ,
               deltaMode: deltaMode
            }
         )
      )
   }

   sendLocalWheelEvent(mod:IWasmModule|IWasmModuleAsync, eventNamePtr: number, deltaX: number, deltaY: number, deltaZ: number, deltaMode: number, elementIDPtr: number) {
      this.internalSendLocalEvent(
         mod,
         new WheelEvent(
            mod.wasmMem.getString(eventNamePtr),
            {
               deltaX: deltaX,
               deltaY: deltaY,
               deltaZ: deltaZ,
               deltaMode: deltaMode
            }
         ),
         elementIDPtr,
         "sendLocalWheelEvent"
      )
   }
   

}


