/* Framebuffer_stubs.ml - stubs for the ocaml framebuffer interface
 * Copyright (C) 2013 Goswin von Brederlow <goswin-v-b@web.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --
 *
 * Glue code to access the Raspberry Pi framebuffer from ocaml
 */

#include <stdint.h>
#include <caml/mlvalues.h>
#include <caml/memory.h>
#include "printf.h"

/*
 * Data memory barrier
 * No memory access after the DMB can run until all memory accesses
 * before it have completed.
 */
static inline void data_memory_barrier(void) {
    asm volatile("mcr p15, #0, %[zero], c7, c10, #5"
		 : : [zero]"r"(0));
}

/*
 * Clean and invalidate entire cache
 * Flush pending writes to main memory
 * Remove all data in data cache
 */
static inline void flush_cache(void) {
    asm volatile("mcr p15, #0, %[zero], c7, c14, #0"
		 : : [zero]"r"(0));
}

enum {
    // Mailbox registers
    MAILBOX0READ   = 0xE000b880,
    MAILBOX0STATUS = 0xE000b898,
    MAILBOX0WRITE  = 0xE000b8a0,
};

// Mailbox status
volatile uint32_t *fb_mailbox0_status = (uint32_t*)MAILBOX0STATUS;

enum StatusFlags {
    MAIL_EMPTY = 1 << 30,
    MAIL_FULL = 1 << 31
};

int mail_empty(void) {
    return *fb_mailbox0_status & MAIL_EMPTY;
}

int mail_full(void) {
    return *fb_mailbox0_status & MAIL_FULL;
}

// Envelope
typedef struct Envelope {
    uint32_t raw;
} Envelope;
    
enum {
    ENVELOPE_SHIFT_CHANNEL = 0, ENVELOPE_SHIFT_ADDR = 4,
    ENVELOPE_MASK_CHANNEL = 0xF, ENVELOPE_MASK_ADDR = 0xFFFFFFF0
};

Envelope envelope(uint32_t chan, volatile uint32_t *address) {
    return (Envelope){ (intptr_t)address | chan };
}

Envelope envelope_raw(uint32_t raw) {
    return (Envelope){ raw };
}

uint32_t envelope_channel(const Envelope e) {
    return (e.raw & ENVELOPE_MASK_CHANNEL) >> ENVELOPE_SHIFT_CHANNEL;
}

volatile uint32_t * envelope_addr(const Envelope e) {
    return (volatile uint32_t *)(e.raw & ENVELOPE_MASK_ADDR);
}

// Mailbox read
volatile uint32_t *fb_mailbox0_read = (volatile uint32_t *)MAILBOX0READ;

volatile uint32_t* mail_read(uint32_t chan) {
    // possibly switching peripheral
    data_memory_barrier();
    while(1) {
	// don't use cached values
	flush_cache();
	// wait for mailbox to contain something
	while(mail_empty()) {
	    flush_cache();
	}
	Envelope e = envelope_raw(*fb_mailbox0_read);
	if (envelope_channel(e) == chan) return envelope_addr(e);
    }
}	

// Mailbox write
volatile uint32_t *fb_mailbox0_write = (volatile uint32_t *)MAILBOX0WRITE;

void mail_write(const Envelope e) {
    // possibly switching peripheral
    data_memory_barrier();
    // don't use cached values
    flush_cache();
    // wait for mailbox to be not full
    while(mail_full()) {
	flush_cache();
    }
    *fb_mailbox0_write = e.raw;
}

// Framebuffer
enum {
    // Channel for framebuffer
    FBCHAN = 8,
};

/* Use some free memory in the area below the kernel/stack */
#define BUFFER_ADDRESS 0x1000

enum Error {
    SUCCESS,
    // Error codes
    FAIL_GET_RESOLUTION,
    FAIL_GOT_INVALID_RESOLUTION,
    FAIL_SETUP_FRAMEBUFFER,
    FAIL_INVALID_TAGS,
    FAIL_INVALID_TAG_RESPONSE,
    FAIL_INVALID_TAG_DATA,
    FAIL_INVALID_PITCH_RESPONSE,
    FAIL_INVALID_PITCH_DATA
};

typedef struct Pixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
} Pixel;
    
typedef struct FB {
    uint32_t base;
    uint32_t size; // in bytes
    uint32_t pitch; // in bytes
    uint32_t width;
    uint32_t height;
} FB;

FB fb;

CAMLprim value ocaml_rpi__fb_init(value unit) {
    CAMLparam1(unit);
    printf("%s()\n", __FUNCTION__);

    volatile uint32_t *mailbuffer = (volatile uint32_t*)BUFFER_ADDRESS;
    Envelope e = envelope(FBCHAN, mailbuffer);
	
    /* Get the display size */
    mailbuffer[0] = 8 * 4; // Total size
    mailbuffer[1] = 0; // Request
    mailbuffer[2] = 0x40003; // Display size
    mailbuffer[3] = 8; // Buffer size
    mailbuffer[4] = 0; // Request size
    mailbuffer[5] = 0; // Space for horizontal resolution
    mailbuffer[6] = 0; // Space for vertical resolution
    mailbuffer[7] = 0; // End tag

    mail_write(e);
    mail_read(FBCHAN);

    /* Valid response in data structure */
    if (mailbuffer[1] != 0x80000000) CAMLreturn(Int_val(FAIL_GET_RESOLUTION));

    fb.width = mailbuffer[5];
    fb.height = mailbuffer[6];

    if (fb.width == 0 || fb.height == 0) CAMLreturn(Int_val(FAIL_GOT_INVALID_RESOLUTION));

    /* Set up screen */

    unsigned int c = 1;
    mailbuffer[c++] = 0; // Request

    mailbuffer[c++] = 0x00048003; // Tag id (set physical size)
    mailbuffer[c++] = 8; // Value buffer size (bytes)
    mailbuffer[c++] = 8; // Req. + value length (bytes)
    mailbuffer[c++] = fb.width; // Horizontal resolution
    mailbuffer[c++] = fb.height; // Vertical resolution

    mailbuffer[c++] = 0x00048004; // Tag id (set virtual size)
    mailbuffer[c++] = 8; // Value buffer size (bytes)
    mailbuffer[c++] = 8; // Req. + value length (bytes)
    mailbuffer[c++] = fb.width; // Horizontal resolution
    mailbuffer[c++] = fb.height; // Vertical resolution

    mailbuffer[c++] = 0x00048005; // Tag id (set depth)
    mailbuffer[c++] = 4; // Value buffer size (bytes)
    mailbuffer[c++] = 4; // Req. + value length (bytes)
    mailbuffer[c++] = 32; // 32 bpp

    mailbuffer[c++] = 0x00040001; // Tag id (allocate framebuffer)
    mailbuffer[c++] = 8; // Value buffer size (bytes)
    mailbuffer[c++] = 4; // Req. + value length (bytes)
    mailbuffer[c++] = 16; // Alignment = 16
    mailbuffer[c++] = 0; // Space for response

    mailbuffer[c++] = 0; // Terminating tag

    mailbuffer[0] = c*4; // Buffer size

    mail_write(e);
    mail_read(FBCHAN);

    /* Valid response in data structure */
    if(mailbuffer[1] != 0x80000000) CAMLreturn(Int_val(FAIL_SETUP_FRAMEBUFFER));

    // Scan replies for allocate response
    unsigned int i = 2; /* First tag */
    uint32_t data;
    while ((data = mailbuffer[i])) {
	// allocate response?
	if (data == 0x40001) break;

	/* Skip to next tag
	 * Advance count by 1 (tag) + 2 (buffer size/value size)
	 * + specified buffer size
	 */
	i += 3 + (mailbuffer[i + 1] >> 2);

	if (i > c) CAMLreturn(Int_val(FAIL_INVALID_TAGS));
    }

    /* 8 bytes, plus MSB set to indicate a response */
    if (mailbuffer[i + 2] != 0x80000008) CAMLreturn(Int_val(FAIL_INVALID_TAG_RESPONSE));

    /* Framebuffer address/size in response */
    fb.base = mailbuffer[i + 3];
    fb.size = mailbuffer[i + 4];

    if (fb.base == 0 || fb.size == 0) CAMLreturn(Int_val(FAIL_INVALID_TAG_DATA));

    fb.base += 0xC0000000; // physical to virtual
	
    /* Get the framebuffer pitch (bytes per line) */
    mailbuffer[0] = 7 * 4; // Total size
    mailbuffer[1] = 0; // Request
    mailbuffer[2] = 0x40008; // Display size
    mailbuffer[3] = 4; // Buffer size
    mailbuffer[4] = 0; // Request size
    mailbuffer[5] = 0; // Space for pitch
    mailbuffer[6] = 0; // End tag

    mail_write(e);
    mail_read(FBCHAN);

    /* 4 bytes, plus MSB set to indicate a response */
    if (mailbuffer[4] != 0x80000004) CAMLreturn(Int_val(FAIL_INVALID_PITCH_RESPONSE));

    fb.pitch = mailbuffer[5];
    if (fb.pitch == 0) CAMLreturn(Int_val(FAIL_INVALID_PITCH_DATA));

    // set alpha to 0xff everywhere
    for(uint32_t n = 3; n < fb.size; n += 4) {
	*(uint8_t*)(fb.base + n) = 0xff;
    }

    // draw chessboard pattern
    for(uint32_t y = 0; y < fb.height; ++y) {
	for(uint32_t x = 0; x < fb.width; ++x) {
	    Pixel *p = (Pixel*)(fb.base + x * sizeof(Pixel) + y * fb.pitch);
	    uint8_t col = ((x & 16) ^ (y & 16)) ? 0x00 : 0xff;
	    p->red = col;
	    p->green = col;
	    p->blue = col;
	}
    }

    // draw back->red fade left to right at the top
    // draw back->blue fade left to right at the bottom
    for(int y = 0; y < 16; ++y) {
	for(int x = 16; x < 256 + 16; ++x) {
	    Pixel *p = (Pixel*)(fb.base + x * sizeof(Pixel) + y * fb.pitch);
	    p->red = x - 16;
	    p->green = 0;
	    p->blue = 0;
	    p = (Pixel*)(fb.base + x * sizeof(Pixel) + (fb.height - y - 1) * fb.pitch);
	    p->red = 0;
	    p->green = 0;
	    p->blue = x - 16;
	}
    }
    // draw back->green fade top to bottom at the left
    // draw back->green fade top to bottom at the right
    for(int y = 16; y < 256 + 16; ++y) {
	for(int x = 0; x < 16; ++x) {
	    Pixel *p = (Pixel*)(fb.base + x * sizeof(Pixel) + y * fb.pitch);
	    p->red = 0;
	    p->green = y - 16;
	    p->blue = 0;
	    p = (Pixel*)(fb.base + (fb.width - x- 1) * sizeof(Pixel) + y * fb.pitch);
	    p->red = y - 16;
	    p->green = y - 16;
	    p->blue = y - 16;		
	}
    }
/* test font
    const char text[] = "MOOSE V1.0";
    struct Arg {
	uint32_t color;
	uint32_t border;
	bool fill;
    } args[] = {
	{~0LU,  0, false},
	{ 0, ~0LU, false},
	{~0LU,  0, true},
	{ 0, ~0LU, true},
	{0xff0000ff,  0,   false},
	{0xff0000ff, ~0LU, false},
	{0xff0000ff,  0,   true},
	{0xff0000ff, ~0LU, true},
	{0xff00ff00,  0,   false},
	{0xff00ff00, ~0LU, false},
	{0xff00ff00,  0,   true},
	{0xff00ff00, ~0LU, true},
	{0xffff0000,  0,   false},
	{0xffff0000, ~0LU, false},
	{0xffff0000,  0,   true},
	{0xffff0000, ~0LU, true},
    };
    int y = 152;
    Arg *arg = args;
    for (i = 0; i < 16; ++i) {
	int x = 152;
	for (const char *p = text; *p; ++p) {
	    Font::putc(fb, x, y, *p, arg->color, arg->border, arg->fill);
	    x += 8;
	}
	y += 16;
	++arg;}
*/
    CAMLreturn(Int_val(SUCCESS));
}
