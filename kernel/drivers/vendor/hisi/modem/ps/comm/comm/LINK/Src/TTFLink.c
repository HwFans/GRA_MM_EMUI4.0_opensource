

#ifdef  __cplusplus
  #if  __cplusplus
  extern "C"{
  #endif
#endif


/******************************************************************************
   1 头文件包含
******************************************************************************/
#include "pslog.h"
#include "TTFLink.h"
/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
/*lint -e(760)*/
/*lint -e767*/
#define    THIS_FILE_ID        PS_FILE_ID_TTF_LINK_C
/*lint +e767*/


/******************************************************************************
   2 外部函数变量声明
******************************************************************************/


/******************************************************************************
   3 私有定义
******************************************************************************/


/******************************************************************************
   4 全局变量定义
******************************************************************************/
VOS_SPINLOCK             g_stTtfLinkSpinLock;


/******************************************************************************
   5 函数实现
******************************************************************************/
/*lint -save -e958 */

VOS_VOID TTF_NodeInit(TTF_NODE_ST *pNode)
{
    pNode->pPrev    = VOS_NULL_PTR;
    pNode->pNext    = VOS_NULL_PTR;
} /* TTF_NodeInit */

VOS_VOID TTF_LinkInit(VOS_UINT32 ulPid, TTF_LINK_ST *pLink)
{
    if (VOS_NULL_PTR == pLink)
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink is Null!");
        return;
    }

    pLink->stHead.pNext = &pLink->stHead;
    pLink->stHead.pPrev = &pLink->stHead;
    pLink->ulCnt        = 0;

} /* TTF_LinkInit */

VOS_UINT32 TTF_LinkCheckNodeInLink(VOS_UINT32 ulPid, TTF_LINK_ST *pLink, TTF_NODE_ST *pCurr)
{
    TTF_NODE_ST     *pNode;

    if ( (VOS_NULL_PTR == pLink) || (VOS_NULL_PTR == pCurr ))
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink Or pCurr is Null!");
        return PS_FAIL;
    }

    pNode = pLink->stHead.pNext;
    while (&(pLink->stHead) != pNode)
    {
        if (pCurr == pNode)
        {
            return PS_TRUE;
        }

        pNode = pNode->pNext;
    }

    /*遍历链表，未查找到对应的结点*/
    return PS_FALSE;
}

#ifndef _lint
VOS_UINT32 TTF_LinkInsertHead(VOS_UINT32 ulPid, TTF_LINK_ST * pLink, TTF_NODE_ST *pInsert)
{

    if ( (VOS_NULL_PTR == pLink) || (VOS_NULL_PTR == pInsert ))
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink Or pInsert is Null!");
        return PS_FAIL;
    }

#ifdef WTTF_PC_ST_SWITCH
    /*检查节点pInsert是否在链表pLink中*/
    if ( PS_TRUE == TTF_LinkCheckNodeInLink(ulPid, pLink, pInsert) )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pInsert has Existed in pLink!");
        return PS_FAIL;
    }
#endif

    pInsert->pPrev              = (TTF_NODE_ST *)(&pLink->stHead);
    pLink->stHead.pNext->pPrev  = pInsert;
    pInsert->pNext              = pLink->stHead.pNext;
    pLink->stHead.pNext         = pInsert;

    pLink->ulCnt++;

    return PS_SUCC;
} /*TTF_LinkInsertHead*/
#endif


#ifndef _lint

VOS_UINT32 TTF_LinkInsertNext(VOS_UINT32 ulPid, TTF_LINK_ST * pLink,
                TTF_NODE_ST *pCurr, TTF_NODE_ST *pInsert)
{

    if ( (VOS_NULL_PTR == pLink) || (VOS_NULL_PTR == pCurr) || (VOS_NULL_PTR == pInsert))
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink、pCurr Or pInsert is Null!");
        return PS_FAIL;
    }

    /*遍历该节点是否在链表内，若无，返回失败*/





#ifdef WTTF_PC_ST_SWITCH
    /*检查节点pInsert是否在链表pLink中*/
    if ( PS_TRUE == TTF_LinkCheckNodeInLink(ulPid, pLink, pInsert) )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pInsert has Existed in pLink!");
        return PS_FAIL;
    }
#endif

    pInsert->pPrev              = pCurr;
    pCurr->pNext->pPrev         = pInsert;
    pInsert->pNext              = pCurr->pNext;
    pCurr->pNext                = pInsert;

    pLink->ulCnt++;

    return PS_SUCC;
}
#endif





TTF_NODE_ST* TTF_LinkRemoveTail(VOS_UINT32 ulPid, TTF_LINK_ST * pLink)
{
    TTF_NODE_ST    *pNode;

    if (VOS_NULL_PTR == pLink)
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink is Null!");
        return VOS_NULL_PTR;
    }

    if (0 == pLink->ulCnt)
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink Cnt is 0!");
        return VOS_NULL_PTR;
    }

    /*异常:链表和链表个数不一致时, 容错处理*/
    if ( pLink->stHead.pNext == &pLink->stHead )
    {
        PS_LOG1(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink is null, but Cnt is <1>!", (VOS_INT32)pLink->ulCnt);
        pLink->ulCnt = 0;
        return VOS_NULL_PTR;
    }

    pNode               = pLink->stHead.pPrev;
    pLink->stHead.pPrev = pNode->pPrev;
    pNode->pPrev->pNext = &pLink->stHead;
    pNode->pNext        = VOS_NULL_PTR;
    pNode->pPrev        = VOS_NULL_PTR;

    pLink->ulCnt--;
    return  pNode;

} /* TTF_LinkRemoveTail */


VOS_VOID TTF_LinkRemoveNode(VOS_UINT32 ulPid, TTF_LINK_ST * pLink, TTF_NODE_ST *pRemoveNode)
{

    pRemoveNode->pNext->pPrev = pRemoveNode->pPrev;
    pRemoveNode->pPrev->pNext = pRemoveNode->pNext;
    pRemoveNode->pPrev        = VOS_NULL_PTR;
    pRemoveNode->pNext        = VOS_NULL_PTR;

    pLink->ulCnt--;
    return;

}

/*****************************************************************************
 函 数 名  : TTF_LinkPeekTail
 功能描述  : 查看链表的尾部，返回尾节点
 输入参数  : pLink   -- 链表指针
 输出参数  : 无
 返 回 值  : pNode   -- 链表节点
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2006年12月8日
    作    者   : 查鸣峰，蒋丽萍
    修改内容   : 新生成函数

*****************************************************************************/
TTF_NODE_ST* TTF_LinkPeekTail(VOS_UINT32 ulPid, const TTF_LINK_ST * pLink)
{
    if ( VOS_NULL_PTR == pLink )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink is Null!");
        return VOS_NULL_PTR;
    }

    if ( 0 == pLink->ulCnt )
    {
        PS_LOG(ulPid, 0, PS_PRINT_INFO, "Info: Tail Node is Null!");
        return VOS_NULL_PTR;
    }

    return pLink->stHead.pPrev;
}

/*****************************************************************************
 函 数 名  : TTF_LinkPeekNext
 功能描述  : 查看链表中指定节点后一个节点，返回该节点的地址。
                若该节点是尾节点，则返回空
 输入参数  : pLink   -- 链表指针
             pCurr   -- 链表中指定的节点
 输出参数  : pCurr后一个节点
 返 回 值  : PS_SUCC, PS_FAIL
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2006年12月8日
    作    者   : 查鸣峰，蒋丽萍
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 TTF_LinkPeekNext(VOS_UINT32 ulPid, const TTF_LINK_ST * pLink,
        TTF_NODE_ST *pCurr, TTF_NODE_ST **ppCurrNext)
{
    TTF_NODE_ST*    pNode;

    if (( VOS_NULL_PTR == pLink ) || ( VOS_NULL_PTR == pCurr ) || ( VOS_NULL_PTR == ppCurrNext ))
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink、pInsert Or ppCurrNext is Null!");
        return PS_FAIL;
    }

    if ( 0 == pLink->ulCnt )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink Cnt is 0!");
        return PS_FAIL;
    }

    if ( (TTF_NODE_ST*)pLink == pCurr )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: Node Address == Link Address!");
        return PS_FAIL;
    }

#ifdef WTTF_PC_ST_SWITCH
    pNode = pLink->stHead.pNext;
    while (pCurr != pNode)
    {
        /*遍历链表，未查找到对应的节点*/
        if (&(pLink->stHead) == pNode)
        {
            PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: Not Find The Node In the Link!");
            return PS_FAIL;
        }

        pNode = pNode->pNext;
    }
#else
    pNode = pCurr;
#endif

    /*若当前节点是尾节点，则返回VOS_NULL_PTR；否则，返回下一个地址*/
    if ( &(pLink->stHead) == pNode->pNext )
    {
        *ppCurrNext = VOS_NULL_PTR;
    }
    else
    {
        *ppCurrNext = pNode->pNext;
    }

    return PS_SUCC;

}

/*****************************************************************************
 函 数 名  : TTF_LinkPeekPrev
 功能描述  : 查看链表中指定节点前一个节点，返回该节点的地址
                若该节点是头节点，则返回空
 输入参数  : pLink   -- 链表指针
             pCurr   -- 链表中指定的节点
 输出参数  : 无
 返 回 值  : pCurr前一个节点
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2006年12月8日
    作    者   : 查鸣峰，蒋丽萍
    修改内容   : 新生成函数

*****************************************************************************/
VOS_UINT32 TTF_LinkPeekPrev(VOS_UINT32 ulPid, TTF_LINK_ST * pLink,
    TTF_NODE_ST *pCurr, TTF_NODE_ST **ppCurrPrev)
{
    TTF_NODE_ST*    pNode;

    if (( VOS_NULL_PTR == pLink ) || ( VOS_NULL_PTR == pCurr ) || ( VOS_NULL_PTR == ppCurrPrev ))
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "\n Warning: pLink、pInsert Or ppCurrPrev is Null!\n");
        return PS_FAIL;
    }

    if ( 0 == pLink->ulCnt )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "\n Warning: pLink Cnt is 0!\n");
        return PS_FAIL;
    }

    if ( (TTF_NODE_ST*)pLink == pCurr )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "\n Warning: Node Address == Link Address!\n");
        return PS_FAIL;
    }

#ifdef WTTF_PC_ST_SWITCH
    pNode = pLink->stHead.pNext;
    while (pCurr != pNode)
    {
        /*遍历链表，未查找到对应的节点*/
        if ( &(pLink->stHead) == pNode )
        {
            PS_LOG(ulPid, 0, PS_PRINT_WARNING, "\n Warning: Not Find The Node In the Link!\n");
            return PS_FAIL;
        }

        pNode = pNode->pNext;
    }
#else
    pNode = pCurr;
#endif

    /*若当前节点是头节点，则返回VOS_NULL_PTR；否则，返回前一个地址*/
    if ( &(pLink->stHead) == pNode->pPrev )
    {
        *ppCurrPrev = VOS_NULL_PTR;
    }
    else
    {
        *ppCurrPrev = pNode->pPrev;
    }

    return PS_SUCC;

}


VOS_UINT32 TTF_LinkStick(VOS_UINT32 ulPid, TTF_LINK_ST *pLink1,
    TTF_LINK_ST *pLink2)
{
    if (VOS_NULL_PTR == pLink1)
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING,
            "TTFCOMM, TTF_LinkStick, ERROR, pLink1 is null\n");
        return VOS_ERR;
    }

    if (VOS_NULL_PTR == pLink2)
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING,
            "TTFCOMM, TTF_LinkStick, ERROR, pLink2 is null\n");
        return VOS_ERR;
    }

    if (TTF_LINK_IS_EMPTY(pLink2))
    {
        return VOS_OK;
    }

    pLink1->stHead.pPrev->pNext = pLink2->stHead.pNext;
    pLink2->stHead.pNext->pPrev = pLink1->stHead.pPrev;
    pLink1->stHead.pPrev        = pLink2->stHead.pPrev;
    pLink2->stHead.pPrev->pNext = (TTF_NODE_ST *)&(pLink1->stHead);

    /*更新长度*/
    pLink1->ulCnt       += pLink2->ulCnt;

    TTF_LINK_INIT(pLink2);

    return VOS_OK;
}
VOS_UINT32 TTF_LinkSafeInsertTail(VOS_UINT32 ulPid, TTF_LINK_ST * pLink, TTF_NODE_ST *pInsert, VOS_UINT32 *pulNonEmptyEvent)
{
    VOS_ULONG       ulLockLevel = 0UL;

    *pulNonEmptyEvent    = PS_FALSE;

    VOS_SpinLockIntLock(&g_stTtfLinkSpinLock, ulLockLevel);

    if ( (VOS_NULL_PTR == pLink) || (VOS_NULL_PTR == pInsert) )
    {
        VOS_SpinUnlockIntUnlock(&g_stTtfLinkSpinLock, ulLockLevel);
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink Or pInsert is Null!");
        return PS_FAIL;
    }

#ifdef WTTF_PC_ST_SWITCH
    /*检查节点pInsert是否在链表pLink中*/
    if ( PS_TRUE == TTF_LinkCheckNodeInLink(ulPid, pLink, pInsert) )
    {
        VOS_SpinUnlockIntUnlock(&g_stTtfLinkSpinLock, ulLockLevel);
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pInsert has Existed in pLink!");
        return PS_FAIL;
    }
#endif

    if (0 == TTF_LINK_CNT(pLink))
    {
        *pulNonEmptyEvent  = PS_TRUE;
    }

    pInsert->pNext              = (TTF_NODE_ST *)(&pLink->stHead);
    pInsert->pPrev              = pLink->stHead.pPrev;

    pLink->stHead.pPrev->pNext  = pInsert;
    pLink->stHead.pPrev         = pInsert;

    pLink->ulCnt++;

    VOS_SpinUnlockIntUnlock(&g_stTtfLinkSpinLock, ulLockLevel);

    return PS_SUCC;
} /* TTF_LinkSafeInsertTail */



TTF_NODE_ST* TTF_LinkSafeRemoveHead(VOS_UINT32 ulPid, TTF_LINK_ST * pLink, VOS_UINT32 *pulRemainCnt)
{
    TTF_NODE_ST    *pNode;
    VOS_ULONG       ulLockLevel = 0UL;

    VOS_SpinLockIntLock(&g_stTtfLinkSpinLock, ulLockLevel);

    *pulRemainCnt   = 0;

    if (VOS_NULL_PTR == pLink)
    {
        VOS_SpinUnlockIntUnlock(&g_stTtfLinkSpinLock, ulLockLevel);
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink is Null!");
        return VOS_NULL_PTR;
    }

    if (0 == pLink->ulCnt)
    {
        VOS_SpinUnlockIntUnlock(&g_stTtfLinkSpinLock, ulLockLevel);
        PS_LOG(ulPid, 0, PS_PRINT_INFO, "Warning: pLink Cnt is 0!");
        return VOS_NULL_PTR;
    }

    /*异常:链表和链表个数不一致时, 容错处理*/
    if ( pLink->stHead.pNext == &pLink->stHead )
    {
        pLink->ulCnt = 0;
        VOS_SpinUnlockIntUnlock(&g_stTtfLinkSpinLock, ulLockLevel);
        PS_LOG1(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink is null, but Cnt is <1>!", (VOS_INT32)pLink->ulCnt);
        return VOS_NULL_PTR;
    }

    pNode               = pLink->stHead.pNext;
    pLink->stHead.pNext = pNode->pNext;
    pNode->pNext->pPrev = &pLink->stHead;
    pNode->pNext        = VOS_NULL_PTR;
    pNode->pPrev        = VOS_NULL_PTR;

    pLink->ulCnt--;

    /* 获取队列里剩余元素个数 */
    *pulRemainCnt   = pLink->ulCnt;

    VOS_SpinUnlockIntUnlock(&g_stTtfLinkSpinLock, ulLockLevel);

    return pNode;
} /* TTF_LinkSafeRemoveHead */

#ifndef _lint
VOS_UINT32 TTF_LinkInsertTail(VOS_UINT32 ulPid, TTF_LINK_ST * pLink, TTF_NODE_ST *pInsert)
{
    if ( (VOS_NULL_PTR == pLink) || (VOS_NULL_PTR == pInsert) )
    {
        /*PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink Or pInsert is Null!");
 */
        return PS_FAIL;
    }

    pInsert->pNext              = (TTF_NODE_ST *)(&pLink->stHead);
    pInsert->pPrev              = pLink->stHead.pPrev;

    pLink->stHead.pPrev->pNext  = pInsert;
    pLink->stHead.pPrev         = pInsert;

    pLink->ulCnt++;

    return PS_SUCC;
} /* TTF_LinkInsertTail */

#endif
/*lint -esym( 528, TTF_LinkRemoveHead )*/
TTF_NODE_ST* TTF_LinkRemoveHead(VOS_UINT32 ulPid, TTF_LINK_ST * pLink)
{
    TTF_NODE_ST    *pNode;

    if (VOS_NULL_PTR == pLink)
    {
        /*PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink is Null!");
 */
        return VOS_NULL_PTR;
    }

    if (0 == pLink->ulCnt)
    {
        /*PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink Cnt is 0!");
 */
        return VOS_NULL_PTR;
    }

    /*异常:链表和链表个数不一致时, 容错处理*/
    if ( pLink->stHead.pNext == &pLink->stHead )
    {
        /*PS_LOG1(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink is null, but Cnt is <1>!", (VOS_INT32)pLink->ulCnt);
 */
        pLink->ulCnt = 0;
        return VOS_NULL_PTR;
    }

    pNode               = pLink->stHead.pNext;
    pLink->stHead.pNext = pNode->pNext;
    pNode->pNext->pPrev = &pLink->stHead;
    pNode->pNext        = VOS_NULL_PTR;
    pNode->pPrev        = VOS_NULL_PTR;

    pLink->ulCnt--;

    return pNode;

} /* TTF_LinkRemoveHead */

/*lint -esym( 528, TTF_LinkCnt )*/
VOS_UINT32 TTF_LinkCnt(VOS_UINT32 ulPid, const TTF_LINK_ST *pLink)
{
    if (VOS_NULL_PTR == pLink)
    {
        /*PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink is Null!");
 */
        return 0;
    }
    return (pLink->ulCnt);
} /* TTF_LinkCnt */

/*lint -esym( 528, TTF_LinkIsEmpty )*/
VOS_UINT32 TTF_LinkIsEmpty(VOS_UINT32 ulPid, const TTF_LINK_ST *pLink)
{
    if (VOS_NULL_PTR == pLink)
    {
        /*PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink is Null!");
 */
        return PS_TRUE;
    }

    if (0 == pLink->ulCnt)
    {
        return PS_TRUE;
    }

    return PS_FALSE;
} /* TTF_LinkIsEmpty */


/*****************************************************************************
 函 数 名  : TTF_LinkPeekHead
 功能描述  : 查看链表的头部，返回头节点
 输入参数  : pLink   -- 链表指针
 输出参数  : 无
 返 回 值  : pNode   -- 链表节点
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2006年12月8日
    作    者   : 查鸣峰，蒋丽萍
    修改内容   : 新生成函数

*****************************************************************************/
/*lint -esym( 528, TTF_LinkPeekHead )*/
TTF_NODE_ST* TTF_LinkPeekHead(VOS_UINT32 ulPid, const TTF_LINK_ST * pLink)
{
    if ( VOS_NULL_PTR == pLink )
    {
        /*PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pLink is Null!");
 */
        return VOS_NULL_PTR;
    }

    if ( 0 == pLink->ulCnt )
    {
        return VOS_NULL_PTR;
    }
    return pLink->stHead.pNext;
}


VOS_UINT32  TTF_RING_Q_Init(VOS_UINT32 ulPid, TTF_RING_Q_ST *pstRingQ, VOS_UINT32 ulMaxSize )
{
    VOS_VOID **ppNode;


    if (VOS_NULL_PTR == pstRingQ )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pstRingQ is Null !");
        return PS_FAIL;
    }

    if ( 2 > ulMaxSize)
    {
        PS_LOG1(ulPid, 0, PS_PRINT_WARNING, "Warning: RingQ  ulMaxSize<1> less than 2 !",(VOS_INT32)ulMaxSize);
        return PS_FAIL;
    }

    #if( VOS_OS_VER == VOS_WIN32 )
    ppNode  = (VOS_VOID **)malloc(sizeof(VOS_VOID *) * ulMaxSize);
    #else
    ppNode  = (VOS_VOID **)VOS_MemAlloc(ulPid, (STATIC_MEM_PT), (sizeof(VOS_VOID *) * ulMaxSize));
    #endif

    if(VOS_NULL_PTR == ppNode )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: RingQ Init Malloc Fail!");
        return PS_FAIL;
    }

    pstRingQ->ppNode       = ppNode;
    pstRingQ->ulFront      = 0;
    pstRingQ->ulRear       = 0;
    pstRingQ->ulMaxSize    = ulMaxSize;
    return PS_SUCC;

} /* TTF_RING_Q_Init */

VOS_UINT32  TTF_RING_Q_NodeAddrInit(VOS_UINT32 ulPid, TTF_RING_Q_ST *pstRingQ, VOS_VOID **ppNodeAddr,VOS_UINT32 ulNodeCnt,VOS_UINT8 ucSize)
{
    VOS_UINT32      ulNodeLoop;

    if ( VOS_NULL_PTR == pstRingQ )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pstRingQ is Null !");
        return PS_FAIL;
    }

    if ( VOS_NULL_PTR == pstRingQ->ppNode )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: ppNode is Null !");
        return PS_FAIL;
    }

    if ( VOS_NULL_PTR == ppNodeAddr )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: ppNodeAddr is Null !");
        return PS_FAIL;
    }

    if ( 2 > ulNodeCnt)
    {
        PS_LOG1(ulPid, 0, PS_PRINT_WARNING, "Warning: ppNodeAddr  ulNodeCnt<1> less than 2 !",(VOS_INT32)ulNodeCnt);
        return PS_FAIL;
    }


    if ( ulNodeCnt != pstRingQ->ulMaxSize )
    {
        PS_LOG2(ulPid, 0, PS_PRINT_WARNING, "Warning: RingQ MaxSize<1> is not matched NodeCnt<2>!", (VOS_INT32)pstRingQ->ulMaxSize, (VOS_INT32)ulNodeCnt);
        return PS_FAIL;
    }

    for (ulNodeLoop = 0; ulNodeLoop < pstRingQ->ulMaxSize; ulNodeLoop++)
    {
        /* 核间共享结点的地址 */
/*      ppNodeAddr = ppNodeAddr + ulNodeLoop*ucSize;
 */
        pstRingQ->ppNode[ulNodeLoop] = (VOS_VOID *)ppNodeAddr;
        ppNodeAddr = ppNodeAddr + ucSize;
    }
    return PS_SUCC;
}




VOS_UINT32  TTF_RING_Q_InitAndNodeAddrSet(VOS_UINT32 ulPid,
                                                        TTF_RING_Q_ST *pstRingQ,
                                                        VOS_VOID **ppNode,
                                                        VOS_VOID *pQueueNode,
                                                        VOS_UINT32 ulBlkSize,
                                                        VOS_UINT32 ulBlkCnt )
{
    VOS_UINT32      ulNodeNoLoop        = 0;
    VOS_UINT8      *pucQueueNodeAddr      = VOS_NULL_PTR;

    if (VOS_NULL_PTR == pstRingQ )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pstRingQ is Null !");
        return PS_FAIL;
    }

    if ( 2 > ulBlkCnt)
    {
        PS_LOG1(ulPid, 0, PS_PRINT_WARNING, "Warning: RingQ  ulMaxSize<1> less than 2 !",(VOS_INT32)ulBlkCnt);
        return PS_FAIL;
    }

    if(VOS_NULL_PTR == ppNode )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: RingQ Init Malloc Fail!");
        return PS_FAIL;
    }

    pstRingQ->ppNode       = ppNode;
    pstRingQ->ulFront      = 0;
    pstRingQ->ulRear       = 0;
    pstRingQ->ulMaxSize    = ulBlkCnt;          /* TTF_RING_Q_ST的ulMaxSize用来存放数据块数 */
    pucQueueNodeAddr         = (VOS_UINT8 *)pQueueNode;

    for(ulNodeNoLoop = 0; ulNodeNoLoop < pstRingQ->ulMaxSize; ulNodeNoLoop ++)
    {
        pstRingQ->ppNode[ulNodeNoLoop] = (pucQueueNodeAddr + ulBlkSize * ulNodeNoLoop);
    }

    return PS_SUCC;
} /* TTF_RING_Q_InitAndNodeAddrSet */


VOS_UINT32  TTF_RING_Q_NodeAddrSet(VOS_UINT32 ulPid, TTF_RING_Q_ST *pstRingQ,
                                    VOS_VOID **ppNodeAddr,VOS_UINT32 ulNodeCnt)
{
    VOS_UINT32      ulNodeNoLoop;


    if (VOS_NULL_PTR == pstRingQ )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pstRingQ is Null !");
        return PS_FAIL;
    }

    if ( VOS_NULL_PTR == pstRingQ->ppNode )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: ppNode is Null !");
        return PS_FAIL;
    }

    if ( VOS_NULL_PTR == ppNodeAddr )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: ppNodeAddr is Null !");
        return PS_FAIL;
    }

    if ( 2 > ulNodeCnt)
    {
        PS_LOG1(ulPid, 0, PS_PRINT_WARNING, "Warning: ppNodeAddr  ulNodeCnt<1> less than 2 !",(VOS_INT32)ulNodeCnt);
        return PS_FAIL;
    }


    if ( ulNodeCnt != pstRingQ->ulMaxSize )
    {
        PS_LOG2(ulPid, 0, PS_PRINT_WARNING, "Warning: RingQ MaxSize<1> is not matched NodeCnt<2>!", (VOS_INT32)pstRingQ->ulMaxSize, (VOS_INT32)ulNodeCnt);
        return PS_FAIL;
    }


    for(ulNodeNoLoop = 0; ulNodeNoLoop < pstRingQ->ulMaxSize; ulNodeNoLoop ++)
    {
        pstRingQ->ppNode[ulNodeNoLoop] = ppNodeAddr[ulNodeNoLoop];
    }
    return PS_SUCC;

} /* TTF_RING_Q_NodeAddrSet */

VOS_VOID* TTF_RING_Q_Rear(VOS_UINT32 ulPid, TTF_RING_Q_ST *pstRingQ)
{
    VOS_VOID    *pNode;


    if (VOS_NULL_PTR == pstRingQ ) /* pstRingQ is Null ! */
    {
        return VOS_NULL_PTR;
    }

    if ( VOS_NULL_PTR == pstRingQ->ppNode) /* pstRingQ is not Initialized ! */
    {
        return VOS_NULL_PTR;
    }

    if ( pstRingQ->ulFront == PS_MOD_ADD(pstRingQ->ulRear, 1, pstRingQ->ulMaxSize) ) /*  RingQ is Full ! */
    {
        return VOS_NULL_PTR;
    }

    pNode = pstRingQ->ppNode[pstRingQ->ulRear];
    return pNode;

} /* TTF_RING_Q_Rear */



VOS_UINT32 TTF_RING_Q_In(VOS_UINT32 ulPid, TTF_RING_Q_ST *pstRingQ )
{

    if (VOS_NULL_PTR == pstRingQ ) /* pstRingQ is Null ! */
    {
        return PS_FAIL;
    }

    if ( pstRingQ->ulFront == PS_MOD_ADD(pstRingQ->ulRear, 1, pstRingQ->ulMaxSize) ) /* RingQ is Full ! */
    {
        return PS_FAIL;
    }

    pstRingQ->ulRear  = PS_MOD_ADD(pstRingQ->ulRear, 1, pstRingQ->ulMaxSize);
    return PS_SUCC;

} /* TTF_RING_Q_In */

VOS_VOID* TTF_RING_Q_Front(VOS_UINT32 ulPid, TTF_RING_Q_ST *pstRingQ)
{
    VOS_VOID    *pNode;

    if (VOS_NULL_PTR == pstRingQ )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pstRingQ is Null !");
        return VOS_NULL_PTR;
    }

    if ( VOS_NULL_PTR == pstRingQ->ppNode) /* pstRingQ is not Initialized ! */
    {
        return VOS_NULL_PTR;
    }

    if ( pstRingQ->ulRear == pstRingQ->ulFront)
    {
        PS_LOG(ulPid, 0, PS_PRINT_INFO, "Info: RingQ is Empty !");
        return VOS_NULL_PTR;
    }

    pNode = pstRingQ->ppNode[pstRingQ->ulFront];

    return pNode;

} /* TTF_RING_Q_Rear */



VOS_VOID* TTF_RING_Q_Out(VOS_UINT32 ulPid, TTF_RING_Q_ST *pstRingQ)
{
    VOS_VOID    *pNode;


    if (VOS_NULL_PTR == pstRingQ ) /* pstRingQ is Null */
    {
        return VOS_NULL_PTR;
    }

    if ( VOS_NULL_PTR == pstRingQ->ppNode) /* pstRingQ is not Initialized ! */
    {
        return VOS_NULL_PTR;
    }

    if ( pstRingQ->ulRear == pstRingQ->ulFront) /* RingQ is Empty */
    {
        return VOS_NULL_PTR;
    }

    pNode               = pstRingQ->ppNode[pstRingQ->ulFront];
    pstRingQ->ulFront   = PS_MOD_ADD(pstRingQ->ulFront, 1, pstRingQ->ulMaxSize);
    return pNode;

} /* TTF_RING_Q_Out */



VOS_UINT32 TTF_RING_Q_IsFull(VOS_UINT32 ulPid, TTF_RING_Q_ST *pstRingQ)
{

    if (VOS_NULL_PTR == pstRingQ ) /* pstRingQ is Null */
    {
        return PS_TRUE;
    }

    if ( pstRingQ->ulFront == PS_MOD_ADD(pstRingQ->ulRear, 1, pstRingQ->ulMaxSize) )
    {
        return PS_TRUE;
    }
    else
    {
        return PS_FALSE;
    }

} /* TTF_RING_Q_IsFull */


VOS_UINT32 TTF_RING_Q_IsEmpty(VOS_UINT32 ulPid, TTF_RING_Q_ST *pstRingQ)
{

    if (VOS_NULL_PTR == pstRingQ ) /* pstRingQ is Null */
    {
        return PS_TRUE;
    }

    if ( pstRingQ->ulRear == pstRingQ->ulFront)
    {
        return PS_TRUE;
    }
    else
    {
        return PS_FALSE;
    }

} /* TTF_RING_Q_IsEmpty */


VOS_UINT32 TTF_RING_Q_Flush(VOS_UINT32 ulPid, TTF_RING_Q_ST *pstRingQ)
{

    if (VOS_NULL_PTR == pstRingQ ) /* pstRingQ is Null ! */
    {
        return PS_FAIL;
    }

    pstRingQ->ulFront = pstRingQ->ulRear;/* 队头追上队尾 */
    return PS_SUCC;

} /* TTF_RING_Q_GetCnt */

VOS_UINT32 TTF_RING_Q_GetCnt(VOS_UINT32 ulPid, TTF_RING_Q_ST *pstRingQ)
{
    VOS_UINT32 ulCnt;

    if (VOS_NULL_PTR == pstRingQ ) /* pstRingQ is Null ! */
    {
        return 0;
    }

    ulCnt   = PS_MOD_SUB( pstRingQ->ulRear, pstRingQ->ulFront, pstRingQ->ulMaxSize);
    return ulCnt;

} /* TTF_RING_Q_GetCnt */



VOS_UINT32  TTF_RING_Q_Free(VOS_UINT32 ulPid, TTF_RING_Q_ST *pstRingQ)
{

    if (VOS_NULL_PTR == pstRingQ )
    {
        PS_LOG(ulPid, 0, PS_PRINT_WARNING, "Warning: pstRingQ is Null !");
        return PS_FAIL;
    }

    if ( VOS_NULL_PTR != pstRingQ->ppNode )
    {
        #if( VOS_OS_VER == VOS_WIN32 )
        free(pstRingQ->ppNode);
        #else
        VOS_MemFree(ulPid, pstRingQ->ppNode);
        #endif

        pstRingQ->ppNode = VOS_NULL_PTR;
    }

    pstRingQ->ulFront   =0;
    pstRingQ->ulRear    =0;
    pstRingQ->ulMaxSize =0;

    return PS_SUCC;

} /* TTF_RING_Q_Free */
/*lint -restore */


#ifdef  __cplusplus
  #if  __cplusplus
  }
  #endif
#endif

