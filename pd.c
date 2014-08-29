#include <pd.h>

xm_tlv int32_t	pd_arena;
void		**pd_arenaLUT = NULL;

xmattr_malloc void *pd_emalloc	(size_t size, int32_t pd_arena) {
	
}


xmattr_malloc void *pd_ecalloc	(size_t count, size_t size, int32_t pd_arena);
void *pd_erealloc		(void *ptr, size_t size, int32_t pd_arena);
void pd_efree			(void *ptr, int32_t pd_arena);
