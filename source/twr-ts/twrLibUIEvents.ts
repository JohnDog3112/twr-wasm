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

   //Generic setup comes from the typescript definition for addEventListener
   //allows automatic mapping and type checking for the event name vs. the event type in the handler function
   internalRegisterGlobalEvent<K extends keyof HTMLElementEventMap>(eventName: K, handler: (ev: HTMLElementEventMap[K]) => void) {
      const eventID = this.nextEventID++;
      
      this.events[eventID] = [EventType.Global, eventName, handler];

      document.addEventListener(eventName, handler);

      return eventID;
   }

   internalRegisterLocalEvent<K extends keyof HTMLElementEventMap>(element: HTMLElement, eventName: K, handler: (this: HTMLElement, ev: HTMLElementEventMap[K]) => any) {
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

   registerGlobalMouseMoveEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number) {
      return this.internalRegisterGlobalEvent(
         'mousemove',
         (e: MouseEvent) => {
            callingMod.postEvent(eventID, e.pageX, e.pageY);
         }
      )
   }

   internalGetMouseOffset(element: HTMLElement, relative: boolean): [number, number] {
      //if it isn't relative, offset by 0
      if (!relative) {
         return [0, 0];
      }

      const rect = element.getBoundingClientRect();

      //get absolute offsets in reference to the entire page
      const x_off = rect.left + window.scrollX;
      const y_off = rect.top + window.scrollY;

      return [x_off, y_off];
   }

   registerLocalMouseMoveEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number, elementIDPtr: number, relative: boolean) {
      const element = this.internalGetElementByID(callingMod, elementIDPtr, "registerLocalMouseMoveEvent");
      
      const [x_off, y_off] = this.internalGetMouseOffset(element, relative);

      return this.internalRegisterLocalEvent(
         element,
         'mousemove',
         (e) => {
            callingMod.postEvent(eventID, e.pageX - x_off, e.pageY - y_off);
         }
      )
   }

   // internalRegisterMouseButtonEvent(callingMod:IWasmModule|IWasmModuleAsync)

   registerGlobalMousePressEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number) {
      return this.internalRegisterGlobalEvent(
         'mousedown',
         (e) => {
            
         }
      )
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


