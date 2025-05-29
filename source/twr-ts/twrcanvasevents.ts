import { keyEventToCodePoint } from "./twrcon.js";
import { IWasmModule } from "./twrmod.js";
import { IWasmModuleAsync } from "./twrmodasync.js";

export enum CanvasEventTypes {
   KEY_DOWN,
   KEY_UP,

   MOUSE_DOWN,
   MOUSE_UP,
   MOUSE_CLICK,
   MOUSE_DBLCLICK,
   MOUSE_MOVE,
   MOUSE_LEAVE,
   MOUSE_CLICKED_OFF,

   WHEEL,

   ANIMATION_FRAME,
   CANVAS_RESIZE,
}
export const NUM_CANVAS_EVENTS = Object.values(CanvasEventTypes).length;

export const CANVAS_EVENTS = [
   "keydown",
   "keyup",
   
   "mousedown",
   "mouseup",
   "click",
   "dblclick",
   "mousemove",
   "mouseleave",
   "MOUSE_CLICKED_OFF",

   "wheel",

   "ANIMATION_FRAME",
   "CANVAS_RESIZE",
];

export interface ICanvasEvents {
   /// Get canvas key events, return True to intercept event and stop it from being passed along
   handleCanvasKeyEvent: (event: CanvasEventTypes, key: number) => boolean;
   /// Get canvas mouse events, return True to intercept event and stop it from being passed along
   /// Button is taken directly from event as specified here: https://developer.mozilla.org/en-US/docs/Web/API/MouseEvent/button
   handleCanvasMouseEvent: (event: CanvasEventTypes, x: number, y: number, button: number) => boolean;
   /// Get canvas wheel events, return True to intercept event and stop it from being passed along
   handleCanvasWheelEvent: (event: CanvasEventTypes, deltaX: number, deltaY: number, deltaZ: number, deltaMode: number) => boolean;
   /// Get canvas animation frame events
   handleCanvasAnimationFrameEvent: (event: CanvasEventTypes, delta: number) => void;
}

let mouseDown = false;
window.document.addEventListener("mousedown", () => {
   mouseDown = true;
});
window.document.addEventListener("mouseup", () => {
   mouseDown = false;
});
function registerSimilarEvents(canvas: HTMLCanvasElement, start: CanvasEventTypes, end: CanvasEventTypes, handler: (eventType: CanvasEventTypes) => (event: any) => void) {
   for (let i = start; i <= end; i++) {
      canvas.addEventListener(CANVAS_EVENTS[i], handler(i));
   }
}
export function bindCanvasEvents(handler: ICanvasEvents, canvas: HTMLCanvasElement) {
   registerSimilarEvents(canvas, CanvasEventTypes.KEY_DOWN, CanvasEventTypes.KEY_UP, 
      (type) => (e: KeyboardEvent) => {
         console.log(e);
         const r=keyEventToCodePoint(e);  // twr-wasm utility function
         if (r) {
            handler.handleCanvasKeyEvent(type, r);
         }
      }
   );

   const bounding = canvas.getBoundingClientRect();
   const top = bounding.top + window.scrollY;
   const left = bounding.left + window.scrollX;

   //if someone clicks and drags, keep track of when the mouse button goes up outside of the tracked canvas
   // uses a global document mouseup event tracker + some local trackers to do so
   let trackMouseDown: Set<number> = new Set();
   let numTrackers = 0;
   let globalMouseUpTracker: ((e: MouseEvent) => void)|undefined = undefined;

   let hasBeenClickedOff = false;
   let mouseIsInCanvas = false;
   registerSimilarEvents(canvas, CanvasEventTypes.MOUSE_DOWN, CanvasEventTypes.MOUSE_MOVE,
      (type) => (e: MouseEvent) => {
         hasBeenClickedOff = false;
         mouseIsInCanvas = true;
         //if we have an event (that's not mouseleave) inside of the canvas again,
         // remove the global tracker
         if (globalMouseUpTracker != undefined) {
            window.document.removeEventListener("mouseup", globalMouseUpTracker);
            globalMouseUpTracker = undefined;
         }
         const x = e.pageX - left;
         const y = e.pageY - top;
         if (type == CanvasEventTypes.MOUSE_DOWN) {
            if (!trackMouseDown.has(e.button))
               numTrackers++;
            trackMouseDown.add(e.button);
         } else if (type == CanvasEventTypes.MOUSE_UP) {
            if (trackMouseDown.delete(e.button))
               numTrackers--;
         }
         handler.handleCanvasMouseEvent(
            type,
            x,
            y,
            e.button
         );
      }
   );

   document.body.addEventListener("click", () => {
      if (!hasBeenClickedOff && !mouseIsInCanvas) {
         hasBeenClickedOff = true;
         handler.handleCanvasMouseEvent(
            CanvasEventTypes.MOUSE_CLICKED_OFF,
            -1,
            -1,
            -1
         );
      }
   })

   //handler for mouseleave and tracking mouseup events outside of canvas
   canvas.addEventListener("mouseleave", (e: MouseEvent) => {
      mouseIsInCanvas = false;
      const x = e.pageX - left;
      const y = e.pageY - top;
      handler.handleCanvasMouseEvent(
         CanvasEventTypes.MOUSE_LEAVE,
         x,
         y,
         e.button
      );

      //if we never did any mouse down button presses before leaving, skip the rest
      if (numTrackers <= 0) return;
      //global tracking function
      globalMouseUpTracker = (e2: MouseEvent) => {
         //if we've stopped it, ignore it (should have been deregistered already)
         // this is here just in case
         if (globalMouseUpTracker == undefined) return;
         //if the button being released is one that we were tracking, run the rest
         if (trackMouseDown.has(e2.button)) {
            //delete it
            trackMouseDown.delete(e2.button);
            //remove it from the amount of buttons being tracked
            numTrackers--;
            //send mouse up event at the position the mouse left the canvas at
            handler.handleCanvasMouseEvent(
               CanvasEventTypes.MOUSE_UP,
               x,
               y,
               e2.button
            );
            //if that was the last tracker, remove this event
            if (numTrackers <= 0) {
               window.document.removeEventListener("mouseup", globalMouseUpTracker);
               globalMouseUpTracker = undefined;
            }
         }

      };
      //register the event
      window.document.addEventListener("mouseup", globalMouseUpTracker);
   });

   canvas.addEventListener("wheel", (e: WheelEvent) => {
      handler.handleCanvasWheelEvent(CanvasEventTypes.WHEEL, e.deltaX, e.deltaY, e.deltaZ, e.deltaMode);
   });

   const animation_loop = (delta: number) => {
      handler.handleCanvasAnimationFrameEvent(CanvasEventTypes.ANIMATION_FRAME, delta);
      requestAnimationFrame(animation_loop);
   };
   requestAnimationFrame(animation_loop);
}

export ///modID -> [module, eventID -> numRegistrations]
class EventRegistrations<T> {
   private events: Map<
      T, // event type enum
      Map< //map of ModID -> [Mod, Map<EventID, extraPtr]>]
         number, //ModID
         [
            WeakRef<IWasmModule|IWasmModuleAsync>, // Mod
            Map<
               number, //EventID 
               number //extraPtr
            > 
         ]
      >
   > = new Map();

   //maps [Mod, EventID] to the type of event
   private linearlyMappedIDs: WeakMap<
      IWasmModule|IWasmModuleAsync,
      Map<
         number,
         T
      >
   > = new WeakMap();

   constructor() {

   }

   registerEvent(mod: IWasmModule|IWasmModuleAsync, eventType: T, eventID: number, extraPtr: number) {
      
      let eventTypeHandlers = this.events.get(eventType);
      if (eventTypeHandlers == undefined) {
         eventTypeHandlers = new Map();
         this.events.set(eventType, eventTypeHandlers);
      } 

      let eventModHandlers = eventTypeHandlers.get(mod.id);
      if (eventModHandlers == undefined) {
         eventModHandlers = [
            new WeakRef(mod),
            new Map()
         ];
         eventTypeHandlers.set(mod.id, eventModHandlers);
      }
      let eventsByID = eventModHandlers[1];

      let linearIDs = this.linearlyMappedIDs.get(mod);
      if (linearIDs == undefined) {
         linearIDs = new Map();
         this.linearlyMappedIDs.set(mod, linearIDs);
      }

      if (eventsByID.has(eventID)) throw new Error("internal error!");
      if (linearIDs.has(eventID)) throw new Error("internal error!");

      eventsByID.set(eventID, extraPtr);
      linearIDs.set(eventID, eventType);

      return eventID;
   }

   unregisterEvent(mod: IWasmModule|IWasmModuleAsync, eventID: number) {
      const linearIds = this.linearlyMappedIDs.get(mod);
      if (linearIds == undefined) return false;

      const eventType = linearIds.get(eventID);
      if (eventType == undefined) return false;
      linearIds.delete(eventID);

      const eventTypeHandlers = this.events.get(eventType);
      if (eventTypeHandlers == undefined) throw new Error("internal error!");

      const eventModHandlers = eventTypeHandlers.get(mod.id);
      if (eventModHandlers == undefined) throw new Error("internal error!");

      const eventsByID = eventModHandlers[1];

      const success = eventsByID.delete(eventID);
      if (!success) throw new Error("internal error!");

      return true;
   }

   unregisterAllEvents(mod: IWasmModule|IWasmModuleAsync) {
      for (const [, eventTypeHandlers] of this.events) {
         eventTypeHandlers.delete(mod.id);
      }
      this.linearlyMappedIDs.delete(mod);
   }

   runEvent(eventType: T, ...args: number[]) {
      const eventTypeHandlers = this.events.get(eventType);
      if (eventTypeHandlers == undefined) return;

      const idsToRemove = [];
      for (const [modID, [mod, eventsByID]] of eventTypeHandlers) {
         const derefMod = mod.deref();
         if (derefMod == undefined) {
            idsToRemove.push(modID);
            continue;
         }
         for (const [eventID, extraPtr] of eventsByID) {
            derefMod.postEvent(eventID, extraPtr, ...args);
         }
      }
   }
}