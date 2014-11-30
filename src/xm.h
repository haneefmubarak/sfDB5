#pragma once

//===	Defines

#define xm_min(a,b) ({	\
		typeof (a) _a = (a);	\
		typeof (b) _b = (b);	\
		_a < _b ? _a : _b;	\
	})
#define xm_max(a,b) ({	\
		typeof (a) _a = (a);	\
		typeof (b) _b = (b);	\
		_a > _b ? _a : _b;	\
	})

#define xmattr_constant	__attribute__((const))
#define xmattr_malloc	__attribute__((malloc))
#define xmattr_pure	__attribute__((pure))

#define xm_likely(x)	__builtin_expect(!!(x), 1)
#define xm_unlikely(x)	__builtin_expect(!!(x), 0)

#define xm_tlv	__thread
