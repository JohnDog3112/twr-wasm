import {IWasmModule, IWasmModuleAsync, twrLibrary, keyEventToCodePoint, TLibImports, twrLibraryInstanceRegistry} from "twr-wasm"


enum EventType {
   Local,
   Global,
   AnimationLoop
};

type MouseEventTypes = "mousemove" | "mousedown" | "mouseup" | "click" | "dblclick";
// Libraries use default export
export default class jsEventsLib extends twrLibrary {
   id: number;

   imports:TLibImports = {
      registerGlobalKeyUpEvent: {},
      registerLocalKeyUpEvent: {},
      registerGlobalKeyDownEvent: {},
      registerLocalKeyDownEvent: {},
      
      registerAnimationLoop: {},

      registerGlobalMouseMoveEvent: {},
      registerLocalMouseMoveEvent: {},
      registerGlobalMouseDownEvent: {},
      registerLocalMouseDownEvent: {},
      registerGlobalMouseUpEvent: {},
      registerLocalMouseUpEvent: {},
      registerGlobalMouseClickEvent: {},
      registerLocalMouseClickEvent: {},
      registerGlobalMouseDoubleClickEvent: {},
      registerLocalMouseDoubleClickEvent: {},
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

   
   internalGlobalMouseEvent(callingMod: IWasmModule|IWasmModuleAsync, eventName: MouseEventTypes, eventID: number) {
      return this.internalRegisterGlobalEvent(
         eventName,
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
   internalLocalMouseEvent(callingMod: IWasmModule|IWasmModuleAsync, eventName: MouseEventTypes, callingName: string, eventID: number, elementIDPtr: number, relative: boolean) {
      const element = this.internalGetElementByID(callingMod, elementIDPtr, callingName);

      const [x_off, y_off] = this.internalGetMouseOffset(element, relative);

      return this.internalRegisterLocalEvent(
         element,
         eventName,
         (e: MouseEvent) => {
            callingMod.postEvent(eventID, e.pageX - x_off, e.pageY - y_off);
         }
      )
   }
   registerGlobalMouseMoveEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number) {
      return this.internalGlobalMouseEvent(callingMod, 'mousemove', eventID);
   }

   registerLocalMouseMoveEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number, elementIDPtr: number, relative: boolean) {
      return this.internalLocalMouseEvent(
         callingMod, 
         'mousemove', "registerLocalMouseMoveEvent", 
         eventID, elementIDPtr, 
         relative
      );
   }

   registerGlobalMouseDownEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number) {
      return this.internalGlobalMouseEvent(
         callingMod, 'mousedown', eventID
      );
   }

   registerLocalMouseDownEvent(callingMod:IWasmModule|IWasmModule, eventID: number, elementIDPtr: number, relative: boolean) {
      return this.internalLocalMouseEvent(
         callingMod, 'mousedown',
         "registerLocalMouseDownEvent", eventID,
         elementIDPtr, relative
      );
   }

   registerGlobalMouseUpEvent(callingMod:IWasmModule|IWasmModule, eventID: number) {
      return this.internalGlobalMouseEvent(
         callingMod, 'mouseup', eventID
      )
   }

   registerLocalMouseUpEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number, elementIDPtr: number, relative: boolean) {
      return this.internalLocalMouseEvent(
         callingMod, 'mouseup', "registerLocalMouseUpEvent",
         eventID, elementIDPtr, relative
      )
   }

   registerGlobalMouseClickEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number) {
      return this.internalGlobalMouseEvent(
         callingMod, 'click', eventID
      )
   }

   registerLocalMouseClickEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number, elementIDPtr: number, relative: boolean) {
      return this.internalLocalMouseEvent(
         callingMod, 'click', 'registerLocalMouseClickEvent',
         eventID, elementIDPtr, relative
      )
   }

   registerGlobalMouseDoubleClickEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number) {
      return this.internalGlobalMouseEvent(
         callingMod, 'dblclick', eventID
      )
   }

   registerLocalMouseDoubleClickEvent(callingMod:IWasmModule|IWasmModuleAsync, eventID: number, elementIDPtr: number, relative: boolean) {
      return this.internalLocalMouseEvent(
         callingMod, 'dblclick', "registerLocalMouseDoubleClickEvent",
         eventID, elementIDPtr, relative
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
         this.stopUIEvent(callingMod, eventHandlerID);
      }
   }
}


