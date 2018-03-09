#ifndef __CHAR_SET_H__
#define __CHAR_SET_H__

#define get_set_ij(_c, _i, _j) \
	do { \
        int cc = (_c) & 0xFF; \
		_i = cc >> 5; \
		_j = 1 << (cc & 0x1f); \
	} while (0)

#define is_in_set(_i, _j, _set)	((_set[_i] & _j) != 0)

/* 0x20, 0x9, 0xd, 0xa */
extern long chars_white_space[8];

/* _, A..Z, a..z */
extern long chars_tag_start[8];

/* . - : 0..9 - A..Z a.. z */
extern long chars_tag[8];

extern long chars_text_start[8];

extern long chars_text[8];
/* 0x20, 0x9, 0xd, 0xa */

#endif

