
import { bindCanvasEvents, CanvasEventTypes, ICanvasEvents } from "./twrcanvasevents.js";
import { IConsoleScreen, keyEventToCodePoint } from "./twrcon.js";
import { twrConsoleWindow } from "./twrconwindow.js";
import { TLibImports, twrLibrary, twrLibraryInstanceRegistry } from "./twrlibrary.js";
import { IWasmModule } from "./twrmod.js";
import { IWasmModuleAsync } from "./twrmodasync.js";



//nominal type - Allows FullID to be considered a unique type compared to number
//can easily remove { readonly '': unique symbol } to make it a simple type alias
// type FullID = number & { readonly '': unique symbol };

type FullID = number;



function calculateID(mod:IWasmModule|IWasmModuleAsync, id: number): FullID {
   if (mod.id >= (2**20)) throw new Error("twrlibaudio was given a module ID greater than 20 bits long!");
   if (id >= (2**32)) throw new Error("twrlibaudio was given an object ID greater than 32 bits!");
   //should be equivalent to (mod.id << 32) | id
   //can't use shift operations without being limited to 32-bit signed integers or using bignumber
   return ((mod.id & (2**20 - 1)) * 2**32 + id) as FullID;
}


function assertTrue(condition: boolean, errMsg?: string) {
   if (!condition) {
      throw new Error(errMsg);
   }
}
function assertDefined<T>(val: T|undefined, errMsg?: string): asserts val is T {
   if (val == undefined) {
      throw new Error(errMsg);
   }
}
class DoublyLinkedListRoot<T> {
   rootNode?: DoublyLinkedListNode<T>;
   tailNode?: DoublyLinkedListNode<T>;
   constructor() {}

   getRoot(): DoublyLinkedListNode<T>|undefined {
      return this.rootNode;
   }
   getTail(): DoublyLinkedListNode<T>|undefined {
      return this.tailNode;
   }

   private createNode(val: T): DoublyLinkedListNode<T> {
      return new DoublyLinkedListNode<T>(val, this);
   }
   createNodeAtFront(val: T): DoublyLinkedListNode<T> {
      const node = this.createNode(val);
      if (this.rootNode) {
         this.rootNode.prev = node;
         node.next = this.rootNode;
         this.rootNode = node;
      } else {
         assertTrue(!this.tailNode);
         this.rootNode = node;
         this.tailNode = node;
      }

      return node;
   }
   createNodeAtEnd(val: T): DoublyLinkedListNode<T> {
      const node = this.createNode(val);
      if (this.tailNode) {
         this.tailNode.next = node;
         node.prev = this.tailNode;
         this.tailNode = node;
      } else {
         assertTrue(!this.rootNode);
         this.rootNode = node;
         this.tailNode = node;
      }

      return node;
   }

}
class DoublyLinkedListNode<T> {
   next?: DoublyLinkedListNode<T>;
   prev?: DoublyLinkedListNode<T>;
   private root: DoublyLinkedListRoot<T>;
   val: T;

   constructor(val: T, root: DoublyLinkedListRoot<T>) {
      this.val = val;
      this.root = root;
   }

   cutConnections() {
      if (this.prev && this.next) {
         this.prev.next = this.next;
         this.next.prev = this.prev;
      } else if (this.prev) {
         this.prev.next = undefined;
         assertTrue(this.root.tailNode == this);
         this.root.tailNode = this.prev;
      } else if (this.next) {
         this.next.prev = undefined;
         assertTrue(this.root.rootNode == this);
         this.root.rootNode = this.next;
      } else {
         if (this.root.rootNode == this)
            this.root.rootNode = undefined;
         if (this.root.tailNode == this)
            this.root.tailNode = undefined;
      }
      this.next = undefined;
      this.prev = undefined;
   }

   makeRootHead() {
      if (this.root.rootNode == this) {
         return;
      } else if (this.root.rootNode == undefined) {
         assertTrue(this.next == undefined && this.prev == undefined);
         this.root.rootNode = this;
         assertTrue(!this.root.tailNode);
         this.root.tailNode = this;
      } else {
         this.cutConnections();
         this.next = this.root.rootNode;
         this.root.rootNode.prev = this;
         this.root.rootNode = this;
      }      
   }

   makeRootTail() {
      if (this.root.tailNode == this) {
         return;
      } else if (this.root.tailNode == undefined) {
         assertTrue(this.next == undefined && this.prev == undefined);
         assertTrue(this.root.rootNode == undefined);
         this.root.rootNode = this;
         this.root.tailNode = this;
      } else {
         this.cutConnections();
         this.prev = this.root.tailNode;
         this.root.tailNode.next = this;
         this.root.tailNode = this;
      }
   }

   getNext() {
      return this.next;
   }
   getPrev() {
      return this.prev;
   }
   
}

export enum ResizedSides {
   Top = 1,
   Right = 2,
   Bottom = 4,
   Left = 8,

   TopRight = ResizedSides.Top | ResizedSides.Right,
   BottomRight = ResizedSides.Bottom | ResizedSides.Right,
   BottomLeft = ResizedSides.Bottom | ResizedSides.Left,
   TopLeft = ResizedSides.Top | ResizedSides.Left,
}
enum WindowParentType {
   Window,
   Layer
}
type WindowParent = [WindowParentType.Window, WindowInfo]
   | [WindowParentType.Layer, number];
interface WindowInfo {
   window: twrConsoleWindow,
   x: number,
   y: number,
   hidden: boolean,
   orderNode?: DoublyLinkedListNode<WindowInfo>,
   minXSize: number,
   minYSize: number,
   cursor: string,
   popupWindows: DoublyLinkedListRoot<WindowInfo>,
   parent: WindowParent,

   childSpawnOffset: WindowSpawnOffset,
   // selectedPopupWindow: boolean,
};
interface ResizeInfo {
   resizeSide: ResizedSides
   startX: number,
   startY: number,
   windowStartX: number,
   windowStartY: number,
   windowStartXSize: number,
   windowStartYSize: number,
}
interface DragInfo {
   startX: number,
   startY: number,
   windowStartX: number,
   windowStartY: number,
}
interface WindowSpawnOffset {
   x: number,
   y: number,
   baseX: number,
   lastSpawnTime: number
}
enum BroadCursorState {
   Background,
   Window,
   Moving,
   Resizing,
};
type CursorState = [BroadCursorState.Background]
   | [BroadCursorState.Window, number]
   | [BroadCursorState.Moving]
   | [BroadCursorState.Resizing, string];
function areCursorStatesEqual(a: CursorState, b: CursorState): boolean {
   if (a[0] != b[0]) return false;
   switch(a[0]) {
      case BroadCursorState.Background:
         return true;
      case BroadCursorState.Window:
         return a[1] == b[1];
      case BroadCursorState.Moving:
         return true;
      case BroadCursorState.Resizing:
         return a[1] == b[1];
   }
}
interface LastHoveredWindow {
   window: WindowInfo,
   lastX: number,
   lastY: number,
}
export class twrConsoleScreen extends twrLibrary implements ICanvasEvents, IConsoleScreen {
   id: number;

   readonly element: HTMLCanvasElement;
   ctx: CanvasRenderingContext2D;

   imports: TLibImports = {
      twrScreenSpawnWindow: {},
      twrScreenSetWindowLayer: {},
      twrScreenMoveWindow: {},
      twrScreenCloseWindow: {},
   };
   

   // every library should have this line
   libSourcePath = new URL(import.meta.url).pathname;

   private windows: Map<number, WindowInfo> = new Map();
   //the window layer to be selected
   // private selectedWindowLayer = 1;
   private selectedWindow?: WeakRef<WindowInfo>;
   //the window order seperated by 3 layers.
   private windowOrder: DoublyLinkedListRoot<WindowInfo>[] = [];

   private mouseX: number = 0;
   private mouseY: number = 0;
   private clickedWindow?: WindowInfo = undefined;
   private dragStart?: DragInfo = undefined;
   private resizeStart?: ResizeInfo = undefined;

   private cursorState: CursorState = [BroadCursorState.Background];

   private setMouseCursor: (cursor: string) => void;
   constructor(canvas: HTMLCanvasElement, selfRegisterEvents: boolean = true, setMouseCursor?: (cursor: string) => void) {
      // all library constructors should start with these two lines
      super();
      this.id=twrLibraryInstanceRegistry.register(this);

      this.element = canvas;
      this.ctx = canvas.getContext("2d")!;

      if (selfRegisterEvents)
         bindCanvasEvents(this, this.element);

      this.setMouseCursor = setMouseCursor ?? ((cursor: string) => {
         this.element.style.cursor = cursor;
      });

      for (let i = 0; i < 3; i++) {
         this.windowOrder[i] = new DoublyLinkedListRoot();
      }
   }
   getProp(propName: string) {
      switch (propName) {
         case "width":
            return this.element.width;
         case "height":
            return this.element.height;
         default:
            return -1;
      }
   }
   twrConGetProp(callingMod: IWasmModule | IWasmModuleAsync, pn: number) {
      return this.getProp(callingMod.getString(pn));
   }
   twrRegisterEvent(callingMod: IWasmModuleAsync | IWasmModule, eventType: number, eventID: number) {

   }
   twrUnregisterEvent(callingMod: IWasmModuleAsync | IWasmModule, eventType: number, eventID: number) {

   }
   twrUnregisterAllEvents(callingMod: IWasmModuleAsync | IWasmModule) {

   }

   private setCursorState(state: CursorState) {
      if (!areCursorStatesEqual(this.cursorState, state)) {
         this.cursorState = state;
         switch (state[0]) {
            case BroadCursorState.Background:
               this.setMouseCursor("auto");
            break;
            case BroadCursorState.Moving:
               this.setMouseCursor("grab");
            break;
            case BroadCursorState.Resizing:
               this.setMouseCursor(state[1]);
            break;
            case BroadCursorState.Window:
               const window = this.windows.get(state[1]);
               if (window) {
                  this.setMouseCursor(window.cursor);
               } else {
                  this.setCursorState([BroadCursorState.Background]);
               }
            break;
         }
      }
   }

   readonly baseWindowSpaceOffset = {
      x: 50,
      y: 50
   };
   private lastInteractionTime = Date.now();
   private windowSpawnOffset: WindowSpawnOffset = {
      x: this.baseWindowSpaceOffset.x,
      y: this.baseWindowSpaceOffset.y,
      //the base x coordinate at the top
      // is incremented by spawnOffsetChange everytime windows loop back to the top
      baseX: this.baseWindowSpaceOffset.x,
      
      lastSpawnTime: this.lastInteractionTime + 10,
   };
   private static internalChildDragFunction(weakThis: WeakRef<twrConsoleScreen>, weakWindowInfo: WeakRef<WindowInfo>, x: number, y: number, event: CanvasEventTypes) {
      const strongThis = weakThis.deref();
      const strongWindowInfo = weakWindowInfo?.deref();
      if (!strongThis || !strongWindowInfo) return;
      strongThis.setCursorState([BroadCursorState.Moving]);
      if (!strongThis.dragStart && strongThis.clickedWindow == strongWindowInfo && event == CanvasEventTypes.MOUSE_DOWN) {
         strongThis.dragStart = {
            startX: x + strongWindowInfo.x,
            startY: y + strongWindowInfo.y,
            windowStartX: strongWindowInfo.x,
            windowStartY: strongWindowInfo.y,
         };
      }
   }
   private static internalChildResizeFunction(weakThis: WeakRef<twrConsoleScreen>, weakWindowInfo: WeakRef<WindowInfo>, x: number, y: number, side: ResizedSides, event: CanvasEventTypes) {
      const strongThis = weakThis.deref();
      const strongWindowInfo = weakWindowInfo?.deref();
      if (!strongThis || !strongWindowInfo) return;
      switch (side) {
         case ResizedSides.BottomLeft:
         case ResizedSides.TopRight:
            strongThis.setCursorState([BroadCursorState.Resizing, 'nesw-resize']);
         break;

         case ResizedSides.BottomRight:
         case ResizedSides.TopLeft:
            strongThis.setCursorState([BroadCursorState.Resizing, 'nwse-resize']);
         break;

         case ResizedSides.Top:
         case ResizedSides.Bottom:
            strongThis.setCursorState([BroadCursorState.Resizing, 'ns-resize']);
         break;

         case ResizedSides.Left:
         case ResizedSides.Right:
            strongThis.setCursorState([BroadCursorState.Resizing, 'ew-resize']);
         break;
      }
      if (!strongThis.resizeStart && strongThis.clickedWindow == strongWindowInfo && event == CanvasEventTypes.MOUSE_DOWN) {
         strongThis.resizeStart = {
            resizeSide: side,
            startX: x + strongWindowInfo.x,
            startY: y + strongWindowInfo.y,
            windowStartX: strongWindowInfo.x,
            windowStartY: strongWindowInfo.y,
            windowStartXSize: strongWindowInfo.window.element.width,
            windowStartYSize: strongWindowInfo.window.element.height,
         };
      }
   }
   private static internalChildSetCursor(weakThis: WeakRef<twrConsoleScreen>, weakWindowInfo: WeakRef<WindowInfo>, cursor: string) {
      const strongThis = weakThis.deref();
      const strongWindowInfo = weakWindowInfo?.deref();
      if (!strongThis || !strongWindowInfo) return;
      if (cursor != strongWindowInfo.cursor) {
         strongWindowInfo.cursor = cursor;
         if (
            strongThis.cursorState[0] == BroadCursorState.Window
            && strongThis.cursorState[1] == strongWindowInfo.window.id
         ) {
            strongThis.setMouseCursor(cursor);
         }
      }
   }
   //how much to change windowSpawnOffset every time a window is spawned
   readonly spawnOffsetChange = 25;
   jsSpawnWindow(title?: string, width: number = 500, height: number = 500, x?: number, y?: number): twrConsoleWindow {
      const canvas = document.createElement("canvas");
      canvas.height = width;
      canvas.width = height;
      if (this.windowSpawnOffset.x >= this.element.width-50) {
         this.windowSpawnOffset.x = this.element.width > 100 ? this.baseWindowSpaceOffset.x : 5;
         this.windowSpawnOffset.y = this.element.height > 100 ? this.baseWindowSpaceOffset.y : 5;
         this.windowSpawnOffset.baseX = this.windowSpawnOffset.x;
      }
      if (this.windowSpawnOffset.y >= this.element.height-50) {
         this.windowSpawnOffset.y = this.element.height > 100 ? this.baseWindowSpaceOffset.y : 5;
         this.windowSpawnOffset.baseX += this.spawnOffsetChange;
         this.windowSpawnOffset.x = this.windowSpawnOffset.baseX;
      }
      if (this.lastInteractionTime > this.windowSpawnOffset.lastSpawnTime) {
         this.windowSpawnOffset.x = this.baseWindowSpaceOffset.x;
         this.windowSpawnOffset.y = this.baseWindowSpaceOffset.y;
      }

      const windowInfo: WindowInfo = {
         //small hack to allow WindowInfo to be defined before window
         // so it can be passed into the handler functions like drag, resize, and cursor
         window: undefined as any as twrConsoleWindow,
         x: x ?? this.windowSpawnOffset.x,
         y: y ?? this.windowSpawnOffset.y,
         hidden: false,
         minXSize: 200,
         minYSize: 200,
         cursor: "auto",
         popupWindows: new DoublyLinkedListRoot(),
         parent: [WindowParentType.Layer, 1],
         // selectedPopupWindow: false,
         childSpawnOffset: {
            x: 0,
            y: 0,
            baseX: 0,
            lastSpawnTime: Date.now(),
         }
      };
      const weakWindowInfo = new WeakRef(windowInfo);
      const weakThis = new WeakRef(this);
      
      const window = new twrConsoleWindow(
         canvas,
         title,
         false, 
         twrConsoleScreen.internalChildDragFunction.bind(undefined, weakThis, weakWindowInfo), 
         twrConsoleScreen.internalChildResizeFunction.bind(undefined, weakThis, weakWindowInfo), 
         twrConsoleScreen.internalChildSetCursor.bind(undefined, weakThis, weakWindowInfo)
      );
      windowInfo.window = window;
      
      this.windowSpawnOffset.x += this.spawnOffsetChange;
      this.windowSpawnOffset.y += this.spawnOffsetChange;
      this.windows.set(window.id, windowInfo);
      const node = this.windowOrder[1].createNodeAtFront(windowInfo);
      // this.selectedWindowLayer = 1;
      this.selectedWindow = new WeakRef(node.val);
      windowInfo.orderNode = node;

      return window;
   }

   private internalSetNegativesUndefined(val: number|undefined): number|undefined {
      if (val == undefined || val < 0) {
         return undefined;
      } else {
         return val;
      }
   }
   twrScreenSpawnWindow(mod: IWasmModule | IWasmModuleAsync, titlePtr?: number, width?: number, height?: number, x?: number, y?: number): number {
      let nTitle;
      if (titlePtr != undefined) {
         nTitle = mod.getString(titlePtr);
      } else {
         nTitle = undefined;
      }
      const window = this.jsSpawnWindow(
         nTitle,
         this.internalSetNegativesUndefined(width), 
         this.internalSetNegativesUndefined(height), 
         this.internalSetNegativesUndefined(x),
         this.internalSetNegativesUndefined(y)
      );
      return window.id;
   }

   private internalSetWindowLayer(windowInfo: WindowInfo, layer: number) {
      if (layer < 0 || layer >= this.windowOrder.length) throw new Error(`twrScreenSetWindow layer was given an invalid layer number (${layer}), expected a number between 0 and ${this.windowOrder.length-1}`);

      windowInfo.orderNode?.cutConnections();
      windowInfo.orderNode = this.windowOrder[layer].createNodeAtFront(windowInfo);
   }
   //set the layer a window should reside on
   jsSetWindowLayer(window: twrConsoleWindow, layer: number) {
      const windowInfo = this.windows.get(window.id);
      if (windowInfo == undefined) throw new Error(`twrScreenSetWindowLayer was given a window not registered with this screen!`);
      this.internalSetWindowLayer(windowInfo, layer);
   } 
   twrScreenSetWindowLayer(mod: IWasmModule | IWasmModuleAsync, window: number, layer: number) {
      const windowInfo = this.windows.get(window);
      if (windowInfo == undefined) throw new Error(`twrScreenSetWindowLayer was given a window not registered with this screen!`);
      this.internalSetWindowLayer(windowInfo, layer);
   }

   jsSpawnPopupWindow(window: twrConsoleWindow, title?: string, width: number = 50, height: number = 50, x?: number, y?: number): twrConsoleWindow {
      const windowInfo = this.windows.get(window.id);
      if (windowInfo == undefined) throw new Error(`twrScreenSpawnPopupWindow was given a window not registered with this screen!`);
      
      const canvas = document.createElement("canvas");
      canvas.height = width;
      canvas.width = height;

      const middleX = windowInfo.x + (windowInfo.window.element.width - width)/2;
      const middleY = windowInfo.y + (windowInfo.window.element.height - height)/2;
      
      const childSpawnOffset = this.windowSpawnOffset;
      if (childSpawnOffset.x+middleX >= this.element.width-50) {
         childSpawnOffset.x = 0;
         childSpawnOffset.y = 0;
         childSpawnOffset.baseX = childSpawnOffset.x;
      }
      if (childSpawnOffset.y+middleY >= this.element.height-50) {
         childSpawnOffset.y = 0;
         childSpawnOffset.baseX += this.spawnOffsetChange;
         childSpawnOffset.x = childSpawnOffset.baseX;
      }
      if (this.lastInteractionTime > childSpawnOffset.lastSpawnTime) {
         childSpawnOffset.x = 0;
         childSpawnOffset.y = 0;
      }
      const offsetSpawnX = middleX + childSpawnOffset.x;
      const offsetSpawnY = middleY + childSpawnOffset.y;
      
      let spawnX = offsetSpawnX+width;
      if (offsetSpawnX <= 0) {
         spawnX = 0;
      } else if (offsetSpawnX+width >= this.element.width) {
         spawnX = this.element.width-width;
      }
      let spawnY = offsetSpawnY+height;
      if (offsetSpawnY <= 0) {
         spawnY = 0;
      } else if (offsetSpawnY+height >= this.element.height) {
         spawnY = this.element.height-height;
      }

      const popupWindowInfo: WindowInfo = {
         //small hack to allow WindowInfo to be defined before window
         // so it can be passed into the handler functions like drag, resize, and cursor
         window: undefined as any as twrConsoleWindow,
         x: x ?? spawnX,
         y: y ?? spawnY,
         hidden: false,
         minXSize: width,
         minYSize: height,
         cursor: "auto",
         popupWindows: new DoublyLinkedListRoot(),
         parent: [WindowParentType.Window, windowInfo],
         childSpawnOffset: {
            x: 0,
            y: 0,
            baseX: 0,
            lastSpawnTime: Date.now(),
         }
      };
      let weakPopupInfo = new WeakRef(popupWindowInfo);
      const weakThis = new WeakRef(this);
      
      const popupWindow = new twrConsoleWindow(
         canvas, 
         title,
         false, 
         twrConsoleScreen.internalChildDragFunction.bind(undefined, weakThis, weakPopupInfo), 
         twrConsoleScreen.internalChildResizeFunction.bind(undefined, weakThis, weakPopupInfo), 
         twrConsoleScreen.internalChildSetCursor.bind(undefined, weakThis, weakPopupInfo)
      );
      popupWindowInfo.window = popupWindow;
      
      childSpawnOffset.x += this.spawnOffsetChange;
      childSpawnOffset.y += this.spawnOffsetChange;
      this.windows.set(popupWindow.id, windowInfo);
      const node = windowInfo.popupWindows.createNodeAtFront(windowInfo);
      this.selectedWindow = new WeakRef(node.val);
      popupWindowInfo.orderNode = node;

      return popupWindow;
   }
   twrScreenSpawnPopupWindow(mod: IWasmModule | IWasmModuleAsync, windowID: number, title?: number, width?: number, height?: number, x?: number, y?: number) {
      const windowInfo = this.windows.get(windowID);
      if (windowInfo == undefined) throw new Error(`twrScreenSpawnPopupWindow was given a window not registered with this screen!`);
      const popupWindow = this.jsSpawnPopupWindow(
         windowInfo.window,
         (title == undefined || title == 0) ? "" : mod.getString(title),
         this.internalSetNegativesUndefined(width),
         this.internalSetNegativesUndefined(height),
         this.internalSetNegativesUndefined(x),
         this.internalSetNegativesUndefined(y),
      );

      return popupWindow.id;
   }

   private internalCloseWindow(windowInfo: WindowInfo) {
      this.internalDeselectWindow(windowInfo);

      windowInfo.parent = [WindowParentType.Layer, -1];

      this.windows.delete(windowInfo.window.id);
      windowInfo.orderNode!.cutConnections();
      
      for (let node = windowInfo.popupWindows.getRoot(); node != undefined; node = node.getNext()) {
         this.internalCloseWindow(node.val);
      }
   }

   jsCloseWindow(window: twrConsoleWindow) {
      const windowInfo = this.windows.get(window.id);
      if (windowInfo == undefined) throw new Error(`jsCloseWindow was given a window not registered with this screen!`);
      
      this.internalCloseWindow(windowInfo);
   }
   twrScreenCloseWindow(mod: IWasmModule | IWasmModuleAsync, windowID: number) {
      const windowInfo = this.windows.get(windowID);
      if (windowInfo == undefined) throw new Error(`twrScreenCloseWindow was given a window not registered with this screen!`);

      this.internalCloseWindow(windowInfo);
   }

   private internalMinimizeWindow(window: WindowInfo) {
      if (!window.hidden) {
         this.internalDeselectWindow(window);
         window.hidden = true;
      }
   }
   jsMinimizeWindow(window: twrConsoleWindow) {
      const windowInfo = this.windows.get(window.id);
      if (windowInfo == undefined) throw new Error(`jsMinimizeWindow was given a window not registered with this screen!`);

      this.internalMinimizeWindow(windowInfo);
   }
   twrScreenMinimizeWindow(mod: IWasmModule | IWasmModuleAsync, windowID: number) {
      const windowInfo = this.windows.get(windowID);
      if (windowInfo == undefined) throw new Error(`twrScreenMinimizeWindow was given a window id not registered with this screen!`);

      this.internalMinimizeWindow(windowInfo);
   }

   private internalUnminimizeWindow(window: WindowInfo) {
      if (window.hidden) {
         window.hidden = false;
      }
   }
   jsUnminimizeWindow(window: twrConsoleWindow) {
      const windowInfo = this.windows.get(window.id);
      if (windowInfo == undefined) throw new Error(`jsUniminimizeWindow was given a window not registered with this screen!`);

      this.internalUnminimizeWindow(windowInfo);
   }
   twrScreenUnminimizeWindow(mod: IWasmModule | IWasmModuleAsync, windowID: number) {
      const windowInfo = this.windows.get(windowID);
      if (windowInfo == undefined) throw new Error(`twrScreenUnminimizeWindow was given a window id not registered with this screen!`);

      this.internalUnminimizeWindow(windowInfo);
   }



   moveWindow(window: twrConsoleWindow, x: number, y: number) {
      const info = this.windows.get(window.id);
      assertDefined(info, "Error! twrConsoleWindow moveWindow: Given a window that isn't registered with this screen!");
      info.x = x;
      info.y = y;
   }
   twrScreenMoveWindow(mod: IWasmModule | IWasmModuleAsync, windowID: number, x: number, y: number) {
      const info = this.windows.get(windowID);
      assertDefined(info, "Error! twrScreenMoveWindow: Given a windowID that isn't registered with this screen!");
      this.moveWindow(info.window, x, y);
   }

   handleCanvasKeyEvent(event: CanvasEventTypes, key: number) {
      this.selectedWindow?.deref()?.window.handleCanvasKeyEvent(event, key);
      return true;
   }

   private internalSetSelected(window: WindowInfo) {
      if (this.selectedWindow?.deref() == window) return;
      this.selectedWindow = new WeakRef(window);
      window.orderNode!.makeRootHead();
      for (let parent = window.parent; parent[0] == WindowParentType.Window; parent = parent[1].parent) {
         parent[1].orderNode!.makeRootHead();
      }
   }
   private internalFindHoveredWindow(x: number, y: number, windows?: DoublyLinkedListRoot<WindowInfo>): WindowInfo|undefined {
      if (windows == undefined) {
         for (const layer of this.windowOrder) {
            const val = this.internalFindHoveredWindow(x, y, layer);
            if (val != undefined) return val;
         }
         return undefined;
      }
      for (let window = windows.getRoot(); window != undefined; window = window.getNext()) {
         const val = this.internalFindHoveredWindow(x, y, window.val.popupWindows);
         if (val != undefined) return val;

         const n_x = x - window.val.x;
         const n_y = y - window.val.y;
         const width = window.val.window.element.width;
         const height = window.val.window.element.height;
         if (0 <= n_x && n_x <= width && 0 <= n_y && n_y <= height) {
            return window.val;
         }
      }
   }
   private lastHoveredWindow?: WeakRef<LastHoveredWindow>;
   private internalDeselectWindow(windowInfo: WindowInfo) {
      if (this.selectedWindow?.deref() == windowInfo) {
         this.selectedWindow = undefined;
         windowInfo.window.handleCanvasMouseEvent(CanvasEventTypes.MOUSE_CLICKED_OFF, -1, -1, -1);
      }
      if (this.lastHoveredWindow?.deref()?.window == windowInfo) {
         const data = this.lastHoveredWindow!.deref()!;
         this.lastHoveredWindow = undefined;
         this.setCursorState([BroadCursorState.Background]);
         windowInfo.window.handleCanvasMouseEvent(CanvasEventTypes.MOUSE_LEAVE, data.lastX, data.lastY, 0);
      }
      if (this.clickedWindow == windowInfo) {
         this.clickedWindow.window.handleCanvasMouseEvent(CanvasEventTypes.MOUSE_UP, 0, 0, 0);

         this.clickedWindow = undefined;
         this.dragStart = undefined;
         this.resizeStart = undefined;
      }
   }
   handleCanvasMouseEvent(event: CanvasEventTypes, x: number, y: number, button: number): boolean {
      this.mouseX = x;
      this.mouseY = y;
      // console.log(CanvasEventTypes[event], x, y, this.clickedWindow, this.resizeStart, this.dragStart);

      if (event == CanvasEventTypes.MOUSE_MOVE && this.resizeStart) {
         assertDefined(this.clickedWindow);
         this.lastInteractionTime = Date.now();
         let nX: number|undefined = undefined;
         let nY: number|undefined = undefined;
         let nXSize: number|undefined = undefined;
         let nYSize: number|undefined = undefined;

         const side = this.resizeStart.resizeSide;
         // console.log(ResizedSides[side], ResizedSides.Left&side, ResizedSides.Right&side, ResizedSides.Top&side, ResizedSides.Bottom&side);

         if ((this.resizeStart.resizeSide & ResizedSides.Left) > 0) {
            nXSize = this.resizeStart.startX - x + this.resizeStart.windowStartXSize;
            nX = x - this.resizeStart.startX + this.resizeStart.windowStartX;
            if (nXSize <= this.clickedWindow.minXSize) {
               if (this.clickedWindow.minXSize == this.clickedWindow.window.element.width) {
                  nXSize = undefined;
                  nX = undefined;
               } else {
                  nXSize = this.clickedWindow.minXSize;
                  nX = this.resizeStart.windowStartX + this.resizeStart.windowStartXSize - this.clickedWindow.minXSize;
               } 
            }
         } else if ((this.resizeStart.resizeSide & ResizedSides.Right) > 0) {
            nXSize = x - this.resizeStart.startX + this.resizeStart.windowStartXSize;
            if (nXSize <= this.clickedWindow.minXSize) {
               if (this.clickedWindow.minXSize == this.clickedWindow.window.element.width)
                  nXSize = undefined
               else
                  nXSize = this.clickedWindow.minXSize;
            }
         }

         if ((this.resizeStart.resizeSide & ResizedSides.Top) > 0) {
            nYSize = this.resizeStart.startY - y + this.resizeStart.windowStartYSize;
            nY = y - this.resizeStart.startY + this.resizeStart.windowStartY;
            if (nYSize <= this.clickedWindow.minYSize) {
               if (this.clickedWindow.minYSize == this.clickedWindow.window.element.height) {
                  nYSize = undefined;
                  nY = undefined;
               } else {
                  nYSize = this.clickedWindow.minYSize;
                  nY = this.resizeStart.windowStartY + this.resizeStart.windowStartYSize - this.clickedWindow.minYSize;
               }
            }
         } else if ((this.resizeStart.resizeSide & ResizedSides.Bottom) > 0) {
            nYSize = y - this.resizeStart.startY + this.resizeStart.windowStartYSize;
            if (nYSize <= this.clickedWindow.minYSize) {
               if (this.clickedWindow.minYSize == this.clickedWindow.window.element.height)
                  nYSize = undefined;
               else
                  nYSize = this.clickedWindow.minYSize;
            }
         }

         if (nX != undefined)
            this.clickedWindow.x = nX;
         if (nY != undefined)
            this.clickedWindow.y = nY;

         // console.log(nY, this.clickedWindow.y, nYSize);
         
         if (nXSize != undefined || nYSize != undefined) {
            this.clickedWindow.window.resizeWindow(
               nXSize ?? this.clickedWindow.window.element.width,
               nYSize ?? this.clickedWindow.window.element.height
            );
         }
         return true;
      } if (event == CanvasEventTypes.MOUSE_MOVE && this.dragStart) {
         this.lastInteractionTime = Date.now();
         assertDefined(this.clickedWindow);
         this.moveWindow(
            this.clickedWindow.window,
            x - this.dragStart.startX + this.dragStart.windowStartX,
            y - this.dragStart.startY + this.dragStart.windowStartY
         );
         return true;
      } else if (event == CanvasEventTypes.MOUSE_MOVE && this.clickedWindow) {
         this.lastInteractionTime = Date.now();
         this.clickedWindow.window.handleCanvasMouseEvent(
            event,
            x - this.clickedWindow.x,
            y - this.clickedWindow.y,
            button
         );
         return true;
      } else if (event == CanvasEventTypes.MOUSE_UP && this.clickedWindow) {
         this.lastInteractionTime = Date.now();
         this.clickedWindow.window.handleCanvasMouseEvent(
            event,
            x - this.clickedWindow.x,
            y - this.clickedWindow.y,
            button
         );
         this.clickedWindow = undefined;
         this.resizeStart = undefined;
         this.dragStart = undefined;
         
         this.handleCanvasMouseEvent(CanvasEventTypes.MOUSE_MOVE, x, y, button);
         return true;
      } else if (event == CanvasEventTypes.MOUSE_LEAVE) {
         this.lastInteractionTime = Date.now();
         const lastWindow = this.lastHoveredWindow?.deref();
         if (lastWindow != undefined) {
            lastWindow.window.window.handleCanvasMouseEvent(
               CanvasEventTypes.MOUSE_LEAVE,
               lastWindow.lastX,
               lastWindow.lastY,
               button
            );
         }
         this.lastHoveredWindow = undefined;
         this.setCursorState([BroadCursorState.Background]);

         return true;
      } else if (event == CanvasEventTypes.MOUSE_CLICKED_OFF) {
         this.lastInteractionTime = Date.now();
         this.clickedWindow?.window.handleCanvasMouseEvent(
            CanvasEventTypes.MOUSE_UP,
            0,
            0,
            0
         );
         this.clickedWindow = undefined;
         this.dragStart = undefined;
         this.resizeStart = undefined;
         this.selectedWindow?.deref()?.window.handleCanvasMouseEvent(
            CanvasEventTypes.MOUSE_CLICKED_OFF,
            -1,
            -1,
            -1,
         );
         this.selectedWindow = undefined;
         this.setCursorState([BroadCursorState.Background]);
         return true;
      }

      const hoveredWindow = this.internalFindHoveredWindow(x, y);
      

      if (hoveredWindow == undefined) {
         this.setCursorState([BroadCursorState.Background]);
      } else {
         this.lastInteractionTime = Date.now();
         switch (event) {
            case CanvasEventTypes.MOUSE_DOWN:
               this.clickedWindow = hoveredWindow;
               //continue down to make this root
            case CanvasEventTypes.MOUSE_CLICK:
            case CanvasEventTypes.MOUSE_DBLCLICK:
               const selectedWindow = this.selectedWindow?.deref();
               if (hoveredWindow != selectedWindow && selectedWindow != undefined) {
                  selectedWindow.window.handleCanvasMouseEvent(
                     CanvasEventTypes.MOUSE_CLICKED_OFF,
                     -1,
                     -1,
                     -1
                  );
               }
               this.internalSetSelected(hoveredWindow);
            break;

            default:
         }
         this.setCursorState([BroadCursorState.Window, hoveredWindow.window.id]);
         const n_x = x - hoveredWindow.x;
         const n_y = y - hoveredWindow.y;
         hoveredWindow.window.handleCanvasMouseEvent(event, n_x, n_y, button);
      }
      const lastWindow = this.lastHoveredWindow?.deref();
      if (lastWindow != hoveredWindow) {
         if (lastWindow != undefined) {
            lastWindow.window.window.handleCanvasMouseEvent(
               CanvasEventTypes.MOUSE_LEAVE,
               x - lastWindow.window.x,
               y - lastWindow.window.y,
               button
            );   
         }
         if (hoveredWindow != undefined) {
            this.lastHoveredWindow = new WeakRef({
               window: hoveredWindow, 
               lastX: x - hoveredWindow.x, 
               lastY: y - hoveredWindow.y
            });
         } else {
            this.lastHoveredWindow = undefined;
         }
      }
      const updatedLastWindow = this.lastHoveredWindow?.deref();
      if (updatedLastWindow != undefined) {
         updatedLastWindow.lastX = x - updatedLastWindow.window.x;
         updatedLastWindow.lastY = y - updatedLastWindow.window.y;
      }
      return true;
   }
   handleCanvasWheelEvent(event: CanvasEventTypes, deltaX: number, deltaY: number, deltaZ: number, deltaMode: number) {
      if (this.clickedWindow)
         return true;
      const hoveredWindow = this.internalFindHoveredWindow(this.mouseX, this.mouseY);
      if (hoveredWindow) {
         this.lastInteractionTime = Date.now();
         hoveredWindow.window.handleCanvasWheelEvent(event, deltaX, deltaY, deltaZ, deltaMode);
      }
      
      return true;
   }
   private internalRenderWindows(event: CanvasEventTypes, delta: number, windows: DoublyLinkedListRoot<WindowInfo>) {
      for (let node = windows.getTail(); node != undefined; node = node.getPrev()) {
         if (node.val.hidden) continue;

         node.val.window.handleCanvasAnimationFrameEvent(event, delta);
         this.ctx.drawImage(
            node.val.window.element,
            node.val.x,
            node.val.y
         );
         this.internalRenderWindows(event, delta, node.val.popupWindows);
      }
   }
   handleCanvasAnimationFrameEvent(event: CanvasEventTypes, delta: number) {
      this.ctx.fillStyle = "#87CEEB";
      this.ctx.fillRect(0, 0, this.element.width, this.element.height);
      for (let layerID = this.windowOrder.length-1; layerID >= 0; layerID--) {
         this.internalRenderWindows(event, delta, this.windowOrder[layerID]);
      }
      return true;
   }


}