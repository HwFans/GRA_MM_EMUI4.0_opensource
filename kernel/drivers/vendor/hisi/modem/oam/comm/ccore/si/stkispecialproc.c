
/*lint --e{740,958,537,767,553,516,718,746,813}*/

#include "si_stk.h"
#include "product_config.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID PS_FILE_ID_STK_SPECIAL_PROC_C


VOS_UINT8                           g_ucSendMsgNum = 0;
VOS_UINT8                           g_ucReceCBNum = 0;

#if (VOS_WIN32 == VOS_OS_VER) /* UT工程编译不过,通过编译宏区分 */
MN_MSG_RAW_TS_DATA_STRU            *f_pstStkMsgRawTsDataInfo = VOS_NULL_PTR;
MN_MSG_SEND_PARM_STRU              *f_pstSendDirectParm = VOS_NULL_PTR;
#else
static MN_MSG_RAW_TS_DATA_STRU      *f_pstStkMsgRawTsDataInfo = VOS_NULL_PTR;
static  MN_MSG_SEND_PARM_STRU       *f_pstSendDirectParm = VOS_NULL_PTR;
#endif

SI_STK_CMD_PROC_STATUS_UINT32       g_enTACmdStatus = SI_STK_NORMAL_CNF;

SI_STK_CMD_PROC_STATUS_UINT32       g_enNMRCmdStatus = SI_STK_NORMAL_CNF;

/*记录当前SS操作的类型*/
VOS_UINT8                           g_ucSendSSType;

/*记录当前Send Dtmf操作的内容*/
SI_SAT_DTMF_DATA_STRU               g_stSTKDtmfData;

/*****************************************************************************
 Prototype      : SI_STK_GetRecGASPID
 Description    : STK发送给TAF消息函数
 Input          : pulTAFPid:接收消息的TAF的PID
 Output         : 无
 Return Value   : VOS_ERR/VOS_OK

 History        : ---
  1.Date        : 2013-05-24
    Author      : zhuli
    Modification: Created function
*****************************************************************************/
VOS_UINT32 SI_STK_GetRecGASPID(VOS_UINT32 *pulGasPid)
{
#if (FEATURE_MULTI_MODEM == FEATURE_ON)
    if (VOS_TRUE == USIMM_IsSvlte())
    {
        if (MODEM_ID_0 == g_enSTKCurCSModem)
        {
            STK_NORMAL_LOG("SI_STK_GetRecGASPID: Send Msg to Modem0 GAS");

            *pulGasPid = I0_UEPS_PID_GAS;

            return VOS_OK;
        }
        else if (MODEM_ID_1 == g_enSTKCurCSModem)
        {
            STK_NORMAL_LOG("SI_STK_GetRecGASPID: Send Msg to Modem1 GAS");

            *pulGasPid = I1_UEPS_PID_GAS;

            return VOS_OK;
        }
        else
        {
            STK_WARNING_LOG("SI_STK_GetRecGASPID: No CS Modem");

            return VOS_ERR;
        }
    }
#endif

    STK_NORMAL_LOG("SI_STK_GetRecGASPID: Send Msg to Auto Modem GAS");

    *pulGasPid = UEPS_PID_GAS;

    return VOS_OK;
}

/*****************************************************************************
 Prototype      : SI_STK_GetRecTAFPID
 Description    : STK发送给TAF消息函数
 Input          : pulTAFPid:接收消息的TAF的PID
 Output         : 无
 Return Value   : VOS_ERR/VOS_OK

 History        : ---
  1.Date        : 2013-05-24
    Author      : zhuli
    Modification: Created function
*****************************************************************************/
VOS_UINT32 SI_STK_GetRecTAFPID(VOS_UINT32 *pulTAFPid)
{
#if (FEATURE_MULTI_MODEM == FEATURE_ON)
    if(VOS_TRUE == USIMM_IsSvlte())
    {
        if (MODEM_ID_0 == g_enSTKCurCSModem)
        {
            STK_NORMAL_LOG("SI_STK_GetRecTAFPID: Send Msg to Modem0 TAF");

            *pulTAFPid = I0_WUEPS_PID_TAF;

            return VOS_OK;
        }
        else if (MODEM_ID_1 == g_enSTKCurCSModem)
        {
            STK_NORMAL_LOG("SI_STK_GetRecTAFPID: Send Msg to Modem1 TAF");

            *pulTAFPid = I1_WUEPS_PID_TAF;

            return VOS_OK;
        }
        else
        {
            STK_WARNING_LOG("SI_STK_GetRecTAFPID:NO CS Modem");

            return VOS_ERR;
        }
    }
#endif

    STK_NORMAL_LOG("SI_STK_GetRecTAFPID: Send Msg to Auto Modem TAF");

    *pulTAFPid = WUEPS_PID_TAF;

    return VOS_OK;
}

/*****************************************************************************
 Prototype      : SI_STK_SendTAFMsg
 Description    : STK发送给TAF消息函数
 Input          : ulRecPID:接收PID
                  ulMsgName:消息名称
                  ucValue:消息中填充位数据
                  pucData:消息数据
                  ulLen:消息长度
 Output         : 无
 Return Value   : VOS_ERR/VOS_OK

 History        : ---
  1.Date        : 2011-10-04
    Author      : zhuli
    Modification: Created function
*****************************************************************************/
VOS_UINT32 SI_STK_SendTAFMsg(VOS_UINT32  ulMsgName,
                                    VOS_UINT8   ucOpID,
                                    VOS_UINT8   ucValue,
                                    VOS_UINT8*  pucData,
                                    VOS_UINT32  ulLen)
{
    SI_STK_TAF_SENDMSG_STRU     *pstMsg;
    VOS_UINT32                  ulRecPID;

    if (VOS_ERR == SI_STK_GetRecTAFPID(&ulRecPID))
    {
        STK_WARNING_LOG("SI_STK_SendTAFMsg: Get Receiver PID Fail");

        return VOS_ERR;
    }

    pstMsg = (SI_STK_TAF_SENDMSG_STRU*)VOS_AllocMsg(MAPS_STK_PID,
                    (sizeof(SI_STK_TAF_SENDMSG_STRU)+ulLen)-VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pstMsg)   /*分配内存失败会重启*/
    {
        STK_ERROR_LOG("SI_STK_SendTAFMsg: ALloc Msg Error");

        return VOS_ERR;
    }

    VOS_MemSet(pstMsg->aucData, 0, sizeof(VOS_UINT8)*4);

    pstMsg->ulReceiverPid       = ulRecPID;
    pstMsg->ulMsgName           = ulMsgName;
    pstMsg->usClientId          = OAM_CLIENT_ID_STK;    /*Nas分配的特殊值*/
    pstMsg->ucOpId              = ucOpID;
    pstMsg->ucValue             = ucValue;

    if (VOS_NULL != ulLen)  /*当前存在消息内容*/
    {
        VOS_MemCpy(pstMsg->aucData, pucData, ulLen);
    }

    if (VOS_OK != VOS_SendMsg(MAPS_STK_PID, pstMsg))
    {
        STK_ERROR_LOG1("SI_STK_SendTAFMsg: Send Msg %d to TAF Error", (VOS_INT32)ulMsgName);

        return VOS_ERR;
    }
    return VOS_OK;
}

/*****************************************************************************
 Prototype      : SI_STK_SVLTECheckMsgPID
 Description    : STK发送给TAF消息函数
 Input          : pulTAFPid:接收消息的TAF的PID
 Output         : 无
 Return Value   : VOS_ERR/VOS_OK

 History        : ---
  1.Date        : 2013-05-24
    Author      : zhuli
    Modification: Created function
*****************************************************************************/
VOS_UINT32 SI_STK_SVLTECheckMsgPID(PS_SI_MSG_STRU *pstMsg, VOS_UINT32 ulPid)
{
    if ((VOS_FALSE == USIMM_IsSvlte()) && (pstMsg->ulSenderPid != ulPid))
    {
        STK_ERROR_LOG("SI_STK_SVLTECheckMsgPID: SVLTE Off and Msg Sender PID is Wrong");

        return VOS_ERR;
    }
    else
    {
        STK_INFO_LOG("SI_STK_SVLTECheckMsgPID: SVLTE ON or Msg Sender PID is OK");

        return VOS_OK;
    }
}


VOS_UINT32 SI_STK_SVLTECheckCsDomainAndMsgPID(PS_SI_MSG_STRU *pstMsg, VOS_UINT32 ulExpectPid)
{
    if (VOS_TRUE == USIMM_IsSvlte())
    {
        STK_ERROR_LOG("SI_STK_SVLTECheckCsDomainAndMsgPID: SVLTE is On, Can only process CS domain event!");

        return VOS_ERR;
    }
    else
    {
        if (pstMsg->ulSenderPid != ulExpectPid)
        {
            STK_ERROR_LOG("SI_STK_SVLTECheckCsDomainAndMsgPID: SVLTE is off, but PID is not expected!");

            return VOS_ERR;
        }
        else
        {
            STK_INFO_LOG("SI_STK_SVLTECheckCsDomainAndMsgPID: SVLTE is off, and PID is expected!");

            return VOS_OK;
        }
    }
}

/*****************************************************************************
 Prototype      : SI_STK_GetCSModemFunc
 Description    : STK发送给TAF消息函数
 Input          : pstMsg:接收消息
 Output         : 无
 Return Value   : 无

 History        : ---
  1.Date        : 2013-05-26
    Author      : zhuli
    Modification: Created function
*****************************************************************************/
VOS_VOID SI_STK_GetCSModemFunc(TAF_STK_CS_SERVICE_IND *pstMsg)
{
#if (FEATURE_MULTI_MODEM == FEATURE_ON)
    if (VOS_TRUE != USIMM_IsSvlte())
    {
        STK_ERROR_LOG("SI_STK_GetCSModemFunc: Receive Wrong Mag From Nas");
        return;
    }

    if (TAF_CS_ON == pstMsg->enCSState)
    {
        if(g_enSTKCurCSModem != VOS_GetModemIDFromPid(pstMsg->ulSenderPid))
        {
            STK_UpdateCsSvcStatus(NAS_STK_SERVICE_STATUS_NO_SERVICE);
        }

        g_enSTKCurCSModem = VOS_GetModemIDFromPid(pstMsg->ulSenderPid);
    }
#endif

    STK_NORMAL_LOG1("SI_STK_GetCSModemFunc: CS Modem is %d", (VOS_INT32)g_enSTKCurCSModem);

    return;
}


VOS_VOID SI_STK_MtCallEventDownload(NAS_STK_MT_CALL_EVENT_STRU *pstMsg)
{
    SI_STK_MT_CALL_EVENT_STRU           stEvent;

    /* 当前不需要下载MT CALL事件 */
    if (VOS_FALSE == g_stSTKEventState.OP_MTCall)
    {
        STK_WARNING_LOG("SI_STK_MtCallEventDownload: The Event is not Need Download");

        return;
    }

    VOS_MemSet(&stEvent, 0, sizeof(SI_STK_MT_CALL_EVENT_STRU));

    /* 对TI赋值 */
    stEvent.OP_TransactionId            = VOS_TRUE;
    stEvent.TransactionId.ulLen         = sizeof(pstMsg->stMtCallEventInfo.ucTi);
    stEvent.TransactionId.pucTransaction= &pstMsg->stMtCallEventInfo.ucTi;

    /* 对号码赋值 */
    if (VOS_NULL != pstMsg->stMtCallEventInfo.stCallerNumber.ucNumLen)
    {
        stEvent.OP_Addr = VOS_TRUE;
        stEvent.Addr.ucNumType = pstMsg->stMtCallEventInfo.stCallerNumber.enNumType;
        stEvent.Addr.ucLen     = pstMsg->stMtCallEventInfo.stCallerNumber.ucNumLen;
        stEvent.Addr.pucAddr   = pstMsg->stMtCallEventInfo.stCallerNumber.aucBcdNum;
    }

    /* 对子号码赋值 */
    if (VOS_FALSE != pstMsg->stMtCallEventInfo.stCallerSubNumber.IsExist)
    {
        stEvent.OP_Subaddr          = VOS_TRUE;
        stEvent.Subaddr.ulLen       = pstMsg->stMtCallEventInfo.stCallerSubNumber.LastOctOffset;
        stEvent.Subaddr.pucSubaddr  = &pstMsg->stMtCallEventInfo.stCallerSubNumber.Octet3;
    }

    SI_STK_EventDownload(&stEvent, sizeof(SI_STK_MT_CALL_EVENT_STRU), SI_STK_EVENT_MT_CALL, SI_STK_DEV_NETWORK);

    return;
}


VOS_VOID SI_STK_CallConnectEventDownload(NAS_STK_CALL_CONNECT_EVENT_STRU *pstMsg)
{
    SI_STK_CALL_CONNECTED_EVENT_STRU    stEvent;
    VOS_UINT8                           ucSDId;

    /* 当前不需要下载CALL CONNECT事件 */
    if (VOS_FALSE == g_stSTKEventState.OP_CallConnect)
    {
        STK_WARNING_LOG("SI_STK_CallConnectEventDownload: The Event is not Need Download");
        return;
    }

    VOS_MemSet(&stEvent, 0, sizeof(SI_STK_CALL_CONNECTED_EVENT_STRU));

    /* 对TI赋值 */
    stEvent.OP_TransactionId            = VOS_TRUE;
    stEvent.TransactionId.ulLen         = sizeof(pstMsg->stCallConnectEventInfo.ucTi);
    stEvent.TransactionId.pucTransaction= &pstMsg->stCallConnectEventInfo.ucTi;

    /* 设置Source Device ID，只有MO CALL是从网络发来，其它都是从本地 */
    if (MN_CALL_DIR_MO == pstMsg->stCallConnectEventInfo.enCallDir)
    {
        ucSDId = SI_STK_DEV_NETWORK;
    }
    else
    {
        ucSDId = SI_STK_DEV_TERMINAL;
    }

    SI_STK_EventDownload(&stEvent, sizeof(SI_STK_CALL_CONNECTED_EVENT_STRU), SI_STK_EVENT_CALL_CONNECTED, ucSDId);

    return;
}
VOS_VOID SI_STK_CallDisconnectEventDownload(NAS_STK_CALL_DISC_EVENT_STRU *pstMsg)
{
    SI_STK_CALL_DISCONNECTED_EVENT_STRU stEvent;
    VOS_UINT8                           ucSDId;

    /* 当前不需要下载CALL DISCONNECT事件 */
    if (VOS_FALSE == g_stSTKEventState.OP_CallDisconnet)
    {
        STK_WARNING_LOG("SI_STK_CallDisconnectEventDownload: The Event is not Need Download");
        return;
    }

    VOS_MemSet(&stEvent, 0, sizeof(SI_STK_CALL_DISCONNECTED_EVENT_STRU));

    /* 对TI赋值 */
    stEvent.OP_TransactionId            = VOS_TRUE;
    stEvent.TransactionId.ulLen         = sizeof(pstMsg->stCallDiscEventInfo.ucTi);
    stEvent.TransactionId.pucTransaction= &pstMsg->stCallDiscEventInfo.ucTi;

    /* 设置Source Device ID，只有远端挂断是NETWORK，其它都是TERMINAL */
    if (VOS_FALSE == pstMsg->stCallDiscEventInfo.ucLocalDiscFlg)
    {
        ucSDId = SI_STK_DEV_NETWORK;
    }
    else
    {
        ucSDId = SI_STK_DEV_TERMINAL;
    }

    /* 设置挂断原因值，即使CAUSE的长度为0时也要在EVENT中带下去 */
    if (VOS_TRUE == pstMsg->stCallDiscEventInfo.stCause.ucExistFlg)
    {
        stEvent.OP_Cause        = VOS_TRUE;
        stEvent.Cause.ulLen     = pstMsg->stCallDiscEventInfo.stCause.ucLength;
        stEvent.Cause.pucCause  = pstMsg->stCallDiscEventInfo.stCause.aucCause;
    }

    SI_STK_EventDownload(&stEvent, sizeof(SI_STK_CALL_DISCONNECTED_EVENT_STRU), SI_STK_EVENT_CALL_DISCONNECTED, ucSDId);

    return;
}


VOS_VOID SI_STK_LocationStatusEventDownload(VOS_VOID)
{
    SI_STK_LOCATION_STATUS_EVENT_STRU   stEvent;
    SI_STK_NAS_INFO_STRU                stSysInfo;
    VOS_UINT8                           aucLociInfo[50] = {0};
    VOS_UINT32                          ulLen;

    /* 当前不需要下载LOCATION STATUS事件 */
    if (VOS_FALSE == g_stSTKEventState.OP_LociStatus)
    {
        STK_WARNING_LOG("SI_STK_LocationStatusEventDownload: The Event is not Need Download");

        return;
    }

    if (VOS_FALSE == SI_STK_CheckCardStatus())
    {
        STK_WARNING_LOG("SI_STK_LocationStatusEventDownload: The Card Status is not available");

        return;
    }

    VOS_MemSet(&stEvent, 0, sizeof(SI_STK_LOCATION_STATUS_EVENT_STRU));

    STK_GetSysInfo(&stSysInfo.stSysInfo);

    stSysInfo.enServiceStatus = STK_GetSvcStatus();

    stEvent.OP_LocStatus = VOS_TRUE;

    /* 无服务状态处理，NAS的无服务、无IMSI和沉睡都是无服务 */
    if (SI_STK_NO_SERVICE == stSysInfo.enServiceStatus)
    {
        stEvent.LocStatus.ucLocStatus   = SI_STK_NO_SERVICE;
    }
    /* 正常服务 */
    else if (SI_STK_NORMAL_SERVICE == stSysInfo.enServiceStatus)
    {
        stEvent.LocStatus.ucLocStatus   = SI_STK_NORMAL_SERVICE;

        stEvent.OP_LocInfo              = VOS_TRUE;
    }
    /* 限制服务 */
    else
    {
        stEvent.LocStatus.ucLocStatus   = SI_STK_LIMITED_SERVICE;
    }

    if (VOS_TRUE == stEvent.OP_LocInfo)
    {
        ulLen = SI_STK_ProvideLocalInfo_NAA(aucLociInfo, &stSysInfo);

        STK_NORMAL_LOG1("SI_STK_LocationStatusEventDownload: ulLen is %d", (VOS_INT32)ulLen);

        stEvent.LocInfo.ulLen = aucLociInfo[STK_TERMINAL_RSP_LEN + 1];

        stEvent.LocInfo.pucATSLI = aucLociInfo + STK_TERMINAL_RSP_LEN + 2;
    }

    SI_STK_EventDownload(&stEvent, sizeof(SI_STK_LOCATION_STATUS_EVENT_STRU), SI_STK_EVENT_LOCATION_STATUS, ((SI_STK_ENVELOPE_LOCI_RESEND<<8)|SI_STK_DEV_TERMINAL));

    return;
}


VOS_VOID SI_STK_AccessTechChangeEventDownload(VOS_VOID)
{
    SI_STK_ACCESS_TECHNOLOGY_CHANGE_EVENT_STRU              stEvent;

    /* 当前不需要下载ACCESS TECHNOLOGY CHANGE事件 */
    if (VOS_FALSE == g_stSTKEventState.OP_AccChange)
    {
        STK_WARNING_LOG("SI_STK_AccessTechChangeEventDownload: The Event is not Need Download");
        return;
    }

    if (VOS_FALSE == SI_STK_CheckCardStatus())
    {
        STK_WARNING_LOG("SI_STK_LocationStatusEventDownload: The Card Status is not available");

        return;
    }

    VOS_MemSet(&stEvent, 0, sizeof(SI_STK_ACCESS_TECHNOLOGY_CHANGE_EVENT_STRU));

    stEvent.OP_AccessTech       = VOS_TRUE;
    stEvent.AccessTech.ucTech   = STK_GetCurRat();

    SI_STK_EventDownload(&stEvent, sizeof(SI_STK_ACCESS_TECHNOLOGY_CHANGE_EVENT_STRU), SI_STK_EVENT_ACCESS_TECHN_CHANGE, ((SI_STK_ENVELOPE_ACC_RESEND<<8)|SI_STK_DEV_TERMINAL));

    return;
}
VOS_VOID SI_STK_NetworkSearchModeChangeEventDownload(VOS_VOID)
{
    SI_STK_NETWORK_SEARCH_MODE_CHANGE_EVENT_STRU            stEvent;

    /* 当前不需要下载NETWORK SEARCH CHANGE事件 */
    if (VOS_FALSE == g_stSTKEventState.OP_NetSearchChg)
    {
        STK_WARNING_LOG("SI_STK_NetworkSearchModeChangeEventDownload: The Event is not Need Download");
        return;
    }

    VOS_MemSet(&stEvent, 0, sizeof(SI_STK_NETWORK_SEARCH_MODE_CHANGE_EVENT_STRU));

    stEvent.OP_NSMode               = VOS_TRUE;
    stEvent.NSMode.ucNetSearchMode  = STK_GetSearchMode();

    SI_STK_EventDownload(&stEvent, sizeof(SI_STK_NETWORK_SEARCH_MODE_CHANGE_EVENT_STRU), SI_STK_EVENT_NET_SEARCHMODE_CHANGE, SI_STK_DEV_TERMINAL);

    return;
}
VOS_VOID SI_STK_NetworkRejectionEventDownload(NAS_STK_NETWORK_REJECTION_EVENT_STRU *pstMsg)
{
    SI_STK_NETWORK_REJ_EVENT_STRU       stEvent;
    VOS_UINT8                           aucInfo[20];

    /* 当前不需要下载NETWORK REJECTION事件 */
    if (VOS_FALSE == g_stSTKEventState.OP_NetRej)
    {
        STK_WARNING_LOG("SI_STK_NetworkRejectionEventDownload: The Event is not Need Download");
        return;
    }

    if (VOS_FALSE == SI_STK_CheckCardStatus())
    {
        STK_WARNING_LOG("SI_STK_LocationStatusEventDownload: The Card Status is not available");

        return;
    }

    VOS_MemSet(&stEvent, 0, sizeof(SI_STK_NETWORK_REJ_EVENT_STRU));

    /* 设置接入技术 */
    stEvent.OP_AccTech = VOS_TRUE;
    if (TAF_PH_RAT_GSM == pstMsg->stNetworkRejectionEvent.enRat)
    {
        stEvent.stAccTech.ucTech = STK_ACCESS_TECH_GSM;
    }
    else if (TAF_PH_RAT_WCDMA == pstMsg->stNetworkRejectionEvent.enRat)
    {
        stEvent.stAccTech.ucTech = STK_ACCESS_TECH_UTRAN;
    }
    else
    {
        stEvent.stAccTech.ucTech = STK_ACCESS_TECH_EUTRAN;
    }

    /* 设置拒绝原因 */
    stEvent.OP_RejCauseCode         = VOS_TRUE;
    stEvent.stRejCauseCode.ucCause  = pstMsg->stNetworkRejectionEvent.ucCauseCode;

    /* 设置UPDATE ATTACH TYPE */
    stEvent.OP_UpdateAttachType     = VOS_TRUE;
    SI_STK_TransferUpdateAttachType(&stEvent.stUpdateAttachType.ucTpye, pstMsg->stNetworkRejectionEvent.enUpdateAttachType);
    if (SI_STK_NETWORK_REJ_TYPE_BUTT == stEvent.stUpdateAttachType.ucTpye)
    {
        STK_ERROR_LOG("SI_STK_NetworkRejectionEventDownload: ucTpye is SI_STK_NETWORK_REJ_TYPE_BUTT");
        return;
    }

    /* LAU被拒 */
    if (NAS_STK_LAU_REJECT == pstMsg->stNetworkRejectionEvent.enRejType)
    {
        /* 转换MCC为IMSI的格式 */
        aucInfo[0] =  (VOS_UINT8)(pstMsg->stNetworkRejectionEvent.uRejectInfo.stLacInfo.stCurPlmnID.Mcc  & 0x00000F);
        aucInfo[0] |= (VOS_UINT8)((pstMsg->stNetworkRejectionEvent.uRejectInfo.stLacInfo.stCurPlmnID.Mcc & 0x000F00)>>4);
        aucInfo[1] =  (VOS_UINT8)((pstMsg->stNetworkRejectionEvent.uRejectInfo.stLacInfo.stCurPlmnID.Mcc & 0x0F0000)>>16);

        /* 转换MNC为IMSI的格式 */
        aucInfo[1] |= (VOS_UINT8)((pstMsg->stNetworkRejectionEvent.uRejectInfo.stLacInfo.stCurPlmnID.Mnc & 0x0F0000)>>12);
        aucInfo[2] =  (VOS_UINT8)(pstMsg->stNetworkRejectionEvent.uRejectInfo.stLacInfo.stCurPlmnID.Mnc  & 0x00000F);
        aucInfo[2] |= (VOS_UINT8)((pstMsg->stNetworkRejectionEvent.uRejectInfo.stLacInfo.stCurPlmnID.Mnc & 0x000F00)>>4);

        /* 转换LAC的格式 */
        aucInfo[3] = (VOS_UINT8)((pstMsg->stNetworkRejectionEvent.uRejectInfo.stLacInfo.usLac & 0xFF00)>>8);
        aucInfo[4] = (VOS_UINT8)(pstMsg->stNetworkRejectionEvent.uRejectInfo.stLacInfo.usLac  & 0x00FF);

        stEvent.OP_LocaInfo         = VOS_TRUE;
        stEvent.stLocaInfo.pucATSLI = aucInfo;
        stEvent.stLocaInfo.ulLen    = 0x05;
    }
    /* RAU被拒 */
    else if ((NAS_STK_ATTACH_REJECT == pstMsg->stNetworkRejectionEvent.enRejType)
          || (NAS_STK_RAU_REJECT == pstMsg->stNetworkRejectionEvent.enRejType))
    {
        /* 转换MCC为IMSI的格式 */
        aucInfo[0] =  (VOS_UINT8)(pstMsg->stNetworkRejectionEvent.uRejectInfo.stRauInfo.stCurPlmnID.Mcc  & 0x00000F);
        aucInfo[0] |= (VOS_UINT8)((pstMsg->stNetworkRejectionEvent.uRejectInfo.stRauInfo.stCurPlmnID.Mcc & 0x000F00)>>4);
        aucInfo[1] =  (VOS_UINT8)((pstMsg->stNetworkRejectionEvent.uRejectInfo.stRauInfo.stCurPlmnID.Mcc & 0x0F0000)>>16);

        /* 转换MNC为IMSI的格式 */
        aucInfo[1] |= (VOS_UINT8)((pstMsg->stNetworkRejectionEvent.uRejectInfo.stRauInfo.stCurPlmnID.Mnc & 0x0F0000)>>12);
        aucInfo[2] =  (VOS_UINT8)(pstMsg->stNetworkRejectionEvent.uRejectInfo.stRauInfo.stCurPlmnID.Mnc  & 0x00000F);
        aucInfo[2] |= (VOS_UINT8)((pstMsg->stNetworkRejectionEvent.uRejectInfo.stRauInfo.stCurPlmnID.Mnc & 0x000F00)>>4);

        /* 转换LAC的格式 */
        aucInfo[3] = (VOS_UINT8)((pstMsg->stNetworkRejectionEvent.uRejectInfo.stRauInfo.usLac & 0xFF00)>>8);
        aucInfo[4] = (VOS_UINT8)(pstMsg->stNetworkRejectionEvent.uRejectInfo.stRauInfo.usLac  & 0x00FF);

        /* 转换RAC格式 */
        aucInfo[5] = pstMsg->stNetworkRejectionEvent.uRejectInfo.stRauInfo.ucRac;

        stEvent.OP_RauInfo          = VOS_TRUE;
        stEvent.stRauInfo.pucRauInfo= aucInfo;
        stEvent.stRauInfo.ulLen     = 0x06;
    }
    /* 暂时不支持TAC的处理 */
    else
    {
        STK_WARNING_LOG("SI_STK_NetworkRejectionEventDownload: NAS_STK_TAU_REJECT is not support!");
        return;
    }

    SI_STK_EventDownload(&stEvent, sizeof(SI_STK_NETWORK_REJ_EVENT_STRU), SI_STK_EVENT_NETWORK_REJECTION, SI_STK_DEV_NETWORK);

    return;
}


VOS_VOID SI_STK_UpdateLocationInfo(NAS_STK_LOC_STATUS_EVENT_INFO_STRU   *pstLocStatusEvent)
{
    NAS_STK_SYS_INFO_STRU               stSysInfo;
    VOS_UINT8                           ucCurRet;

    /* 把当前技术技术保存下来 */
    ucCurRet = STK_GetCurRat();

    STK_UpdateSvcStatus(pstLocStatusEvent->enServiceStatus);

    /* 无服务状态处理，无服务时把其它所有的系统信息全部清除 */
    if ((NAS_STK_SERVICE_STATUS_NO_SERVICE == pstLocStatusEvent->enServiceStatus)
     || (NAS_STK_SERVICE_STATUS_DEEP_SLEEP == pstLocStatusEvent->enServiceStatus))
    {
        /* 上面STK_UpdateCsSvcStatus会在无服务的状态下把系统信息都清除 */
        STK_UpdateCurRat(pstLocStatusEvent->enRat);
    }
    /* 限制服务&正常服务 */
    else
    {
        stSysInfo.enRat      = pstLocStatusEvent->enRat;
        stSysInfo.stPlmn.Mcc = pstLocStatusEvent->stPlmn.Mcc;
        stSysInfo.stPlmn.Mnc = pstLocStatusEvent->stPlmn.Mnc;
        stSysInfo.ulCellId   = pstLocStatusEvent->ulCellId;
        stSysInfo.usLac      = pstLocStatusEvent->usLac;
        stSysInfo.enUtranMode= g_stStkSysInfo.stSysInfo.enUtranMode;

        STK_UpdateSysInfo(&stSysInfo);
    }

    /* 比较之前的接入技术和现在的接入技术是否相同 */
    if (ucCurRet != STK_GetCurRat())
    {
        SI_STK_AccessTechChangeEventDownload();
    }

    return;
}
VOS_VOID SI_STK_TransferUpdateAttachType(VOS_UINT8 *pucType, NAS_STK_UPDATE_TYPE_ENUM_UINT8 ucNasType)
{
    VOS_UINT8                               ucLoop;

    /* UPDATE/ATTACH转换映射表，目前只支持GU */
    STK_UPDATA_ATTACH_TYPE_TRANSFER_STRU    astTypeTransfer[] =
        {
            {NAS_STK_LUT_NORMAL_LU,              SI_STK_NETWORK_REJ_TYPE_NORMAL_LOCATION_UPDATING,                0},
            {NAS_STK_LUT_PERIODIC_UPDATING,      SI_STK_NETWORK_REJ_TYPE_PERIODIC_UPDATING_LAU,                   0},
            {NAS_STK_LUT_IMSI_ATTACH,            SI_STK_NETWORK_REJ_TYPE_IMSI_ATTACH,                             0},
            {NAS_STK_ATTACH_COMBINED,            SI_STK_NETWORK_REJ_TYPE_COMBINED_GPRS_IMSI_ATTACH,               0},
            {NAS_STK_ATTACH_WHILE_IMSI_ATTACHED, SI_STK_NETWORK_REJ_TYPE_COMBINED_GPRS_IMSI_ATTACH,               0},
            {NAS_STK_ATTACH_NORMAL,              SI_STK_NETWORK_REJ_TYPE_GPRS_ATTACH,                             0},
            {NAS_STK_ATTACH_NORMAL_CS_TRANS,     SI_STK_NETWORK_REJ_TYPE_GPRS_ATTACH,                             0},
            {NAS_STK_RAU_COMBINED,               SI_STK_NETWORK_REJ_TYPE_COMBINED_RALA_UPDATING,                  0},
            {NAS_STK_RAU_WITH_IMSI_ATTACH,       SI_STK_NETWORK_REJ_TYPE_COMBINED_RALA_UPDATING_WITH_IMSI_ATTACH, 0},
            {NAS_STK_RAU_NORMAL,                 SI_STK_NETWORK_REJ_TYPE_RA_UPDATING,                             0},
            {NAS_STK_RAU_NORMAL_CS_TRANS,        SI_STK_NETWORK_REJ_TYPE_RA_UPDATING,                             0},
            {NAS_STK_RAU_NORMAL_CS_UPDATED,      SI_STK_NETWORK_REJ_TYPE_RA_UPDATING,                             0},
            {NAS_STK_RAU_PERIODIC_UPDATING,      SI_STK_NETWORK_REJ_TYPE_PERIODIC_UPDATING_RAU,                   0}
        };

    /* 初始化*pucType为无效值 */
    *pucType = SI_STK_NETWORK_REJ_TYPE_BUTT;

    /* 遍历转换表 */
    for (ucLoop = 0; ucLoop < (sizeof(astTypeTransfer) / sizeof(STK_UPDATA_ATTACH_TYPE_TRANSFER_STRU)); ucLoop++)
    {
        if (astTypeTransfer[ucLoop].enOrigType == ucNasType)
        {
            *pucType = astTypeTransfer[ucLoop].enDestType;
            break;
        }
    }

    return;
}
VOS_VOID SI_STK_LociInfoIndMsgProc(NAS_STK_LOCATION_INFO_IND_STRU *pstLociInfo)
{
    VOS_UINT8                           ucCurRet;

    STK_UpdateCsSvcStatus(pstLociInfo->stServiceStatus.enCsServiceStatus);

    /* 如果是无服务，所有系统信息都被清除，不再处理直接返回 */
    if (SI_STK_NO_SERVICE == STK_GetCsSvcStatus())
    {
        return;
    }

    ucCurRet = STK_GetCurRat();

    STK_UpdateSysInfo(&pstLociInfo->stSysInfo);

    /* 比较之前的接入技术和现在的接入技术是否相同 */
    if (ucCurRet != STK_GetCurRat())
    {
        SI_STK_AccessTechChangeEventDownload();
    }

    return;
}

#ifndef COMM_ITT


VOS_UINT8 SI_STK_SsOrUssdEvtUssReqCnfDataProc(
    VOS_UINT8                          *pucText,
    VOS_UINT8                           ucDcs,
    VOS_UINT8                           ucDateLen,
    VOS_UINT8                          *pucDate)
{
    VOS_UINT8                           ucTextStringLen;
    VOS_UINT8                           ucTag;
    VOS_UINT8                          *pucDest;

    ucTag = TEXT_STRING_TAG|STK_DATA_TAG_MASK;

    pucText[0] = ucTag;

    /* TEXT String数据结构长度默认按TAG+LEN+DCS+数据来计算 */
    ucTextStringLen = sizeof(ucTag) + ucDateLen + sizeof(ucDcs) + sizeof(ucDateLen);

    /* 如果数据长度+DCS大于0x7F，长度要变为2字节 */
    if ((ucDateLen + sizeof(ucDcs)) > 0x7F)
    {
        pucText[1] = 0x81;
        pucText[2] = ucDateLen + sizeof(ucDcs);
        pucText[3] = ucDcs;

        pucDest = pucText + 4;

        ucTextStringLen++;
    }
    else
    {
        pucText[1] = ucDateLen + sizeof(ucDcs);
        pucText[2] = ucDcs;

        pucDest = pucText + 3;
    }

    VOS_MemCpy(pucDest, pucDate, ucDateLen);

    return ucTextStringLen;
}


VOS_VOID SI_STK_SsOrUssdEvtUssReqCnfProc(
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU *pstEvent,
    VOS_UINT8                           aucTr[],
    VOS_UINT8                          *pucLen)
{
    MN_MSG_CBDCS_CODE_STRU              stDcsInfo;
    VOS_UINT32                          ulSeptetLen;
    VOS_UINT32                          ulOctetLen;
    VOS_UINT8                           aucUnpackedChar[260]={0};
    VOS_UINT8                           aucPackedChar[260] = {0};
    VOS_UINT8                           ucDcs;

    VOS_MemSet(&stDcsInfo, 0, sizeof(stDcsInfo));
    if (VOS_OK != MN_MSG_DecodeCbsDcs(pstEvent->DataCodingScheme, pstEvent->UssdString.aucUssdStr, pstEvent->UssdString.usCnt, &stDcsInfo))
    {
        return;
    }

    SI_STK_UssdDcsHook(pstEvent->DataCodingScheme,
                       pstEvent->UssdString.aucUssdStr,
                       pstEvent->UssdString.usCnt,
                       &stDcsInfo);

    if (MN_MSG_MSG_CODING_7_BIT == stDcsInfo.enMsgCoding)
    {
        ucDcs = SI_STK_TEXT_STRING_DCS_7BIT;
    }
    else if (MN_MSG_MSG_CODING_8_BIT == stDcsInfo.enMsgCoding)
    {
        ucDcs = SI_STK_TEXT_STRING_DCS_8BIT;
    }
    else
    {
        ucDcs = SI_STK_TEXT_STRING_DCS_UCS;
    }

    if (VOS_NULL != stDcsInfo.ucLangIndLen)
    {
        if (MN_MSG_MSG_CODING_7_BIT == stDcsInfo.enMsgCoding)
        {
            ulSeptetLen = (pstEvent->UssdString.usCnt * 8)/7;

            VOS_MemSet(aucUnpackedChar, 0, sizeof(aucUnpackedChar));

            if (VOS_OK != TAF_STD_UnPack7Bit(pstEvent->UssdString.aucUssdStr, ulSeptetLen, 0, aucUnpackedChar))
            {
                return;
            }

            ulOctetLen = 0;

            VOS_MemSet(aucPackedChar, 0, sizeof(aucPackedChar));

            if (VOS_OK != TAF_STD_Pack7Bit(aucUnpackedChar+3, ulSeptetLen-3, 0, aucPackedChar, &ulOctetLen))
            {
                return;
            }

            *pucLen += SI_STK_SsOrUssdEvtUssReqCnfDataProc(aucTr, ucDcs, (VOS_UINT8)ulOctetLen, aucPackedChar);
        }
        /* UNICODE编码或8BIT编码 */
        else
        {
            *pucLen += SI_STK_SsOrUssdEvtUssReqCnfDataProc(aucTr, ucDcs, (VOS_UINT8)(pstEvent->UssdString.usCnt - 2), pstEvent->UssdString.aucUssdStr+2);
        }
    }
    else
    {
        *pucLen += SI_STK_SsOrUssdEvtUssReqCnfDataProc(aucTr, ucDcs, (VOS_UINT8)(pstEvent->UssdString.usCnt), pstEvent->UssdString.aucUssdStr);
    }

    return;
}
VOS_VOID SI_STK_SsEvtErrProc(
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU *pstEvent,
    VOS_UINT8                           aucTr[],
    VOS_UINT8                          *pucLen)
{
    *pucLen = 2;

    /* 如果TAF回复的错误码大于250，是网络侧带回的，需要减去250以满足STK需求 */
    if (TAF_ERR_SS_BASE < pstEvent->ErrorCode)
    {
        aucTr[0] = SS_RETURN_ERROR;
        aucTr[1] = (VOS_UINT8)(pstEvent->ErrorCode - TAF_ERR_SS_BASE);
    }
    /* 当前有SS正在发送中 */
    else if (TAF_ERR_BUSY_ON_SS == pstEvent->ErrorCode)
    {
        aucTr[0] = TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND;
        aucTr[1] = ME_CURRENTLY_BUSY_ON_SS_TRANSACTION;
    }
    /* 当前有USSD正在发送中 */
    else if (TAF_ERR_BUSY_ON_USSD == pstEvent->ErrorCode)
    {
        aucTr[0] = TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND;
        aucTr[1] = ME_CURRENTLY_BUSY_ON_USSD_TRANSACTION;
    }
    else if (TAF_ERR_USSD_NET_TIMEOUT == pstEvent->ErrorCode)
    {
        aucTr[0] = NETWORK_CURRENTLY_UNABLE_TO_PROCESS_COMMAND;
        aucTr[1] = NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_NETWORK;
    }
    else if (TAF_ERR_USSD_USER_TIMEOUT == pstEvent->ErrorCode)
    {
        aucTr[0] = TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND;
        aucTr[1] = NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_TERMINAL;
    }
    /* 业务被用户终止 */
    else if (TAF_ERR_USSD_TERMINATED_BY_USER == pstEvent->ErrorCode)
    {
        aucTr[0] = SS_TRANS_TERMINATED_BY_USER;

        *pucLen = 1;
    }
    else
    {
        aucTr[0] = TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND;
        aucTr[1] = NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_TERMINAL;
    }

    STK_ERROR_LOG1("SI_STK_SsCallBackSsEvtErrProc:ErrorCode %d",(VOS_INT)pstEvent->ErrorCode);

    return;
}


VOS_VOID SI_STK_SsEvtProblemProc(
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU *pstEvent,
    VOS_UINT8                           aucTr[],
    VOS_UINT8                          *pucLen)
{
    /* 不需要减250，ADDITIONAL原因值为NO_SPECIFIC_CAUSE_CAN_BE_GIVEN */
    aucTr[0] = SS_RETURN_ERROR;
    aucTr[1] = NO_SPECIFIC_CAUSE_CAN_BE_GIVEN;

    *pucLen = 2;

    STK_ERROR_LOG1("SI_STK_SsCallBackSsEvtProblemProc:ProblemCode %d",(VOS_INT)pstEvent->ProblemCode);

    return;
}
VOS_VOID SI_STK_SsEvtCauseProc(
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU *pstEvent,
    VOS_UINT8                           aucTr[],
    VOS_UINT8                          *pucLen)
{
    /* 不需要减250，ADDITIONAL原因值为TAF事件上报的CAUSE值，不需要减250 */
    aucTr[0] = SS_RETURN_ERROR;
    aucTr[1] = pstEvent->Cause;

    *pucLen = 2;

    STK_ERROR_LOG1("SI_STK_SsCallBackSsEvtCauseProc:CauseCode %d",(VOS_INT)pstEvent->Cause);

    return;
}
VOS_VOID SI_STK_SsEvtCnfProc(
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU *pstEvent,
    VOS_UINT8                           aucTr[],
    VOS_UINT8                          *pucLen)
{
    *pucLen = 1;    /*至少有一个字节的返回值*/

    /* 指示有数据，拷贝数据至回复码流 */
    if (VOS_TRUE == pstEvent->OP_UnParsePara)
    {
        aucTr[1] = g_ucSendSSType;                      /* 记录SS操作类型 */

        *pucLen = pstEvent->UnParsePara.ucCnt + 2;      /* 包括数据类型和结果，和后面额外数据 */

        VOS_MemCpy(aucTr + 2, pstEvent->UnParsePara.aucUnParsePara, pstEvent->UnParsePara.ucCnt);
    }

    /* 指示有错误，记下错误码 */
    if (VOS_TRUE == pstEvent->OP_Error)
    {
        STK_ERROR_LOG1("SI_STK_SsCallBackSsEvtCnfProc: Error %d",(VOS_INT)pstEvent->ErrorCode);

        *pucLen = 2;

        aucTr[0] = SS_RETURN_ERROR;

        if (pstEvent->ErrorCode > TAF_ERR_SS_BASE)
        {
            /* 发生错误，减去NAS加的250个字节偏移 */
            aucTr[1] = (VOS_UINT8)(pstEvent->ErrorCode - TAF_ERR_SS_BASE);
        }
        else
        {
            aucTr[1] = (VOS_UINT8)pstEvent->ErrorCode;
        }
    }
    else
    {
        aucTr[0] = COMMAND_PERFORMED_SUCCESSFULLY;     /* 命令执行成功 */
    }

    return;
}
VOS_VOID SI_STK_SendSsCnfProc(TAF_SS_CALL_INDEPENDENT_EVENT_STRU *pstEvent)
{
    VOS_UINT8                           aucTemp[255]    = {0};
    VOS_UINT8                           aucResult[3]    = {0};
    VOS_UINT8                           ucResultLen     = 0;
    VOS_UINT8                           ucStringLen     = 0;
    VOS_UINT8                           *pucResult      = aucResult;

    switch (pstEvent->SsEvent)
    {
        case TAF_SS_EVT_ERASESS_CNF:
        case TAF_SS_EVT_REGISTERSS_CNF:
        case TAF_SS_EVT_ACTIVATESS_CNF:
        case TAF_SS_EVT_DEACTIVATESS_CNF:
        case TAF_SS_EVT_REG_PASSWORD_CNF:
        case TAF_SS_EVT_INTERROGATESS_CNF:
            pucResult = aucTemp;
            SI_STK_SsEvtCnfProc(pstEvent, pucResult, &ucResultLen);
            break;

        case TAF_SS_EVT_PROCESS_USS_REQ_CNF:
        case TAF_SS_EVT_ERROR:
        case TAF_SS_EVT_PROBLEM:
        case TAF_SS_EVT_USS_RELEASE_COMPLETE_IND:
            if (VOS_TRUE == pstEvent->OP_ProblemCode)
            {
                SI_STK_SsEvtProblemProc(pstEvent, aucResult, &ucResultLen);
            }
            else if (VOS_TRUE == pstEvent->OP_Error)
            {
                SI_STK_SsEvtErrProc(pstEvent, aucResult, &ucResultLen);
            }
            else if (VOS_TRUE == pstEvent->OP_Cause)
            {
                SI_STK_SsEvtCauseProc(pstEvent, aucResult, &ucResultLen);
            }
            else
            {
                ucResultLen = 0x01;
                aucResult[0]= COMMAND_PERFORMED_SUCCESSFULLY;
            }

            if (VOS_TRUE == pstEvent->OP_UssdString)
            {
                SI_STK_SsOrUssdEvtUssReqCnfProc(pstEvent, aucTemp, &ucStringLen);
            }

            if (VOS_TRUE == pstEvent->OP_USSData)
            {
                SI_STK_SsOrUssdEvtUssReqCnfProc(pstEvent, aucTemp, &ucStringLen);
            }

            break;

        default:
            STK_ERROR_LOG("SI_STK_SendSsCnfProc:Unknown EVT Type");
            return;
    }

    SI_STKSendTRWithData(&gstUsatCmdDecode.CmdDetail, ucResultLen, pucResult, ucStringLen, aucTemp);

    return;
}


VOS_VOID SI_STK_UssdEvtErrProc(
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU *pstEvent,
    VOS_UINT8                           aucTr[],
    VOS_UINT8                          *pucLen)
{
    *pucLen = 2;

    /* 如果TAF回复的错误码大于250，是网络侧带回的，需要减去250以满足STK需求 */
    if (pstEvent->ErrorCode > TAF_ERR_SS_BASE)
    {
        aucTr[0] = USSD_RETURN_ERROR;
        aucTr[1] = (VOS_UINT8)(pstEvent->ErrorCode - TAF_ERR_SS_BASE);
    }
    /* 当前有SS正在发送中 */
    else if (TAF_ERR_BUSY_ON_SS == pstEvent->ErrorCode)
    {
        aucTr[0] = TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND;
        aucTr[1] = ME_CURRENTLY_BUSY_ON_SS_TRANSACTION;
    }
    /* 当前有USSD正在发送中 */
    else if (TAF_ERR_BUSY_ON_USSD == pstEvent->ErrorCode)
    {
        aucTr[0] = TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND;
        aucTr[1] = ME_CURRENTLY_BUSY_ON_USSD_TRANSACTION;
    }
    else if (TAF_ERR_USSD_NET_TIMEOUT == pstEvent->ErrorCode)
    {
        aucTr[0] = NETWORK_CURRENTLY_UNABLE_TO_PROCESS_COMMAND;
        aucTr[1] = NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_NETWORK;
    }
    else if (TAF_ERR_USSD_USER_TIMEOUT == pstEvent->ErrorCode)
    {
        aucTr[0] = TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND;
        aucTr[1] = NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_TERMINAL;
    }
    /* 业务被用户终止 */
    else if (TAF_ERR_USSD_TERMINATED_BY_USER == pstEvent->ErrorCode)
    {
        aucTr[0] = SS_TRANS_TERMINATED_BY_USER;

        *pucLen = 1;
    }
    else
    {
        aucTr[0] = TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND;
        aucTr[1] = NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_TERMINAL;
    }

    STK_ERROR_LOG1("SI_STK_UssdCallBackSsEvtErrProc:TAF_SS_EVT_ERROR Error %d",(VOS_INT)pstEvent->ErrorCode);

    return;
}


VOS_VOID SI_STK_UssdEvtProblemProc(
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU *pstEvent,
    VOS_UINT8                           aucTr[],
    VOS_UINT8                          *pucLen)
{
    /* 不需要减250，ADDITIONAL原因值为NO_SPECIFIC_CAUSE_CAN_BE_GIVEN */
    aucTr[0] = USSD_RETURN_ERROR;
    aucTr[1] = NO_SPECIFIC_CAUSE_CAN_BE_GIVEN;

    *pucLen = 2;

    STK_ERROR_LOG1("SI_STK_UssdCallBackSsEvtProblemProc:ProblemCode %d",(VOS_INT)pstEvent->ProblemCode);

    return;
}
VOS_VOID SI_STK_UssdEvtCauseProc(
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU *pstEvent,
    VOS_UINT8                           aucTr[],
    VOS_UINT8                          *pucLen)
{
    /* 不需要减250，ADDITIONAL原因值为TAF事件上报的CAUSE值，不需要减250 */
    aucTr[0] = USSD_RETURN_ERROR;
    aucTr[1] = pstEvent->Cause;

    *pucLen = 2;

    STK_ERROR_LOG1("SI_STK_UssdCallBackSsEvtCauseProc:CauseCode %d",(VOS_INT)pstEvent->Cause);

    return;
}
VOS_VOID SI_STK_SendUssdCnfProc(TAF_SS_CALL_INDEPENDENT_EVENT_STRU *pstEvent)
{
    VOS_UINT8                           aucTemp[255]    = {0};
    VOS_UINT8                           aucResult[3]    = {0};
    VOS_UINT8                           ucResultLen     = 0;
    VOS_UINT8                           ucStringLen     = 0;

    switch (pstEvent->SsEvent)
    {
        case TAF_SS_EVT_PROCESS_USS_REQ_CNF:
        case TAF_SS_EVT_ERROR:
        case TAF_SS_EVT_PROBLEM:
        case TAF_SS_EVT_USS_RELEASE_COMPLETE_IND:
            if (VOS_TRUE == pstEvent->OP_ProblemCode)
            {
                SI_STK_UssdEvtProblemProc(pstEvent, aucResult, &ucResultLen);
            }
            else if (VOS_TRUE == pstEvent->OP_Error)
            {
                SI_STK_UssdEvtErrProc(pstEvent, aucResult, &ucResultLen);
            }
            else if (VOS_TRUE == pstEvent->OP_Cause)
            {
                SI_STK_UssdEvtCauseProc(pstEvent, aucResult, &ucResultLen);
            }
            else
            {
                ucResultLen = 0x01;
                aucResult[0]= COMMAND_PERFORMED_SUCCESSFULLY;
            }

            if (VOS_TRUE == pstEvent->OP_UssdString)
            {
                SI_STK_SsOrUssdEvtUssReqCnfProc(pstEvent, &aucTemp[0], &ucStringLen);
            }

            if (VOS_TRUE == pstEvent->OP_USSData)
            {
                SI_STK_SsOrUssdEvtUssReqCnfProc(pstEvent, aucTemp, &ucStringLen);
            }

            break;

        default:
            STK_ERROR_LOG("SI_STK_SendUssdCnfProc:Unknown EVT Type");
            return;
    }

    SI_STKSendTRWithData(&gstUsatCmdDecode.CmdDetail, ucResultLen, aucResult, ucStringLen, aucTemp);

    return;
}


VOS_VOID SI_STK_SmsGlobalClear(VOS_VOID)
{
    VOS_MemFree(MAPS_STK_PID, f_pstSendDirectParm);

    if(VOS_NULL_PTR != f_pstStkMsgRawTsDataInfo)
    {
       VOS_MemFree(MAPS_STK_PID, f_pstStkMsgRawTsDataInfo);
    }

    g_ucReceCBNum       = 0;
    g_ucSendMsgNum      = 0;

    return;
}


VOS_VOID SI_STK_SmsCBSendContinue(VOS_VOID)
{
    g_ucReceCBNum++;    /* 回调次数++计数 */

    if (g_ucReceCBNum < g_ucSendMsgNum)  /* 还有短信需要发送  */
    {
        VOS_MemCpy(&f_pstSendDirectParm->stMsgInfo.stTsRawData,
                              (f_pstStkMsgRawTsDataInfo + g_ucReceCBNum),
                              sizeof(f_pstSendDirectParm->stMsgInfo.stTsRawData));

        if(VOS_OK != SI_STK_SendTAFMsg(STK_MSG_SEND_REQ,
                                        STK_OPID_VALUE,
                                        VOS_NULL,
                                        (VOS_UINT8*)f_pstSendDirectParm,
                                        sizeof(MN_MSG_SEND_PARM_STRU)))
        {
            /* 失败可能不会重启，需要回复TR */
            STK_ERROR_LOG("SI_STK_SmsCBSendContinue: fail to send Sms To TAF.");

            SI_STKSimpleResponse(&gstUsatCmdDecode.CmdDetail, COMMAND_BEYOND_TERMINALS_CAPABILITIES);

            SI_STK_SmsGlobalClear();

            return;
        }
        else
        {
            return;     /* 退出去等待下一次的回调 */
        }
    }

    if (g_ucSendMsgNum == g_ucReceCBNum)    /* 所有短信已经发送完毕  */
    {
        SI_STKSimpleResponse(&gstUsatCmdDecode.CmdDetail, VOS_OK);
    }

    SI_STK_SmsGlobalClear();

    return;
}
VOS_VOID SI_STK_SMSCallBackFunc(MN_CLIENT_ID_T      clientId,
                                    MN_MSG_EVENT_ENUM_U32    enEventType,
                                    MN_MSG_EVENT_INFO_STRU   *pstEvent)
{
    VOS_UINT8                           aucRpError[]={SMS_RP_ERROR, 0 , NO_SPECIFIC_CAUSE_CAN_BE_GIVEN};
    VOS_UINT8                           aucRspNet[2] = {NETWORK_CURRENTLY_UNABLE_TO_PROCESS_COMMAND,
                                                        NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_NETWORK};

    if (MN_MSG_EVT_SUBMIT_RPT != enEventType)
    {
        STK_WARNING_LOG1("SI_STK_SMSCallBackFunc:Warning Recevie Error Event %d", (VOS_UINT8)enEventType);

        return;
    }

    /* 如果不支持Android则不需要处理正式响应 */
    if (VOS_FALSE == SI_STK_CheckSupportAP())
    {
        STK_INFO_LOG("SI_STK_SMSCallBackFunc: No need Handle This Msg");

        return;
    }

    /* 正式响应指示发送短信失败，回复TERMINAL RESPONSE指示SEND SMS失败，仅PHONE形态需要处理 */
    if (TAF_MSG_ERROR_NO_ERROR == pstEvent->u.stSubmitRptInfo.enErrorCode)
    {
        SI_STK_SmsCBSendContinue();
    }
    /* 网络侧回复RP ERROR */
    else if (TAF_MSG_ERROR_RP_ERROR_BEGIN == (pstEvent->u.stSubmitRptInfo.enErrorCode & TAF_MSG_ERROR_RP_ERROR_BEGIN))
    {
        aucRpError[1] = (VOS_UINT8)(pstEvent->u.stSubmitRptInfo.enErrorCode & 0x7F);

        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucRpError), aucRpError);

        SI_STK_SmsGlobalClear();
    }
    else
    {
        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucRspNet), aucRspNet);

        SI_STK_SmsGlobalClear();
    }

    return;
}
VOS_VOID SI_STK_SendSmsMnCmdCnfProcFunc(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    TAF_UINT32                          ulErrorCode)
{
    VOS_UINT8 aucCallCtrlNotAllow[2]    = {INTERACTION_WITH_CALL_CONTROL_BY_NAA_PERMANENT_PROBLEM,
                                           ACTION_NOT_ALLOWED};
    VOS_UINT8 aucCallCtrlErr[2]         = {INTERACTION_WITH_CALL_CONTROL_BY_NAA_PERMANENT_PROBLEM,
                                           NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_CALL_CONTROL};
    VOS_UINT8 aucRspData[2]             = {TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND,
                                           NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_TERMINAL};


    if (MN_ERR_NO_ERROR == ulErrorCode)
    {
        /* 如果不支持Android则需要处理正确的临时响应 */
        if (VOS_FALSE == SI_STK_CheckSupportAP())
        {
            STK_INFO_LOG("SI_STK_SendSmsMnCmdCnfCallBackFunc: Send Msg Continue");
            SI_STK_SmsCBSendContinue();
        }

        return;
    }

    if (MN_ERR_CLASS_SMS_MO_CTRL_ACTION_NOT_ALLOWED == ulErrorCode)
    {
        STK_INFO_LOG("SI_STK_SendSmsMnCmdCnfCallBackFunc: Mo Sms Ctrl return Error");
        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucCallCtrlNotAllow), aucCallCtrlNotAllow);
    }
    else if (MN_ERR_CLASS_SMS_MO_CTRL_USIM_PARA_ERROR == ulErrorCode)
    {
        STK_INFO_LOG("SI_STK_SendSmsMnCmdCnfCallBackFunc: Mo Sms Ctrl return Error");
        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucCallCtrlErr), aucCallCtrlErr);
    }
    else
    {
        STK_WARNING_LOG("SI_STK_SendSmsMnCmdCnfCallBackFunc: Other Error");
        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucRspData), aucRspData);
    }

    return;
}
VOS_VOID SI_STK_SetupCallOrigCnfProc(TAF_UINT32 ulErrorCode)
{
    VOS_UINT8 aucCallCtrlNotAllow[2]    = {INTERACTION_WITH_CALL_CONTROL_BY_NAA_PERMANENT_PROBLEM,
                                           ACTION_NOT_ALLOWED};
    VOS_UINT8 aucCallCtrlTypeChg[2]     = {INTERACTION_WITH_CALL_CONTROL_BY_NAA_PERMANENT_PROBLEM,
                                           TYPE_OF_REQUEST_HAS_CHANGED};
    VOS_UINT8 aucSetupCallError[2]      = {TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND,
                                           NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_TERMINAL};

    if (TAF_CS_CAUSE_SUCCESS == ulErrorCode)
    {
        return;
    }

    /* NAS本地消息指示CALL CONTROL不允许发起呼叫 */
    if (TAF_CS_CAUSE_CALL_CTRL_NOT_ALLOWED == ulErrorCode)
    {
        STK_INFO_LOG("SI_STK_SetupCallMnCmdCnfCallBackFunc: Call Ctrl return Error");
        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucCallCtrlNotAllow), aucCallCtrlNotAllow);
    }
    /* NAS本地消息指示CALL CONTROL将类型变更 */
    else if (TAF_CS_CAUSE_CALL_CTRL_BEYOND_CAPABILITY == ulErrorCode)
    {
        STK_INFO_LOG("SI_STK_SetupCallMnCmdCnfCallBackFunc: Call Ctrl Modify the Not Support Data");
        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucCallCtrlTypeChg), aucCallCtrlTypeChg);
    }
    else
    {
        STK_INFO_LOG("SI_STK_SetupCallMnCmdCnfCallBackFunc: Setup call local Error");
        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucSetupCallError), aucSetupCallError);
    }

    /* 如果Duration定时器存在的话，收到本地错误后停掉此定时器 */
    SI_STK_SetUpCallStopDurationTimer(gstUsatCmdDecode.CmdStru.SetupCall.OP_Duration);

    /* SETUP CALL主动命令发起的CALL本地错误，清除DTMF数据 */
    STK_SetCallCleanGobal();

    return;
}


VOS_VOID SI_STK_SendSsOrUssdMnCmdCnfProcFunc(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    TAF_UINT32                          ulErrorCode)
{
    VOS_UINT8 aucCallCtrlNotAllow[2]    = {INTERACTION_WITH_CALL_CONTROL_BY_NAA_PERMANENT_PROBLEM,
                                           ACTION_NOT_ALLOWED};
    VOS_UINT8 aucCallCtrlTypeChg[2]     = {INTERACTION_WITH_CALL_CONTROL_BY_NAA_PERMANENT_PROBLEM,
                                           TYPE_OF_REQUEST_HAS_CHANGED};
    VOS_UINT8 aucCallCtrlOtherErr[2]    = {INTERACTION_WITH_CALL_CONTROL_BY_NAA_PERMANENT_PROBLEM,
                                           NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_CALL_CONTROL};

    if (TAF_CS_CAUSE_SUCCESS == ulErrorCode)
    {
        return;
    }
    /* NAS本地消息指示CALL CONTROL不允许发起SS */
    else if (TAF_CS_CAUSE_CALL_CTRL_NOT_ALLOWED == ulErrorCode)
    {
        STK_INFO_LOG("SI_STK_SendSsMnCmdCnfCallBackFunc: Call Ctrl return Error");
        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucCallCtrlNotAllow), aucCallCtrlNotAllow);
    }
    /* NAS本地消息指示CALL CONTROL将类型变更 */
    else if (TAF_CS_CAUSE_CALL_CTRL_BEYOND_CAPABILITY == ulErrorCode)
    {
        STK_INFO_LOG("SI_STK_SendSsMnCmdCnfCallBackFunc: Call Ctrl Modify the Not Support Data");
        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucCallCtrlTypeChg), aucCallCtrlTypeChg);
    }
    else
    {
        STK_INFO_LOG("SI_STK_SendSsMnCmdCnfCallBackFunc: Call Control other error");
        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucCallCtrlOtherErr), aucCallCtrlOtherErr);
    }

    return;
}


VOS_VOID SI_STK_SendDTMFCnfProc(TAF_CALL_EVT_DTMF_CNF_STRU *pstEvent)
{
    VOS_UINT8 aucResponse[2]    = {TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND,
                                   NOT_IN_SPEECH_CALL};

    if (OAM_CLIENT_ID_STK != pstEvent->usClientId)
    {
        STK_WARNING_LOG("SI_STK_SendDtmfCnfProc: ClientID Error");

        return;
    }

    if (TAF_CS_CAUSE_SUCCESS == pstEvent->enCause)
    {
        STK_NORMAL_LOG("SI_STK_SendDtmfCnfProc: Send DTMF Cnf OK");

        return;
    }

    /* SETUP CALL主动命令发起的SEND DTMF的本地错误，清除DTMF数据，重置状态机 */
    if (STK_SETUP_CALL_OP_ID == pstEvent->opId)
    {
        STK_INFO_LOG("SI_STK_SendDTMFCnfProc: Setup call DTMF Error");

        STK_SetCallCleanGobal();

        return;
    }

    /* 当前没有呼叫的时候收到DTMF主动命令，直接给单板回复原因值 */
    if (TAF_CS_CAUSE_NOT_IN_SPEECH_CALL == pstEvent->enCause)
    {
        STK_WARNING_LOG("SI_STK_SendDtmfCnfProc: TAF_CS_CAUSE_STATE_ERROR");

        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucResponse), aucResponse);
    }
    else
    {
        STK_WARNING_LOG("SI_STK_SendDtmfCnfProc: Other Error");

        SI_STKSimpleResponse(&gstUsatCmdDecode.CmdDetail, SI_STK_NOTSUPPORT);
    }

    STK_ClearPauseCharGobal();

    return;
}


VOS_VOID SI_STK_SetupCallSsMnCmdCnfProcFunc(TAF_UINT32 ulErrorCode)
{
    VOS_UINT8 aucSetupCallError[2]      = {TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND,
                                           NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_TERMINAL};

    if (TAF_CS_CAUSE_SUCCESS == ulErrorCode)
    {
        return;
    }
    else
    {
        STK_SetCallCleanGobal();

        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucSetupCallError), aucSetupCallError);
    }

    return;
}


VOS_VOID SI_STK_MnCmdCnfProcFunc(
    MN_CLIENT_ID_T                      clientId,
    MN_OPERATION_ID_T                   opId,
    TAF_UINT32                          ulErrorCode)
{
    LogPrint1("\r\nSI_STK_MnCmdCnfProcFunc: ulErrorCode is %d", (VOS_INT32)ulErrorCode);

    if (STK_SEND_SMS_OP_ID == opId)
    {
        SI_STK_SendSmsMnCmdCnfProcFunc(clientId, opId, ulErrorCode);
    }
    /* NAS代码调整后这里只处理SETUP CALL主动命令导致的补充业务，如挂起和挂断的临时响应 */
    else if (STK_SETUP_CALL_OP_ID == opId)
    {
        SI_STK_SetupCallSsMnCmdCnfProcFunc(ulErrorCode);
    }
    else if ((STK_SEND_SS_OP_ID == opId)
          || (STK_SEND_USSD_OP_ID == opId))
    {
        SI_STK_SendSsOrUssdMnCmdCnfProcFunc(clientId, opId, ulErrorCode);
    }
    else
    {
        /* make pclint happy */
    }

    return;
}


VOS_VOID SI_STK_SsCnfProcFunc(
    MN_CLIENT_ID_T                      clientId,
    MN_CALL_EVENT_ENUM_U32              enEventType,
    TAF_SS_CALL_INDEPENDENT_EVENT_STRU *pstEvent)
{
    if (SI_STK_SENDSS == gstUsatCmdDecode.SatType)
    {
        SI_STK_SendSsCnfProc(pstEvent);
    }
    else if (SI_STK_SENDUSSD == gstUsatCmdDecode.SatType)
    {
        SI_STK_SendUssdCnfProc(pstEvent);
    }
    else
    {
        return;
    }

    return;
}


VOS_VOID SI_STK_SetupCallStatusTable(
    MN_CALL_EVENT_ENUM_U32              enEventType,
    MN_CALL_INFO_STRU                  *pstEvent)
{
    /* 呼叫ID,ID从1到7 */
    if ((STK_CALL_MAX_NUM < pstEvent->callId)
     || (VOS_NULL == pstEvent->callId))
    {
        STK_WARNING_LOG("SI_STK_SetupCallStatusTable: Call Id error");

        return;
    }

    /* 只有收到RELEASE */
    if (MN_CALL_EVT_RELEASED == enEventType)
    {
        g_stStkSetupCallCtrl.astStkCallStatusTable[pstEvent->callId - 1].enStatus   =   SI_STK_CALL_ID_STATUS_IDLE;
    }
    else
    {
        g_stStkSetupCallCtrl.astStkCallStatusTable[pstEvent->callId - 1].enStatus   =   SI_STK_CALL_ID_STATUS_BUSY;
    }

    return;
}


VOS_VOID SI_STK_CallCBProc(
    MN_CALL_EVENT_ENUM_U32                  enEventType,
    MN_CALL_INFO_STRU                      *pstEvent)
{
    VOS_UINT8   ucCardStatus;
    VOS_UINT8   ucCardType;

    USIMM_GetCardType(&ucCardStatus, &ucCardType);

    if ((USIMM_CARD_ROM_SIM == ucCardType) || (USIMM_CARD_NOCARD == ucCardType))
    {
        return;
    }

    if(MN_CALL_TYPE_EMERGENCY == pstEvent->enCallType)
    {
        return;
    }

    if ((MN_CALL_EVT_ORIG == enEventType)||(MN_CALL_EVT_INCOMING == enEventType))
    {
        SI_PIH_ChangePollTimer(SI_PIH_TIMER_NAME_CALL, SI_PIH_POLLTIMER_ENABLE, SI_PIH_CALL_TIMER_LEN, USIMM_POLL_NEED_FCP);

        SI_PIH_ChangePollTimer(SI_PIH_TIMER_NAME_CHECKSTATUS, SI_PIH_POLLTIMER_DISABLE, SI_PIH_TIMER_LEN_NO_CHANGE, USIMM_POLL_NO_NEED_FCP);
    }

    if (MN_CALL_EVT_RELEASED == enEventType)
    {
        SI_PIH_ChangePollTimer(SI_PIH_TIMER_NAME_CHECKSTATUS, SI_PIH_POLLTIMER_ENABLE, SI_PIH_TIMER_LEN_NO_CHANGE, USIMM_POLL_NO_NEED_FCP);

        SI_PIH_ChangePollTimer(SI_PIH_TIMER_NAME_CALL, SI_PIH_POLLTIMER_DISABLE, SI_PIH_CALL_TIMER_LEN, USIMM_POLL_NEED_FCP);
    }

    return;
}


VOS_VOID SI_STK_SendDTMFResultProc(
    MN_CALL_EVENT_ENUM_U32              enEventType,
    TAF_CALL_EVT_DTMF_CNF_STRU          *pstEvent)
{
    SI_STK_DTMF_SEND_ENUM_UINT32              enResult;

    /* 不属于STK的DTMF */
    if (OAM_CLIENT_ID_STK != pstEvent->usClientId)
    {
        STK_ERROR_LOG1("SI_STK_SendDTMFResultProc: ClinetID Error:", (VOS_INT)pstEvent->usClientId);

        return;
    }

    if (TAF_CS_CAUSE_SUCCESS != pstEvent->enCause)
    {
        STK_WARNING_LOG1("SI_STK_SendDTMFResultProc: TAF Return Error:", (VOS_INT)pstEvent->enCause);

        enResult = SI_STK_DTMF_SEND_REJECT;
    }
    else
    {
        /* START DTMF发送成功，发送STOP DTMF */
        if (MN_CALL_EVT_START_DTMF_RSLT == enEventType)
        {
            enResult = SI_STK_SendStopDtmfMsg((VOS_UINT8)(pstEvent->opId));
        }
        /* STOP DTMF发送成功，发送START DTMF */
        else if (MN_CALL_EVT_STOP_DTMF_RSLT == enEventType)
        {
            if (STK_SETUP_CALL_DTMF_OP_ID == pstEvent->opId)
            {
                enResult = SI_STK_SendDtmfToTAF(STK_SETUP_CALL_DTMF_OP_ID,
                                                g_stStkSetupCallCtrl.ucSetupCallId);
            }
            else
            {
                enResult = SI_STK_SendDtmfToTAF(STK_SEND_DTMF_OP_ID, 0);
            }
        }
        else
        {
            STK_ERROR_LOG1("SI_STK_SendDTMFResultProc: unknow the TAF Event Type", (VOS_INT)enEventType);

            enResult = SI_STK_DTMF_SEND_REJECT;
        }
    }

    if (SI_STK_DTMF_SEND_OK == enResult)
    {
        STK_NORMAL_LOG("SI_STK_SendDTMFResultProc: Send DTMF OK");
        return;
    }

    /* 如果SETUP CALL中的DTMF发送有返回错误或发送已完成，清空DTMF资源，重置状态机 */
    if (STK_SETUP_CALL_DTMF_OP_ID == pstEvent->opId)
    {
        STK_SetCallCleanGobal();
    }

    /* 如果SEND DTMF处理有返回错误或发送已完成，清空DTMF资源 */
    if (STK_SEND_DTMF_OP_ID == pstEvent->opId)
    {
        STK_ClearPauseCharGobal();

        if (SI_STK_DTMF_SEND_FINISH == enResult)  /* DTMF发送完成 */
        {
            SI_STKSimpleResponse(&gstUsatCmdDecode.CmdDetail, SI_STK_OK);
        }
        else
        {
            SI_STKSimpleResponse(&gstUsatCmdDecode.CmdDetail, SI_STK_NOTSUPPORT);
        }
    }

    return;
}


VOS_VOID SI_STK_SaveCallId(MN_CALL_ID_T ucCallId, MN_CLIENT_ID_T usClientId)
{
    /* 参数非法 */
    if ((STK_CALL_MAX_NUM < ucCallId)
     || (VOS_NULL == ucCallId))
    {
        STK_WARNING_LOG("SI_STK_SaveCallId: Call Id invalid");

        return;
    }

    /* 非set up call 发起的呼叫 */
    if (OAM_CLIENT_ID_STK != usClientId)
    {
        STK_WARNING_LOG("SI_STK_SaveCallId: Not STK call");

        return;
    }

    g_stStkSetupCallCtrl.ucSetupCallId = ucCallId;

    return;
}


VOS_VOID SI_STK_CallReleasedProc(MN_CALL_EVENT_ENUM_U32 enEventType, MN_CALL_ID_T ucCallId, VOS_UINT8 ucIsUser)
{
    VOS_UINT32                          ultimerLen;
    VOS_UINT8                           aucRspNet[2] = {NETWORK_CURRENTLY_UNABLE_TO_PROCESS_COMMAND,
                                                        NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_NETWORK};

    if (MN_CALL_EVT_RELEASED != enEventType)
    {
        return;
    }

    /* 非set up call 发起的呼叫 */
    if (g_stStkSetupCallCtrl.ucSetupCallId != ucCallId)
    {
        return;
    }

    /* 只有没收到网络侧connect消息时收到release的情况下才下发terminal response */
    if (SI_STK_CALL_STATUS_WAIT_CONNECT == g_stStkSetupCallCtrl.enCallStatus)
    {
        /* 如果未启动定时器或获取定时器剩余时间失败，在等待CONNECT消息时收到RELEASE消息情况下直接回复TR结果 */
        if (VOS_OK != VOS_GetRelTmRemainTime(&gstSTKSetupCallDurationTimer, &ultimerLen))
        {
            /* 网络挂断 */
            if (VOS_FALSE == ucIsUser)
            {
                SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucRspNet), aucRspNet);
            }
            /* 用户挂断 */
            else
            {
                SI_STKSimpleResponse(&gstUsatCmdDecode.CmdDetail, USER_CLEARED_DOWN_CALL_BEFORE_CONNECTION_OR_NETWORK_RELEASE);
            }

            /* 如果有启动过定时器，把定时器停止 */
            SI_STK_SetUpCallStopDurationTimer(gstUsatCmdDecode.CmdStru.SetupCall.OP_Duration);

            /* 清除当前呼叫状态，关键是把呼叫ID清除 */
            STK_SetCallCleanGobal();
        }
        else
        {
            /* 网络挂断 */
            if (VOS_FALSE == ucIsUser)
            {
                /* 如果当前有DURATION定时器在运行且剩余时间不为0时什么都不做，等定时器超时(GCF用例REDIAL用例1.12) */
            }
            /* 用户挂断 */
            else
            {
                SI_STKSimpleResponse(&gstUsatCmdDecode.CmdDetail, USER_CLEARED_DOWN_CALL_BEFORE_CONNECTION_OR_NETWORK_RELEASE);

                /* 如果有启动过定时器，把定时器停止 */
                SI_STK_SetUpCallStopDurationTimer(gstUsatCmdDecode.CmdStru.SetupCall.OP_Duration);

                /* 清除当前呼叫状态，关键是把呼叫ID清除 */
                STK_SetCallCleanGobal();
            }
        }
    }

    return;
}


VOS_VOID SI_STK_CallConnectProc(MN_CALL_EVENT_ENUM_U32 enEventType, MN_CALL_ID_T ucCallId)
{
    if (MN_CALL_EVT_CONNECT != enEventType)
    {
        return;
    }

    /* 非set up call 发起的呼叫 */
    if (g_stStkSetupCallCtrl.ucSetupCallId != ucCallId)
    {
        return;
    }

    /* 如果Duration定时器存在的话，收到connect后停掉此定时器 */
    SI_STK_SetUpCallStopDurationTimer(gstUsatCmdDecode.CmdStru.SetupCall.OP_Duration);

    SI_STKSimpleResponse(&gstUsatCmdDecode.CmdDetail, VOS_OK);

    /* 当前不需要发送DTMF */
    if (VOS_NULL == g_stSTKDtmfData.ulDtmfLen)
    {
        STK_CallStateMachineSet(SI_STK_CALL_STATUS_WAIT_IDLE);
        return;
    }

    /* 发送DTMF成功，设置状态机 */
    if (SI_STK_DTMF_SEND_OK == SI_STK_SendDtmfToTAF(STK_SETUP_CALL_DTMF_OP_ID, g_stStkSetupCallCtrl.ucSetupCallId))
    {
        STK_CallStateMachineSet(SI_STK_CALL_STATUS_SEND_DTMF);
    }
    else
    {
        STK_SetCallCleanGobal();
    }

    return;
}


VOS_VOID SI_STK_CsCallBackFunc(MN_CALL_EVENT_ENUM_U32    enEventType,
                                      MN_CALL_INFO_STRU         *pstEvent)
{
    switch(enEventType)
    {

        case MN_CALL_EVT_ORIG:
            /* 保存呼叫ID */
            SI_STK_SaveCallId(pstEvent->callId, pstEvent->clientId);
            SI_STK_CallCBProc(enEventType, pstEvent);
            SI_STK_SetupCallStatusTable(enEventType, pstEvent);
            break;

        case MN_CALL_EVT_INCOMING:
        case MN_CALL_EVT_RELEASED:
            SI_STK_CallCBProc(enEventType, pstEvent);
            SI_STK_SetupCallStatusTable(enEventType, pstEvent);
            SI_STK_CallReleasedProc(enEventType, pstEvent->callId, pstEvent->stDiscDir.ucIsUser);
            break;

        case MN_CALL_EVT_CONNECT:
            /* 停定时器，回terminal response，发dtmf */
            SI_STK_CallConnectProc(enEventType, pstEvent->callId);
            SI_STK_SetupCallStatusTable(enEventType, pstEvent);
            break;

        case MN_CALL_EVT_ALERTING:
        case MN_CALL_EVT_CALL_PROC:
        case MN_CALL_EVT_HOLD:
        case MN_CALL_EVT_RETRIEVE:
            SI_STK_SetupCallStatusTable(enEventType, pstEvent);
            break;

        case MN_CALL_EVT_SS_CMD_RSLT:
            SI_STK_SuspendCnfMsgProc(pstEvent);
            break;

        case MN_CALL_EVT_START_DTMF_CNF:
        case MN_CALL_EVT_STOP_DTMF_CNF:
            SI_STK_SendDTMFCnfProc((TAF_CALL_EVT_DTMF_CNF_STRU *)pstEvent);
            break;

        case MN_CALL_EVT_START_DTMF_RSLT:
        case MN_CALL_EVT_STOP_DTMF_RSLT:
            SI_STK_SendDTMFResultProc(enEventType, (TAF_CALL_EVT_DTMF_CNF_STRU *)pstEvent);
            break;

        case MN_CALL_EVT_CALL_ORIG_CNF:
            SI_STK_SetupCallOrigCnfProc(pstEvent->enCause);
            break;

        default:
            break;
    }

    return;
}

#endif


VOS_VOID SI_STK_TAFMsgProc(PS_SI_MSG_STRU *pMsg)
{
    SI_STK_TAFCNF_MSG_STRU              *pstMsg;
    MN_APP_CMD_CNF_STRU                 *pstCnfMsg;

    if(VOS_OK != SI_STK_SVLTECheckMsgPID(pMsg, WUEPS_PID_TAF))
    {
        return;
    }

    pstMsg = (SI_STK_TAFCNF_MSG_STRU *)pMsg;

    switch (pMsg->ulMsgName)
    {
        case TAF_STK_MSG_EVENT_IND:
            SI_STK_SMSCallBackFunc(0, pstMsg->enEventType, (MN_MSG_EVENT_INFO_STRU *)pstMsg->aucEvent);
            break;

        case TAF_STK_CMD_CNF:
            pstCnfMsg = (MN_APP_CMD_CNF_STRU *)pMsg;
            SI_STK_MnCmdCnfProcFunc(pstCnfMsg->stCmdCnfEvent.clientId,
                                    pstCnfMsg->stCmdCnfEvent.opId,
                                    pstCnfMsg->stCmdCnfEvent.ulErrorCode);
            break;

        case TAF_STK_SS_EVENT_IND:
            SI_STK_SsCnfProcFunc(0, pstMsg->enEventType, (TAF_SS_CALL_INDEPENDENT_EVENT_STRU*)pstMsg->aucEvent);
            break;

        case TAF_STK_CS_CALLINFO_EVENT_IND:
            SI_STK_CsCallBackFunc(pstMsg->enEventType, (MN_CALL_INFO_STRU*)pstMsg->aucEvent);
            break;

        case TAF_STK_CS_STATE_IND:
            SI_STK_GetCSModemFunc((TAF_STK_CS_SERVICE_IND *)pMsg);
            break;

        case ID_NAS_STK_MT_CALL_EVENT:
            SI_STK_MtCallEventDownload((NAS_STK_MT_CALL_EVENT_STRU *)pMsg);
            break;

        case ID_NAS_STK_CALL_CONNECT_EVENT:
            SI_STK_CallConnectEventDownload((NAS_STK_CALL_CONNECT_EVENT_STRU *)pMsg);
            break;

        case ID_NAS_STK_CALL_DISC_EVENT:
            SI_STK_CallDisconnectEventDownload((NAS_STK_CALL_DISC_EVENT_STRU *)pMsg);
            break;

        default:
            STK_WARNING_LOG("SI_STK_TAFMsgProc: Rec Unknow Msg");
            break;
    }

    return;
}

/*****************************************************************************
函 数 名  : SI_STK_CSServiceHook
功能描述  : STK数据勾包
输入参数  : 无
输出参数  : 无
返 回 值  : 无
被调函数  :
修订记录  :
1.日    期  : 2012年05月02日
  作    者  : fuyingjun
  修改内容  : Create
*****************************************************************************/
VOS_VOID SI_STK_HookData(VOS_UINT32 ulHookName, VOS_UINT32 ulDataLen, VOS_UINT8 *pucData)
{
    SI_HOOKMSG_STRU                     *pstMsg;

    pstMsg = (SI_HOOKMSG_STRU *)VOS_MemAlloc(MAPS_STK_PID, DYNAMIC_MEM_PT, sizeof(SI_HOOKMSG_STRU) + ulDataLen - 4);

    if (VOS_NULL_PTR == pstMsg)
    {
        return;
    }

    pstMsg->ulSenderCpuId     = VOS_LOCAL_CPUID;
    pstMsg->ulSenderPid       = MAPS_STK_PID;
    pstMsg->ulReceiverCpuId   = VOS_LOCAL_CPUID;
    pstMsg->ulReceiverPid     = MAPS_STK_PID;
    pstMsg->ulLength          = ulDataLen + sizeof(VOS_UINT32);
    pstMsg->ulMsgName         = ulHookName;

    /*拷贝数据内容*/
    VOS_MemCpy(pstMsg->aucData, pucData, ulDataLen);

    OM_TraceMsgHook(pstMsg);

    VOS_MemFree(MAPS_STK_PID, pstMsg);

    return;
}

/*****************************************************************************
函 数 名  : SI_STK_CSServiceHook
功能描述  : STK数据勾包
输入参数  : 无
输出参数  : 无
返 回 值  : 无
被调函数  :
修订记录  :
1.日    期  : 2012年05月02日
  作    者  : fuyingjun
  修改内容  : Create
*****************************************************************************/
VOS_VOID SI_STK_CSServiceHook(VOS_VOID)
{
    VOS_UINT32          ulCsServiceStatus;

    ulCsServiceStatus = STK_GetCsSvcStatus();

    SI_STK_HookData(STK_TAF_CS_SERVICE_STATUS, sizeof(VOS_UINT32), (VOS_UINT8*)&ulCsServiceStatus);

    return;
}


VOS_VOID SI_STK_UssdDcsHook(VOS_UINT8 ucInputDcs, VOS_UINT8 *pUssdString, VOS_UINT16 usStringLen, MN_MSG_CBDCS_CODE_STRU *pstCbDcs)
{
    STK_CBS_DCS_STRU                   *pstCbsDcs;
    VOS_UINT32                          ulDataLen;

    ulDataLen = sizeof(STK_CBS_DCS_STRU) + usStringLen - 4;

    pstCbsDcs = (STK_CBS_DCS_STRU *)VOS_MemAlloc(MAPS_STK_PID, DYNAMIC_MEM_PT, ulDataLen);

    if (VOS_NULL_PTR == pstCbsDcs)
    {
        return;
    }

    pstCbsDcs->ucInputDcs               = ucInputDcs;
    pstCbsDcs->usStringLen              = usStringLen;

    VOS_MemCpy(&pstCbsDcs->stCbDcs, pstCbDcs, sizeof(MN_MSG_CBDCS_CODE_STRU));

    VOS_MemCpy(pstCbsDcs->aucString, pUssdString, usStringLen);

    SI_STK_HookData(STK_TAF_CBS_DCS, ulDataLen, (VOS_UINT8*)pstCbsDcs);

    VOS_MemFree(MAPS_STK_PID, pstCbsDcs);

    return;
}


VOS_VOID SI_STK_SmsGobleInit(VOS_VOID)
{
    if (VOS_NULL_PTR != f_pstSendDirectParm)
    {
        VOS_MemFree(MAPS_STK_PID, f_pstSendDirectParm);
        STK_ERROR_LOG("SI_STK_SmsGobleInit: f_pstSendDirectParm did not free.");
    }

    if (VOS_NULL_PTR != f_pstStkMsgRawTsDataInfo)
    {
        VOS_MemFree(MAPS_STK_PID, f_pstStkMsgRawTsDataInfo);
        STK_ERROR_LOG("SI_STK_SmsGobleInit: f_pstStkMsgRawTsDataInfo did not free.");
    }

    g_ucReceCBNum = 0;          /*清空短信发送结果回调函数记录次数*/

    g_ucSendMsgNum = 0;         /*清空短信发送条数记录*/

    return;
}


VOS_VOID SI_STK_FillSmsPara(SI_STK_SEND_SHORT_MESSAGE_STRU *pstSendSMS)
{
    if(VOS_NULL_PTR == f_pstSendDirectParm)
    {
        STK_ERROR_LOG("SI_STK_FillSmsPara: the f_pstSendDirectParm point is Null");

        return;
    }

    /*填写短信中心等短信发送外围数据，发送短信的PDU码流待填写*/
    if(1 == pstSendSMS->OP_Addr)
    {
        /* 拷贝短信中心号码并设置号码长度 */
        VOS_MemCpy(f_pstSendDirectParm->stMsgInfo.stScAddr.aucBcdNum,
                    pstSendSMS->Addr.pucAddr, pstSendSMS->Addr.ucLen);

        f_pstSendDirectParm->stMsgInfo.stScAddr.addrType = pstSendSMS->Addr.ucNumType;

        f_pstSendDirectParm->stMsgInfo.stScAddr.ucBcdLen = pstSendSMS->Addr.ucLen;
    }
    else
    {
        /*如果没有短信中心号码则设置长度为0，由短信模块自动填充*/
        f_pstSendDirectParm->stMsgInfo.stScAddr.ucBcdLen = 0;
    }

    /*填写固定内容*/
    f_pstSendDirectParm->enMemStore     = MN_MSG_MEM_STORE_NONE;
    f_pstSendDirectParm->enDomain       = MN_MSG_SEND_DOMAIN_CS_PREFERRED;
    f_pstSendDirectParm->enClientType   = MN_MSG_CLIENT_STK;

    return;
}


VOS_UINT32 SI_STK_SendSmsSimple(
    MN_MSG_TPDU_TYPE_ENUM_U8            enTpduType,
    SI_STK_SEND_SHORT_MESSAGE_STRU     *pstSendSMS)
{
    VOS_UINT32 ulResult = SI_STK_NORP;

    f_pstSendDirectParm =
        (MN_MSG_SEND_PARM_STRU *)VOS_MemAlloc(MAPS_STK_PID, DYNAMIC_MEM_PT,sizeof(MN_MSG_SEND_PARM_STRU));

    if (VOS_NULL_PTR == f_pstSendDirectParm)
    {
        return SI_STK_DATA_ERROR;
    }

    SI_STK_FillSmsPara(pstSendSMS);

    /*填写TPDU内容*/
    f_pstSendDirectParm->stMsgInfo.stTsRawData.enTpduType   = enTpduType;

    VOS_MemCpy(f_pstSendDirectParm->stMsgInfo.stTsRawData.aucData,
                          pstSendSMS->WTPDU.pucTPDU,
                          pstSendSMS->WTPDU.ulLen);

    f_pstSendDirectParm->stMsgInfo.stTsRawData.ulLen = pstSendSMS->WTPDU.ulLen;

    if(VOS_OK != SI_STK_SendTAFMsg(STK_MSG_SEND_REQ,
                                    STK_OPID_VALUE,
                                    VOS_NULL,
                                    (VOS_UINT8*)f_pstSendDirectParm,
                                    sizeof(MN_MSG_SEND_PARM_STRU)))
    {
        VOS_MemFree(MAPS_STK_PID, f_pstSendDirectParm);

        STK_ERROR_LOG("SI_STK_SendSmsSimple: MN_MSG_Send fail to send.");

        ulResult = SI_STK_DATA_ERROR;
    }

    g_ucSendMsgNum = 1;

    return ulResult;
}
VOS_UINT32 SI_STK_CheckSmsPara(SI_STK_SMS_TPDU_STRU             *pstTPDU,
                                            SI_STK_COMMAND_DETAILS_STRU     *pstCmdDetail,
                                            VOS_UINT32                      *pulMsgNeedPress,
                                            VOS_UINT32                      *pulSmsNumber)
{
    VOS_UINT8                   ucUdl;
    VOS_UINT8                   ucUdhl;
    VOS_UINT32                  ulResult;
    VOS_UINT32                  ulMaxLen;
    MN_MSG_MSG_CODING_ENUM_U8   enMsgCoding;
    VOS_UINT32                  ulTotalLen;
    OM_VIVO_STK_CODEC_STRU      stFlag;
    VOS_UINT32                  ulDcsOffset = 0;
    MN_MSG_DCS_CODE_STRU        stDcs;

    stFlag.usVivo = VOS_FALSE;

    ulResult = MN_MSG_GetSubmitInfoForStk((MN_MSG_TPDU_STRU *)pstTPDU, &ucUdl, &ucUdhl, &enMsgCoding, &ulDcsOffset);

    if(MN_ERR_NO_ERROR != ulResult)
    {
        STK_ERROR_LOG("SI_STK_CheckSmsPara: fail to call function MN_MSG_GetSubmitInfoForStk.");

        return SI_STK_DATA_ERROR;
    }

    if (NV_OK != NV_Read(en_NV_Item_BRAZIL_VIVO_STK_CODEC_CORRECT, &stFlag, sizeof(stFlag)))
    {
        STK_INFO_LOG("SI_STK_CheckSmsPara: Read NV Fail.");
    }

    /* 根据(U)SIM消息指示修改短信编码格式；*/
    if((0x01 ==(pstCmdDetail->ucCommandQua&0x01))&&(MN_MSG_MSG_CODING_8_BIT == enMsgCoding))
    {
        *pulMsgNeedPress = VOS_TRUE;    /*当前需要压缩数据*/

        ulMaxLen    = MN_MSG_MAX_7_BIT_LEN;
    }
    else if((0x01 ==(pstCmdDetail->ucCommandQua&0x01))&&(MN_MSG_MSG_CODING_7_BIT == enMsgCoding)&&(VOS_TRUE == stFlag.usVivo))
    {
        *pulMsgNeedPress = VOS_TRUE;    /*当前需要压缩数据*/

        (VOS_VOID)MN_MSG_DecodeDcs(pstTPDU->pucTPDU[ulDcsOffset], &stDcs);

        stDcs.enMsgCoding = MN_MSG_MSG_CODING_8_BIT;

        stDcs.bRawDcsValid = VOS_FALSE;

        (VOS_VOID)MN_MSG_EncodeDcs(&stDcs, &pstTPDU->pucTPDU[ulDcsOffset]);

        ulMaxLen    = MN_MSG_MAX_7_BIT_LEN;
    }
    else if(MN_MSG_MSG_CODING_7_BIT == enMsgCoding)
    {
        *pulMsgNeedPress = VOS_FALSE;   /*当前不需要压缩数据*/

        ulMaxLen    = MN_MSG_MAX_7_BIT_LEN;
    }
    else
    {
        *pulMsgNeedPress = VOS_FALSE;   /*当前不需要压缩数据*/

        ulMaxLen    = MN_MSG_MAX_8_BIT_LEN;
    }

    ulTotalLen = ucUdl;

    if ((MN_MSG_MSG_CODING_7_BIT == enMsgCoding)/*压缩编码需要计算*/
     || (VOS_TRUE == (*pulMsgNeedPress)))
    {
        if (0 != ucUdhl)
        {
            ulTotalLen += (((ucUdhl + 1) * 8) + 6)/7;
        }
    }
    else
    {
        if (0 != ucUdhl)
        {
            ulTotalLen += (ucUdhl + 1);
        }
    }

    if (ulTotalLen > ulMaxLen)  /*当前需要分成两条发送*/
    {
        *pulSmsNumber = 2;
    }
    else
    {
        *pulSmsNumber = 1;
    }

    return SI_STK_OK;
}


VOS_UINT32 SI_STK_SendSubmitSms(
    SI_STK_SEND_SHORT_MESSAGE_STRU     *pstSendSMS,
    SI_STK_COMMAND_DETAILS_STRU        *pstCmdDetail)
{
    VOS_UINT32                  ulMsgNeedPress;
    MN_MSG_SUBMIT_LONG_STRU     *pstLongSubmit;
    VOS_UINT32                  ulSmsNumber;
    VOS_UINT32                  ulResult;

    ulResult = SI_STK_CheckSmsPara(&pstSendSMS->WTPDU, pstCmdDetail, &ulMsgNeedPress, &ulSmsNumber);

    if (SI_STK_OK != ulResult)
    {
        STK_ERROR_LOG("SI_STK_SendSubmitSms: fail to call function SI_STK_CheckSmsPara.");

        return SI_STK_DATA_ERROR;
    }

    if ((VOS_FALSE == ulMsgNeedPress)&&(1 == ulSmsNumber))
    {
        return SI_STK_SendSmsSimple(MN_MSG_TPDU_SUBMIT, pstSendSMS);
    }

    pstLongSubmit = (MN_MSG_SUBMIT_LONG_STRU *)VOS_MemAlloc(MAPS_STK_PID, DYNAMIC_MEM_PT,
                                                sizeof(MN_MSG_SUBMIT_LONG_STRU));

    if (VOS_NULL_PTR == pstLongSubmit)
    {
        return SI_STK_DATA_ERROR;
    }

    pstLongSubmit->stLongUserData.pucOrgData = (VOS_UINT8 *)VOS_MemAlloc(MAPS_STK_PID, DYNAMIC_MEM_PT,
                                                    MN_MSG_MAX_LEN);

    if (VOS_NULL_PTR == pstLongSubmit->stLongUserData.pucOrgData)
    {
        VOS_MemFree(MAPS_STK_PID, pstLongSubmit);

        return SI_STK_DATA_ERROR;
    }

    f_pstStkMsgRawTsDataInfo = (MN_MSG_RAW_TS_DATA_STRU *)VOS_MemAlloc(MAPS_STK_PID,DYNAMIC_MEM_PT,
                                                sizeof(MN_MSG_RAW_TS_DATA_STRU) * ulSmsNumber);

    f_pstSendDirectParm = (MN_MSG_SEND_PARM_STRU *)VOS_MemAlloc(MAPS_STK_PID,DYNAMIC_MEM_PT,
                                                sizeof(MN_MSG_SEND_PARM_STRU));

    if ((VOS_NULL_PTR == f_pstStkMsgRawTsDataInfo)
     || (VOS_NULL_PTR == f_pstSendDirectParm))
    {
        VOS_MemFree(MAPS_STK_PID, pstLongSubmit->stLongUserData.pucOrgData);

        VOS_MemFree(MAPS_STK_PID, pstLongSubmit);

        SI_STK_SmsGobleInit();

        return SI_STK_DATA_ERROR;
    }

    VOS_MemSet(f_pstStkMsgRawTsDataInfo, 0x00, sizeof(MN_MSG_RAW_TS_DATA_STRU)*ulSmsNumber);

    VOS_MemSet(f_pstSendDirectParm, 0x00, sizeof(MN_MSG_SEND_PARM_STRU));

    SI_STK_FillSmsPara(pstSendSMS);

    ulResult = MN_MSG_Decode_UsimMsg(pstSendSMS->WTPDU.pucTPDU,
                                    pstSendSMS->WTPDU.ulLen, pstLongSubmit);
    if (MN_ERR_NO_ERROR != ulResult)
    {
        VOS_MemFree(MAPS_STK_PID, pstLongSubmit->stLongUserData.pucOrgData);

        VOS_MemFree(MAPS_STK_PID, pstLongSubmit);

        SI_STK_SmsGobleInit();

        STK_ERROR_LOG("SI_STK_SendSubmitSms: fail to call function MN_MSG_Decode_UsimMsg.");

        return SI_STK_DATA_ERROR;
    }

    if (VOS_TRUE == ulMsgNeedPress)
    {
        pstLongSubmit->stDcs.bRawDcsValid   = 0;

        pstLongSubmit->stDcs.enMsgCoding    = MN_MSG_MSG_CODING_7_BIT;
    }

    ulResult = MN_MSG_Segment(pstLongSubmit, &g_ucSendMsgNum, f_pstStkMsgRawTsDataInfo);

    VOS_MemFree(MAPS_STK_PID, pstLongSubmit->stLongUserData.pucOrgData);

    VOS_MemFree(MAPS_STK_PID, pstLongSubmit);

    if ((g_ucSendMsgNum > STK_SMS_MAX_MSG_SEGMENT_NUM)||(MN_ERR_NO_ERROR != ulResult))
    {
        SI_STK_SmsGobleInit();

        STK_ERROR_LOG("SI_STK_SendSubmitSms: fail to call function MN_MSG_Segment.");

        return SI_STK_DATA_ERROR;
    }

    /* 发送分段短信到网络*/
    VOS_MemCpy(&f_pstSendDirectParm->stMsgInfo.stTsRawData,
               f_pstStkMsgRawTsDataInfo,
               sizeof(f_pstSendDirectParm->stMsgInfo.stTsRawData));

    if(VOS_OK != SI_STK_SendTAFMsg(STK_MSG_SEND_REQ,
                                    STK_OPID_VALUE,
                                    VOS_NULL,
                                    (VOS_UINT8*)f_pstSendDirectParm,
                                    sizeof(MN_MSG_SEND_PARM_STRU)))
    {
        SI_STK_SmsGobleInit();
        STK_ERROR_LOG("SI_STK_SendSubmitSms: fail to send.");
        ulResult = SI_STK_DATA_ERROR;
    }
    else
    {
        ulResult = SI_STK_NORP;
    }

    return ulResult;
}
VOS_UINT32 SI_STK_SendSMSSpecialProc(SI_STK_DATA_INFO_STRU *pCmdData)
{
    SI_STK_SEND_SHORT_MESSAGE_STRU      *pstSendSMS;
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucMTI;
    SI_STK_EVENT_INFO_STRU              stSTKEvent;

    pstSendSMS = (SI_STK_SEND_SHORT_MESSAGE_STRU*)pCmdData;

    if (VOS_NULL_PTR == pstSendSMS->WTPDU.pucTPDU)
    {
        STK_ERROR_LOG("SI_STK_SendSMSSpecialProc:pstSendSMS->WTPDU.pucTPDU is null.");
        return SI_STK_DATA_ERROR;
    }

    if (pstSendSMS->WTPDU.ulLen > MN_MSG_MAX_LEN)
    {
        STK_ERROR_LOG("SI_STK_SendSMSSpecialProc:pstSendSMS->WTPDU.ulLen > MN_MSG_MAX_LEN.");
        return SI_STK_DATA_ERROR;
    }

    SI_STK_SmsGobleInit();

    MN_MSG_GetMsgMti(pstSendSMS->WTPDU.pucTPDU[0], &ucMTI);

    if (MN_MSG_TP_MTI_SUBMIT == ucMTI)
    {
        ulResult = SI_STK_SendSubmitSms(pstSendSMS, &pCmdData->CmdDetail);
    }
    else if (MN_MSG_TP_MTI_COMMAND == ucMTI)
    {
        ulResult = SI_STK_SendSmsSimple(MN_MSG_TPDU_COMMAND, pstSendSMS);
    }
    else
    {
        STK_ERROR_LOG("SI_STK_SendSMSSpecialProc:ucMTI is either command or submit.");

        ulResult = SI_STK_DATA_ERROR;
    }

    /* 阿根廷Claro定制需求，发送短信时通知后台 */
    if (VOS_TRUE == gusSTKSMSIndEnable)
    {
        stSTKEvent.ClientId   = TAF_AT_CLIENT;
        stSTKEvent.STKErrorNo = VOS_OK;
        stSTKEvent.STKCBEvent = SI_STK_CMD_IND_EVENT;
        stSTKEvent.STKCmdStru.SatType = SI_STK_SENDSMS;
        SI_STKCallBack_BroadCast(&stSTKEvent);
    }
    return ulResult;
}


VOS_UINT32 SI_STK_SendSSSpecialProc(SI_STK_DATA_INFO_STRU *pCmdData)
{
    MN_MMI_OPERATION_PARAM_STRU         stMmiOpParam;
    SI_STK_SEND_SS_STRU                *pstSendSS;
    VOS_UINT32                          ulResult = VOS_ERR;
    VOS_BOOL                            inCall;
    VOS_CHAR                           *pSSString;
    VOS_CHAR                           *pOrgSSString;
    VOS_UINT32                          ulSSStrLen;
    TAF_SS_PROCESS_USS_REQ_STRU         stProcUssRwq;
    SI_STK_USSD_STRING_STRU             stUssdString;
    VOS_UINT8                           aucRspTerminal[2] = {TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND,
                                                             NO_SERVICE};
    VOS_MemSet(&stProcUssRwq, 0, sizeof(stProcUssRwq));
    if (SI_STK_NORMAL_SERVICE != STK_GetCsSvcStatus())
    {
        STK_ERROR_LOG("SI_STK_SendSSSpecialProc: CS is not Normal Service");

        SI_STK_CSServiceHook();

        /* 回复TERMINAL RESPONSE结果:当前UE无法处理无服务 */
        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucRspTerminal), aucRspTerminal);

        return SI_STK_NORP;
    }

    pstSendSS = (SI_STK_SEND_SS_STRU *)pCmdData;

    if ((1 != pstSendSS->OP_SSStr)||/*按协议规定Icon Qualifier为1时，AlphaId必须存在*/
     ((1 == pstSendSS->OP_IconId)&&(1 != pstSendSS->OP_AlphaId)&&(1 == pstSendSS->IconId.ucQualifier)))
    {
        STK_ERROR_LOG("SI_STK_SendSSSpecialProc: 1 != pstSendSS->OP_SSStr");

        return SI_STK_DATA_ERROR;
    }

    /* 执行命令操作 */
    pSSString = (VOS_CHAR *)VOS_MemAlloc(MAPS_STK_PID, DYNAMIC_MEM_PT, (pstSendSS->SSString.ucLen*2)+1);

    if (VOS_NULL_PTR == pSSString)
    {
        STK_ERROR_LOG("SI_STK_SendSSSpecialProc: Memory Alloc Error");

        return SI_STK_NOTSUPPORT;
    }

    pOrgSSString = pSSString;

    SI_STK_BcdStrToAscii(pstSendSS->SSString.ucLen,
                            pstSendSS->SSString.pucSSString,
                            (VOS_UINT8*)pSSString,
                            &ulSSStrLen,
                            SI_STK_STR_CHANGESMALL);

    pSSString[ulSSStrLen] = '\0';

    inCall = STK_IsCallInService();
    if(VOS_TRUE != inCall)
    {
       STK_ERROR_LOG1("SI_STK_SendSSSpecialProc: stk is call in service Error %d", (long)inCall); 
    }

    ulResult = MN_MmiStringParse(pSSString, inCall, &stMmiOpParam, &pSSString);

    if (VOS_OK != ulResult)
    {
        STK_ERROR_LOG1("SI_STK_SendSSSpecialProc: Parse SSString Error %d", (VOS_UINT8)ulResult);

        VOS_MemFree(MAPS_STK_PID, pOrgSSString);

        return SI_STK_DATA_ERROR;
    }

    switch (stMmiOpParam.MmiOperationType)
    {
        case TAF_MMI_REGISTER_SS:
            stMmiOpParam.RegisterSsReq.OP_NumType           = 1;/*需填写号码类型*/
            stMmiOpParam.RegisterSsReq.OP_LongFTN_Supported = 0;/*设置成不支持Long FTN*/
            stMmiOpParam.RegisterSsReq.NumType              = pstSendSS->SSString.ucNumType;

            ulResult = SI_STK_SendTAFMsg(STK_SS_REGISTERSS_REQ,
                                        STK_SEND_SS_OP_ID,
                                        VOS_NULL,
                                        (VOS_UINT8*)&stMmiOpParam.RegisterSsReq,
                                        sizeof(TAF_SS_REGISTERSS_REQ_STRU));
            g_ucSendSSType = TAF_SS_REGISTERSS;
            break;

        case TAF_MMI_ERASE_SS:
            ulResult = SI_STK_SendTAFMsg(STK_SS_ERASESS_REQ,
                                        STK_SEND_SS_OP_ID,
                                        VOS_NULL,
                                        (VOS_UINT8*)&stMmiOpParam.EraseSsReq,
                                        sizeof(TAF_SS_ERASESS_REQ_STRU));
            g_ucSendSSType = TAF_SS_ERASESS;
            break;

        case TAF_MMI_ACTIVATE_SS:
            ulResult = SI_STK_SendTAFMsg(STK_SS_ACTIVATESS_REQ,
                                        STK_SEND_SS_OP_ID,
                                        VOS_NULL,
                                        (VOS_UINT8*)&stMmiOpParam.ActivateSsReq,
                                        sizeof(TAF_SS_ACTIVATESS_REQ_STRU));
            g_ucSendSSType = TAF_SS_ACTIVATESS;
            break;

        case TAF_MMI_DEACTIVATE_SS:
            ulResult = SI_STK_SendTAFMsg(STK_SS_DEACTIVATESS_REQ,
                                        STK_SEND_SS_OP_ID,
                                        VOS_NULL,
                                        (VOS_UINT8*)&stMmiOpParam.DeactivateSsReq,
                                        sizeof(TAF_SS_DEACTIVATESS_REQ_STRU));
            g_ucSendSSType = TAF_SS_DEACTIVATESS;
            break;

        case TAF_MMI_INTERROGATE_SS:
            ulResult = SI_STK_SendTAFMsg(STK_SS_INTERROGATESS_REQ,
                                        STK_SEND_SS_OP_ID,
                                        VOS_NULL,
                                        (VOS_UINT8*)&stMmiOpParam.InterrogateSsReq,
                                        sizeof(TAF_SS_INTERROGATESS_REQ_STRU));
            g_ucSendSSType = TAF_SS_INTERROGATESS;
            break;

        case TAF_MMI_REGISTER_PASSWD:
            ulResult = SI_STK_SendTAFMsg(STK_SS_REGPWD_REQ,
                                        STK_SEND_SS_OP_ID,
                                        VOS_NULL,
                                        (VOS_UINT8*)&stMmiOpParam.RegPwdReq,
                                        sizeof(TAF_SS_REGPWD_REQ_STRU));
            g_ucSendSSType = TAF_SS_REGISTERPASSWORD;
            break;

        case TAF_MMI_PROCESS_USSD_REQ:

            stUssdString.pucUSSDString  = stMmiOpParam.ProcessUssdReq.UssdStr.aucUssdStr;
            stUssdString.ucDCScheme     = MN_MSG_MSG_CODING_7_BIT;
            stUssdString.ucLen          = (VOS_UINT8)stMmiOpParam.ProcessUssdReq.UssdStr.usCnt;

            /* NAS会忽略这个值重新计算 */
            stProcUssRwq.DatacodingScheme = MN_MSG_MSG_CODING_7_BIT;

            if (SI_STK_OK != SI_STK_EncodeUssdString(&stUssdString, &stProcUssRwq.UssdStr))
            {
                ulResult = VOS_ERR;/* [false alarm]:fortify */

                break;
            }

            ulResult = SI_STK_SendTAFMsg(STK_SS_USSD_REQ,
                                        STK_SEND_SS_OP_ID,
                                        VOS_NULL,
                                        (VOS_UINT8*)&stProcUssRwq,
                                        sizeof(TAF_SS_PROCESS_USS_REQ_STRU));

            g_ucSendSSType = TAF_MMI_PROCESS_USSD_REQ;
            break;

        default:
            STK_ERROR_LOG("SI_STK_SendSSSpecialProc: Unknown SS Type");
            ulResult = VOS_ERR;
            break;
    }

    VOS_MemFree(MAPS_STK_PID, pOrgSSString);

    return (VOS_OK != ulResult)?SI_STK_DATA_ERROR:SI_STK_NORP;/* [false alarm]: 屏蔽Fortify 错误 */
}


VOS_UINT32 SI_STK_EncodeUssdString(
    SI_STK_USSD_STRING_STRU            *pstSrcUssdString,
    TAF_SS_USSD_STRING_STRU            *pstDestUssdString)
{
    MN_MSG_CBDCS_CODE_STRU              stMsgCbDcs;   /* 由调用者清空数据结构 */

    VOS_MemSet(&stMsgCbDcs, 0, sizeof(stMsgCbDcs));

    if (VOS_OK != MN_MSG_DecodeCbsDcs(pstSrcUssdString->ucDCScheme,
                                      pstSrcUssdString->pucUSSDString,
                                      pstSrcUssdString->ucLen,
                                      &stMsgCbDcs))
    {
        STK_ERROR_LOG("SI_STK_EncodeUssdString: SSA_GetDcsMsgCoding fail");

        stMsgCbDcs.enMsgCoding = MN_MSG_MSG_CODING_7_BIT;
    }

    SI_STK_UssdDcsHook(pstSrcUssdString->ucDCScheme, pstSrcUssdString->pucUSSDString, pstSrcUssdString->ucLen, &stMsgCbDcs);

    if (MN_MSG_MSG_CODING_7_BIT == stMsgCbDcs.enMsgCoding)
    {
        if (TAF_SS_MAX_USSDSTRING_LEN < pstSrcUssdString->ucLen)
        {
            return SI_STK_DATA_ERROR;
        }

        /* 转为ASCII码后长度变为原来两倍 */
        SI_STK_SsStr2ASCII(pstSrcUssdString->pucUSSDString, pstDestUssdString->aucUssdStr, pstSrcUssdString->ucLen);

        pstDestUssdString->usCnt = 2 * pstSrcUssdString->ucLen;
    }
    else
    {
        VOS_MemCpy(pstDestUssdString->aucUssdStr,
                   pstSrcUssdString->pucUSSDString,
                   pstSrcUssdString->ucLen);

        pstDestUssdString->usCnt = pstSrcUssdString->ucLen;
    }

    return SI_STK_OK;
}


VOS_UINT32 SI_STK_SendUSSDSpecialProc(SI_STK_DATA_INFO_STRU *pCmdData)
{
    TAF_SS_PROCESS_USS_REQ_STRU         stProcUssRwq = {0};
    SI_STK_SEND_USSD_STRU               *pstSendUSSD;
    VOS_UINT32                          ulResult = VOS_ERR;
    VOS_UINT8                           aucRspTerminal[2] = {TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND,
                                                             NO_SERVICE};
    if (SI_STK_NORMAL_SERVICE != STK_GetCsSvcStatus())
    {
        STK_ERROR_LOG("SI_STK_SendUSSDSpecialProc: CS is not Normal Service");

        SI_STK_CSServiceHook();

        /* 回复TERMINAL RESPONSE结果:当前UE无法处理无服务 */
        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucRspTerminal), aucRspTerminal);

        return SI_STK_NORP;
    }

    pstSendUSSD = (SI_STK_SEND_USSD_STRU *)&pCmdData->CmdStru;

    /*按协议规定Icon Qualifier为1时，AlphaId必须存在*/
    if ((VOS_TRUE != pstSendUSSD->OP_USSDStr)
        ||((VOS_TRUE == pstSendUSSD->OP_IconId)
            &&(VOS_TRUE != pstSendUSSD->OP_AlphaId)
            &&(VOS_TRUE == pstSendUSSD->IconId.ucQualifier)))
    {
        STK_ERROR_LOG("SI_STK_SendUSSDSpecialProc: 1 != pstSendSS->OP_SSStr");

        return SI_STK_DATA_ERROR;
    }

    /* NAS会忽略这个值重新计算 */
    stProcUssRwq.DatacodingScheme = pstSendUSSD->USSDString.ucDCScheme;
    stProcUssRwq.enCusdType       = TAF_SS_CUSD_TYPE_ENABLE;

    ulResult = SI_STK_EncodeUssdString(&pstSendUSSD->USSDString, &stProcUssRwq.UssdStr);

    if (SI_STK_OK != ulResult)
    {
        ulResult = VOS_ERR;
    }
    else
    {
        ulResult = SI_STK_SendTAFMsg(STK_SS_USSD_REQ,
                                    STK_SEND_USSD_OP_ID,
                                    VOS_NULL,
                                    (VOS_UINT8*)&stProcUssRwq,
                                    sizeof(TAF_SS_PROCESS_USS_REQ_STRU));
    }

    return (VOS_OK != ulResult)?SI_STK_DATA_ERROR:SI_STK_NORP;/* [false alarm]: 屏蔽Fortify 错误 */
}


VOS_UINT32 SI_STK_SendDTMFSpecialProc(SI_STK_DATA_INFO_STRU *pCmdData)
{
    SI_STK_SEND_DTMF_STRU           *pstSendDTMF;
    SI_UINT32                       ulResult;

    if (VOS_FALSE == SI_STK_CheckSupportAP())
    {
        STK_ERROR_LOG("SI_STK_SendDTMFSpecialProc: Not Support AP");

        return SI_STK_NOTSUPPORT;
    }

    if (SI_STK_CALL_STATUS_SEND_DTMF == g_stStkSetupCallCtrl.enCallStatus)
    {
        STK_ERROR_LOG("SI_STK_SendDTMFSpecialProc: The UE is Busy to Send DTMP of Call");

        return SI_STK_NOTSUPPORT;
    }

    pstSendDTMF = (SI_STK_SEND_DTMF_STRU *)pCmdData;

    if (VOS_FALSE == pstSendDTMF->OP_DTMF)
    {
        STK_ERROR_LOG("SI_STK_SendDTMFSpecialProc: 1 != pstSendDTMF->OP_DTMF");

        return SI_STK_DATA_ERROR;
    }

    STK_ClearPauseCharGobal();

    SI_STK_BcdStrToAscii((SI_UINT8)pstSendDTMF->DTMFStr.ulLen,
                            pstSendDTMF->DTMFStr.pucDTMFStr,
                            (VOS_UINT8*)g_stSTKDtmfData.acDtfm,
                            &g_stSTKDtmfData.ulDtmfLen,
                            SI_STK_STR_CHANGEBIG);

    /* Send DTMF的不需要指明Call ID */
    ulResult = SI_STK_SendDtmfToTAF(STK_SEND_DTMF_OP_ID, 0);

    if (SI_STK_DTMF_SEND_OK != ulResult)
    {
        STK_ClearPauseCharGobal();

        STK_ERROR_LOG("SI_STK_SendDTMFSpecialProc: send DTMF to TAF is Error");

        return SI_STK_NOTSUPPORT;
    }

    return SI_STK_NORP;
}
VOS_UINT32 SI_STK_SetCallSpecialProc(SI_STK_DATA_INFO_STRU *pCmdData)
{
    if (VOS_TRUE == SI_STK_IsProactiveCmdNeedSnd2Csima())
    {
        SI_STK_SndProactiveCmd2CBP(pCmdData->SatCmd.SatDataLen, pCmdData->SatCmd.SatCmdData);

        return SI_STK_OK_WAITRP;
    }

    if(VOS_FALSE == SI_STK_CheckSupportAP())
    {
        return SI_STK_NOTSUPPORT;
    }

    /* 当前还有SETUP CALL命令中的DTMF数据在发送，不处理新上报的主动命令 */
    if (SI_STK_CALL_STATUS_SEND_DTMF == g_stStkSetupCallCtrl.enCallStatus)
    {
        return SI_STK_NOTSUPPORT;
    }

    return SI_STKDataIndCallback(pCmdData);
}
VOS_VOID SI_STK_IMSIChgBegin(SI_STK_REQ_STRU *STKReqMsg)
{
    gstSTKCtrlStru.enSTKLock  = SI_STK_PROLOCK;

    gstSTKCtrlStru.usClientID = STKReqMsg->ClientId;

    gstSTKCtrlStru.ucOpID     = STKReqMsg->OpId;

    gstSTKIMSIChgCtrl.ucTryFlag      = VOS_FALSE;

    gstSTKIMSIChgCtrl.enIMSIChgState = SI_STK_IMSICHG_SELECT_ITEM;

    return;
}


VOS_UINT32 SI_STK_IMSIChgNullProc(PS_SI_MSG_STRU *pMsg)
{
    return VOS_OK;
}


VOS_UINT32 SI_STK_IMSIChgSelItemProc(PS_SI_MSG_STRU *pMsg)
{
    VOS_UINT32             ulResult;
    VOS_UINT32             i;
    VOS_UINT32             j;
    VOS_UINT8              aucMenuSelect[] = {0xD3, 0x07, 0x82, 0x02, 0x01, 0x81,
                                              0x90, 0x01, 0xFF};

    /* 判断是否存在主菜单 */
    if ((VOS_OK != USIMM_GetCardIMSI(gstSTKIMSIChgCtrl.aucOldImsi))
        || (SI_STK_MENU_EXIST != gstSetUpMenuData.ucUsatTag))
    {

        STK_WARNING_LOG("SI_STK_IMSIChgSelItemProc: There is any Main Menu Content");

        return TAF_ERR_SAT_NO_MAIN_MENU;
    }

    ulResult = VOS_FALSE;

    /* 在主菜单中匹配，如果匹配失败回调失败，*/
    for (i = 0; i < gstSetUpMenuData.stSetUpMenu.ulItemNum; i++)
    {
        for (j = 0; j < gstSTKIMSIMatch.usMatchStrCnt; j++)
        {
            if (VOS_TRUE == SI_STK_StrStr(gstSetUpMenuData.stSetUpMenu.Item[i].pucItemText,
                                          gstSTKIMSIMatch.astMatchStr[j].aucMatchStr,
                                          gstSetUpMenuData.stSetUpMenu.Item[i].ucLen,
                                          gstSTKIMSIMatch.astMatchStr[j].usStrLen))
            {
                ulResult = VOS_TRUE;

                break;
            }
        }

        if (VOS_TRUE == ulResult)
        {
            break;
        }
    }

    if (VOS_FALSE == ulResult)
    {
        STK_WARNING_LOG("SI_STK_IMSIChgBegin: Text String not match");

        return TAF_ERR_SAT_STORAGE_OP_FAIL;
    }

    aucMenuSelect[8] = gstSetUpMenuData.stSetUpMenu.Item[i].ucItemId;

    SI_STK_SaveEnvelopeCmd(sizeof(aucMenuSelect), aucMenuSelect);

    /* 迁移当前状态为SI_STK_IMSICHG_SELECT_ITEM,选择对应菜单项 */
    if (VOS_OK != USIMM_EnvelopeReq(MAPS_STK_PID, VOS_NULL, sizeof(aucMenuSelect), aucMenuSelect))
    {
        STK_ERROR_LOG("SI_STK_IMSIChgBegin: USIMM_EnvelopeReq failed");

        return TAF_ERR_SIM_FAIL;
    }

    gstSTKIMSIChgCtrl.enIMSIChgState = SI_STK_IMSICHG_REFRESH;

    return VOS_OK;
}
VOS_UINT32 SI_STK_IMSIChgRefreshProc(PS_SI_MSG_STRU *pMsg)
{
    VOS_UINT32 i;
    VOS_UINT8  aucSelectItem[]  = {0x81, 0x03, 0x01, 0x24, 0x00, 0x82, 0x02, 0x82,
                                   0x81, 0x83, 0x01, 0x00, 0x10, 0x01, 0xFF};
    VOS_UINT8  aucDisplayText[] = {0x81, 0x03, 0x01, 0x21, 0x00, 0x82, 0x02, 0x82,
                                   0x81, 0x83, 0x01, 0x00};

    VOS_UINT8  aucNewImsi[9];

    /*不处理非USIMM模块发来的消息*/
    if (WUEPS_PID_USIM != pMsg->ulSenderPid)
    {
        return VOS_OK;
    }

    switch (pMsg->ulMsgName)
    {

        case PS_USIM_ENVELOPE_CNF:

            break;

        case PS_USIM_SAT_IND:
            if (SI_STK_SELECTITEM == gstUsatCmdDecode.SatType)
            {
                for (i = 0; i < gstUsatCmdDecode.CmdStru.SelectItem.ItemNum; i++)
                {
                    if (gstSTKIMSIChgCtrl.ucOldItemId
                        != gstUsatCmdDecode.CmdStru.SelectItem.Item[i].ucItemId)
                    {
                        aucSelectItem[14] = gstUsatCmdDecode.CmdStru.SelectItem.Item[i].ucItemId;

                        gstSTKIMSIChgCtrl.ucOldItemId =
                            gstUsatCmdDecode.CmdStru.SelectItem.Item[i].ucItemId;

                        break;
                    }
                }

                if (i >= gstUsatCmdDecode.CmdStru.SelectItem.ItemNum)
                {
                    STK_ERROR_LOG("SI_STK_IMSIChgRefreshProc: No Item Selected");
                    return VOS_ERR;
                }

                aucSelectItem[2] = gstUsatCmdDecode.CmdDetail.ucCommandNum;
                aucSelectItem[4] = gstUsatCmdDecode.CmdDetail.ucCommandQua;

                if (VOS_OK != USIMM_TerminalResponseReq(MAPS_STK_PID,sizeof(aucSelectItem),aucSelectItem))
                {
                    STK_ERROR_LOG("SI_STK_IMSIChgRefreshProc:SelectItem TrReq failed");
                    return TAF_ERR_SIM_FAIL;
                }

            }

            if (SI_STK_DISPLAYTET == gstUsatCmdDecode.SatType)
            {
                aucDisplayText[2] = gstUsatCmdDecode.CmdDetail.ucCommandNum;
                aucDisplayText[4] = gstUsatCmdDecode.CmdDetail.ucCommandQua;

                if (VOS_OK != USIMM_TerminalResponseReq(MAPS_STK_PID,sizeof(aucDisplayText),aucDisplayText))
                {
                    STK_ERROR_LOG("SI_STK_IMSIChgRefreshProc:DisplayText TrReq failed");
                    return TAF_ERR_SIM_FAIL;
                }
            }

            break;

        case PS_USIM_TERMINALRESPONSE_CNF:/*Select ITEM终端回复*/

            /*判断是否有后续STK 主动命令*/
            if (0x91 == ((PS_USIM_TERMINALRESPONSE_CNF_STRU *)pMsg)->ucSW1)
            {
                break;
            }

            if (VOS_OK != USIMM_GetCardIMSI(aucNewImsi))
            {
                STK_ERROR_LOG("SI_STK_IMSIChgRefreshProc:USIMM_GetCardIMSI failed");
                return TAF_ERR_SIM_FAIL;
            }

            if (VOS_FALSE != VOS_MemCmp(aucNewImsi, gstSTKIMSIChgCtrl.aucOldImsi, sizeof(aucNewImsi)))
            {
                gstSTKIMSIChgCtrl.enIMSIChgState = SI_STK_IMSICHG_NULL;

                gstSTKIMSIChgCtrl.ulCurImsiSign  = gstSTKIMSIChgCtrl.ulCurImsiSign^0x01;

                return VOS_OK;
            }

            if (VOS_FALSE == gstSTKIMSIChgCtrl.ucTryFlag)
            {
                gstSTKIMSIChgCtrl.ucTryFlag      = VOS_TRUE;

                gstSTKIMSIChgCtrl.ucCycleFlag    = VOS_TRUE;

                gstSTKIMSIChgCtrl.enIMSIChgState = SI_STK_IMSICHG_SELECT_ITEM;

                STK_NORMAL_LOG("SI_STK_IMSIChgRefreshProc:Try to change IMSI again");

                return VOS_OK;
            }

            STK_WARNING_LOG("SI_STK_IMSIChgRefreshProc:IMSI didn't Change");

            return VOS_ERR;
        default:
            STK_WARNING_LOG("SI_STK_IMSIChgRefreshProc:Unkown CMD Type");
            break;
    }

    return VOS_OK;
}
SI_STK_IMSICHG_PROC_STRU gstIMSIChgProc[] =
{
    {SI_STK_IMSICHG_NULL,            SI_STK_IMSIChgNullProc},
    {SI_STK_IMSICHG_SELECT_ITEM,     SI_STK_IMSIChgSelItemProc},
    {SI_STK_IMSICHG_REFRESH,         SI_STK_IMSIChgRefreshProc},
};

VOS_VOID SI_STK_IMSIChgProc(PS_SI_MSG_STRU *pMsg)
{
    VOS_UINT32              i;
    VOS_UINT32              ulResult = VOS_ERR;
    SI_STK_EVENT_INFO_STRU  stSTKEvent;

    if (SI_STK_IMSICHG_NULL == gstSTKIMSIChgCtrl.enIMSIChgState)
    {
        return;
    }

    for (i = 0; i < (sizeof(gstIMSIChgProc)/sizeof(SI_STK_IMSICHG_PROC_STRU)); i++)
    {
        if (gstSTKIMSIChgCtrl.enIMSIChgState == gstIMSIChgProc[i].enIMSIChgState)
        {
            ulResult = gstIMSIChgProc[i].pIMSIChgProc(pMsg);

            /* 需要轮转到初始状态再次尝试切换 */
            if ((VOS_OK == ulResult)&&(VOS_TRUE == gstSTKIMSIChgCtrl.ucCycleFlag))
            {
                gstSTKIMSIChgCtrl.ucCycleFlag = VOS_FALSE;

                i = 0;

                continue;
            }

            break;
        }
    }

    if (VOS_OK != ulResult)
    {
        STK_WARNING_LOG("SI_STK_IMSIChgProc:Result not OK");
        gstSTKIMSIChgCtrl.enIMSIChgState = SI_STK_IMSICHG_NULL;
    }

    stSTKEvent.STKErrorNo = ulResult;
    stSTKEvent.ClientId   = gstSTKCtrlStru.usClientID;
    stSTKEvent.OpId       = gstSTKCtrlStru.ucOpID;
    stSTKEvent.STKCBEvent = SI_STK_CMD_IMSICHG_EVENT;

    SI_STKCallback(&stSTKEvent);

    return;
}
VOS_UINT32 SI_STK_GetPauseCharFromBCD(SI_STK_ADDRESS_STRU  *pstAddr)
{
    VOS_UINT32                          i;
    VOS_UINT8                           ucFirstNumber   = 0;
    VOS_UINT8                           ucSecondNumber  = 0;
    VOS_UINT32                          ulSrcBcdLen     = pstAddr->ucLen;
    VOS_UINT32                          ulPauseCharLen;
    VOS_UINT8                           ucFlag          = VOS_FALSE;

    /* 参数检查 */
    if ((VOS_NULL_PTR == pstAddr->pucAddr) || (0 == pstAddr->ucLen))
    {
        return VOS_ERR;
    }

    STK_ClearPauseCharGobal();

    for(i = 0; i < ulSrcBcdLen; i++)
    {
        if(0xFF == pstAddr->pucAddr[i])
        {
            break;
        }

        ucFirstNumber  = (VOS_UINT8)((pstAddr->pucAddr[i] >> 4) & 0x0F);    /*取出高半字节*/
        ucSecondNumber = (VOS_UINT8)(pstAddr->pucAddr[i]& 0x0F);            /*取出低半字节*/

        if ((STK_PAUSE_CHAR == ucFirstNumber)||(STK_PAUSE_CHAR == ucSecondNumber))
        {
            ucFlag = VOS_TRUE;

            break;
        }
    }

    if (VOS_FALSE == ucFlag) /* 找不到暂停 */
    {
        return VOS_OK;
    }

    if (STK_PAUSE_CHAR == ucFirstNumber)
    {
        g_stSTKDtmfData.acDtfm[0] = 'P';

        g_stSTKDtmfData.ulDtmfLen = 1;

        pstAddr->pucAddr[i] = (VOS_UINT8)(0xF0 | ucSecondNumber);

        i++;
    }

    SI_STK_BcdStrToAscii((SI_UINT8)(ulSrcBcdLen - i),
                            &pstAddr->pucAddr[i],
                            (VOS_UINT8*)&g_stSTKDtmfData.acDtfm[g_stSTKDtmfData.ulDtmfLen],
                            &ulPauseCharLen,
                            SI_STK_STR_CHANGEBIG);

    /* 找到pause字符后需要修改原BCD码长度 */
    pstAddr->ucLen      = (VOS_UINT8)(i);

    g_stSTKDtmfData.ulDtmfLen += ulPauseCharLen;

    return VOS_OK;
}
VOS_UINT32  SI_STK_SendStartDtmfMsg(VOS_CHAR cDtmf, VOS_UINT8 ucOpId, VOS_UINT8 ucCallId)
{
    TAF_CALL_DTMF_PARAM_STRU            stTafStartDtmfPara;
    VOS_UINT32                          ulResult;

    VOS_MemSet(&stTafStartDtmfPara, 0, sizeof(TAF_CALL_DTMF_PARAM_STRU));

    stTafStartDtmfPara.CallId       = VOS_NULL; /*默认使用当前的呼叫ID*/
    stTafStartDtmfPara.cKey         = cDtmf;
    stTafStartDtmfPara.usOnLength   = TAF_CALL_DTMF_DEFAULT_ONLENGTH;
    stTafStartDtmfPara.usOffLength  = VOS_NULL;

    ulResult = SI_STK_SendTAFMsg(STK_CALL_START_DTMF_REQ,
                                 ucOpId,
                                 g_stStkSetupCallCtrl.ucSetupCallId,
                                 (VOS_UINT8*)&stTafStartDtmfPara,
                                 sizeof(stTafStartDtmfPara));

    if (VOS_OK != ulResult)
    {
        STK_ERROR_LOG("SI_STK_SendStartDtmfMsg:Send msg fail.");

        return SI_STK_DTMF_SEND_ERR;
    }

    return SI_STK_DTMF_SEND_OK;
}
SI_STK_DTMF_SEND_ENUM_UINT32  SI_STK_SendStopDtmfMsg(VOS_UINT8 ucOpID)
{
    VOS_UINT32                      ulResult;
    TAF_CALL_DTMF_PARAM_STRU        stTafStartDtmfPara = {0};   /*保证输入给TAF的内容是全0*/

    ulResult = SI_STK_SendTAFMsg(STK_CALL_STOP_DTMF_REQ,
                                ucOpID,
                                g_stStkSetupCallCtrl.ucSetupCallId,
                                (VOS_UINT8*)&stTafStartDtmfPara,
                                sizeof(stTafStartDtmfPara));

    if (VOS_OK != ulResult)
    {
        STK_ERROR_LOG("SI_STK_SendStopDtmfMsg:Send msg fail.");

        return SI_STK_DTMF_SEND_ERR;
    }

    return SI_STK_DTMF_SEND_OK;
}
SI_STK_DTMF_SEND_ENUM_UINT32 SI_STK_SendDtmfToTAF(VOS_UINT8 ucOpId, VOS_UINT8 ucCallId)
{
    VOS_CHAR                            cDtmf = 0;
    VOS_UINT32                          ulCurrSnd;
    VOS_UINT32                          ulDtmfStringLen;
    VOS_UINT32                          ulCounter;
    VOS_UINT32                          i;

    /* 当前已经没有DTMF发送，状态机重置 */
    if (g_stSTKDtmfData.ulDtmfLen <= g_stSTKDtmfData.ulCurDtmfPtr)
    {
        STK_INFO_LOG("SI_STK_SendDtmfToTAF: The DTMF is Send All");

        return SI_STK_DTMF_SEND_FINISH;
    }

    /* 设置DTMF opid和callid */
    g_stSTKDtmfData.ucOpId      =   ucOpId;
    g_stSTKDtmfData.ucCallId    =   ucCallId;

    /* 当前还有DTMF要发送 */
    ulDtmfStringLen             =   (g_stSTKDtmfData.ulDtmfLen - g_stSTKDtmfData.ulCurDtmfPtr);
    ulCounter                   =   0;

    for (i = 0; i < ulDtmfStringLen; i++)
    {
        ulCurrSnd   = g_stSTKDtmfData.ulCurDtmfPtr;
        cDtmf       = (VOS_CHAR)g_stSTKDtmfData.acDtfm[ulCurrSnd];

        if ('P' == cDtmf)
        {
            /* 统计P的个数，按协议规定发次发送DTMF遇到PAUSE字符要暂停3秒 */
            ulCounter++;
            g_stSTKDtmfData.ulCurDtmfPtr++;
        }
        else
        {
            break;
        }
    }

    /* DTMF字符串中全部都是P字符 */
    if (i >= ulDtmfStringLen)
    {
        return SI_STK_DTMF_SEND_FINISH;
    }

    /* DTMF中第一个字符是PAUSE字符 */
    if (VOS_NULL != ulCounter)
    {
    /* coverity[check_return] */
        STK_DTMF_PAUSE_TIMER_START(ulCounter * STK_SEND_DTMF_PAUSE_TIME);

        return SI_STK_DTMF_SEND_OK;
    }

    if (VOS_OK != SI_STK_SendStartDtmfMsg(cDtmf, ucOpId, ucCallId))
    {
        return SI_STK_DTMF_SEND_ERR;
    }

    g_stSTKDtmfData.ulCurDtmfPtr++;

    return SI_STK_DTMF_SEND_OK;
}
VOS_VOID SI_STK_SuspendCnfMsgProc(MN_CALL_INFO_STRU *pstEvent)
{
    VOS_UINT32                          ulResult;
    VOS_UINT8                           aucRspNet[2] = {NETWORK_CURRENTLY_UNABLE_TO_PROCESS_COMMAND,
                                                        NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_NETWORK};
    VOS_UINT8                           aucRspTerminal[2] = {TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND,
                                                            NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_TERMINAL};

    ulResult =  VOS_OK;

    /* 当前SETUP CALL主动命令未处理完，等待NAS回复DISCONNECT的消息 */
    if (SI_STK_CALL_STATUS_WAIT_DISCONNECT == g_stStkSetupCallCtrl.enCallStatus)
    {
        /* 挂断请求操作失败 */
        if (MN_CALL_SS_RES_SUCCESS != pstEvent->enSsResult)
        {
            /* 回复TERMINAL RESPONSE结果:当前网络无法处理无特定原因，整个SETUP CALL命令处理结果 */
            SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucRspNet), aucRspNet);

            /* 清除状态机，清空DTMF资源 */
            STK_SetCallCleanGobal();

            return;
        }
        /* 挂断请求操作成功，发起呼叫 */
        else
        {
            ulResult =  SI_STK_SetUpCall();
        }
    }
    /* 当前SETUP CALL主动命令未处理完，等待NAS回复SUSPEND的回复消息 */
    else if (SI_STK_CALL_STATUS_WAIT_HOLD == g_stStkSetupCallCtrl.enCallStatus)
    {
        /* 挂起请求操作失败 */
        if (MN_CALL_SS_RES_SUCCESS != pstEvent->enSsResult)
        {
            /* 回复TERMINAL RESPONSE结果:当前网络无法处理无特定原因，整个SETUP CALL命令处理结果 */
            SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucRspNet), aucRspNet);

            /* 清除状态机，清空DTMF资源 */
            STK_SetCallCleanGobal();

            return;
        }
        /* 挂起或挂断请求操作成功，发起呼叫 */
        else
        {
            ulResult =  SI_STK_SetUpCall();
        }
    }
    else
    {
        /* do nothing */
    }

    if (VOS_OK != ulResult)
    {
        /* 回复TERMINAL RESPONSE结果:当前网络无法处理无特定原因，整个SETUP CALL命令处理结果 */
        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucRspTerminal), aucRspTerminal);

        /* 清除状态机，清空DTMF资源 */
        STK_SetCallCleanGobal();
    }

    return;
}
VOS_UINT32 SI_STK_SendCallSupsMsg(MN_CALL_SUPS_CMD_ENUM_U8 enSupsCmd, MN_CALL_ID_T ucCallId)
{
    MN_CALL_SUPS_PARAM_STRU             stSupsPara;

    /* 填充消息内容结构 */
    stSupsPara.callId        =   ucCallId;
    stSupsPara.enCallSupsCmd =   enSupsCmd;

    return SI_STK_SendTAFMsg(STK_CALL_SUPS_CMD_REQ,
                                STK_SETUP_CALL_OP_ID,
                                g_stStkSetupCallCtrl.ucSetupCallId,
                                (VOS_UINT8*)&stSupsPara,
                                sizeof(stSupsPara));
}
VOS_UINT32 SI_STK_SetUpCall(VOS_VOID)
{
    MN_APP_CALL_CALLORIG_REQ_STRU      *pstOrgiCallReq;
    VOS_UINT32                          ulRcvPid;

    if (VOS_OK != SI_STK_GetRecTAFPID(&ulRcvPid))
    {
        STK_ERROR_LOG("SI_STK_SetUpCall: NO CS Modem");

        return VOS_ERR;
    }

    pstOrgiCallReq  = (MN_APP_CALL_CALLORIG_REQ_STRU *)VOS_AllocMsg(MAPS_STK_PID,
                                                                    sizeof(MN_APP_CALL_CALLORIG_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pstOrgiCallReq)
    {
        STK_ERROR_LOG("SI_STK_SetUpCall: Alloc Msg call fail");

        return VOS_ERR;
    }

    //VOS_MemSet(&(pstOrgiCallReq->stSubAddr), 0, sizeof(pstOrgiCallReq->stSubAddr));
    //VOS_MemSet(&(pstOrgiCallReq->stBc), 0, sizeof(pstOrgiCallReq->stBc));
    VOS_MemSet((VOS_CHAR*)pstOrgiCallReq+VOS_MSG_HEAD_LENGTH, 0, sizeof(MN_APP_CALL_CALLORIG_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    /* 填充消息结构，callID暂时填0，等待NAS回复消息分配正式的ID */
    pstOrgiCallReq->ulReceiverPid       =   ulRcvPid;
    pstOrgiCallReq->callID              =   VOS_NULL;
    pstOrgiCallReq->opID                =   STK_SETUP_CALL_OP_ID;
    pstOrgiCallReq->usClientId          =   OAM_CLIENT_ID_STK;
    pstOrgiCallReq->ulMsgId             =   STK_CALL_CALLORIG_REQ;

    pstOrgiCallReq->stCalledAddr.ucAddrType =   gstUsatCmdDecode.CmdStru.SetupCall.Addr.ucNumType;
    pstOrgiCallReq->stCalledAddr.ucLen      =   gstUsatCmdDecode.CmdStru.SetupCall.Addr.ucLen;

    VOS_MemCpy(pstOrgiCallReq->stCalledAddr.aucAddr,
               gstUsatCmdDecode.CmdStru.SetupCall.Addr.pucAddr,
               gstUsatCmdDecode.CmdStru.SetupCall.Addr.ucLen);

    /* 如果子地址存在 */
    if (VOS_TRUE == gstUsatCmdDecode.CmdStru.SetupCall.OP_Subaddr)
    {
        if (STK_CALL_SUBADDR_MAX_LEN < (VOS_UINT8)gstUsatCmdDecode.CmdStru.SetupCall.Subaddr.ulLen)
        {
            pstOrgiCallReq->stSubAddr.ucLen =   STK_CALL_SUBADDR_MAX_LEN;
        }
        else
        {
            pstOrgiCallReq->stSubAddr.ucLen =   (VOS_UINT8)gstUsatCmdDecode.CmdStru.SetupCall.Subaddr.ulLen;
        }

        VOS_MemCpy(pstOrgiCallReq->stSubAddr.aucSubAddr,
                   gstUsatCmdDecode.CmdStru.SetupCall.Subaddr.pucSubaddr,
                   pstOrgiCallReq->stSubAddr.ucLen);
    }

    /* 如果CCP参数存在 */
    if (VOS_TRUE == gstUsatCmdDecode.CmdStru.SetupCall.OP_CCPara)
    {
        if (STK_CALL_BC_MAX_LEN < gstUsatCmdDecode.CmdStru.SetupCall.CCPara.pucCCP[0])
        {
            pstOrgiCallReq->stBc.ucLen          =   STK_CALL_BC_MAX_LEN;
        }
        else
        {
            pstOrgiCallReq->stBc.ucLen          =   (VOS_UINT8)gstUsatCmdDecode.CmdStru.SetupCall.CCPara.pucCCP[0];
        }

        VOS_MemCpy(pstOrgiCallReq->stBc.aucBc,
                   &gstUsatCmdDecode.CmdStru.SetupCall.CCPara.pucCCP[1],
                   pstOrgiCallReq->stBc.ucLen);
    }

    if (VOS_OK != VOS_SendMsg(MAPS_STK_PID, pstOrgiCallReq))
    {
        STK_ERROR_LOG("SI_STK_SetUpCall: Send Msg call fail");

        return VOS_ERR;
    }

    /* 启动DURATION定时器 */
    SI_STK_SetUpCallStartDurationTimer(gstUsatCmdDecode.CmdStru.SetupCall.OP_Duration,
                                       &gstUsatCmdDecode.CmdStru.SetupCall.Duration);

    /* 设置等待NAS回复CONNECT消息 */
    STK_CallStateMachineSet(SI_STK_CALL_STATUS_WAIT_CONNECT);

    return VOS_OK;
}
VOS_UINT32 SI_STK_HoldCall(VOS_VOID)
{
    /* 发送消息，挂起所有呼叫 */
    if (VOS_OK != SI_STK_SendCallSupsMsg(MN_CALL_SUPS_CMD_HOLD_ACT_ACPT_OTH, STK_ALL_CALL_ID))
    {
        STK_ERROR_LOG("SI_STK_HoldCall: Send Suspend Msg call fail");

        /* STK主动命令处理完成 */
        STK_CallStateMachineSet(SI_STK_CALL_STATUS_WAIT_IDLE);

        return VOS_ERR;
    }
    else
    {
        /* 设置等待NAS回复MN_CALL_EVT_SS_CMD_CNF消息 */
        STK_CallStateMachineSet(SI_STK_CALL_STATUS_WAIT_HOLD);

        return VOS_OK;
    }
}
VOS_UINT32 SI_STK_DisconnectCall(MN_CALL_ID_T ucCallId)
{
    /* 发送消息，挂断指定呼叫 */
    if (VOS_OK != SI_STK_SendCallSupsMsg(MN_CALL_SUPS_CMD_REL_ALL_CALL, ucCallId))
    {
        STK_ERROR_LOG("SI_STK_DisconnectCall: Send Suspend Msg call fail");

        return VOS_ERR;
    }

    return VOS_OK;
}
VOS_UINT32 SI_STK_DisconnectAllCall(VOS_VOID)
{
    if (VOS_OK != SI_STK_DisconnectCall(STK_ALL_CALL_ID))
    {
        STK_ERROR_LOG("SI_STK_DisconnectAllCall: Disconnect call fail");

        /* STK主动命令处理完成 */
        STK_CallStateMachineSet(SI_STK_CALL_STATUS_WAIT_IDLE);

        return VOS_ERR;
    }

    /* 设置等待NAS回复MN_CALL_EVT_SS_CMD_CNF消息 */
    STK_CallStateMachineSet(SI_STK_CALL_STATUS_WAIT_DISCONNECT);

    return VOS_OK;
}


VOS_UINT32 SI_STK_DisconnectStkCall(VOS_VOID)
{
    if (VOS_OK != SI_STK_DisconnectCall(g_stStkSetupCallCtrl.ucSetupCallId))
    {
        STK_WARNING_LOG("SI_STK_DisconnectStkCall: Disconnect call fail");
    }

    /* STK主动命令处理完成 */
    STK_CallStateMachineSet(SI_STK_CALL_STATUS_WAIT_IDLE);

    return VOS_OK;
}
VOS_VOID SI_STK_SetUpCallStartDurationTimer(VOS_UINT32 ulFlg, SI_STK_DURATION_STRU *pstDuration)
{
    VOS_UINT32                          ulTimerLen;

    if (VOS_TRUE == ulFlg)
    {
        /* 计算DURATION的时长 */
        switch(pstDuration->ucUnit)
        {
            case STK_DURATION_MIN:
                ulTimerLen = 60 * pstDuration->ucInternal * 1000;
                break;
            case STK_DURATION_SEC:
                ulTimerLen = pstDuration->ucInternal * 1000;
                break;
            case STK_DURATION_TENTH_SEC:
                ulTimerLen = pstDuration->ucInternal * 100;
                break;
            default:
                ulTimerLen = STK_SETUP_CALL_DURATION_DEFAULT * 1000;
                break;
        }

        /* 启动DURATION定时器 */
        /* coverity[check_return] */
        STK_SETUP_CALL_DURATION_TIMER_START(ulTimerLen);

        /* 如果启动定时器就把主动命令的保护定时器停止 */
        STK_PROTECT_TIMER_STOP;
    }

    return;
}


VOS_VOID SI_STK_SetUpCallStopDurationTimer(VOS_UINT32 ulFlg)
{
    if (VOS_TRUE == ulFlg)
    {
        STK_SETUP_CALL_DURATION_TIMER_STOP;
    }

    return;
}


VOS_UINT32 SI_STK_SetUpCallOnlyNotBusyOnCall(VOS_VOID)
{
    VOS_UINT32                          ulResult = VOS_ERR;
    VOS_UINT8                           aucRspData[2] = {TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND,
                                                        ME_CURRENTLY_BUSY_ON_CALL};

    if (VOS_TRUE == STK_IsCallInService())
    {
        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucRspData), aucRspData);

        STK_WARNING_LOG("SI_STK_SetUpCallOnlyNotBusyOnCall: Busy on call");
    }
    else
    {
        ulResult = SI_STK_SetUpCall();
    }

    return ulResult;
}


VOS_UINT32 SI_STK_SetUpCallDisconnectBusyCall(VOS_VOID)
{
    VOS_UINT32                          ulResult = VOS_OK;
    VOS_UINT8                           aucRspData[2] = {TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND,
                                                        NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_TERMINAL};

    if (VOS_TRUE == STK_IsCallInService())
    {
        ulResult = SI_STK_DisconnectAllCall();
    }
    else
    {
        ulResult = SI_STK_SetUpCall();
    }

    if (VOS_OK != ulResult)
    {
        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucRspData), aucRspData);

        STK_WARNING_LOG("SI_STK_SetUpCallDisconnectBusyCall fail");
    }

    return ulResult;
}


VOS_UINT32 SI_STK_SetUpCallHoldBusyCall(VOS_VOID)
{
    VOS_UINT32                  ulResult = VOS_OK;
    VOS_UINT8                           aucRspData[2] = {TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND,
                                                         NO_SPECIFIC_CAUSE_CAN_BE_GIVEN_BY_TERMINAL};

    if (VOS_TRUE == STK_IsCallInService())
    {
        ulResult = SI_STK_HoldCall();
    }
    else
    {
        ulResult = SI_STK_SetUpCall();
    }

    if (VOS_OK != ulResult)
    {
        SI_STKSimpleResponseData(&gstUsatCmdDecode.CmdDetail, sizeof(aucRspData), aucRspData);

        STK_WARNING_LOG("SI_STK_SetUpCallHoldBusyCall:SI_STK_SetUpCallHoldBusyCall fail");
    }

    return ulResult;
}


VOS_VOID SI_STK_SetUpCallAllow(SI_STK_SETUPCALLCONFIRM_ENUM_UINT32 enAction)
{
    VOS_UINT32                  ulResult = VOS_ERR;

    /* 当前是否在处理SETUP CALL命令 */
    if (SI_STK_SETUPCALL != gstUsatCmdDecode.SatType)
    {
        STK_ERROR_LOG("SI_STK_SetUpCallAllow: STK Command is not SetUpCall.");

        return;
    }

    if (SI_STK_SETUPCALL_ALLOW == enAction)
    {
        if (VOS_OK != SI_STK_GetPauseCharFromBCD(&gstUsatCmdDecode.CmdStru.SetupCall.Addr))
        {
            STK_ERROR_LOG("SI_STK_SetUpCallAllow:Parse DTMF fail.");

            /* 清空DTMF资源，TERMINAL RESPONSE回复数据错误 */
            STK_ClearPauseCharGobal();

            SI_STKSimpleResponse(&gstUsatCmdDecode.CmdDetail, SI_STK_DATA_ERROR);

            return;
        }

        if (STK_CALL_ADDR_MAX_LEN < gstUsatCmdDecode.CmdStru.SetupCall.Addr.ucLen)
        {
            STK_ERROR_LOG("SI_STK_SetUpCallAllow: CS call address to long");

            /* 清空DTMF资源，TERMINAL RESPONSE回复数据错误 */
            SI_STKSimpleResponse(&gstUsatCmdDecode.CmdDetail, SI_STK_DATA_ERROR);

            return;
        }

        /* 仅当前无呼叫发起呼叫 */
        if ((SI_STK_SETUP_CALL_ONLY_NOT_BUSY_ON_ANOTHER_CALL == gstUsatCmdDecode.CmdDetail.ucCommandQua)
         || (SI_STK_SETUP_CALL_ONLY_NOT_BUSY_ON_ANOTHER_CALL_REDIAL == gstUsatCmdDecode.CmdDetail.ucCommandQua))
        {
            ulResult    =   SI_STK_SetUpCallOnlyNotBusyOnCall();
        }
        /* 当前有呼叫挂起呼叫再发起呼叫 */
        else if((SI_STK_SETUP_CALL_PUTTING_ALL_OTHER_CALL_ON_HOLD == gstUsatCmdDecode.CmdDetail.ucCommandQua)
         || (SI_STK_SETUP_CALL_PUTTING_ALL_OTHER_CALL_ON_HOLD_REDIAL == gstUsatCmdDecode.CmdDetail.ucCommandQua))
        {
            ulResult    =   SI_STK_SetUpCallHoldBusyCall();
        }
        /* 当前有呼叫挂断呼叫再发起呼叫 */
        else if((SI_STK_SETUP_CALL_DISCONNECT_ALL_OTHER_CALL == gstUsatCmdDecode.CmdDetail.ucCommandQua)
         || (SI_STK_SETUP_CALL_DISCONNECT_ALL_OTHER_CALL_REDIAL == gstUsatCmdDecode.CmdDetail.ucCommandQua))
        {
            ulResult    =   SI_STK_SetUpCallDisconnectBusyCall();
        }
        else
        {
            /* CQ不正确，回复数据错误 */
            STK_ERROR_LOG("SI_STK_SetUpCallAllow:Incorrect command qualifier.");

            SI_STKSimpleResponse(&gstUsatCmdDecode.CmdDetail, SI_STK_DATA_ERROR);
        }
    }
    else
    {
        /* 用户不同意发起呼叫 */
        SI_STKSimpleResponse(&gstUsatCmdDecode.CmdDetail, USER_DO_NOT_ACCEPT_THE_PROACTIVE_COMMAND);
    }

    if (VOS_OK != ulResult)
    {
        STK_ERROR_LOG("SI_STK_SetUpCallAllow:Result error.");

        STK_ClearPauseCharGobal();
    }

    return;
}


VOS_UINT32 SI_STK_TimerManageProc(SI_STK_DATA_INFO_STRU *pCmdData)
{
    VOS_UINT32              ultimerID;
    VOS_UINT32              ultimerLen;
    VOS_UINT32              ulResult = VOS_OK;
    VOS_UINT8               ucType;
    SI_STK_TIMERMANAGE_STRU *pstTimerMag;
    VOS_UINT8               aucTpr[] = {0x81,0x03,0x00,0x27,0x00,0x82,0x02,0x82,0x81,0x83,0x01,0x00,0xA4,0x01,0x00,0xA5,0x03,0x00,0x00,0x00};
    VOS_UINT8               ucTrLen = sizeof(aucTpr);

    pstTimerMag = (SI_STK_TIMERMANAGE_STRU *)&pCmdData->CmdStru;

    ultimerID = pstTimerMag->TimerID.ucTimerId;

    if (0 == pstTimerMag->OP_TimerID)
    {
        STK_ERROR_LOG("SI_STK_TimerManageProc:OP_TimerID is NULL.");

        return SI_STK_DATA_ERROR;
    }

    if ((STK_TIMER_MAX_NUM-1) < pstTimerMag->TimerID.ucTimerId)
    {
        STK_ERROR_LOG("SI_STK_TimerManageProc:OP_TimerID is too Big.");

        return SI_STK_DATA_ERROR;
    }

    VOS_MemCpy(&aucTpr[2], &pCmdData->CmdDetail, 3);

    ucType = (VOS_UINT8)(pCmdData->CmdDetail.ucCommandQua & 0x03);

    if (STK_TIME_MANAGEMENT_START == ucType)
    {
        if (VOS_NULL == pstTimerMag->OP_TimerValue)
        {
            STK_ERROR_LOG("SI_STK_TimerManageProc:OP_TimerValue is NULL.");

            return SI_STK_DATA_ERROR;
        }

        /*STK主动命令上报的定时器时长都是BCD码，需要转换成16进制数来处理*/
        gstSTKTimer[ultimerID].ucHour = STK_BCD2HEX(pstTimerMag->TimerValue.ucHour);
        gstSTKTimer[ultimerID].ucMin  = STK_BCD2HEX(pstTimerMag->TimerValue.ucMinute);
        gstSTKTimer[ultimerID].ucSec  = STK_BCD2HEX(pstTimerMag->TimerValue.ucSecond);

        if ( (STK_TIMER_MINUTE_VALUE_MAX < gstSTKTimer[ultimerID].ucMin)
          || (STK_TIMER_SECOND_VALUE_MAX < gstSTKTimer[ultimerID].ucSec) )
        {
            STK_ERROR_LOG("SI_STK_TimerManageProc:TimerValue is too Big.");

            return SI_STK_NOTSUPPORT;
        }

        gstSTKTimer[ultimerID].ucHour = (gstSTKTimer[ultimerID].ucHour > STK_TIMER_HOUR_VALUE_MAX)?\
                                            STK_TIMER_HOUR_VALUE_MAX:gstSTKTimer[ultimerID].ucHour;

        aucTpr[14] = (VOS_UINT8)ultimerID;

        ultimerLen = ((((gstSTKTimer[ultimerID].ucHour*60) + gstSTKTimer[ultimerID].ucMin)*60) + gstSTKTimer[ultimerID].ucSec) * 1000;
        /* coverity[check_return] */
        ulResult = STK_TIMER_MANAGEMENT_TIMER_START(ultimerID, ultimerLen);

        ucTrLen = 15;

    }
    else if((STK_TIME_MANAGEMENT_DEACTIVE == ucType)
            ||(STK_TIME_MANAGEMENT_GET_VALUE == ucType))
    {
        if(VOS_NULL_PTR != gstSTKTimer[ultimerID].stTimer)/*未起该定时器*/
        {
            if(VOS_OK != VOS_GetRelTmRemainTime(&gstSTKTimer[ultimerID].stTimer, &ultimerLen))
            {
                STK_ERROR_LOG("SI_STK_TimerManageProc:Start Timer Error.");

                return ACTION_IN_CONTRADICTION_WITH_THE_CURRENT_TIMER_STATE;
            }

            ultimerLen = ultimerLen / 100;

            aucTpr[14] = (VOS_UINT8)ultimerID;
            aucTpr[17] = STK_HEX2BCD((VOS_UINT8)(ultimerLen/3600));
            aucTpr[18] = STK_HEX2BCD((VOS_UINT8)((ultimerLen/60)%60));
            aucTpr[19] = STK_HEX2BCD((VOS_UINT8)(ultimerLen%60));

        }
        /*如果未起该定时器或者获取剩余时间失败，则返回对应错误原因值*/
        else
        {
            STK_ERROR_LOG("SI_STK_TimerManageProc:NO Timer is Running.");

            return ACTION_IN_CONTRADICTION_WITH_THE_CURRENT_TIMER_STATE;
        }

        if(STK_TIME_MANAGEMENT_DEACTIVE == ucType)
        {
            ulResult = STK_TIMER_MANAGEMENT_TIMER_STOP(ultimerID);
        }
    }
    else
    {
        ulResult = VOS_ERR;
    }

    if(VOS_ERR != ulResult)
    {
        if(VOS_ERR != USIMM_TerminalResponseReq(MAPS_STK_PID, ucTrLen, aucTpr))
        {
            STK_ERROR_LOG("SI_STK_TimerManageProc:USIMM_TerminalResponseReq is Failed.");

            return SI_STK_NORP;
        }
    }

    return SI_STK_NOTSUPPORT;
}


VOS_UINT32 SI_STK_ProvideLocalInfo_IMEI(VOS_UINT8 *pTrData)
{
    VOS_UINT8   aucIMEI[10];
    VOS_UINT8   aucIMEIData[STK_IMEI_LEN+2];

    if(VOS_OK != NV_Read(en_NV_Item_IMEI, (VOS_VOID*)(aucIMEIData+1), STK_IMEI_LEN))
    {
        STK_ERROR_LOG("SI_STK_ProvideLocalInfo_IMEI:Read NV Failed.");

        pTrData[STK_TERMINAL_RSP_LEN - 1] = COMMAND_BEYOND_TERMINALS_CAPABILITIES;

        return STK_TERMINAL_RSP_LEN;
    }

    aucIMEIData[0]  = STK_IMEI_TYPE;
    aucIMEIData[15] = 0;
    aucIMEI[0]      = IMEI_TAG|0x80;
    aucIMEI[1]      = 1+(STK_IMEI_LEN/2);

    SI_STK_Num2BCD(aucIMEIData, aucIMEI+2, STK_IMEI_LEN+2);

    VOS_MemCpy((pTrData+STK_TERMINAL_RSP_LEN), aucIMEI, sizeof(aucIMEI));

    return (sizeof(aucIMEI)+STK_TERMINAL_RSP_LEN);
}


VOS_UINT32 SI_STK_ProvideLocalInfo_IMEISV(VOS_UINT8 *pTrData)
{
    VOS_UINT8   aucIMEISV[11];
    VOS_UINT8   aucIMEISVData[STK_IMEISV_LEN+1];

    if(VOS_OK != NV_Read(en_NV_Item_IMEI, (VOS_VOID*)(aucIMEISVData+1), STK_IMEISV_LEN))
    {
        STK_ERROR_LOG("SI_STK_ProvideLocalInfo_IMEISV:Read NV Failed.");

        pTrData[STK_TERMINAL_RSP_LEN - 1] = COMMAND_BEYOND_TERMINALS_CAPABILITIES;

        return STK_TERMINAL_RSP_LEN;
    }

    aucIMEISVData[0] = STK_IMEISV_TYPE;
    aucIMEISV[0]     = IMEISV_TAG | 0x80;
    aucIMEISV[1]     = 1+(STK_IMEISV_LEN/2);

    SI_STK_Num2BCD(aucIMEISVData, aucIMEISV+2, STK_IMEISV_LEN+1);

    VOS_MemCpy((pTrData+STK_TERMINAL_RSP_LEN), aucIMEISV, sizeof(aucIMEISV));

    return (sizeof(aucIMEISV)+STK_TERMINAL_RSP_LEN);
}


VOS_UINT32 SI_STK_ProvideLocalInfo_ACCESS(VOS_UINT8 *pTrData, SI_STK_ACCESS_TECH_ENUM_UINT8 ucAccessTech)
{
    VOS_UINT8                           aucAccessTech[3] = {0x3F,0x01,0x00};
    VOS_UINT8                           ucProfileLen;
    VOS_UINT8                          *pucProfile;

    STK_GetProfileInfo(&pucProfile, &ucProfileLen);

    if(VOS_FALSE == SI_STKGetBitFromBuf(pucProfile, SI_STK_LOCALINFO_ACCESS, ucProfileLen))
    {
        pTrData[STK_TERMINAL_RSP_LEN - 1] = COMMAND_BEYOND_TERMINALS_CAPABILITIES;

        STK_ERROR_LOG("SI_STK_ProvideLocalInfo_ACCESS:Terminal Profile is Set Not Support.");

        return STK_TERMINAL_RSP_LEN;
    }

    if (STK_ACCESS_TECH_BUTT == ucAccessTech)
    {
        pTrData[10] = 0x02;
        pTrData[11] = TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND;
        pTrData[12] = NO_SERVICE;

        STK_ERROR_LOG("SI_STK_ProvideLocalInfo_ACCESS: No Service.");

        return STK_TERMINAL_RSP_LEN + 1;
    }
    else
    {
        aucAccessTech[2] = ucAccessTech;
    }

    VOS_MemCpy((pTrData+STK_TERMINAL_RSP_LEN), aucAccessTech, sizeof(aucAccessTech));

    return (sizeof(aucAccessTech)+STK_TERMINAL_RSP_LEN);
}


VOS_UINT32 SI_STK_ProvideLocalInfo_NAA(VOS_UINT8 *pTrData, SI_STK_NAS_INFO_STRU *pstMmaInfo)
{
    VOS_UINT8                           aucNaa[] = {0x93,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    VOS_UINT8                           ucProfileLen;
    VOS_UINT8                          *pucProfile;

    STK_GetProfileInfo(&pucProfile, &ucProfileLen);

    if (VOS_FALSE == SI_STKGetBitFromBuf(pucProfile, SI_STK_LOCALINFO_NETINFO, ucProfileLen))
    {
        pTrData[STK_TERMINAL_RSP_LEN - 1] = COMMAND_BEYOND_TERMINALS_CAPABILITIES;

        STK_ERROR_LOG("SI_STK_ProvideLocalInfo_NAA:Terminal Profile is Set Not Support.");

        return STK_TERMINAL_RSP_LEN;
    }

    if (SI_STK_NO_SERVICE == STK_GetSvcStatus())
    {
        STK_ERROR_LOG("SI_STK_ProvideLocalInfo_NAA:ulServiceStatus is NULL.");

        pTrData[10] = 0x02;
        pTrData[11] = TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND;
        pTrData[12] = NO_SERVICE;

        return (STK_TERMINAL_RSP_LEN + 1);
    }

    /* 转换MCC为IMSI的格式 */
    aucNaa[2] =  (VOS_UINT8)(pstMmaInfo->stSysInfo.stCurPlmnId.Mcc&0x00000F);
    aucNaa[2] |= (VOS_UINT8)((pstMmaInfo->stSysInfo.stCurPlmnId.Mcc&0x000F00)>>4);
    aucNaa[3] =  (VOS_UINT8)((pstMmaInfo->stSysInfo.stCurPlmnId.Mcc&0x0F0000)>>16);

    /*转换MNC为IMSI的格式*/
    aucNaa[3] |= (VOS_UINT8)((pstMmaInfo->stSysInfo.stCurPlmnId.Mnc&0x0F0000)>>12);
    aucNaa[4] =  (VOS_UINT8)(pstMmaInfo->stSysInfo.stCurPlmnId.Mnc&0x00000F);
    aucNaa[4] |= (VOS_UINT8)((pstMmaInfo->stSysInfo.stCurPlmnId.Mnc&0x000F00)>>4);

    /*转换LAC的格式*/
    aucNaa[5] = (VOS_UINT8)((pstMmaInfo->stSysInfo.usLac&0xFF00)>>8);
    aucNaa[6] = (VOS_UINT8)(pstMmaInfo->stSysInfo.usLac&0x00FF);

    /*转换CellId的格式*/
    aucNaa[7] = (VOS_UINT8)((pstMmaInfo->stSysInfo.ulCellId&0xFF00)>>8);
    aucNaa[8] = (VOS_UINT8)(pstMmaInfo->stSysInfo.ulCellId&0x00FF);

    if (SI_STK_NORMAL_SERVICE == STK_GetSvcStatus())
    {
        pTrData[11] = COMMAND_PERFORMED_SUCCESSFULLY;
    }
    else
    {
        pTrData[11] = COMMAND_PERFORMED_SUCCESSFULLY_LIMITED_SERVICE;
    }

    if (STK_ACCESS_TECH_GSM == pstMmaInfo->stSysInfo.enCurRat)
    {
        aucNaa[1] = 0x07;

        VOS_MemCpy((pTrData+STK_TERMINAL_RSP_LEN), aucNaa, STK_LOCAL_INFO_2G);

        return (STK_LOCAL_INFO_2G+STK_TERMINAL_RSP_LEN);
    }
    else if(STK_ACCESS_TECH_EUTRAN == pstMmaInfo->stSysInfo.enCurRat)
    {
        /*转换CellId的格式*/
        aucNaa[7] = (VOS_UINT8)((pstMmaInfo->stSysInfo.ulCellId&0xFF00000)>>20);
        aucNaa[8] = (VOS_UINT8)((pstMmaInfo->stSysInfo.ulCellId&0xFF000)>>12);
        aucNaa[9] = (VOS_UINT8)((pstMmaInfo->stSysInfo.ulCellId&0x0FF0)>>4);
        aucNaa[10] = (VOS_UINT8)(((pstMmaInfo->stSysInfo.ulCellId&0x000F)<<4)|0x0F);

        VOS_MemCpy((pTrData+STK_TERMINAL_RSP_LEN), aucNaa, sizeof(aucNaa));

        return (sizeof(aucNaa)+STK_TERMINAL_RSP_LEN);
    }
    else
    {
        aucNaa[1] = 0x09;

        aucNaa[9] = (VOS_UINT8)((pstMmaInfo->stSysInfo.ulCellId&0xFF0000)>>16);

        aucNaa[10] = (VOS_UINT8)((pstMmaInfo->stSysInfo.ulCellId&0xFF000000)>>24);

        VOS_MemCpy((pTrData+STK_TERMINAL_RSP_LEN), aucNaa, sizeof(aucNaa));

        return (sizeof(aucNaa)+STK_TERMINAL_RSP_LEN);
    }
}
VOS_UINT32 SI_STK_ProvideLocalInfo_SearchMode(VOS_UINT8 *pTrData)
{
    VOS_UINT8   aucShMode[3] = {0x65,0x01,0x00};

    aucShMode[2] = STK_GetSearchMode();

    /* 当前没有搜网模式 */
    if ((SI_STK_SEARCH_MODE_AUTO != aucShMode[2])
     && (SI_STK_SEARCH_MODE_MANUAL != aucShMode[2]))
    {
        pTrData[STK_TERMINAL_RSP_LEN - 2] = 0x2;
        pTrData[STK_TERMINAL_RSP_LEN - 1] = TERMINAL_CURRENTLY_UNABLE_TO_PROCESS_COMMAND;
        pTrData[STK_TERMINAL_RSP_LEN] = NO_SERVICE;

        STK_ERROR_LOG("SI_STK_ProvideLocalInfo_ACCESS: No Service.");

        return STK_TERMINAL_RSP_LEN + 1;
    }

    VOS_MemCpy((pTrData+STK_TERMINAL_RSP_LEN), aucShMode, sizeof(aucShMode));

    return (sizeof(aucShMode)+STK_TERMINAL_RSP_LEN);
}
VOS_UINT32 SI_STK_GetGasTAReq(VOS_VOID)
{
    STK_AS_TA_INFO_REQ_STRU            *pstTAReqMsg;
    VOS_UINT32                          ulRecPID;

    if (VOS_OK != SI_STK_GetRecGASPID(&ulRecPID))
    {
        STK_ERROR_LOG("SI_STK_GetGasTAReq: get Receiver PID Error");

        return VOS_ERR;
    }

    pstTAReqMsg = (STK_AS_TA_INFO_REQ_STRU *)VOS_AllocMsg(MAPS_STK_PID, sizeof(STK_AS_TA_INFO_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if(VOS_NULL_PTR == pstTAReqMsg)
    {
        STK_ERROR_LOG("SI_STK_GetGasTAReq: VOS_AllocMsg Return Error");

        return VOS_ERR;
    }

    pstTAReqMsg->ulMsgName      = STK_AS_TA_INFO_REQ;
    pstTAReqMsg->ulReceiverPid  = ulRecPID;

    if(VOS_OK != VOS_SendMsg(MAPS_STK_PID, pstTAReqMsg))
    {
        STK_ERROR_LOG("SI_STK_GetGasTAReq: VOS_SendMsg Return Error");

        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 SI_STK_GetGasNmrReq(VOS_VOID)
{
    STK_AS_NMR_INFO_REQ_STRU    *pstNmrReqMsg;
    VOS_UINT32                  ulRecPID;

    if (VOS_OK != SI_STK_GetRecGASPID(&ulRecPID))
    {
        STK_ERROR_LOG("SI_STK_GetGasNmrReq: Get Receiver PID Error");

        return VOS_ERR;
    }

    pstNmrReqMsg = (STK_AS_NMR_INFO_REQ_STRU *)VOS_AllocMsg(MAPS_STK_PID, sizeof(STK_AS_NMR_INFO_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if(VOS_NULL_PTR == pstNmrReqMsg)
    {
        STK_ERROR_LOG("SI_STK_GetGasNmrReq: VOS_AllocMsg Return Error");

        return VOS_ERR;
    }

    pstNmrReqMsg->ulMsgName     = STK_AS_NMR_INFO_REQ;
    pstNmrReqMsg->ulReceiverPid = ulRecPID;

    if(VOS_OK != VOS_SendMsg(MAPS_STK_PID, pstNmrReqMsg))
    {
        STK_ERROR_LOG("SI_STK_GetGasNmrReq: VOS_SendMsg Return Error");

        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_UINT32 SI_STK_SendNmrReq(VOS_UINT32 ulReceiverPid, VOS_UINT32 ulQueryType)
{
    STK_AS_NMR_INFO_REQ_STRU *pstNmrReqMsg;

    pstNmrReqMsg = (STK_AS_NMR_INFO_REQ_STRU *)VOS_AllocMsg(MAPS_STK_PID, sizeof(STK_AS_NMR_INFO_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if(VOS_NULL_PTR == pstNmrReqMsg)
    {
        STK_ERROR_LOG("SI_STK_SendNmrReq: VOS_AllocMsg Return Error");
        return VOS_ERR;
    }

    pstNmrReqMsg->ulMsgName     = STK_AS_NMR_INFO_REQ;
    pstNmrReqMsg->ulQueryType   = ulQueryType;
    pstNmrReqMsg->ulReceiverPid = ulReceiverPid;

    if(VOS_OK != VOS_SendMsg(MAPS_STK_PID, pstNmrReqMsg))
    {
        STK_ERROR_LOG("SI_STK_SendNmrReq: VOS_SendMsg Return Error");
        return VOS_ERR;
    }

    return VOS_OK;
}

/*****************************************************************************
 Function   : SI_STK_ProvideLocalInfo_Nmr
 Description: Provide Local Info NMR
 Input      :
 Return     : VOID
 History    :
1.日    期  : 2010年08月15日
  作    者  : zhuli
  修改内容  : Create
 *****************************************************************************/
VOS_VOID SI_STK_ProvideLocalInfo_Nmr(VOS_UINT32 ulNetType, NAS_STK_UTRAN_MODE_ENUM_UINT8 enUtranMode, SI_STK_DATA_INFO_STRU *pCmdData)
{
    VOS_UINT8                           aucTrData[] = {0x81, 0x03, 0x01, 0x26, 0x00, 0x02, 0x02, 0x82, 0x81, 0x03, 0x01, 0x30};
    VOS_UINT32                          ulResult;
    VOS_UINT8                           ucProfileLen;
    VOS_UINT8                          *pucProfile;

    STK_GetProfileInfo(&pucProfile, &ucProfileLen);

    aucTrData[2] = pCmdData->CmdDetail.ucCommandNum;
    aucTrData[4] = pCmdData->CmdDetail.ucCommandQua;

    if(VOS_FALSE == SI_STKGetBitFromBuf(pucProfile, SI_STK_LOCALINFO_NMR, ucProfileLen))
    {
        STK_ERROR_LOG("SI_STK_ProvideLocalInfo_Nmr: Terminal Profile Set not Support");

        ulResult = VOS_ERR;
    }
    else
    {
        if((STK_ACCESS_TECH_UTRAN == ulNetType)&&(1 == pCmdData->CmdStru.PLocalInfo.OP_Nmr))
        {
            if(NAS_STK_UTRAN_MODE_FDD == enUtranMode)
            {
                ulResult = SI_STK_SendNmrReq(WUEPS_PID_WRR, pCmdData->CmdStru.PLocalInfo.UtranNmr.ucNmr);
            }
            else if(NAS_STK_UTRAN_MODE_TDD == enUtranMode)
            {
                ulResult = SI_STK_SendNmrReq(TPS_PID_RRC, pCmdData->CmdStru.PLocalInfo.UtranNmr.ucNmr);
            }
            else
            {
                ulResult = VOS_ERR;
            }
        }
        else if((STK_ACCESS_TECH_GSM == ulNetType)&&(0 == pCmdData->CmdStru.PLocalInfo.OP_Nmr))
        {
            ulResult = SI_STK_GetGasNmrReq();
        }
        else if((STK_ACCESS_TECH_EUTRAN == ulNetType)&&(1 == pCmdData->CmdStru.PLocalInfo.OP_Nmr))
        {
            ulResult = SI_STK_SendNmrReq(PS_PID_ERRC, pCmdData->CmdStru.PLocalInfo.UtranNmr.ucNmr);
        }
        else /*Unkown network mode*/
        {
            STK_ERROR_LOG("SI_STK_ProvideLocalInfo_Nmr: Cur Network is unknow or Data Error");

            ulResult = VOS_ERR;
        }
    }

    if(ulResult == VOS_ERR)
    {
        g_enNMRCmdStatus = SI_STK_NORMAL_CNF;

        if (VOS_OK != USIMM_TerminalResponseReq(MAPS_STK_PID, sizeof(aucTrData),aucTrData))
        {
            STK_WARNING_LOG("SI_STK_ProvideLocalInfo_Nmr: USIMM_TerminalResponseReq Fail");
        }
    }
    else
    {
        /*启动10s保护定时器*/
        if(VOS_ERR == STK_NMR_TIMER_START(10))
        {
            STK_ERROR_LOG("SI_STK_ProvideLocalInfo_Nmr: Start The Protect Timer Error.");
        }

        g_enNMRCmdStatus = SI_STK_WAITING_CNF;
    }

    return;
}

/*****************************************************************************
 Function   : SI_STK_ProvideLocalINfo_TA
 Description: Provide Local Info TA
 Input      :
 Return     : VOID
 History    :
1.日    期  : 2012年02月18日
  作    者  : wangxiaofei
  修改内容  : Create
 *****************************************************************************/
VOS_VOID SI_STK_ProvideLocalINfo_TA(VOS_UINT32 ulNetType, SI_STK_DATA_INFO_STRU *pCmdData)
{
    VOS_UINT8       aucTrData[] = {0x81, 0x03, 0x01, 0x26, 0x00, 0x02, 0x02, 0x82, 0x81, 0x03, 0x01, 0x30};
    VOS_UINT32                          ulResult = VOS_ERR;
    VOS_UINT8                           ucProfileLen;
    VOS_UINT8                          *pucProfile;

    STK_GetProfileInfo(&pucProfile, &ucProfileLen);


    aucTrData[2] = pCmdData->CmdDetail.ucCommandNum;
    aucTrData[4] = pCmdData->CmdDetail.ucCommandQua;

    /*Profile 是否支持主动命令*/
    if (VOS_FALSE == SI_STKGetBitFromBuf(pucProfile, SI_STK_LOCALINFO_TA, ucProfileLen))
    {
        STK_ERROR_LOG("SI_STK_ProvideLocalINfo_TA: Terminal Profile Set not Support");

        ulResult = VOS_ERR;
    }
    else
    {
        /*判断模式，只有G模才去获取TA值*/
        if(STK_ACCESS_TECH_GSM == ulNetType)
        {
            ulResult = SI_STK_GetGasTAReq();
        }
    }

    if (ulResult == VOS_ERR)
    {
        STK_ERROR_LOG("SI_STK_ProvideLocalINfo_TA: Modem not support, need return Terminal Response");

        g_enTACmdStatus = SI_STK_NORMAL_CNF;

        if(VOS_OK != USIMM_TerminalResponseReq(MAPS_STK_PID, sizeof(aucTrData), aucTrData))
        {
            STK_WARNING_LOG("SI_STK_ProvideLocalINfo_TA: USIMM_TerminalResponseReq fail");
        }
    }
    else
    {
        /*启动超时定时器*/
        if (VOS_OK != STK_GETTA_TIMER_START)
        {
             STK_WARNING_LOG("SI_STK_ProvideLocalINfo_TA: Start Time Fail");
        }

        g_enTACmdStatus = SI_STK_WAITING_CNF;
    }

    return;
}
VOS_UINT32 SI_STK_ProvideLocIProc(SI_STK_DATA_INFO_STRU *pCmdData)
{
    VOS_UINT8                           aucTrData[50] = {0x81, 0x03, 0x01, 0x26, 0x00, 0x02, 0x02, 0x82, 0x81, 0x03, 0x01, 0x00};
    VOS_UINT32                          ulDataLen = 0;
    SI_STK_NAS_INFO_STRU                stMmaInfo;

    STK_GetSysInfo(&stMmaInfo.stSysInfo);

    aucTrData[2] = pCmdData->CmdDetail.ucCommandNum;
    aucTrData[4] = pCmdData->CmdDetail.ucCommandQua;

    switch(pCmdData->CmdDetail.ucCommandQua)
    {
        case STK_PROVIDE_LOCAL_INFO_IMEI:
            ulDataLen = SI_STK_ProvideLocalInfo_IMEI(aucTrData);
            break;

        case STK_PROVIDE_LOCAL_INFO_IMEISV:
            ulDataLen = SI_STK_ProvideLocalInfo_IMEISV(aucTrData);
            break;

        case STK_PROVIDE_LOCAL_INFO_ACCESS_TECH:
            if (VOS_TRUE == SI_STK_IsProactiveCmdNeedSnd2Csima())
            {
                SI_STK_SndProactiveCmd2CBP(pCmdData->SatCmd.SatDataLen, pCmdData->SatCmd.SatCmdData);

                return SI_STK_OK_WAITRP;
            }
            else
            {
                ulDataLen = SI_STK_ProvideLocalInfo_ACCESS(aucTrData, stMmaInfo.stSysInfo.enCurRat);/* [false alarm]:fortify */
                break;
            }

        case STK_PROVIDE_LOCAL_INFO_NAA:
            if (VOS_TRUE == SI_STK_IsProactiveCmdNeedSnd2Csima())
            {
                SI_STK_SndProactiveCmd2CBP(pCmdData->SatCmd.SatDataLen, pCmdData->SatCmd.SatCmdData);

                return SI_STK_OK_WAITRP;
            }
            else
            {
                ulDataLen = SI_STK_ProvideLocalInfo_NAA(aucTrData, &stMmaInfo);/* [false alarm]:fortify */
                break;
            }

        case STK_PROVIDE_LOCAL_INFO_SEARCH_MODE:
            if (VOS_TRUE == SI_STK_IsProactiveCmdNeedSnd2Csima())
            {
                SI_STK_SndProactiveCmd2CBP(pCmdData->SatCmd.SatDataLen, pCmdData->SatCmd.SatCmdData);

                return SI_STK_OK_WAITRP;
            }
            else
            {
                ulDataLen = SI_STK_ProvideLocalInfo_SearchMode(aucTrData);/* [false alarm]:fortify */
                break;
            }

        case STK_PROVIDE_LOCAL_INFO_NMR:/*使用消息机制获取*/
            if (VOS_TRUE == SI_STK_IsProactiveCmdNeedSnd2Csima())
            {
                SI_STK_SndProactiveCmd2CBP(pCmdData->SatCmd.SatDataLen, pCmdData->SatCmd.SatCmdData);

                return SI_STK_OK_WAITRP;
            }
            else
            {
                SI_STK_ProvideLocalInfo_Nmr(stMmaInfo.stSysInfo.enCurRat, stMmaInfo.stSysInfo.enUtranMode, pCmdData);
                return SI_STK_NORP;
            }

        case STK_PROVIDE_LOCAL_INFO_RES_GSM:/* 使用消息机制 */
            SI_STK_ProvideLocalINfo_TA(stMmaInfo.stSysInfo.enCurRat, pCmdData);
            return SI_STK_NORP;

        /* Balong不支持的功能 */
        case STK_PROVIDE_LOCAL_INFO_ESN:
        case STK_PROVIDE_LOCAL_INFO_MEID:
            if (VOS_TRUE == g_stLCEnableCfg.ucLCEnableFlg)
            {
                SI_STK_SndProactiveCmd2CBP(pCmdData->SatCmd.SatDataLen, pCmdData->SatCmd.SatCmdData);

                return SI_STK_OK_WAITRP;
            }
            else
            {
                aucTrData[11]   = COMMAND_BEYOND_TERMINALS_CAPABILITIES;
                ulDataLen       = STK_TERMINAL_RSP_LEN;
            }

            break;

        default:
            /* 与AP对接，Balong通过^CSIN上报 */
            SI_STKDataIndCallback(pCmdData);
            return SI_STK_OK_WAITRP;
    }

    if(VOS_ERR != USIMM_TerminalResponseReq(MAPS_STK_PID, (VOS_UINT8)ulDataLen, aucTrData))
    {
        return SI_STK_NORP;
    }

    return SI_STK_NOTSUPPORT;
}
VOS_UINT32 SI_STK_SetUpEeventListCbpProc(SI_STK_DATA_INFO_STRU *pCmdData)
{
    if (VOS_NULL_PTR != g_stCbpEventList.pucCmd)
    {
        VOS_MemFree(MAPS_STK_PID, g_stCbpEventList.pucCmd);

        g_stCbpEventList.pucCmd            = VOS_NULL_PTR;
        g_stCbpEventList.ulProactiveCmdLen = VOS_NULL;
    }

    g_stCbpEventList.pucCmd = VOS_MemAlloc(MAPS_STK_PID, STATIC_MEM_PT, pCmdData->SatCmd.SatDataLen);

    if (VOS_NULL_PTR == g_stCbpEventList.pucCmd)
    {
        return SI_STK_DATA_ERROR;
    }

    g_stCbpEventList.ulProactiveCmdLen = pCmdData->SatCmd.SatDataLen;

    VOS_MemCpy(g_stCbpEventList.pucCmd, pCmdData->SatCmd.SatCmdData, g_stCbpEventList.ulProactiveCmdLen);

    SI_STK_SndProactiveCmd2CBP(pCmdData->SatCmd.SatDataLen, pCmdData->SatCmd.SatCmdData);

    return SI_STK_OK;
}


VOS_UINT32 SI_STK_SetUpEeventListSpecialProc(SI_STK_DATA_INFO_STRU *pCmdData)
{
    VOS_UINT32                          ulRslt;

    if (VOS_TRUE == SI_STK_IsProactiveCmdNeedSnd2Csima())
    {
        ulRslt = SI_STK_SetUpEeventListCbpProc(pCmdData);
    }
    else
    {
        ulRslt = SI_STK_SetUpEeventListProc(pCmdData);
    }

    return ulRslt;
}


VOS_UINT32 SI_STK_SetUpEeventListProc(SI_STK_DATA_INFO_STRU *pCmdData)
{
    SI_STK_SET_UP_EVENT_LIST_STRU   *pstEventList;
    VOS_UINT32                      i;
    VOS_UINT32                      ulResult;

    VOS_MemSet(&g_stSTKEventState, 0, sizeof(g_stSTKEventState));

    pstEventList = (SI_STK_SET_UP_EVENT_LIST_STRU*)&pCmdData->CmdStru;

    if(pstEventList->OP_List == 0)
    {
        /*广播事件上报*/
        SI_STKDataIndCallback(pCmdData);
        return SI_STK_OK;
    }

    for(i=0; i<pstEventList->EventList.ulLen; i++)
    {
        ulResult = SI_STK_CheckProfileCfg(pstEventList->EventList.pucEventList[i],
                                          gastSTKEventProfileCfg,
                                          STK_ARRAYSIZE(gastSTKEventProfileCfg));


        if(ulResult == VOS_FALSE)
        {
            STK_WARNING_LOG("SI_STK_SetUpEeventListProc: Termial Profile Config is not Support This Event");
            continue;
        }

        switch(pstEventList->EventList.pucEventList[i])
        {
            case SI_STK_EVENT_MT_CALL:
                STK_INFO_LOG("SI_STK_SetUpEeventListProc: MT CALL Event");
                g_stSTKEventState.OP_MTCall         = VOS_TRUE;
                break;

            case SI_STK_EVENT_CALL_CONNECTED:
                STK_INFO_LOG("SI_STK_SetUpEeventListProc: CALL CONNECTED Event");
                g_stSTKEventState.OP_CallConnect    = VOS_TRUE;
                break;

            case SI_STK_EVENT_CALL_DISCONNECTED:
                STK_INFO_LOG("SI_STK_SetUpEeventListProc: CALL DISCONNECTED Event");
                g_stSTKEventState.OP_CallDisconnet  = VOS_TRUE;
                break;

            case SI_STK_EVENT_LOCATION_STATUS:
                STK_INFO_LOG("SI_STK_SetUpEeventListProc: Location Status Event");
                g_stSTKEventState.OP_LociStatus     = VOS_TRUE;
                break;

            case SI_STK_EVENT_ACCESS_TECHN_CHANGE:
                STK_INFO_LOG("SI_STK_SetUpEeventListProc: Access Mode Change Event");
                g_stSTKEventState.OP_AccChange      = VOS_TRUE;
                break;

            case SI_STK_EVENT_NET_SEARCHMODE_CHANGE:
                STK_INFO_LOG("SI_STK_SetUpEeventListProc: Network Search Mode Change Event");
                g_stSTKEventState.OP_NetSearchChg   = VOS_TRUE;
                break;
            case SI_STK_EVENT_NETWORK_REJECTION:
                STK_INFO_LOG("SI_STK_SetUpEeventListProc: Network Rejection Event");
                g_stSTKEventState.OP_NetRej         = VOS_TRUE;
                break;
            default:
                STK_INFO_LOG("SI_STK_SetUpEeventListProc: Event not Support By Modem");
                break;
            }
    }

    SI_STKDataIndCallback(pCmdData);

    return SI_STK_OK;
}
VOS_BOOL STK_IsCallInService(VOS_VOID)
{
    VOS_UINT8                           i;

    /* NAS上报的呼叫ID从1到7 */
    for (i = 0; i < (STK_CALL_MAX_NUM); i++)
    {
        if (SI_STK_CALL_ID_STATUS_BUSY ==  g_stStkSetupCallCtrl.astStkCallStatusTable[i].enStatus)
        {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}
VOS_BOOL STK_IsCPBCsimRefreshCmd(SI_STK_DATA_INFO_STRU *pCmdData)
{
    VOS_UINT8                           aucCsimAid[USIMM_AID_LEN_MAX];
    VOS_UINT32                          ulAidLen;

    if (VOS_TRUE == g_stLCEnableCfg.ucLCEnableFlg)
    {
        SI_STK_SndProactiveCmd2CBP(pCmdData->SatCmd.SatDataLen, pCmdData->SatCmd.SatCmdData);
    }
    else
    {
        return VOS_FALSE;
    }

    if (VOS_FALSE == pCmdData->CmdStru.Refresh.OP_AID)
    {
        return VOS_FALSE;
    }

    if (VOS_OK != USIMM_GetAid(USIMM_AID_TYPE_CSIM, &ulAidLen, aucCsimAid))
    {
        return VOS_FALSE;
    }

    if (VOS_OK != VOS_MemCmp(aucCsimAid, pCmdData->CmdStru.Refresh.AID.pucAID, ulAidLen))
    {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}



VOS_UINT32 SI_STK_GetResendCfg(NV_EVENT_RESEND_CFG_STRU *pstNVCfg)
{
    NV_EVENT_RESEND_CFG_STRU    stNVCfg;

    if (NV_OK != NV_Read(en_NV_Item_EVENT_RESEND_CFG, &stNVCfg, sizeof(NV_EVENT_RESEND_CFG_STRU)))
    {
        return VOS_ERR;
    }

    if((EVENT_RESEND_ON != stNVCfg.enResendFlag)
        ||(0             == stNVCfg.ucRetryTime)
        ||(0             == stNVCfg.ucTimerLen))
    {
        return VOS_ERR;
    }

    VOS_MemCpy(pstNVCfg, &stNVCfg, sizeof(NV_EVENT_RESEND_CFG_STRU));

    return VOS_OK;
}


VOS_VOID SI_STK_ResendTimerMsgHandle(REL_TIMER_MSG *pstMsg)
{
    VOS_UINT32                  ulResult;
    VOS_UINT32                  ulSaveIndex;
    NV_EVENT_RESEND_CFG_STRU    stNVCfg;

    if (STK_LOCIEVENT_TIMER_NAME == pstMsg->ulName)
    {
        ulSaveIndex = 0;

        ulResult = USIMM_EnvelopeReq(MAPS_STK_PID, SI_STK_ENVELOPE_LOCI_RESEND, (VOS_UINT8)g_astEventResend[0].ulDataLen, g_astEventResend[0].aucData);
    }
    else if (STK_ACCEVENT_TIMER_NAME == pstMsg->ulName)
    {
        ulSaveIndex = 1;

        ulResult = USIMM_EnvelopeReq(MAPS_STK_PID, SI_STK_ENVELOPE_ACC_RESEND, (VOS_UINT8)g_astEventResend[1].ulDataLen, g_astEventResend[1].aucData);
    }
    else
    {
        return;
    }

    if (USIMM_API_SUCCESS == ulResult)
    {
        if (VOS_OK != SI_STK_GetResendCfg(&stNVCfg))
        {
            return;
        }

        g_astEventResend[ulSaveIndex].ulSTKResendCounter++;

        /* coverity[uninit_use] */
        if (g_astEventResend[ulSaveIndex].ulSTKResendCounter > stNVCfg.ucRetryTime)
        {
            VOS_MemSet((VOS_VOID*)(&g_astEventResend[ulSaveIndex]), 0, sizeof(STK_EVENTDATA_RESEND_STRU));

            return;
        }
        
        /* coverity[uninit_use] */
        STK_RESEND_TIMER_START(&g_astEventResend[ulSaveIndex].stSTKResendTimer, STK_LOCIEVENT_TIMER_NAME + ulSaveIndex, stNVCfg.ucTimerLen);
    }

    return;
}


VOS_VOID SI_STK_EventResendClean(SI_STK_ENVELOPE_RESENT_FLAG_UINT32 enDataType)
{
    VOS_UINT32                  ulSaveIndex;

    if ((enDataType  != SI_STK_ENVELOPE_LOCI_RESEND)&&(enDataType != SI_STK_ENVELOPE_ACC_RESEND))
    {
        return;
    }

    ulSaveIndex = enDataType - 1;

    STK_RESEND_TIMER_STOP(&g_astEventResend[ulSaveIndex].stSTKResendTimer);

    VOS_MemSet(&g_astEventResend[ulSaveIndex], 0, sizeof(g_astEventResend[ulSaveIndex]));

    return;
}



VOS_VOID SI_STK_EventResendSave(SI_STK_ENVELOPE_RESENT_FLAG_UINT32 enDataType, VOS_UINT32 ulEventLen, VOS_UINT8 *pucData)
{
    NV_EVENT_RESEND_CFG_STRU    stNVCfg;
    VOS_UINT32                  ulSaveIndex;

    if (((enDataType  != SI_STK_ENVELOPE_LOCI_RESEND)&&(enDataType != SI_STK_ENVELOPE_ACC_RESEND))
        ||(0            == ulEventLen)
        ||(VOS_NULL_PTR == pucData))
    {
        return;
    }

    if(VOS_OK != SI_STK_GetResendCfg(&stNVCfg))
    {
        return;
    }

    ulSaveIndex = enDataType - 1;

    g_astEventResend[ulSaveIndex].ulDataLen = ulEventLen;

    VOS_MemCpy(g_astEventResend[ulSaveIndex].aucData, pucData, ulEventLen);

    /* coverity[uninit_use] */
    STK_RESEND_TIMER_START(&g_astEventResend[ulSaveIndex].stSTKResendTimer, STK_LOCIEVENT_TIMER_NAME+ulSaveIndex, stNVCfg.ucTimerLen);

    g_astEventResend[ulSaveIndex].ulSTKResendCounter = 1;

    return;
}


#if (FEATURE_VSIM == FEATURE_ON)
VOS_VOID SI_STK_VsimSmsPpDownloadProc(
    VOS_UINT32                          ulRcvPid,
    VOS_UINT32                          ulSendPara,
    SI_STK_SMS_PP_DOWNLOAD_STRU        *pstSmsPpDlData)
{
    SI_STK_EVENT_INFO_STRU              stEvent;
    PS_USIM_ENVELOPE_CNF_STRU          *pstSendMsg;

    VOS_MemSet(&stEvent, 0, sizeof(stEvent));

    stEvent.STKCBEvent                                  = SI_STK_SMS_PP_DL_DATA_IND_EVENT;
    stEvent.STKErrorNo                                  = VOS_OK;
    stEvent.STKCmdStru.CmdStru.SmsPpDlDataInfo.ucLen    = (VOS_UINT8)pstSmsPpDlData->TPDU.ulLen;

    VOS_MemCpy(stEvent.STKCmdStru.CmdStru.SmsPpDlDataInfo.aucData,
               pstSmsPpDlData->TPDU.pucTPDU,
               pstSmsPpDlData->TPDU.ulLen);

    SI_STKCallBack_BroadCast(&stEvent);

    pstSendMsg = (PS_USIM_ENVELOPE_CNF_STRU *)VOS_AllocMsg(MAPS_STK_PID, sizeof(PS_USIM_ENVELOPE_CNF_STRU)-VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pstSendMsg)
    {
        return;
    }

    pstSendMsg->ulSenderPid     = MAPS_STK_PID;
    pstSendMsg->ulReceiverPid   = ulRcvPid;
    pstSendMsg->ulMsgName       = PS_USIM_ENVELOPE_CNF;
    pstSendMsg->ulSendPara      = ulSendPara;
    pstSendMsg->ulErrorCode     = VOS_OK;
    pstSendMsg->ulResult        = VOS_OK;
    pstSendMsg->ucDataLen       = VOS_NULL;
    pstSendMsg->ucSW1           = 0x90;
    pstSendMsg->ucSW2           = 0x00;
    pstSendMsg->ucDataType      = SI_STK_ENVELOPE_PPDOWN;

    (VOS_VOID)VOS_SendMsg(pstSendMsg->ulSenderPid, pstSendMsg);

    return;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */



