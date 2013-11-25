#ifndef __GMI_GTP_LIST_H__
#define __GMI_GTP_LIST_H__

#include <gmi_type_definitions.h>
#include <gmi_errors.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct tagGtpListNode
{
    struct tagGtpListNode * s_Next;
    struct tagGtpListNode * s_Prev;
} GtpListNode, GtpList;

#ifndef GTP_OFFSET_OF
#define GTP_OFFSET_OF(type, member)    ((size_t)&((type *)0)->member)
#endif
#ifndef GTP_CONTAINER_OF
#define GTP_CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - GTP_OFFSET_OF(type, member)))
#endif

#define GTP_LIST_INIT(list)            GtpListInit(list)
#define GTP_LIST_DESTROY(list)         GtpListDestroy(list)
#define GTP_LIST_ADD(list, item)       GtpListAdd(list, &(item)->s_ListNode)
#define GTP_LIST_REMOVE(list, item)    GtpListRemove(list, &(item)->s_ListNode)
#define GTP_LIST_CLEAR                 GTP_LIST_DESTROY
#define GTP_LIST_IS_EMPTY(list)        ((list)->s_Next == (list))
#define GTP_LIST_FIRST(list)           ((list)->s_Next)

#define GTP_LIST_FOR_EACH(pos, list) \
    for (pos = (list)->s_Next; pos != (list); pos = pos->s_Next)

#define GTP_LIST_ENTRY(ptr, type) \
    GTP_CONTAINER_OF(ptr, type, s_ListNode)

void_t GtpListInit(GtpList * List);

void_t GtpListDestroy(GtpList * List);

void_t GtpListAdd(GtpList * List, GtpListNode * Node);

GtpListNode * GtpListRemove(GtpList * List, GtpListNode * Node);

#ifdef __cplusplus
}
#endif

#endif // __GMI_GTP_LIST_H__
