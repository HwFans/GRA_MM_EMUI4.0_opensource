

#ifndef __TTFMEMATINTERFACE_H__
#define __TTFMEMATINTERFACE_H__


/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "v_typdef.h"



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/


/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
枚举名    : TTF_AT_MSG_ID_ENUM_UINT32
结构说明  : AT和TTF的之间的消息

  1.日    期   : 2011年12月22日
    作    者   :
    修改内容   : AT->TTF 查询CCPU MEM消息

*****************************************************************************/
enum TTF_AT_MSG_ID_ENUM
{
    /* AT->TTF 查询CCPU MEM消息 */
    ID_TTF_AT_QRY_CCPU_MEM_INFO_IND,                                             /* _H2ASN_MsgChoice TAFAGENT_APS_FIND_CID_FOR_DIAL_CNF_STRU */

    ID_TTF_AT_MSG_ID_ENUM_BUTT
};
typedef VOS_UINT32  TTF_AT_MSG_ID_ENUM_UINT32;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/


typedef struct
{
    VOS_MSG_HEADER
    TTF_AT_MSG_ID_ENUM_UINT32     enMsgId;

} TTF_MEM_RB_CCPU_MEM_INFO_IND_STRU;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/



#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of TTFMemInterface.h */


