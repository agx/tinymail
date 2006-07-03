#ifndef TNY_LIST_PRIV_H
#define TNY_LIST_PRIV_H

typedef struct _TnyListPriv TnyListPriv;

struct _TnyListPriv
{
	GMutex *iterator_lock;
	GList *first;
};

#define TNY_LIST_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_LIST, TnyListPriv))

#endif
