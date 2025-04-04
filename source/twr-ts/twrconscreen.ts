
import { bindCanvasEvents, CanvasEventTypes, ICanvasEvents } from "./twrcanvasevents.js";
import { keyEventToCodePoint } from "./twrcon.js";
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
   selectedPopupWindow: boolean,
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

export class twrConsoleScreen extends twrLibrary implements ICanvasEvents {
   id: number;

   readonly canvas: HTMLCanvasElement;
   ctx: CanvasRenderingContext2D;

   imports: TLibImports = {
      
   };
   

   // every library should have this line
   libSourcePath = new URL(import.meta.url).pathname;

   private windows: Map<number, WindowInfo> = new Map();
   //the window layer to be selected
   private selectedWindowLayer = 1;
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

      this.canvas = canvas;
      this.ctx = canvas.getContext("2d")!;

      if (selfRegisterEvents)
         bindCanvasEvents(this, this.canvas);

      this.setMouseCursor = setMouseCursor ?? ((cursor: string) => {
         this.canvas.style.cursor = cursor;
      });

      for (let i = 0; i < 3; i++) {
         this.windowOrder[i] = new DoublyLinkedListRoot();
      }
   }

   private setCursorState(state: CursorState) {
      if (
         this.cursorState[0] != state[0]
         || this.cursorState.length != state.length
         || (this.cursorState.length >= 2 && this.cursorState[1] != state[1])
      ) {
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
   private windowSpawnOffset = {
      x: this.baseWindowSpaceOffset.x,
      y: this.baseWindowSpaceOffset.y,
      //the base x coordinate at the top
      // is incremented by spawnOffsetChange everytime windows loop back to the top
      baseX: this.baseWindowSpaceOffset.x,
   };
   //how much to change windowSpawnOffset every time a window is spawned
   readonly spawnOffsetChange = 25;
   jsSpawnWindow(): twrConsoleWindow {
      const canvas = document.createElement("canvas");
      canvas.height = 500;
      canvas.width = 500;
      let weakWindowInfo: WeakRef<WindowInfo> | undefined;
      const weakThis = new WeakRef(this);
      const dragFunction = (x: number, y: number, event: CanvasEventTypes) => {
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
      };
      const resizeFunction = (x: number, y: number, side: ResizedSides, event: CanvasEventTypes) => {
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
            this.resizeStart = {
               resizeSide: side,
               startX: x + strongWindowInfo.x,
               startY: y + strongWindowInfo.y,
               windowStartX: strongWindowInfo.x,
               windowStartY: strongWindowInfo.y,
               windowStartXSize: strongWindowInfo.window.element.width,
               windowStartYSize: strongWindowInfo.window.element.height,
            };
         }
      };
      const windowMouseSetCursor = (cursor: string) => {
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
      };
      if (this.windowSpawnOffset.x >= this.canvas.width-50) {
         this.windowSpawnOffset.x = this.canvas.width > 100 ? this.baseWindowSpaceOffset.x : 5;
         this.windowSpawnOffset.y = this.canvas.height > 100 ? this.baseWindowSpaceOffset.y : 5;
         this.windowSpawnOffset.baseX = this.windowSpawnOffset.x;
      }
      if (this.windowSpawnOffset.y >= this.canvas.height-50) {
         this.windowSpawnOffset.y = this.canvas.height > 100 ? this.baseWindowSpaceOffset.y : 5;
         this.windowSpawnOffset.baseX += this.spawnOffsetChange;
         this.windowSpawnOffset.x = this.windowSpawnOffset.baseX;
      }
      const window = new twrConsoleWindow(canvas, false, dragFunction, resizeFunction, windowMouseSetCursor);
      const windowInfo: WindowInfo = {
         window: window,
         x: this.windowSpawnOffset.x,
         y: this.windowSpawnOffset.y,
         hidden: false,
         minXSize: 200,
         minYSize: 200,
         cursor: "auto",
         popupWindows: new DoublyLinkedListRoot(),
         selectedPopupWindow: false,
      };
      this.windowSpawnOffset.x += this.spawnOffsetChange;
      this.windowSpawnOffset.y += this.spawnOffsetChange;
      weakWindowInfo = new WeakRef(windowInfo);
      this.windows.set(window.id, windowInfo);
      const node = this.windowOrder[1].createNodeAtFront(windowInfo);
      this.selectedWindowLayer = 1;
      windowInfo.orderNode = node;

      return window;
   }

   
   twrScreenSpawnWindow(mod: IWasmModule | IWasmModuleAsync): number {
      const window = this.jsSpawnWindow();
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



   moveWindow(window: twrConsoleWindow, x: number, y: number) {
      const info = this.windows.get(window.id);
      assertDefined(info, "Error! twrConsoleWindow moveWindow: Given a window that isn't registered with this screen!");
      info.x = x;
      info.y = y;
   }

   closeWindow(window: twrConsoleWindow) {
      const info = this.windows.get(window.id);
      assertDefined(info, "Error! twrConsoleWindow closeWindow: Given a window that isn't registered with this screen!");

      window.handleClose().then(() => {
         if (this.clickedWindow == info) {
            this.clickedWindow = undefined;
         }
         info.orderNode!.cutConnections();
         this.windows.delete(window.id);
      });
   }

   handleCanvasKeyEvent(event: CanvasEventTypes, key: number) {
      const selected = this.windowOrder[this.selectedWindowLayer].getRoot();
      if (selected) {
         if (selected.val.selectedPopupWindow)
            selected.val.popupWindows.getRoot()!
               .val.window.handleCanvasAnimationFrameEvent(event, key);
         else
            selected.val.window.handleCanvasKeyEvent(event, key);
      }
      return true;
   }

   private lastHoveredWindow?: WeakRef<WindowInfo>;
   handleCanvasMouseEvent(event: CanvasEventTypes, x: number, y: number, button: number): boolean {
      this.mouseX = x;
      this.mouseY = y;
      // console.log(CanvasEventTypes[event], x, y, this.clickedWindow, this.resizeStart, this.dragStart);

      if (event == CanvasEventTypes.MOUSE_MOVE && this.resizeStart) {
         assertDefined(this.clickedWindow);
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
         assertDefined(this.clickedWindow);
         this.moveWindow(
            this.clickedWindow.window,
            x - this.dragStart.startX + this.dragStart.windowStartX,
            y - this.dragStart.startY + this.dragStart.windowStartY
         );
         return true;
      } else if (event == CanvasEventTypes.MOUSE_MOVE && this.clickedWindow) {
         this.clickedWindow.window.handleCanvasMouseEvent(
            event,
            x - this.clickedWindow.x,
            y - this.clickedWindow.y,
            button
         );
         return true;
      } else if (event == CanvasEventTypes.MOUSE_UP && this.clickedWindow) {
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
         const lastWindow = this.lastHoveredWindow?.deref();
         if (lastWindow != undefined) {
            lastWindow.window.handleCanvasMouseEvent(
               CanvasEventTypes.MOUSE_LEAVE,
               x - lastWindow.x,
               y - lastWindow.y,
               button
            );
         }
         this.lastHoveredWindow = undefined;
         this.setCursorState([BroadCursorState.Background]);

         return true;
      } else if (event == CanvasEventTypes.MOUSE_CLICKED_OFF) {
         const rootNode = this.windowOrder[this.selectedWindowLayer].getRoot();
         if (rootNode == undefined) return true;
         rootNode.val.window.handleCanvasMouseEvent(
            CanvasEventTypes.MOUSE_CLICKED_OFF,
            -1,
            -1,
            -1,
         );

         this.setCursorState([BroadCursorState.Background]);
         return true;
      }


      let handledWindow: WindowInfo|undefined = undefined;
      layerloop: for (let layerID = 0; layerID < this.windowOrder.length; layerID++) {
         const layer = this.windowOrder[layerID];
         for (let node = layer.getRoot(); node != undefined; node = node.getNext()) {
            const n_x = x - node.val.x;
            const n_y = y - node.val.y;
            const width = node.val.window.element.width;
            const height = node.val.window.element.height;
            if (0 <= n_x && n_x <= width && 0 <= n_y && n_y <= height) {
               switch (event) {
                  case CanvasEventTypes.MOUSE_DOWN:
                     this.clickedWindow = node.val;
                     //continue down to make this root
                  case CanvasEventTypes.MOUSE_CLICK:
                  case CanvasEventTypes.MOUSE_DBLCLICK:
                     if (node != layer.getRoot()) {
                        layer.getRoot()!.val.window.handleCanvasMouseEvent(
                           CanvasEventTypes.MOUSE_CLICKED_OFF,
                           -1,
                           -1,
                           -1
                        );
                     }
                     node.makeRootHead();
                     this.selectedWindowLayer = layerID;
                  break;
   
                  default:
               }
               this.setCursorState([BroadCursorState.Window, node.val.window.id]);
               handledWindow = node.val;
               node.val.window.handleCanvasMouseEvent(event, n_x, n_y, button);
               
               break layerloop;
            }
         }
      }
      

      if (handledWindow == undefined) {
         this.setCursorState([BroadCursorState.Background]);
      }
      const lastWindow = this.lastHoveredWindow?.deref();
      if (lastWindow != handledWindow) {
         if (lastWindow != undefined) {
            lastWindow.window.handleCanvasMouseEvent(
               CanvasEventTypes.MOUSE_LEAVE,
               x - lastWindow.x,
               y - lastWindow.y,
               button
            );   
         }
         if (handledWindow != undefined)
            this.lastHoveredWindow = new WeakRef(handledWindow);
         else
            this.lastHoveredWindow = undefined;
      }
      return true;
   }
   handleCanvasWheelEvent(event: CanvasEventTypes, deltaX: number, deltaY: number, deltaZ: number, deltaMode: number) {
      if (this.clickedWindow)
         return true;
      layerLoop: for (let layerID = 0; layerID < this.windowOrder.length; layerID++) {
         const layer = this.windowOrder[layerID];
         for (let node = layer.getRoot(); node != undefined; node = node.getNext()) {
            const n_x = this.mouseX - node.val.x;
            const n_y = this.mouseY - node.val.y;
            const width = node.val.window.element.width;
            const height = node.val.window.element.height;
            if (0 <= n_x && n_y <= width && 0 <= n_y && n_y <= height) {
               node.val.window.handleCanvasWheelEvent(event, deltaX, deltaY, deltaZ, deltaMode);
               break layerLoop;
            }
         }
      }
      
      return true;
   }
   handleCanvasAnimationFrameEvent(event: CanvasEventTypes, delta: number) {
      this.ctx.fillStyle = "#87CEEB";
      this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
      for (let layerID = this.windowOrder.length-1; layerID >= 0; layerID--) {
         const layer = this.windowOrder[layerID];
         const renderLoop = (windows: DoublyLinkedListRoot<WindowInfo>) => {
            const tail = windows.getTail();
            if (tail == undefined) return;
            for (let node = windows.getTail(); node != undefined; node = node.getPrev()) {
               node.val.window.handleCanvasAnimationFrameEvent(event, delta);
               this.ctx.drawImage(
                  node.val.window.element, 
                  node.val.x, 
                  node.val.y
               );
               renderLoop(node.val.popupWindows);
            }
         };
         renderLoop(layer);
      }
      return true;
   }


}