import {parseModOptions, IModOpts} from "./twrmodutil.js"
import {IConsole, logToCon} from "./twrcon.js"
import {twrLibraryInstanceRegistry} from "./twrlibrary.js";
import {IWasmMemory} from './twrwasmmem.js'
import {twrWasmCall} from "./twrwasmcall.js"
import {twrWasmBase, TOnEventCallback, getNextModuleID} from "./twrwasmbase.js"
import {twrEventQueueReceive} from "./twreventqueue.js"
import {twrLibBuiltIns} from "./twrlibbuiltin.js"

/*********************************************************************/

// New code should use wasmMem, not deprecated memory access functions at module level
export interface IWasmModule {

// deprecated memory functions
   memory:WebAssembly.Memory;
   mem8:Uint8Array;
   mem32:Uint32Array;
   memD:Float64Array;
   stringToU8(sin:string, codePage?:number):Uint8Array;
   copyString(buffer:number, buffer_size:number, sin:string, codePage?:number):void;
   getLong(idx:number): number;
   setLong(idx:number, value:number):void;
   getDouble(idx:number): number;
   setDouble(idx:number, value:number):void;
   getShort(idx:number): number;
   getString(strIndex:number, len?:number, codePage?:number): string;
   getU8Arr(idx:number): Uint8Array;
   getU32Arr(idx:number): Uint32Array;

   malloc:(size:number)=>number;
   free:(size:number)=>void;   // I added this one, but its still deprecated.
   putString(sin:string, codePage?:number):number;
   putU8(u8a:Uint8Array):number;
   putArrayBuffer(ab:ArrayBuffer):number;

// non deprecated functions
   wasmMem: IWasmMemory;
   wasmCall: twrWasmCall;
   callC:twrWasmCall["callC"];
   isTwrWasmModuleAsync:false;   // to avoid circular references -- check if twrWasmModule without importing twrWasmModule
   //TODO!! put these into IWasmModuleBase (some could be implemented in twrWasmModuleBase, but many have different implementations)
   loadWasm: (pathToLoad:string)=>Promise<void>;
   postEvent: TOnEventCallback;
   fetchAndPutURL: (fnin:URL)=>Promise<[number, number]>;
   divLog:(...params: string[])=>void;
   log:(...params: string[])=>void;
   readonly id: number;
}


/*********************************************************************/

export class twrWasmModule extends twrWasmBase implements IWasmModule {
   io:{[key:string]: IConsole};
   ioNamesToID: {[key: string]: number};
   isTwrWasmModuleAsync:false=false;

   // divLog is deprecated.  Use IConsole.putStr or log
   divLog:(...params: string[])=>void;
   log:(...params: string[])=>void;

   // IWasmMemory
   // These are deprecated, use wasmMem instead.
   memory!:WebAssembly.Memory;
   mem8!:Uint8Array;
   mem32!:Uint32Array;
   memD!:Float64Array;
   stringToU8!:(sin:string, codePage?:number)=>Uint8Array;
   copyString!:(buffer:number, buffer_size:number, sin:string, codePage?:number)=>void;
   getLong!:(idx:number)=>number;
   setLong!:(idx:number, value:number)=>void;
   getDouble!:(idx:number)=>number;
   setDouble!:(idx:number, value:number)=>void;
   getShort!:(idx:number)=>number;
   getString!:(strIndex:number, len?:number, codePage?:number)=>string;
   getU8Arr!:(idx:number)=>Uint8Array;
   getU32Arr!:(idx:number)=>Uint32Array;

   malloc!:(size:number)=>number;
   free!:(size:number)=>void;
   putString!:(sin:string, codePage?:number)=>number;
   putU8!:(u8a:Uint8Array)=>number;
   putArrayBuffer!:(ab:ArrayBuffer)=>number;

   readonly id: number;
   /*********************************************************************/

   constructor(opts:IModOpts={}) {
      super();
      [this.io, this.ioNamesToID] = parseModOptions(opts);
      this.log=logToCon.bind(undefined, this.io.stdio);
      this.divLog=this.log;

      this.id = getNextModuleID();
   }

   /*********************************************************************/

   async loadWasm(pathToLoad:string) {

      // load builtin libraries
      await twrLibBuiltIns();

      let imports:WebAssembly.ModuleImports={};
      for (let i=0; i<twrLibraryInstanceRegistry.libInterfaceInstances.length; i++) {
         const lib=twrLibraryInstanceRegistry.libInterfaceInstances[i];
         imports={...imports, ...lib.getImports(this)};
      }

      await super.loadWasm(pathToLoad, imports);

      if (!(this.wasmMem.memory.buffer instanceof ArrayBuffer))
         console.log("twrWasmModule does not require shared Memory. Okay to remove wasm-ld --shared-memory --no-check-features");

      // backwards compatible
      this.memory = this.wasmMem.memory;
      this.mem8 = this.wasmMem.mem8u;
      this.mem32 = this.wasmMem.mem32u;
      this.memD = this.wasmMem.memD;
      this.malloc = this.wasmMem.malloc.bind(this.wasmMem);
      this.free = this.wasmMem.free.bind(this.wasmMem);
      this.stringToU8=this.wasmMem.stringToU8.bind(this.wasmMem);
      this.copyString=this.wasmMem.copyString.bind(this.wasmMem);
      this.getLong=this.wasmMem.getLong.bind(this.wasmMem);
      this.setLong=this.wasmMem.setLong.bind(this.wasmMem);
      this.getDouble=this.wasmMem.getDouble.bind(this.wasmMem);
      this.setDouble=this.wasmMem.setDouble.bind(this.wasmMem);
      this.getShort=this.wasmMem.getShort.bind(this.wasmMem);
      this.getString=this.wasmMem.getString.bind(this.wasmMem);
      this.getU8Arr=this.wasmMem.getU8Arr.bind(this.wasmMem);
      this.getU32Arr=this.wasmMem.getU32Arr.bind(this.wasmMem);
   
      this.putString=this.wasmMem.putString.bind(this.wasmMem);
      this.putU8=this.wasmMem.putU8.bind(this.wasmMem);
      this.putArrayBuffer=this.wasmMem.putArrayBuffer.bind(this.wasmMem);

      // init C runtime
      const init=this.exports.twr_wasm_init as Function;
      init(this.ioNamesToID.stdio, this.ioNamesToID.stderr, this.ioNamesToID.std2d==undefined?-1:this.ioNamesToID.std2d, this.wasmMem.mem8u.length);
   }
   
   /*********************************************************************/

   // given a url, load its contents, and stuff into Wasm memory similar to Unint8Array
   // TODO!! Doc that this is no longer a CallC option, and must be called here manually
   async fetchAndPutURL(fnin:URL):Promise<[number, number]> {

      if (!(typeof fnin === 'object' && fnin instanceof URL))
         throw new Error("fetchAndPutURL param must be URL");

      try {
         let response=await fetch(fnin);
         let buffer = await response.arrayBuffer();
         let src = new Uint8Array(buffer);
         let dest=this.wasmMem.putU8(src);
         return [dest, src.length];
         
      } catch(err:any) {
         console.log('fetchAndPutURL Error. URL: '+fnin+'\n' + err + (err.stack ? "\n" + err.stack : ''));
         throw err;
      }
   }

   postEvent(eventID:number, ...params:number[]) {
      //TODO!! PostEvent into eventQueueSend, then processEvents -- to enable non callback events when i add them
      if (!(eventID in twrEventQueueReceive.onEventCallbacks))
         throw new Error("twrWasmModule.postEvent called with invalid eventID: "+eventID+", params: "+params);

      const onEventCallback=twrEventQueueReceive.onEventCallbacks[eventID];
      if (onEventCallback) 
         onEventCallback(eventID, ...params);
      else
         throw new Error("twrWasmModule.postEvent called with undefined callback.  eventID: "+eventID+", params: "+params);
   }

   peekEvent(eventName:string) {
      // get earliest inserted entry in event Map
      //const ev=this.events.get(eventName)
   }
  // called (RPC) by twrLibraryProxy
  // waitEvent(eventName:string) {
  //    const evIdx=peekTwrEvent(eventName);
  //    if (evIdx) {
  //       this.returnValue!.write(evIdx);
  //    }
  //    else {
  //       this.addEventListner(eventName, (evIdx:number)=> {
  //          this.returnValue!.write(evIdx);
  //       });
  //    }
}




