import {IWasmModule, IWasmModuleAsync, twrLibrary, keyEventToCodePoint, TLibImports, twrLibraryInstanceRegistry} from "twr-wasm"


enum EventType {
   Local,
   Global,
   AnimationLoop
};

const KEY_EVENT_SET = new Set(["keydown", "keyup"]);
type KeyEventTypes = "keydown" | "keyup";

const MOUSE_EVENT_SET = new Set(["mousemove", "mousedown", "mouseup", "click", "dblclick"]);
type MouseEventTypes = "mousemove" | "mousedown" | "mouseup" | "click" | "dblclick";
// Libraries use default export
export default class jsEventsLib extends twrLibrary {
   id: number;

   imports:TLibImports = {
      twrRegisterGlobalKeyEvent: {},
      twrRegisterLocalKeyEvent: {},
      
      registerAnimationLoop: {},

      twrRegisterGlobalMouseEvent: {},
      twrRegisterLocalMouseEvent: {},

      registerGlobalWheelEvent: {},
      registerLocalWheelEvent: {},

      stopUIEvent: {},
      stopAllUIEvents: {}
   };

   // every library should have this line
   libSourcePath = new URL(import.meta.url).pathname;

   constructor() {
      super();
      this.id=twrLibraryInstanceRegistry.register(this);
   }

   nextEventHandlerID: number = 0;

   events: (
      [EventType.Global, string, (e: any) => void]
      | [EventType.Local, HTMLElement, string, (e: any) => void]
      //deleting this elements tell the animation loop to stop
      | [EventType.AnimationLoop] 
   )[] = [];

   //Generic setup comes from the typescript definition for addEventListener
   //allows automatic mapping and type checking for the event name vs. the event type in the handler function
   internalRegisterGlobalEvent<K extends keyof HTMLElementEventMap>(eventName: K, handler: (ev: HTMLElementEventMap[K]) => void) {
      const eventHandlerID = this.nextEventHandlerID++;
      
      this.events[eventHandlerID] = [EventType.Global, eventName, handler];

      document.addEventListener(eventName, handler);

      return eventHandlerID;
   }

   internalRegisterLocalEvent<K extends keyof HTMLElementEventMap>(element: HTMLElement, eventName: K, handler: (this: HTMLElement, ev: HTMLElementEventMap[K]) => any) {
      const eventHandlerID = this.nextEventHandlerID++;

      this.events[eventHandlerID] = [EventType.Local, element, eventName, handler];

      element.addEventListener(eventName, handler);

      return eventHandlerID;
   }

   internalGetElementByID(callingMod:IWasmModule|IWasmModuleAsync, elementIDPtr: number, calling_name: string) {
      const elementID = callingMod.wasmMem.getString(elementIDPtr);
      const element = document.getElementById(elementID);
      if (element == null) throw new Error(`Error! ${calling_name} was given invalid element ID (${elementID})!`);

      return element;
   }

   internalCreateKeyHandler(callingMod: IWasmModule|IWasmModuleAsync, eventID: number) {
      return (event: KeyboardEvent) => {
         const r=keyEventToCodePoint(event);  // twr-wasm utility function
         if (r) {
            callingMod.postEvent(eventID, r);
         }
      }
   }

   twrRegisterGlobalKeyEvent(callingMod:IWasmModule|IWasmModuleAsync, eventNamePtr: number, eventID: number) {
      const keyEvent = callingMod.wasmMem.getString(eventNamePtr) as KeyEventTypes;
      if (!KEY_EVENT_SET.has(keyEvent)) throw new Error(`twrRegisterGlobalKeyEvent was given an unrecognized keyboard event (${keyEvent})!`);
      return this.internalRegisterGlobalEvent(
         keyEvent,
         this.internalCreateKeyHandler(callingMod, eventID)
      )
   }
   twrRegisterLocalKeyEvent(callingMod:IWasmModule|IWasmModuleAsync, eventNamePtr: number, eventID: number, elementIDPtr: number) {
      const keyEvent = callingMod.wasmMem.getString(eventNamePtr) as KeyEventTypes;
      if (!KEY_EVENT_SET.has(keyEvent)) throw new Error(`twrRegisterLocalKeyEvent was given an unrecognized keyboard event (${keyEvent})!`);
      return this.internalRegisterLocalEvent(
         this.internalGetElementByID(callingMod, elementIDPtr, "twrRegisterLocalKeyEvent"),
         keyEvent,
         this.internalCreateKeyHandler(callingMod, eventID)
      )
   }

   registerAnimationLoop(callingMod:IWasmModule|IWasmModuleAsync, eventID: number) {
      const intEventID = this.nextEventHandlerID++;
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

   internalGetMouseOffset(element: HTMLElement|undefined): [number, number] {
      //if it's global (no element), offset by 0
      if (element == undefined) {
         return [0, 0];
      }

      const rect = element.getBoundingClientRect();

      //get absolute offsets in reference to the entire page
      const x_off = rect.left + window.scrollX;
      const y_off = rect.top + window.scrollY;

      return [x_off, y_off];
   }
   internalCreateMouseHandler(callingMod: IWasmModule|IWasmModuleAsync, eventName: MouseEventTypes, eventID: number, element: HTMLElement|undefined = undefined) {
      const [x_off, y_off] = this.internalGetMouseOffset(element);
      return (e: MouseEvent) => {
         callingMod.postEvent(eventID, e.pageX, e.pageY, e.pageX - x_off, e.pageY - y_off);
      };
   }
   twrRegisterGlobalMouseEvent(callingMod: IWasmModule|IWasmModuleAsync, eventNamePtr: number, eventID: number) {
      const eventName = callingMod.wasmMem.getString(eventNamePtr) as MouseEventTypes;
      if (!MOUSE_EVENT_SET.has(eventName)) throw new Error(`twrRegisterGlobalMouseEvent was given an unrecognized event name (${eventName})!`);
      return this.internalRegisterGlobalEvent(
         eventName,
         this.internalCreateMouseHandler(callingMod, eventName, eventID)
      );
   }
   twrRegisterLocalMouseEvent(callingMod: IWasmModule|IWasmModuleAsync, eventNamePtr: number, eventID: number, elementIDPtr: number) {
      const eventName = callingMod.wasmMem.getString(eventNamePtr) as MouseEventTypes;
      if (!MOUSE_EVENT_SET.has(eventName)) throw new Error(`twrRegisterLocalMouseEvent was given an unrecognized event name (${eventName})!`);

      const element = this.internalGetElementByID(callingMod, elementIDPtr, "twrRegisterInternalMouseEvent");

      return this.internalRegisterLocalEvent(
         element,
         eventName,
         this.internalCreateMouseHandler(callingMod, eventName, eventID, element)
      )
   }

   registerGlobalWheelEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number) {
      return this.internalRegisterGlobalEvent(
         'wheel',
         (e) => {
            callingMod.postEvent(eventID, e.deltaX, e.deltaY, e.deltaZ, e.deltaMode)
         }
      )
   }

   registerLocalWheelEvent(callingMod: IWasmModule|IWasmModuleAsync, eventID: number, elementIDPtr: number) {
      return this.internalRegisterLocalEvent(
         this.internalGetElementByID(callingMod, elementIDPtr, "registerLocalWheelEvent"),
         'wheel',
         (e) => {
            callingMod.postEvent(eventID, e.deltaX, e.deltaY, e.deltaZ, e.deltaMode);
         }
      )
   }

   stopUIEvent(callingMod:IWasmModule|IWasmModuleAsync, eventHandlerID: number) {
      console.log(this.events);
      if (!(eventHandlerID in this.events)) throw new Error(`stop event was given an invalid eventHandlerID (${eventHandlerID})!`);
      const eventHandler = this.events[eventHandlerID];

      switch (eventHandler[0]) {
         case EventType.Local:
         {
            eventHandler[1].removeEventListener(eventHandler[2], eventHandler[3]);
         }
         break;

         case EventType.Global:
         {
            document.removeEventListener(eventHandler[1], eventHandler[2]);
         }
         break;

         case EventType.AnimationLoop:
         {
            //do nothing, the deletion of the event is enough to stop it
         }
         break;

         default:
            throw new Error(`stopEvent: Unknown event type (${eventHandler[0]})!`);
      }

      //delete event from list of event handlers
      delete this.events[eventHandlerID];
   }

   stopAllUIEvents(callingMod:IWasmModule|IWasmModuleAsync) {
      for (const eventHandlerID of this.events.keys()) {
         if (this.events[eventHandlerID] == undefined) continue;
         this.stopUIEvent(callingMod, eventHandlerID);
      }
   }
}


