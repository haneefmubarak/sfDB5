#pragma once

//===	Includes

#include <stdint.h>
#include <stdlib.h>

//===	Defines

#define xm_min(a,b)	({	\
		typeof (a) _a = (a);	\
		typeof (b) _b = (b);	\
		_a < _b ? _a : _b;	\
	})
#define xm_max(a,b)	({	\
		typeof (a) _a = (a);	\
		typeof (b) _b = (b);	\
		_a > _b ? _a : _b;	\
	})

#define xm_strndupa(s, l)	({	\
		uint8_t *_s = (uint8_t *) (s);	\
		size_t _l = strnlen (_s, l);	\
		uint8_t *_r = alloca (_l + 1);	\
		strncpy (_r, _s, _l);	\
		_r[_l + 1] = 0;	\
		_r;	\
	})

#define xmattr_constant	__attribute__((const))
#define xmattr_malloc	__attribute__((malloc))
#define xmattr_pure	__attribute__((pure))

#define xm_likely(x)	__builtin_expect(!!(x), 1)
#define xm_unlikely(x)	__builtin_expect(!!(x), 0)

#define xm_tlv	__thread
