#include "gtp_list.h"

#include "debug.h"

void_t GtpListInit(GtpList * List)
{
    ASSERT(List != NULL, "List MUST NOT be non-pointer");
    List->s_Next = List;
    List->s_Prev = List;
}

void_t GtpListDestroy(GtpList * List)
{
    GtpListNode * p = NULL;

    ASSERT(List != NULL, "List MUST NOT be non-pointer");

    p = List->s_Next;
    while (p != List)
    {
        GtpListNode * q = p->s_Next;
        p->s_Next = NULL;
        p->s_Prev = NULL;
        p = q;
    }

    List->s_Next = List;
    List->s_Prev = List;
}

void_t GtpListAdd(GtpList * List, GtpListNode * Node)
{
    ASSERT(List != NULL, "List MUST NOT be non-pointer");
    ASSERT(Node != NULL, "Node MUST NOT be non-pointer");

    ASSERT(NULL == Node->s_Next, "Node MUST NOT in any list");
    ASSERT(NULL == Node->s_Prev, "Node MUST NOT in any list");

    // Add node at last of the list
    Node->s_Next = List;
    Node->s_Prev = List->s_Prev;
    List->s_Prev->s_Next = Node;
    List->s_Prev = Node;
    
}

GtpListNode * GtpListRemove(GtpList * List, GtpListNode * Node)
{
    GtpListNode * PrevNode = NULL;

    ASSERT(List != NULL, "List MUST NOT be non-pointer");
    ASSERT(Node != NULL, "Node MUST NOT be non-pointer");
    ASSERT(Node != List, "NOT permit to remove head node");

    if (NULL == Node->s_Next || NULL == Node->s_Prev)
    {
        // PRINT_LOG(VERBOSE, "Node is not in any list");
        return NULL;
    }

    PrevNode = Node->s_Prev;
    Node->s_Next->s_Prev = PrevNode;
    PrevNode->s_Next = Node->s_Next;

    Node->s_Next = NULL;
    Node->s_Prev = NULL;

    return PrevNode;
}