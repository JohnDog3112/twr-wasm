#include <assert.h>
#include "twr-crt.h"


/*
	__heap_base: This variable points to the start of the heap memory region.
	__data_end: This variable points to the end of the data memory region.
	__stack_pointer: This variable points to the top of the stack memory region.
	__global_base: This variable points to the start of the global variable region.
	__table_base: This variable points to the start of the table region.
	__memory_base: This variable points to the start of the memory region.
*/

extern unsigned char __heap_base;
//extern unsigned char __stack_pointer;
extern unsigned char __global_base;
extern unsigned char __table_base;
extern unsigned char __memory_base;
extern unsigned char __data_end;

/* pf 0 - printf goes to web browser debug console */
/* pf 1 - printf goes to web browser DIV */
/* pf 2 - printf goes to web browser Canvas */
/* pf 3 - printf goes to null console (default if this call not made) */
/* width, height only used when pf is windowcon (Canvas) */

void __wasm_call_ctors();

__attribute__((export_name("twr_wasm_init")))
void twr_wasm_init(int pf, unsigned long mem_size) {

// init stderr
	twr_set_stderr_con(twr_debugcon());

//
// init heap
//
	size_t base=(size_t)&__heap_base;
	assert((base&7)==0);  // seems to always be the case

	twr_init_malloc((uint64_t*)base, mem_size-base);
	//malloc_unit_test();   usually needs to be called by test.c -- stack size usually needs increasing, and also can't be called twice (due to cache malloc tests)

//
// set stdio 
//
	struct IoConsole* con;

	switch (pf) {
		case 0:
			con=twr_debugcon();
			break;

		case 1:
			con=twr_divcon();
			break;

		case 2:
			con=twr_windowcon();
			break;

		case 3:
			con=io_nullcon();
			break;

		default:
			con=twr_debugcon();
	}

	twr_set_stdio_con(con);

//
// init global constructors
//

	// __wasm_call_ctors() is generated by wasm-ld as needed
	// note there is also a __wasm_call_dtors, which i don't currently call
	// if i don't explicitly call __wasm_call_ctors(), the linker appears to generate code to call it prior to every function call.
	// that's what my tests showed, and perhaps this post is relevant:
	// https://reviews.llvm.org/D81689
	// that doesn't seem like what I want to happen.
  	__wasm_call_ctors();   // call all the global static constructors.  Higher -O levels will/may use this to init static data to NULL.

}

__attribute__((export_name("twr_wasm_print_mem_debug_stats")))
void twr_wasm_print_mem_debug_stats(void) {
	twr_mem_debug_stats(twr_get_stderr_con());
}

void twr_mem_debug_stats(struct IoConsole* outcon) {
	io_printf(outcon, "wasm module memory map:\n");
	io_printf(outcon, "   __memory_base: 0x%x\n", &__memory_base);
	io_printf(outcon, "   __table_base: 0x%x\n", &__table_base);
	io_printf(outcon, "   __global_base: 0x%x\n", &__global_base);
	io_printf(outcon, "   __data_end: 0x%x\n", &__data_end);
	//io_printf(outcon, "   top of stack: %x", &__stack_pointer);
	io_printf(outcon, "   __heap_base: 0x%x\n", &__heap_base);
	const size_t stack_size = &__heap_base-&__data_end;
	io_printf(outcon, "   code+global size: %d\n", &__heap_base-stack_size);
	io_printf(outcon, "   stack size: %d\n", stack_size);

	twr_malloc_debug_stats(outcon);
}