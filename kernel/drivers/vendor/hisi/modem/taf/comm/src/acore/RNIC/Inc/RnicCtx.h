
#ifndef _RNIC_CTX_H_
#define _RNIC_CTX_H_

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "PsLogdef.h"
#include "PsTypeDef.h"
#include "AtRnicInterface.h"
#include "RnicLinuxInterface.h"
#include "RnicTimerMgmt.h"
#include "RnicProcMsg.h"
#include "RnicEntity.h"

 #include "RnicConfigInterface.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

/*****************************************************************************
  1 全局变量定义
*****************************************************************************/

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#define RNIC_NET_ID_MAX_NUM             (RNIC_RM_NET_ID_BUTT)
#define RNIC_MODEM_ID_MAX_NUM           (MODEM_ID_BUTT)

#define RNIC_MAX_IPV6_ADDR_LEN          (16)                                    /* IPV6地址长度 */

#define RNIC_MAX_DSFLOW_BYTE            (0xFFFFFFFF)                            /* RNIC最大流量值 */

#define RNIC_DIAL_DEMA_IDLE_TIME        (600)

#define RNIC_SET_FLOW_CTRL_STATUS(status, index)    (g_stRnicCtx.astSpecCtx[index].enFlowCtrlStatus = (status))

#define RNIC_GET_FLOW_CTRL_STATUS(index)            (g_stRnicCtx.astSpecCtx[index].enFlowCtrlStatus)

/* RabId的高两位表示ModemId, 00表示Modem0, 01表示Modem1 */
#define RNIC_RABID_TAKE_MODEM_1_MASK    (0x40)                                  /* Rabid携带Modem1的掩码 */
#define RNIC_RABID_UNTAKE_MODEM_1_MASK  (0x3F)                                  /* Rabid去除Modem1的掩码 */

/* 最大RABID值 */
#define RNIC_RAB_ID_MAX_NUM             (11)
#define RNIC_RAB_ID_OFFSET              (5)

/* Rab Id的最小值 */
#define RNIC_RAB_ID_MIN                 (5)

/* Rab Id的最大值 */
#define RNIC_RAB_ID_MAX                 (15)

#define RNIC_RAB_ID_INVALID             (0xFF)

#define RNIC_RAB_ID_IS_VALID(ucRabId)\
    (((ucRabId) >= RNIC_RAB_ID_MIN) && ((ucRabId) <= RNIC_RAB_ID_MAX))

#define RNIC_PDN_ID_MAX_NUM             (20)
#define RNIC_PDN_ID_OFFSET              (1)

/* PDN Id的最小值 */
#define RNIC_PDN_ID_MIN                 (1)

/* PDN Id的最大值 */
#define RNIC_PDN_ID_MAX                 (20)

#define RNIC_PDN_ID_INVALID             (0xFF)

#define RNIC_PDN_ID_IS_VALID(ucPdnId)\
    (((ucPdnId) >= RNIC_PDN_ID_MIN) && ((ucPdnId) <= RNIC_PDN_ID_MAX))


/* 获取IPV4类型PDP激活的PDNID */
#define RNIC_GET_SPEC_NET_IPV4_PDNID(index)         (RNIC_GET_SPEC_NET_CTX(index)->stIpv4PdpInfo.ucPdnId)

/* 获取IPV6类型PDP激活的PDNID */
#define RNIC_GET_SPEC_NET_IPV6_PDNID(index)         (RNIC_GET_SPEC_NET_CTX(index)->stIpv6PdpInfo.ucPdnId)

/* 获取指定Modem的PDNID信息 */
#define RNIC_GET_SPEC_MODEM_PDNID_INFO()            (&g_stRnicCtx.stPdnIdInfo)

/* 获取PDNID对应的网卡ID */
#define RNIC_GET_RM_NET_ID_BY_PDN_ID(PDNId) \
        (RNIC_GET_SPEC_MODEM_PDNID_INFO()->aucRmNetId[PDNId - RNIC_PDN_ID_OFFSET])


/* 获取指定网卡的PDP上下文地址 */
#define RNIC_GET_SPEC_NET_CTX(index)                (&g_stRnicCtx.astSpecCtx[index].stPdpCtx)

/* 获取IPV4类型PDP激活的RABID */
#define RNIC_GET_SPEC_NET_IPV4_RABID(index)         (RNIC_GET_SPEC_NET_CTX(index)->stIpv4PdpInfo.ucRabId)

/* 获取IPV6类型PDP激活的RABID */
#define RNIC_GET_SPEC_NET_IPV6_RABID(index)         (RNIC_GET_SPEC_NET_CTX(index)->stIpv6PdpInfo.ucRabId)

/* 获取IPV4类型PDP激活的RABID */
#define RNIC_GET_SPEC_NET_IPV4_REG_STATE(index)     (RNIC_GET_SPEC_NET_CTX(index)->stIpv4PdpInfo.enRegStatus)

/* 获取当前的拨号模式 */
#define RNIC_GET_DIAL_MODE()                        (g_stRnicCtx.stDialMode.enDialMode)

/* 获取上行上下文地址 */
#define RNIC_GET_UL_CTX_ADDR(index)                 (&g_stRnicCtx.astSpecCtx[index].stUlCtx)

/* 获取下行上下文地址 */
#define RNIC_GET_DL_CTX_ADDR(index)                 (&g_stRnicCtx.astSpecCtx[index].stDlCtx)

/* 获取指定网卡对应的modem type */
#define RNIC_GET_SPEC_NET_MODEM_TYPE(index)         (g_stRnicCtx.astSpecCtx[index].enModemType)

/* 获取指定Modem的RABID信息 */
#define RNIC_GET_SPEC_MODEM_RABID_INFO(index)       (&g_stRnicCtx.astRabIdInfo[index])

/* 获取网卡私有数据地址 */
#define RNIC_GET_SPEC_PRIV_INFO_ADDR(index)          (g_stRnicCtx.astSpecCtx[index].pstNetDevInfo)


/*****************************************************************************
  3 枚举定义
*****************************************************************************/

enum RNIC_PDP_REG_STATUS_ENUM
{
    RNIC_PDP_REG_STATUS_DEACTIVE,                                               /* 未注册上 */
    RNIC_PDP_REG_STATUS_ACTIVE,                                                 /* 已注册上 */
    RNIC_PDP_REG_STATUS_BUTT
};
typedef VOS_UINT32 RNIC_PDP_REG_STATUS_ENUM_UINT32;


enum RNIC_FLOW_CTRL_STATUS_ENUM
{
    RNIC_FLOW_CTRL_STATUS_STOP          = 0x00,                                 /* 流控停止 */
    RNIC_FLOW_CTRL_STATUS_START         = 0x01,                                 /* 流控启动 */
    RNIC_FLOW_CTRL_STATUS_BUTT          = 0xFF
};
typedef VOS_UINT32 RNIC_FLOW_CTRL_STATUS_ENUM_UINT32;

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


typedef struct RNIC_DL_DATA_STATS
{
    VOS_UINT32                          ulDLCurDataRate;                        /* 当前下行速率，保存PDP激活后2秒的速率，去激活后清空 */
    VOS_UINT32                          ulDLTotalDroppedPkts;                   /* 下行丢包个数，整个PDP激活期间生效 */
    VOS_UINT32                          ulDLPeriodRcvPkts;                      /* 下行收到数据包个数,统计一个拨号断开定时器周期内收到的个数，超时后清空 */
    VOS_UINT32                          ulDLPeriodRcvBytes;                     /* 一个流量统计周期内收到的byte数 */
    VOS_UINT32                          ulDLTotalRcvBytes;                      /* 下行收到数据包byte数，PDP激活后开始统计，PDP去激活后清空 */
    VOS_UINT8                           aucReserved[4];
}RNIC_DL_DATA_STATS_STRU;
typedef struct RNIC_UL_DATA_STATS
{
    VOS_UINT32                          ulULTotalRcvPktsInDeactive;             /* PDP未激活的情况下，收到上行数据包个数，拨号保护定时器超时后清空 */
    VOS_UINT32                          ulULCurDataRate;                        /* 当前上行速率，保存PDP激活后2秒的速率，去激活后清空 */
    VOS_UINT32                          ulULTotalDroppedPkts;                   /* 上行丢包个数 */
    VOS_UINT32                          ulULPeriodSndPkts;                      /* 上行发送有效个数,统计一个拨号断开定时器周期内收到的个数，超时后清空 */
    VOS_UINT32                          ulULPeriodSndBytes;                     /* 一个流量统计周期内发送的byte数 */
    VOS_UINT32                          ulULTotalSndBytes;                      /* 上行发送byte数，PDP激活后开始统计，PDP去激活后清空 */
}RNIC_UL_DATA_STATS_STRU;


typedef struct
{
    VOS_UINT32                          OP_EnableDestAddr      : 1;             /* 使能目的IP地址作为过滤条件 */
    VOS_UINT32                          OP_EnableDestMask      : 1;             /* 使能目的网络掩码作为过滤条件 */
    VOS_UINT32                          OP_EnableDestPortLow   : 1;             /* 使能目的端口号下限作为过滤条件 */
    VOS_UINT32                          OP_EnableDestPortHigh  : 1;             /* 使能目的端口号上限作为过滤条件 */
    VOS_UINT32                          OP_EnableSrcPortLow    : 1;             /* 使能源端口号下限作为过滤条件 */
    VOS_UINT32                          OP_EnableSrcPortHigh   : 1;             /* 使能源端口号上限作为过滤条件 */
    VOS_UINT32                          OP_EnableReserved      : 26;            /* 保留 */
    VOS_UINT32                          ulDestAddr;                             /* 目的IP地址 */
    VOS_UINT32                          ulDestMask;                             /* 目的网络掩码 */
    VOS_UINT16                          usDestPortLow;                          /* 目的端口号下限 */
    VOS_UINT16                          usDestPortHigh;                         /* 目的端口号上限 */
    VOS_UINT16                          usSrcPortLow;                           /* 源端口号下限 */
    VOS_UINT16                          usSrcPortHigh;                          /* 源端口号上限 */
}RNIC_FILTER_INFO_STRU;
typedef struct RNIC_FTI
{
    VOS_INT32                           ulFilterId;                             /* 发送过滤器的ID */
    RNIC_FILTER_INFO_STRU               stFilter;                               /* 发送过滤器的信息 */
    struct RNIC_FTI                    *pNextItem;                              /* 指向下一个列表项 */
}RNIC_FILTER_LST_STRU;
typedef struct
{
    RNIC_UL_DATA_STATS_STRU             stULDataStats;                           /* 上行数据统计 */
}RNIC_UL_CTX_STRU;
typedef struct
{
    RNIC_DL_DATA_STATS_STRU             stDLDataStats;                          /* 下行数据统计 */
}RNIC_DL_CTX_STRU;
typedef struct
{
    RNIC_PDP_REG_STATUS_ENUM_UINT32     enRegStatus;                            /* 标识该PDP上下文是否注册 */

    VOS_UINT8                           ucRabId;                                /* 承载标识 */
    VOS_UINT8                           ucPdnId;
    VOS_UINT8                           aucRsv[2];                              /* 保留 */

    VOS_UINT32                          ulIpv4Addr;                             /* IP地址 */
}RNIC_IPV4_PDP_INFO_STRU;


typedef struct
{
    RNIC_PDP_REG_STATUS_ENUM_UINT32     enRegStatus;                            /* 标识该PDP上下文是否注册 */

    VOS_UINT8                           ucRabId;                                /* 承载标识 */
    VOS_UINT8                           ucPdnId;
    VOS_UINT8                           aucRsv[2];                              /* 保留 */

    VOS_UINT8                           aucIpv6Addr[RNIC_MAX_IPV6_ADDR_LEN];    /* 从AT带来的IPV6地址长度，不包括":" */
}RNIC_IPV6_PDP_INFO_STRU;


typedef struct
{
    RNIC_PDP_REG_STATUS_ENUM_UINT32     enRegStatus;                            /* 标识该PDP上下文是否注册 */

    VOS_UINT8                           ucRabId;                                /* 承载标识 */
    VOS_UINT8                           ucPdnId;
    VOS_UINT8                           aucRsv[2];                              /* 保留 */

    VOS_UINT32                          ulIpv4Addr;                             /* IP地址 */
    VOS_UINT8                           aucIpv6Addr[RNIC_MAX_IPV6_ADDR_LEN];    /* 从AT带来的IPV6地址长度，不包括":" */
}RNIC_IPV4V6_PDP_INFO_STRU;
typedef struct
{
    RNIC_IPV4_PDP_INFO_STRU             stIpv4PdpInfo;                          /* IPV4的PDP信息 */
    RNIC_IPV6_PDP_INFO_STRU             stIpv6PdpInfo;                          /* IPV6的PDP信息 */
    RNIC_IPV4V6_PDP_INFO_STRU           stIpv4v6PdpInfo;                        /* IPV4V6的PDP信息 */

    /* 为后续扩展，以下2个公共域保留 */
    RNIC_FILTER_LST_STRU               *pstFilterList;                          /* 发送过滤器列表 */
    VOS_UINT8                          *pucFragmBuf;                            /* 用于IP分片的缓存 */
} RNIC_PDP_CTX_STRU;


typedef struct
{
    RNIC_RM_NET_ID_ENUM_UINT8           aucRmNetId[RNIC_RAB_ID_MAX_NUM];        /* RABID对应的网卡ID */
    VOS_UINT8                           aucReserved[5];
}RNIC_RABID_INFO_STAU;
typedef struct
{
    RNIC_RM_NET_ID_ENUM_UINT8           aucRmNetId[RNIC_PDN_ID_MAX_NUM];        /* PDNID对应的网卡ID */
    VOS_UINT8                           aucReserved[4];
}RNIC_PDNID_INFO_STAU;
typedef struct
{
    RNIC_UL_CTX_STRU                    stUlCtx;                                /* RNIC上行上下文 */
    RNIC_DL_CTX_STRU                    stDlCtx;                                /* RNIC下行上下文 */
    RNIC_PDP_CTX_STRU                   stPdpCtx;                               /* RNIC的PDP上下文 */

    RNIC_FLOW_CTRL_STATUS_ENUM_UINT32   enFlowCtrlStatus;                       /* 流控状态 */

    RNIC_TIMER_ID_ENUM_UINT16           enTiDsFlowStats;                        /* 流量统计定时器 */
    MODEM_ID_ENUM_UINT16                enModemId;                              /* 网卡归属哪个modem */

    RNIC_MODEM_TYPE_ENUM_UINT8          enModemType;                            /* 记录当前网卡连接的是内置modem和还是外接modem */
    VOS_UINT8                           ucReserved[7];

    /* 保存系统分配的Netcard私有数据虚存地址 */
    RNIC_NETCARD_DEV_INFO_STRU         *pstNetDevInfo;                          /* 网卡设备信息 */
}RNIC_SPEC_CTX_STRU;
typedef struct
{
    /****** RNIC每个网卡专有的上下文 ******/
    RNIC_SPEC_CTX_STRU                  astSpecCtx[RNIC_NET_ID_MAX_NUM];        /* 特定网卡上下文信息 */

    RNIC_RABID_INFO_STAU                astRabIdInfo[RNIC_MODEM_ID_MAX_NUM];    /* RabId对应的网卡ID */

    RNIC_PDNID_INFO_STAU                stPdnIdInfo;                            /* 记录sdio pdn id对应的网卡id*/

    /****** RNIC定时器上下文 ******/
    RNIC_TIMER_CTX_STRU                 astTimerCtx[RNIC_MAX_TIMER_NUM];

    /****** RNIC公共上下文 ******/
    RNIC_DIAL_MODE_STRU                 stDialMode;                             /* 拨号模式 */
    VOS_UINT32                          ulTiDialDownExpCount;                   /* 拨号断开定时器超时次数参数统计 */

    VOS_UINT32                          ulSetTimer4WakeFlg;                     /* 是否设置Timer4唤醒标志 */

    VOS_SEM                             hResetSem;                              /* 二进制信号量，用于复位处理  */

}RNIC_CTX_STRU;
/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

extern RNIC_CTX_STRU                                g_stRnicCtx;


/*****************************************************************************
  10 函数声明
*****************************************************************************/
VOS_VOID RNIC_InitCtx(
    RNIC_CTX_STRU                      *pstRnicCtx
);
VOS_VOID RNIC_InitDlCtx(
    RNIC_DL_CTX_STRU                   *pstDlCtx
);
VOS_VOID RNIC_InitUlCtx(
    RNIC_UL_CTX_STRU                   *pstUlCtx
);
VOS_VOID RNIC_InitPdpCtx(
    RNIC_PDP_CTX_STRU                  *pstPdpCtx,
    VOS_UINT8                           ucRmNetId
);
VOS_VOID RNIC_InitDialMode(
    RNIC_DIAL_MODE_STRU                *pstDialMode
);
VOS_VOID RNIC_InitIpv4PdpCtx(
    RNIC_IPV4_PDP_INFO_STRU            *pstIpv4PdpCtx
);
VOS_VOID RNIC_InitIpv6PdpCtx(
    RNIC_IPV6_PDP_INFO_STRU            *pstIpv6PdpCtx
);
VOS_VOID RNIC_InitIpv4v6PdpCtx(
    RNIC_IPV4V6_PDP_INFO_STRU          *pstIpv4v6PdpCtx,
    VOS_UINT8                           ucRmNetId
);
RNIC_CTX_STRU* RNIC_GetRnicCtxAddr(VOS_VOID);
VOS_UINT32 RNIC_GetTiDialDownExpCount( VOS_VOID);
VOS_VOID RNIC_IncTiDialDownExpCount( VOS_VOID);
VOS_VOID RNIC_ClearTiDialDownExpCount( VOS_VOID);

VOS_UINT32 RNIC_GetCurrentUlRate(VOS_UINT8 ucIndex);
VOS_VOID RNIC_SetCurrentUlRate(
    VOS_UINT32                          ulULDataRate,
    VOS_UINT8                           ucIndex
);
VOS_UINT32 RNIC_GetCurrentDlRate(VOS_UINT8 ucIndex);
VOS_VOID RNIC_SetCurrentDlRate(
    VOS_UINT32                          ulDLDataRate,
    VOS_UINT8                           ucIndex
);
RNIC_DIAL_MODE_STRU* RNIC_GetDialModeAddr(VOS_VOID);
RNIC_PDP_CTX_STRU* RNIC_GetPdpCtxAddr(VOS_UINT8 ucIndex);
RNIC_UL_CTX_STRU* RNIC_GetUlCtxAddr(VOS_UINT8 ucIndex);
RNIC_DL_CTX_STRU* RNIC_GetDlCtxAddr(VOS_UINT8 ucIndex);

RNIC_TIMER_CTX_STRU*  RNIC_GetTimerAddr( VOS_VOID );
VOS_VOID RNIC_SetTimer4WakeFlg(VOS_UINT32 ulFlg);
VOS_UINT32 RNIC_GetTimer4WakeFlg(VOS_VOID);

VOS_VOID RNIC_InitRabidInfo(
    RNIC_RABID_INFO_STAU               *pstRabIdInfo
);

VOS_VOID RNIC_InitPdnIdInfo(
    RNIC_PDNID_INFO_STAU               *pstPdnIdInfo
);

RNIC_NETCARD_DEV_INFO_STRU *RNIC_GetSpecNetCardPrivateAddr(VOS_UINT8 ucIndex);
VOS_VOID RNIC_SetSpecNetCardPrivate(
    RNIC_NETCARD_DEV_INFO_STRU         *pstPriv,
    VOS_UINT8                           ucIndex
);
RNIC_SPEC_CTX_STRU *RNIC_GetSpecNetCardCtxAddr(VOS_UINT8 ucIndex);
RNIC_RABID_INFO_STAU *RNIC_GetSpecModemRabIdInfoAddr(
    MODEM_ID_ENUM_UINT16                enModemId
);

VOS_VOID RNIC_ResetDialMode(
    RNIC_DIAL_MODE_STRU                *pstDialMode
);
VOS_VOID RNIC_InitResetSem(VOS_VOID);
VOS_SEM RNIC_GetResetSem(VOS_VOID);

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

#endif /* end of RnicCtx.h */

