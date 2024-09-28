import {IWasmModule, IWasmModuleAsync, twrLibrary, keyEventToCodePoint, TLibImports, twrLibraryInstanceRegistry} from "twr-wasm"


enum EventType {
   Local,
   Global,
   AnimationLoop
};

// Libraries use default export
export default class jsEventsLib extends twrLibrary {
   id: number;

   imports:TLibImports = {
      registerKeyUpEvent: {},
      registerKeyDownEvent: {},
      registerAnimationLoop: {},
      registerMousePressEvent: {},
      registerMouseMoveEvent: {},

      setElementText: {},
   };

   // every library should have this line
   libSourcePath = new URL(import.meta.url).pathname;

   constructor() {
      super();
      this.id=twrLibraryInstanceRegistry.register(this);
   }

   nextEventID: number = 0;

   events: (
      [EventType.Global, string, (e: any) => void]
      | [EventType.Local, HTMLElement, string, (e: any) => void]
      //deleting this elements tell the animation loop to stop
      | [EventType.AnimationLoop] 
   )[] = [];

   internalRegisterGlobalEvent(eventName: string, handler: (e: any) => void) {
      const eventID = this.nextEventID++;
      
      this.events[eventID] = [EventType.Global, eventName, handler];

      document.addEventListener(eventName, handler);

      return eventID;
   }

   internalRegisterLocalEvent(element: HTMLElement, eventName: string, handler: (e: any) => void) {
      const eventID = this.nextEventID++;

      this.events[eventID] = [EventType.Local, element, eventName, handler];

      element.addEventListener(eventName, handler);

      return eventID;
   }

   internalCreateKeyHandler(callingMod: IWasmModule|IWasmModuleAsync, eventID: number) {
      return (event: KeyboardEvent) => {
         const r=keyEventToCodePoint(event);  // twr-wasm utility function
         if (r) {
            callingMod.postEvent(eventID, r);
         }
      }
   }

   registerGlobalKeyUpEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number) {
      return this.internalRegisterGlobalEvent(
         'keyup', 
         this.internalCreateKeyHandler(callingMod, eventID)
      );
   }

   registerGlobalKeyDownEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number) {
      return this.internalRegisterGlobalEvent(
         'keydown', 
         this.internalCreateKeyHandler(callingMod, eventID)
      );
   }

   internalGetElementByID(callingMod:IWasmModule|IWasmModuleAsync, elementIDPtr: number, calling_name: string) {
      const elementID = callingMod.wasmMem.getString(elementIDPtr);
      const element = document.getElementById(elementID);
      if (element == null) throw new Error(`Error! ${calling_name} was given invalid element ID (${elementID})!`);

      return element;
   }

   registerLocalKeyUpEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number, elementIDPtr: number) {
      return this.internalRegisterLocalEvent(
         this.internalGetElementByID(callingMod, elementIDPtr, "registerLocalKeyUpEvent"),
         'keyup', 
         this.internalCreateKeyHandler(callingMod, eventID)
      );
   }

   registerLocalKeyDownEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number, elementIDPtr: number) {
      return this.internalRegisterLocalEvent(
         this.internalGetElementByID(callingMod, elementIDPtr, "registerLocalKeyDownEvent"),
         'keydown', 
         this.internalCreateKeyHandler(callingMod, eventID)
      );
   }

   registerAnimationLoop(callingMod:IWasmModule|IWasmModuleAsync, eventID: number) {
      const intEventID = this.nextEventID++;
      this.events[intEventID] = [EventType.AnimationLoop];

      const loop: FrameRequestCallback = (time) => {
         //run until it's item in the event list is deleted
         if (intEventID in this.events) {
            callingMod.postEvent(eventID, time);
            requestAnimationFrame(loop);
         }
      }
      requestAnimationFrame(loop);

      return intEventID;
   }

   internalCreateMouseHandler(callingMod:IWasmModule|IWasmModuleAsync, eventID: number, relative: boolean, element: HTMLElement) {
      return (e: MouseEvent) => {
         callingMod.postEvent(eventID, e.pageX, e.pageY);
      }
   }
   registerMouseMoveEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number, elementIDPtr: number, relative: boolean) {
      const elementID = callingMod.wasmMem.getString(elementIDPtr);
      const element = document.getElementById(elementID)!;

      if (element == null) throw new Error("registerMouseEvent was given a non-existant element ID!");
      
      if (relative) {
         const x_off = element.offsetLeft;
         const y_off = element.offsetTop;
         element.addEventListener('mousemove', (e) => {
            callingMod.postEvent(eventID, e.clientX - x_off, e.clientY - y_off);
         });
      } else {
         element.addEventListener('mousemove', (e) => {
            callingMod.postEvent(eventID, e.clientX, e.clientY);
         });
      }
   }

   registerMousePressEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number, elementIDPtr: number, relative: boolean) {
      const elementID = callingMod.wasmMem.getString(elementIDPtr);
      const element = document.getElementById(elementID)!;

      if (element == null) throw new Error("registerMouseEvent was given a non-existent element ID!");

      if (relative) {
         const x_off = element.offsetLeft;
         const y_off = element.offsetTop;
         element.addEventListener('mousedown', (e) => {
            callingMod.postEvent(eventID, e.clientX - x_off, e.clientY - y_off, e.button);
         });
      } else {
         element.addEventListener('mousedown', (e) => {
            callingMod.postEvent(eventID, e.clientX, e.clientY, e.button);
         });
      }
   }

}


