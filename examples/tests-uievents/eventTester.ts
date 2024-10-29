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
      //add timeout, otherwise event callbacks are *immediately* called
      setTimeout(() => {
         document.dispatchEvent(event);
      }, 10);
   }
   internalSendLocalEvent(mod:IWasmModule|IWasmModuleAsync, event: Event, elementIDPtr: number, rootFuncName: string) {
      const elementID = mod.wasmMem.getString(elementIDPtr);
      const elem = document.getElementById(elementID);
      if (!elem) throw new Error(`${rootFuncName} was given an invalid element ID (${elementID})!`);

      setTimeout(() => {
         elem.dispatchEvent(event);
      }, 10);
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
               clientX: pageX - window.scrollX,
               clientY: pageY - window.scrollY
            }
         )
      )
   }
   sendLocalMouseEvent(mod:IWasmModule|IWasmModuleAsync, eventNamePtr: number, pageX: number, pageY: number, elementIDPtr: number, relative: boolean) {
      let [x_off, y_off] = [0, 0];
      if (relative) {
         const elementID = mod.wasmMem.getString(elementIDPtr);
         const elem = document.getElementById(elementID);
         if (!elem) throw new Error(`sendLocalMouseEvent was given an invalid element ID (${elementID})!`);

         const rect = elem.getBoundingClientRect();
         x_off = rect.left + window.scrollX;
         y_off = rect.top + window.scrollY;
      }
      this.internalSendLocalEvent(
         mod,
         new MouseEvent(
            mod.wasmMem.getString(eventNamePtr),
            {
               clientX: pageX - window.scrollX + x_off,
               clientY: pageY - window.scrollY + y_off
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


