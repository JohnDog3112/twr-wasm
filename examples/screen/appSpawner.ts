import {IWasmModule, IWasmModuleAsync, twrLibrary, keyEventToCodePoint, TLibImports, twrLibraryInstanceRegistry, twrConsoleScreen, twrWasmModuleAsync, twrWasmModule, IConsole} from "twr-wasm"

// Libraries use default export
export default class appSpawner extends twrLibrary {
   id: number;

   imports:TLibImports = {
      spawnApplication: {},
   };

   // every library should have this line
   libSourcePath = new URL(import.meta.url).pathname;

   isAsync: boolean;
   screen: twrConsoleScreen;
   spawnedApps: Map<number, twrWasmModule | twrWasmModuleAsync> = new Map();
   nextID: number = 0; 
   constructor(isAsync: boolean, screen: twrConsoleScreen) {
      super();
      this.id=twrLibraryInstanceRegistry.register(this);

      this.isAsync = isAsync;
      this.screen = screen;
   }

   spawnApplication(callingMod: IWasmModule|IWasmModuleAsync, titlePtr: number, pathPtr: number, initPtr: number, initArgsPtr: number, initArgsLen: number, bindWindow: boolean) {
      const title = callingMod.getString(titlePtr);
      const path = callingMod.getString(pathPtr);
      const initFunc = callingMod.getString(initPtr);

      let initArgs: number[] = [];
      if (initArgsPtr != 0 && initArgsLen > 0) {
         const idx32=Math.floor(initArgsPtr/4);
         if (idx32*4!=initArgsPtr) throw new Error("initArgsPtr isn't a long aligned address")
         if (idx32<0 || idx32 >= callingMod.wasmMem.mem32u.length) throw new Error("invalid initArgsPtr index: "+initArgsPtr+", this.mem32.length: "+callingMod.wasmMem.mem32u.length);
         if (initArgsLen+idx32 >= callingMod.wasmMem.mem32u.length) throw new Error(`invalid argsPtr length: ${initArgsPtr}+${initArgsLen}=${initArgsPtr+initArgsLen} >= memory size ${callingMod.wasmMem.mem32u.length}`);

         initArgs = [...callingMod.wasmMem.mem32u.slice(idx32, idx32+initArgsLen)];
      }
      
      const id = this.nextID;
      this.nextID++;

      const modOpts: {[key: string]: IConsole} = {
         screen: this.screen
      };

      if (bindWindow) {
         const windowConsole = this.screen.jsSpawnWindow(title);
         const d2dCanvas = windowConsole.jsGetDrawCanvas();

         modOpts.window = windowConsole;
         modOpts.d2dcanvas = d2dCanvas;
      }
      

      const modCon = this.isAsync ? twrWasmModuleAsync : twrWasmModule;

      const mod = new modCon({io: modOpts});
      this.spawnedApps.set(id, mod);

      mod.loadWasm(path).then(() => {
         mod.callC([initFunc, ...initArgs]);
      });

      return id;
   }


}


