

#ifndef __ADSDEVINTERFACE_H__
#define __ADSDEVINTERFACE_H__

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "ImmInterface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)


/*****************************************************************************
  2 宏定义
*****************************************************************************/
/* 定义IPV6 MTU最大长度值 */
#define ADS_MTU_LEN_MAX                 (1500)

/* !!!!!!!!!!!暂时定义，最终的值由北京确定 */
#define ADS_NDIS_MSG_HDR                (0x00)
#define NDIS_ADS_MSG_HDR                (0x00)

#define ADS_IPV4_ADDR_LEN               (4)
#define ADS_IPV6_ADDR_LEN               (16)
#define ADS_IPV6_ADDR_HALF_LEN          (8)
#define ADS_IPV6_ADDR_QUARTER_LEN       (4)

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

enum ADS_PKT_TYPE_ENUM
{
    ADS_PKT_TYPE_IPV4 = 0x00,                                                   /* IPV4 */
    ADS_PKT_TYPE_IPV6 = 0x01,                                                   /* IPV6 */
    ADS_PKT_TYPE_BUTT
};
typedef VOS_UINT8 ADS_PKT_TYPE_ENUM_UINT8;


enum ADS_NDIS_IP_PACKET_TYPE_ENUM
{
    ADS_NDIS_IP_PACKET_TYPE_DHCPV4      = 0x00,
    ADS_NDIS_IP_PACKET_TYPE_DHCPV6      = 0x01,
    ADS_NDIS_IP_PACKET_TYPE_ICMPV6      = 0x02,
    ADS_NDIS_IP_PACKET_TYPE_LINK_FE80   = 0x03,
    ADS_NDIS_IP_PACKET_TYPE_LINK_FF     = 0x04,
    ADS_NDIS_IP_PACKET_TYPE_BUTT
};
typedef VOS_UINT8 ADS_NDIS_IP_PACKET_TYPE_ENUM_UINT8;


enum ADS_NDIS_MSG_ID_ENUM
{
    ID_ADS_NDIS_DATA_IND               = ADS_NDIS_MSG_HDR + 0x00,               /* ADS->CDS IP PACKET IND */
    ID_ADS_NDIS_MSG_ID_ENUM_BUTT
};
typedef VOS_UINT32  ADS_NDIS_MSG_ID_ENUM_UINT32;

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
    VOS_MSG_HEADER                                                              /*_H2ASN_Skip*/
    ADS_NDIS_MSG_ID_ENUM_UINT32         enMsgId;                                /*_H2ASN_Skip*/
    MODEM_ID_ENUM_UINT16                enModemId;
    VOS_UINT8                           ucRabId;                                /* RAB标识，取值范围:[5,15] */
    ADS_NDIS_IP_PACKET_TYPE_ENUM_UINT8  enIpPacketType;
    VOS_UINT8                           aucReserved[4];
    IMM_ZC_STRU                        *pstSkBuff;                              /* 数据包结构指针 */
} ADS_NDIS_DATA_IND_STRU;



typedef struct
{
    VOS_UINT32                          bitOpIpv4Addr       : 1;
    VOS_UINT32                          bitOpIpv6Addr       : 1;
    VOS_UINT32                          bitOpSpare          : 30;

    VOS_UINT8                           aucIpv4Addr[ADS_IPV4_ADDR_LEN];
    VOS_UINT8                           aucIpv6Addr[ADS_IPV6_ADDR_LEN];
} ADS_FILTER_IP_ADDR_INFO_STRU;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/
/*****************************************************************************
 函 数 名  : ADS_UL_SendPacket
 功能描述  : ADS上行为上层模块提供的数据发送函数，本接口不释放内存。
            上层模块根据返回值判断是否需要释放内存
 输入参数  : IMM_ZC_STRU                        *pstData
             VOS_UINT8                           ucRabId
             (*****ucRabId为扩展的RabId，高2bit表示ModemId，低6bit表示RabId*****)
 输出参数  : 无
 返 回 值  : VOS_UINT32
*****************************************************************************/

VOS_UINT32 ADS_UL_SendPacket(
    IMM_ZC_STRU                        *pstData,
    VOS_UINT8                           ucRabId
);

typedef VOS_UINT32 (*RCV_DL_DATA_FUNC)(VOS_UINT8 ucExRabId, IMM_ZC_STRU *pData, ADS_PKT_TYPE_ENUM_UINT8 enPktType, VOS_UINT32 ulExParam);

/*****************************************************************************
 函 数 名  : ADS_DL_RegDlDataCallback
 功能描述  : ADS下行数据处理模块为上层模块提供的注册下行数据接收函数接口
 输入参数  : ucExRabId --- 扩展承载号
             (*****ucExRabId为扩展的RabId，高2bit表示ModemId，低6bit表示RabId*****)
             pFunc     --- 数据接收回调函数指针
             ulExParam --- 扩展参数
 输出参数  : 无
 返 回 值  : VOS_OK/VOS_ERR
*****************************************************************************/
VOS_UINT32 ADS_DL_RegDlDataCallback(
    VOS_UINT8                           ucExRabId,
    RCV_DL_DATA_FUNC                    pFunc,
    VOS_UINT32                          ulExParam
);

/*****************************************************************************
 函 数 名  : ADS_UL_IsSendPermit
 功能描述  : 上行是否允许发送
 输入参数  : VOS_VOID
 输出参数  : 无
 返 回 值  : VOS_TRUE   - 允许发送
             VOS_FALSE  - 不允许发送
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 ADS_UL_IsSendPermit(VOS_VOID);


/*****************************************************************************
 函 数 名  : ADS_DL_RegFilterDataCallback
 功能描述  : 注册下行数据过滤回调扩展
 输入参数  : ucRabId         - RABID [5, 15]
             pstFilterIpAddr - IP地址信息
             pFunc           - 函数指针
 输出参数  : 无
 返 回 值  : VOS_OK          - 注册成功
             VOS_ERR         - 注册失败
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 ADS_DL_RegFilterDataCallback(
    VOS_UINT8                           ucRabId,
    ADS_FILTER_IP_ADDR_INFO_STRU       *pstFilterIpAddr,
    RCV_DL_DATA_FUNC                    pFunc
);

/*****************************************************************************
 函 数 名  : ADS_DL_DeregFilterDataCallback
 功能描述  : 去注册下行数据过滤回调扩展
 输入参数  : ucRabId - RABID [5, 15]
 输出参数  : 无
 返 回 值  : VOS_OK  - 去注册成功
             VOS_ERR - 去注册失败
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 ADS_DL_DeregFilterDataCallback(VOS_UINT8 ucRabId);

/*****************************************************************************
 函 数 名  : ADS_UL_SendPacketEx
 功能描述  : 上行发送数据扩展接口, 使用该接口发送的数据在ADS记录其信息, 用于
             下行数据过滤匹配, 该接口必须和ADS_DL_RegFilterDataCallback配合
             使用, 只有使用ADS_DL_RegFilterDataCallback注册过下行过滤回调后,
             下行数据才需要根据使用该接口记录的信息进行过滤
 输入参数  : pstData  - IMM数据
             enIpType - IP类型
             ucRabId  - RABID [5, 15]
 输出参数  : 无
 返 回 值  : VOS_OK   - 去注册成功
             VOS_ERR  - 去注册失败
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 ADS_UL_SendPacketEx(
    IMM_ZC_STRU                        *pstData,
    ADS_PKT_TYPE_ENUM_UINT8             enIpType,
    VOS_UINT8                           ucRabId
);



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

#endif

