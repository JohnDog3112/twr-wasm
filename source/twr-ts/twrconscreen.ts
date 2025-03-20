
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

interface WindowInfo {
   window: twrConsoleWindow,
   x: number,
   y: number,
   hidden: boolean,
   orderNode?: DoublyLinkedListNode<WindowInfo>,
};
export default class twrConsoleScreen extends twrLibrary implements ICanvasEvents {
   id: number;

   readonly canvas: HTMLCanvasElement;
   ctx: CanvasRenderingContext2D;

   imports: TLibImports = {
      
   };
   

   // every library should have this line
   libSourcePath = new URL(import.meta.url).pathname;

   windows: Map<number, WindowInfo> = new Map();
   windowOrder: DoublyLinkedListRoot<WindowInfo> = new DoublyLinkedListRoot();

   constructor(canvas: HTMLCanvasElement, selfRegisterEvents: boolean = true) {
      // all library constructors should start with these two lines
      super();
      this.id=twrLibraryInstanceRegistry.register(this);

      this.canvas = canvas;
      this.ctx = canvas.getContext("2d")!;

      if (selfRegisterEvents)
         bindCanvasEvents(this, this.canvas);
   }

   spawnWindow(mod: IWasmModule | IWasmModuleAsync): number {
      const canvas = new HTMLCanvasElement();
      canvas.height = 100;
      canvas.width = 100;
      const window = new twrConsoleWindow(canvas, false);
      const windowInfo: WindowInfo = {
         window: window,
         x: 20,
         y: 20,
         hidden: false
      };
      this.windows.set(window.id, windowInfo);
      const node = this.windowOrder.createNodeAtFront(windowInfo);
      windowInfo.orderNode = node;

      return window.id;
   }

   moveWindow(window: twrConsoleWindow, x: number, y: number) {
      const info = this.windows.get(window.id);
      assertDefined(info, "Error! twrConsoleWindow moveWindow: Given a window not registered with this screen!");
      info.x = x;
      info.y = y;
   }

   handleCanvasKeyEvent(event: CanvasEventTypes, key: number) {
      const selected = this.windowOrder.getRoot();
      if (selected) {
         selected.val.window.handleCanvasKeyEvent(event, key);
      }
      return true;
   }
   mouseX: number = 0;
   mouseY: number = 0;
   clickedWindow?: WindowInfo = undefined;
   handleCanvasMouseEvent(event: CanvasEventTypes, x: number, y: number, button: number) {
      this.mouseX = x;
      this.mouseY = y;

      if (event == CanvasEventTypes.MOUSE_UP && this.clickedWindow) {
         this.clickedWindow.window.handleCanvasMouseEvent(
            event,
            x - this.clickedWindow.x,
            y - this.clickedWindow.y,
            button
         );
         this.clickedWindow = undefined;
         return true;
      } else if (event == CanvasEventTypes.MOUSE_MOVE && this.clickedWindow) {
         this.clickedWindow.window.handleCanvasMouseEvent(
            event,
            x - this.clickedWindow.x,
            y - this.clickedWindow.y,
            button
         );
         return true;
      }

      for (let node = this.windowOrder.getRoot(); node != undefined; node = node.getNext()) {
         const n_x = x - node.val.x;
         const n_y = y - node.val.y;
         const width = node.val.window.element.width;
         const height = node.val.window.element.height;
         if (0 <= n_x && n_y <= width && 0 <= n_y && n_y <= height) {
            switch (event) {
               case CanvasEventTypes.MOUSE_DOWN:
                  this.clickedWindow = node.val;
                  //continue down to make this root
               case CanvasEventTypes.MOUSE_CLICK:
               case CanvasEventTypes.MOUSE_DBLCLICK:
                  node.makeRootHead();
               break;

               default:
            }
            node.val.window.handleCanvasMouseEvent(event, n_x, n_y, button);
            break;
         }
      }
      return true;
   }
   handleCanvasWheelEvent(event: CanvasEventTypes, deltaX: number, deltaY: number, deltaZ: number, deltaMode: number) {
      if (this.clickedWindow)
         return true;
      for (let node = this.windowOrder.getRoot(); node != undefined; node = node.getNext()) {
         const n_x = this.mouseX - node.val.x;
         const n_y = this.mouseY - node.val.y;
         const width = node.val.window.element.width;
         const height = node.val.window.element.height;
         if (0 <= n_x && n_y <= width && 0 <= n_y && n_y <= height) {
            node.val.window.handleCanvasWheelEvent(event, deltaX, deltaY, deltaZ, deltaMode);
            break;
         }
      }
      return true;
   }
   handleCanvasAnimationFrameEvent(event: CanvasEventTypes, delta: number) {
      for (const [, windowInfo] of this.windows) {
         if (!windowInfo.hidden) {
            windowInfo.window.handleCanvasAnimationFrameEvent(event, delta);
         }
      }
      return true;
   }


}