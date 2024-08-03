#include "twr-io.h"
#include <stddef.h>  /* NULL*/

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

static void IoNullPutc(struct IoConsole* io, unsigned char c)
{
	UNUSED(io);
	UNUSED(c);
	
	return;
}


static char IoNullInkey(struct IoConsole* io)
{
	UNUSED(io);
	return 0;
}


static int IoNullGetc(struct IoConsole* io)
{
	UNUSED(io);
	return 0;
}


static int IoNullChkBrk(struct IoConsole* io)
{
	UNUSED(io);
	return 0;  // no break
}

struct IoConsole* io_nullcon()
{
	static struct IoConsole ionull;

	ionull.charin.io_inkey	= IoNullInkey;
	ionull.charin.io_getc32	= IoNullGetc;
	ionull.charout.io_putc	= IoNullPutc;
	ionull.header.io_chk_brk= IoNullChkBrk;
	ionull.header.io_close	= NULL;		// null okay, just won't call any close function
	ionull.header.io_get_prop = NULL;
	ionull.header.type=0;

	return &ionull;
}
