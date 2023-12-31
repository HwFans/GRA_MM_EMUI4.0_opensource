

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
/*lint --e{537,451,958,740,718,746}*/
#include "om.h"
#include "spysystem.h"
#include "TafClientApi.h"
#include "TafAppMma.h"
#include "NVIM_Interface.h"
#include "sleepsleep.h"
#include "errorlog.h"
#include "apminterface.h"
#include "hpaoperatertt.h"
/* Added by ouyangfei for AT Project, 2011-11-08, begin */
#include "TafDrvAgent.h"
/* Added by ouyangfei for AT Project, 2011-11-08, end */
#include "OmApi.h"
#include "phyoaminterface.h"
#include "TafOamInterface.h"
#include "DspInterface.h"

#if (FEATURE_ON == FEATURE_MERGE_OM_CHAN)
#include "omoperator.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_SPYSYSTEM_C


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

HTIMER                          g_stSpyTempProtect_Timer = VOS_NULL_PTR;/*用于温度保护查询定时器*/

HTIMER                          g_stSpyFlowCtrlTimer = VOS_NULL_PTR;/*用于流控定时器*/

SPY_SYSCUR_STATUS               g_stSysCurStatus;/*用于保存当前系统状态*/

SPY_STATE_ENUM_UINT32           g_enSpyCurState;/*用于保存当前系统调节状态*/

SPY_TEMP_STATE_CTRL_STRU        g_stTempStateCtrl; /*保存当前温度保护模块的状态*/

SPY_TEMP_PROTECT_NV_STRU        g_stTempProtectNv; /*温度保护NV项的结构*/

VOS_UINT16                      g_usSpyClientId;    /*注册的clientId*/

/*Reserve SD card and Antenna status*/
VOS_UINT32                      g_ulUsbStatus   = 0;
VOS_UINT32                      g_ulAntenStatus = 0;


SPY_DOWNGRADE_PROC_FUNC         g_pfunRegDowngradeProc = VOS_NULL_PTR;  /*用于流控降速执行函数*/

SPY_DOWNGRADE_PROC_FUNC         g_pfunRegUpgradeProc = VOS_NULL_PTR;  /*用于流控升速执行函数*/

SPY_RESTORE_PROC_FUNC           g_pfunRegRestoreProc = VOS_NULL_PTR;  /*用于软关机恢复初速执行函数*/

#ifdef SPY_STATISTICS_DEBUG
/* SPY温保措施执行统计信息 */
SPY_STATISTICS_INFO_STRU        g_stSpyStatisticsInfo;

VOS_UINT32                      g_ulSpyRecordFlag = 1;
SPY_RECORD_STRU                 g_stSpyTimeRecord;
#define SPY_GetTime(ulIndex)    (g_stSpyTimeRecord.astRecord[ulIndex].ulTime)
#define SPY_GetType(ulIndex)    (g_stSpyTimeRecord.astRecord[ulIndex].acType)
#endif

#if (FEATURE_OFF == FEATURE_MERGE_OM_CHAN)
extern VOS_UINT32 OM_SysCtrlCmd(VOS_UINT16 usClientId, VOS_UINT32 ulMode, MODEM_ID_ENUM_UINT16 enModemID);
#endif

#if (RAT_MODE != RAT_GU)
extern VOS_VOID L_ExtAntenStatus(VOS_INT32 para);
#endif

/* 上报给NAS的温保状态，只有开启和关闭两个状态 */
SPY_TEMPROTECT_SWITCH_STATE_ENUM_UINT32                    g_ulSpySwitchState = SPY_TEMPROTECT_SWITCH_OFF;

/* NAS 通知 SPY 的紧急呼叫的状态 */
TAF_OAM_EMERGENCY_CALL_STATUS_ENUM_UINT8                   g_ucSpyEmergencyCallState = TAF_OAM_EMERGENCY_CALL_END;

/*****************************************************************************
  3 函数实现
*****************************************************************************/

#ifdef SPY_STATISTICS_DEBUG

VOS_VOID Spy_TimeRecord(VOS_CHAR * pcString)
{
    VOS_UINT32 ulRecordPosition = 0;

    if (1 != g_ulSpyRecordFlag)
    {
        return;
    }

    g_stSpyTimeRecord.ulCount++;

    ulRecordPosition = ((g_stSpyTimeRecord.ulCount-1)%SPY_RECORD_NUM);

    SPY_GetTime(ulRecordPosition) = VOS_GetTick();

    VOS_StrCpy(SPY_GetType(ulRecordPosition),pcString);

    return;
}
#endif

#if (defined(BOARD_ASIC) && (RAT_MODE == RAT_GU))

#if 0

VOS_INT32 Spy_UsbStatusProc(VOS_UINT8 ucUsbStatus)
{
    if (0x5A == ucUsbStatus)
    {
        if (CPHY_STATE_CONNECT == g_enSPYCPHYState)
        {
            HPA_Write32RegMask(SOC_DSP_SMIM_INT_RAW_REG, BIT_N(DSP_USB_START_BIT));
            HPA_Write32RegMask(SOC_DSP_SMIM_INT_EN_REG,  BIT_N(DSP_USB_START_BIT));
        }
        g_ulUsbStatus = BIT_N(DSP_USB_START_BIT);
    }
    else if (0xA5 == ucUsbStatus)
    {
        if (CPHY_STATE_CONNECT == g_enSPYCPHYState)
        {
            HPA_Write32RegMask(SOC_DSP_SMIM_INT_RAW_REG, BIT_N(DSP_USB_STOP_BIT));
            HPA_Write32RegMask(SOC_DSP_SMIM_INT_EN_REG,  BIT_N(DSP_USB_STOP_BIT));
        }
        g_ulUsbStatus = BIT_N(DSP_USB_STOP_BIT);
    }
    else
    {
        /*Do nothing*/
    }

    return VOS_OK;
}

#endif
#endif
VOS_VOID Spy_AntenStatusProc(MODEM_ID_ENUM_UINT16 enModemID, VOS_UINT32 ulStatus)
{
    SPY_MSG_STRU                       *pstSarMsg;

    pstSarMsg = (SPY_MSG_STRU *)VOS_AllocMsg(WUEPS_PID_SAR,
                    sizeof(SPY_MSG_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pstSarMsg)
    {
        return;
    }

    pstSarMsg->ulReceiverPid    = WUEPS_PID_SAR;
    pstSarMsg->ulMsgName        = SPY_SAR_ANTEN_STATUS_IND;
    pstSarMsg->lAntenStatus     = (VOS_INT32)ulStatus;
    pstSarMsg->enModemID        = enModemID;

    if (VOS_OK != VOS_SendMsg(WUEPS_PID_SAR, pstSarMsg))
    {
        return;
    }

    return;
}


VOS_VOID Spy_Anten0StatusProc(VOS_VOID)
{
    VOS_UINT                            ulStatus = 0; /* 无线连接 */

    if (VOS_OK != DRV_GET_ANTEN_LOCKSTATE(PWC_COMM_MODEM_0, &ulStatus))
    {
        SAR_ERROR_LOG("Spy_Anten0StatusProc: get anten status from drv fail.\r\n");
        return;
    }

    DRV_ANTEN_INT_INSTALL(PWC_COMM_MODEM_0, Spy_Anten0StatusProc, (VOS_INT)ulStatus);

    Spy_AntenStatusProc(MODEM_ID_0, ulStatus);
}

#if ( FEATURE_MULTI_MODEM == FEATURE_ON )
VOS_VOID Spy_Anten1StatusProc(VOS_VOID)
{
    VOS_UINT                            ulStatus = 0; /* 无线连接 */;

    if (VOS_OK != DRV_GET_ANTEN_LOCKSTATE(PWC_COMM_MODEM_1, &ulStatus))
    {
        SAR_ERROR_LOG("Spy_Anten1StatusProc: get anten status from drv fail.\r\n");
        return;
    }

    DRV_ANTEN_INT_INSTALL(PWC_COMM_MODEM_1, Spy_Anten1StatusProc, (VOS_INT)ulStatus);

    Spy_AntenStatusProc(MODEM_ID_1, ulStatus);

}
#endif
VOS_VOID Spy_SarSendToDsp(MODEM_ID_ENUM_UINT16 enModemID, MN_APP_SAR_INFO_STRU *pstSpyToDsp)
{
    PHY_OAM_SAR_CTRL_REQ_STRU          *pstSarMsg;
    VOS_UINT32                          ulCurrentMode;

    pstSarMsg = (PHY_OAM_SAR_CTRL_REQ_STRU*)VOS_AllocMsg(WUEPS_PID_SAR,
                        sizeof(PHY_OAM_SAR_CTRL_REQ_STRU) - VOS_MSG_HEAD_LENGTH);

    if ( VOS_NULL_PTR == pstSarMsg )
    {
        SAR_WARNING_LOG("Spy_SarSendToDsp: Msg Alloc Failed!");
        return;
    }

    pstSarMsg->ulMask  = pstSpyToDsp->ulSarType;
    pstSarMsg->enMsgId = ID_OAM_PHY_SAR_CTRL_REQ;

    /* 判断由哪个模式的DSP任务接收消息 */
    ulCurrentMode = SHPA_GetRateType(enModemID);

    if (VOS_RATMODE_GSM == ulCurrentMode)
    {
        if(MODEM_ID_0 == enModemID)
        {
            pstSarMsg->ulReceiverPid    = I0_DSP_PID_GPHY;
        }
        else
        {
            pstSarMsg->ulReceiverPid    = I1_DSP_PID_GPHY;
        }
    }
    else if (VOS_RATMODE_WCDMA == ulCurrentMode)
    {
        pstSarMsg->ulReceiverPid    = DSP_PID_WPHY;
    }
    else
    {
        SAR_WARNING_LOG("Spy_SarSendToDsp: get current mode id fail!");
        VOS_FreeMsg(WUEPS_PID_SAR, pstSarMsg);
        return;
    }

    /* 天线状态 */
    if ( PHY_OM_SAR_MASK_ANTSTATE == (PHY_OM_SAR_MASK_ANTSTATE & pstSpyToDsp->ulSarType) )
    {
        pstSarMsg->lAntenStatus = pstSpyToDsp->lAntenStatus;
    }

    /* 功率回退等级 */
    if ( PHY_OM_SAR_MASK_REDUCTION == (PHY_OM_SAR_MASK_REDUCTION & pstSpyToDsp->ulSarType) )
    {
        pstSarMsg->ulSarReduction = pstSpyToDsp->ulSarReduction;
    }

    if (VOS_OK != VOS_SendMsg(WUEPS_PID_SAR, pstSarMsg))
    {
        SAR_ERROR_LOG("Spy_SarSendToDsp: Send Msg Failed!");
        return;
    }

    return;
}
VOS_VOID Spy_SarSendToTaf(MODEM_ID_ENUM_UINT16 enModemID, VOS_INT32  lAntenStatus)
{
    MN_APP_SAR_ANTENSTATUS_MSG_STRU *pstAntenStatusStru;

    pstAntenStatusStru = (MN_APP_SAR_ANTENSTATUS_MSG_STRU *)VOS_AllocMsg(WUEPS_PID_SAR,
                            sizeof(MN_APP_SAR_ANTENSTATUS_MSG_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pstAntenStatusStru)
    {
        SAR_WARNING_LOG("Spy_SarSendToTaf: Msg Alloc Failed!");
        return;
    }

    if (MODEM_ID_0 == enModemID)
    {
        pstAntenStatusStru->ulReceiverPid   = I0_WUEPS_PID_DRV_AGENT;
    }
    else
    {
        pstAntenStatusStru->ulReceiverPid   = I1_WUEPS_PID_DRV_AGENT;
    }

    pstAntenStatusStru->ulMsgId         = SAR_ANTSTATE_IND;
    pstAntenStatusStru->lAntenStatus    = lAntenStatus;

    if (VOS_OK != VOS_SendMsg(WUEPS_PID_SAR, pstAntenStatusStru))
    {
        SAR_ERROR_LOG("Spy_SarSendToTaf: SendMsg Failed!");
        return;
    }

    return;
}


VOS_VOID Spy_SendAntenStatusToLte(MODEM_ID_ENUM_UINT16 enModemID, VOS_INT32 lAntenStatus)
{
#if (RAT_MODE != RAT_GU)
    PLATAFORM_RAT_CAPABILITY_STRU       stPlatFormRat;
    VOS_UINT32                          i;

    if (MODEM_ID_0 == enModemID)
    {
        /* 读取天线状态，判断是否需要给 L 模发 */
        if (NV_OK != NV_ReadEx(enModemID, en_NV_Item_Platform_RAT_CAP, &stPlatFormRat,
                              sizeof(PLATAFORM_RAT_CAPABILITY_STRU)))
        {
            SAR_ERROR_LOG("Spy_SendAntenStatusToLte: read nv fail.\r\n");
            return;
        }

        for (i=0; i<stPlatFormRat.usRatNum; i++)
        {
            /* 平台支持LTE */
            if (PLATFORM_RAT_LTE == stPlatFormRat.aenRatList[i])
            {
                L_ExtAntenStatus(lAntenStatus);
            }
        }
    }
#endif
    return;
}


VOS_UINT32  Spy_SarMsgProc(MsgBlock *pMsg)
{
    SPY_MSG_STRU                        *pstMsg;
    MN_APP_SAR_INFO_STRU                      stSpyToDsp;

    pstMsg = (SPY_MSG_STRU *)pMsg;

    switch(pstMsg->ulMsgName)
    {
        case SPY_SAR_ANTEN_STATUS_IND:

            if (MODEM_ID_BUTT <= pstMsg->enModemID)
            {
                SPY_ERROR1_LOG("Spy_SarMsgProc: Modem ID is: %d error!", pstMsg->enModemID);
                return VOS_ERR;
            }

            /* 给DSP发送天线状态 */
            VOS_MemSet(&stSpyToDsp, 0, sizeof(MN_APP_SAR_INFO_STRU));
            stSpyToDsp.lAntenStatus = pstMsg->lAntenStatus;
            stSpyToDsp.ulSarType    = PHY_OM_SAR_MASK_ANTSTATE;
            Spy_SarSendToDsp(pstMsg->enModemID,&stSpyToDsp);

            /* 给Taf发送天线状态 */
            Spy_SarSendToTaf(pstMsg->enModemID,
                             pstMsg->lAntenStatus);

            /* 给L模发送天线状态 */
            Spy_SendAntenStatusToLte(pstMsg->enModemID,
                                     pstMsg->lAntenStatus);
            break;

        default:
            break;
    }

    return VOS_OK;
}
VOS_VOID Spy_AntenStatusInit(VOS_VOID)
{
    VOS_UINT                    ulStatus = 0; /* 无线连接 */

    if (VOS_OK == DRV_GET_ANTEN_LOCKSTATE(PWC_COMM_MODEM_0, &ulStatus))
    {
        /* 给L模发送天线状态 (只有卡0 存在L模) */
        Spy_SendAntenStatusToLte(MODEM_ID_0, (VOS_INT32)ulStatus);
    }
    else
    {
        SAR_WARNING_LOG("Spy_AntenStatusInit: Get Anten 0 status Failed!");
    }

    DRV_ANTEN_INT_INSTALL(PWC_COMM_MODEM_0, Spy_Anten0StatusProc, (VOS_INT)ulStatus);

#if ( FEATURE_MULTI_MODEM == FEATURE_ON )
    if (VOS_OK != DRV_GET_ANTEN_LOCKSTATE(PWC_COMM_MODEM_1, &ulStatus))
    {
        SAR_WARNING_LOG("Spy_AntenStatusInit: Anten 1 Get Failed!");
    }

    DRV_ANTEN_INT_INSTALL(PWC_COMM_MODEM_1, Spy_Anten1StatusProc, (VOS_INT)ulStatus);
#endif

    return;
}


VOS_VOID Spy_ReportEvent(SPY_EVENT_NAME_ENUM_UINT32 enEventId, VOS_INT lSimTemp)
{
    PS_OM_EVENT_IND_STRU stEventInd;

    stEventInd.ulLength   = sizeof(PS_OM_EVENT_IND_STRU) - sizeof(stEventInd.ulLength);

    stEventInd.usEventId  = (VOS_UINT16)enEventId;

    stEventInd.ulModuleId = WUEPS_PID_SPY;

    *((VOS_INT*)stEventInd.aucData) = lSimTemp;
    
	/* coverity[check_return] */
    OM_Event(&stEventInd);

    return;
}

#if (RAT_MODE != RAT_GU)
VOS_VOID Spy_RegPhoneCallback(TAF_PHONE_EVENT_INFO_STRU *pEvent)
{
    if(VOS_YES != pEvent->OP_OperMode)/*判断当前是否是开关机事件*/
    {
        return;
    }

    if((VOS_OK != pEvent->OP_PhoneError)
        &&(TAF_PH_MODE_LOWPOWER == pEvent->OperMode.PhMode))/*判断当前关机事件操作的结果*/
    {
        SPY_WARNING_LOG("Spy_RegPhoneCallback: TAF Return Error");

        g_enSpyCurState = SPY_STATE_NORMAL;
    }

    if((TAF_PH_MODE_FULL == pEvent->OperMode.PhMode)
        &&(SPY_STATE_PSOFF == g_enSpyCurState))/*如果当前系统是关机但是消息中是开机，表明用户手动开机*/
    {
        SPY_WARNING_LOG("Spy_RegPhoneCallback: User Open the PS when the PS State is TAF_PH_MODE_FULL");

        g_enSpyCurState = SPY_STATE_NORMAL;
    }

    return;
}

#endif  /*defined(VERSION_V7R1_C010)*/

/*****************************************************************************
 函 数 名  : Spy_SendAtTempProtectMsg
 功能描述  : 给AT任务发送温保状态变化指示消息
 输入参数  : ulTempProtectEvent -- 温度保护状态变化事件
             ulTempProtectParam -- 预留以备扩展
 输出参数  : 无
 返 回 值  : VOS_OK
             VOS_ERR
 修改历史  :
*****************************************************************************/
VOS_UINT32 Spy_SendAtTempProtectMsg(VOS_UINT32 ulTempProtectEvent, VOS_UINT32 ulTempProtectParam)
{
    VOS_UINT32                       ulTempState;
    TEMP_PROTECT_EVENT_AT_IND_STRU  *pstSpyMsg;

#if (RAT_MODE != RAT_GU)
    pstSpyMsg = (TEMP_PROTECT_EVENT_AT_IND_STRU *)VOS_AllocMsg(WUEPS_PID_SPY,
                sizeof(TEMP_PROTECT_EVENT_AT_IND_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pstSpyMsg)
    {
        SPY_ERROR_LOG("Spy_SendAtTempProtectMsg: alloc normal msg Return Null");

        return VOS_ERR;
    }

    pstSpyMsg->ulReceiverPid        = WUEPS_PID_AT;
    pstSpyMsg->ulMsgId              = ID_TEMPPRT_AT_EVENT_IND;
    pstSpyMsg->ulTempProtectEvent   = ulTempProtectEvent;
    pstSpyMsg->ulTempProtectParam   = ulTempProtectParam;

    if (VOS_OK != VOS_SendMsg(WUEPS_PID_SPY, pstSpyMsg))
    {
        SPY_ERROR_LOG("Spy_SendAtTempProtectMsg: send normal Return Failed");

        return VOS_ERR;
    }
#endif

    if ( SPY_STATE_NORMAL == ulTempProtectEvent )
    {
        ulTempState = SPY_TEMPROTECT_SWITCH_OFF;
    }
    else
    {
        ulTempState = SPY_TEMPROTECT_SWITCH_ON;
    }

    if ( ulTempState == g_ulSpySwitchState )
    {
        SPY_INFO_LOG("Spy_SendAtTempProtectMsg: No Action");

        return VOS_OK;
    }

    g_ulSpySwitchState = ulTempState;

    pstSpyMsg = (TEMP_PROTECT_EVENT_AT_IND_STRU *)VOS_AllocMsg(WUEPS_PID_SPY,
                sizeof(TEMP_PROTECT_EVENT_AT_IND_STRU) - VOS_MSG_HEAD_LENGTH);

    if (VOS_NULL_PTR == pstSpyMsg)
    {
        SPY_ERROR_LOG("Spy_SendAtTempProtectMsg:  alloc switch msg Return Null");

        return VOS_ERR;
    }

    pstSpyMsg->ulReceiverPid        = WUEPS_PID_AT;
    pstSpyMsg->ulMsgId              = ID_TEMPPRT_STATUS_AT_EVENT_IND;
    pstSpyMsg->ulTempProtectEvent   = ulTempState;
    pstSpyMsg->ulTempProtectParam   = ulTempProtectParam;

    if (VOS_OK != VOS_SendMsg(WUEPS_PID_SPY, pstSpyMsg))
    {
        SPY_ERROR_LOG("Spy_SendAtTempProtectMsg: send switch Return Failed");

        return VOS_ERR;
    }

    return VOS_OK;
}



VOS_VOID Spy_CountInit(VOS_UINT16 usState)
{
    if (0 != (usState & TEMP_CLOSE_ITEM))
    {
        g_stTempStateCtrl.ulTempCloseCount = TEMP_CLOSE_ADC_COUNT;

        SPY_NORMAL_LOG("Spy_CountInit: Clean the Temp Close Count");
    }

    if (0 != (usState & TEMP_ALARM_ITEM))
    {
        g_stTempStateCtrl.ulTempAlarmCount = TEMP_ALARM_ADC_COUNT;

        SPY_NORMAL_LOG("Spy_CountInit: Clean the Temp Alarm Count");
    }

    if (0 != (usState & TEMP_RESUME_ITEM))
    {
        g_stTempStateCtrl.ulTempResumeCount = TEMP_RESUME_ADC_COUNT;

        SPY_NORMAL_LOG("Spy_CountInit: Clean the Temp Resume Count");
    }

    if (0 != (usState & TEMP_POWEROFF_ITEM))
    {
        g_stTempStateCtrl.ulTempPowerOffCount = TEMP_POWEROFF_ADC_COUNT;

        SPY_NORMAL_LOG("Spy_CountInit: Clean the Temp Power Off Count");
    }

    return;
}


VOS_UINT32 Spy_TempCheckInit(VOS_VOID)
{
    g_stTempStateCtrl.enTempEnable = TEMP_CLOSE_STATE;

    /*读取热保护门限值的NV项*/
    if(NV_OK != NV_Read(en_NV_Item_USIM_TEMP_PROTECT_NEW,
                            (VOS_VOID*)&g_stTempProtectNv,
                            sizeof(SPY_TEMP_PROTECT_NV_STRU)))
    {

        SPY_ERROR_LOG("Spy_TempCheckInit: NV_Read en_NV_Item_USIM_TEMP_PROTECT!");

        return VOS_ERR;
    }

    g_stTempStateCtrl.lOldTemperature = 0;

    /*初始化温度保护控制变量*/
    g_stTempStateCtrl.enTempEnable      = g_stTempProtectNv.ulIsEnable;

    g_stTempStateCtrl.enTempState       = TEMP_NORMAL_STATE;

    g_stTempStateCtrl.ulTempCloseCount  = TEMP_CLOSE_ADC_COUNT;

    g_stTempStateCtrl.ulTempAlarmCount  = TEMP_ALARM_ADC_COUNT;

    g_stTempStateCtrl.ulTempPowerOffCount   = TEMP_POWEROFF_ADC_COUNT;

    g_stTempStateCtrl.ulTempResumeCount = TEMP_RESUME_ADC_COUNT;

    g_stTempStateCtrl.ulTempWaitTimeLen = TEMP_CLOSE_TIMER_LEN;

    g_stTempStateCtrl.ulBatteryTempAlarmCount = 0;

    return VOS_OK;
}


VOS_UINT32 Spy_TempProtectInit(VOS_VOID)
{
    VOS_INT16                           sTemp;

    g_enSpyCurState = SPY_STATE_NORMAL;

#ifdef SPY_STATISTICS_DEBUG
    VOS_MemSet(&g_stSpyStatisticsInfo, 0, sizeof(SPY_STATISTICS_INFO_STRU));
    VOS_MemSet(&g_stSpyTimeRecord, 0, sizeof(SPY_RECORD_STRU));
#endif

    if(VOS_OK != Spy_TempCheckInit())
    {
        SPY_ERROR_LOG("Spy_TempProtectInit: Spy_TempCheckInit Failed!");

        return VOS_ERR;
    }

    if(TEMP_ENABLE_STATE == g_stTempStateCtrl.enTempEnable)
    {
        /*读取当前温度值*/
        if(VOS_OK !=  DRV_HKADC_GET_TEMP((HKADC_TEMP_PROTECT_E)(g_stTempProtectNv.ulLocation),
                                          VOS_NULL_PTR, &sTemp, HKADC_CONV_DELAY))
        {
            SPY_WARNING_LOG("Spy_TempProtectInit: The Drv Interface Return Failed!");

            return VOS_ERR;
        }

        g_stTempStateCtrl.lOldTemperature   = sTemp;

        if(VOS_OK != VOS_StartRelTimer(&g_stSpyTempProtect_Timer, WUEPS_PID_SPY,
                                    SPY_TIMER_LEN, 1, 0, VOS_RELTIMER_NOLOOP,VOS_TIMER_NO_PRECISION))
        {
            SPY_ERROR_LOG("Spy_TempProtectInit: Start The Check Timer Failed!");

            return VOS_ERR;
        }
    }

    return VOS_OK;
}
VOS_INT32 Spy_CheckTemp(VOS_VOID)
{
    VOS_INT16               sTemp;
    VOS_INT                 lSimTemp = 0;

    /* 紧急呼叫期间不考虑温度保护,但是如果温保已经开始了，那么不考虑紧急呼叫 */
    if ( (SPY_STATE_NORMAL == g_enSpyCurState) && (TAF_OAM_EMERGENCY_CALL_START == g_ucSpyEmergencyCallState) )
    {
        return SPY_VOTE_NOMAL;
    }

    /*温度监控没有启动 或者 当前处于FT模式下*/
    if ((TEMP_CLOSE_STATE == g_stTempStateCtrl.enTempEnable)
        || (TAF_PH_MODE_FT == g_stSysCurStatus.ulCurPSStatus))
    {
        SPY_NORMAL_LOG("Spy_CheckTemp: The Tempture Check is Disable");

        return SPY_VOTE_NOMAL;
    }

    if (VOS_OK != DRV_HKADC_GET_TEMP((HKADC_TEMP_PROTECT_E)(g_stTempProtectNv.ulLocation),
                                      VOS_NULL_PTR, &sTemp, HKADC_CONV_DELAY))
    {
        SPY_WARNING_LOG("Spy_CheckTemp: The Drv Interface Return Failed");

        return SPY_VOTE_NOMAL;
    }

    lSimTemp    = sTemp;

    /*判断两次温度差是否超标*/
    if (SPY_TEMP_MAX_INTERVAL < GET_TEMP_INTERVAL(g_stTempStateCtrl.lOldTemperature, lSimTemp))
    {
        g_stTempStateCtrl.lOldTemperature = lSimTemp;

        SPY_NORMAL1_LOG("Spy_CheckTemp: The Tempture is Too Larger then Old.", lSimTemp);

        return SPY_VOTE_NOMAL;
    }

    SPY_INFO1_LOG("Spy_CheckTemp: SIM temperature is %d", lSimTemp);

    g_stTempStateCtrl.lOldTemperature = lSimTemp;

    if (lSimTemp >= TEMP_POWEROFF_THRESHOLD)
    {
        g_stTempStateCtrl.ulTempPowerOffCount--;

        SPY_WARNING1_LOG("Spy_CheckTemp: The Tempture is Higher then Power Off Level, %d", lSimTemp);

        if(0 == g_stTempStateCtrl.ulTempPowerOffCount)
        {
            Spy_CountInit(TEMP_ALL_ITEM);

            g_stTempStateCtrl.enTempState = TEMP_WAIT_STATE;

            SPY_WARNING_LOG("Spy_CheckTemp: Vote the Power Off");

            return SPY_VOTE_POWEROFF;
        }
    }
    else
    {
        Spy_CountInit(TEMP_POWEROFF_ITEM);
    }

    /*判断是否超过实施关机的条件*/
    if (lSimTemp >= TEMP_CLOSE_THRESHOLD)
    {
        g_stTempStateCtrl.ulTempCloseCount--;

        SPY_WARNING1_LOG("Spy_CheckTemp: The Tempture is Higher then PS Off Level, %d", lSimTemp);

        if (0 == g_stTempStateCtrl.ulTempCloseCount)
        {
            /*进行软关机操作，并进入到低功耗状态*/
            Spy_CountInit(TEMP_ALL_ITEM);

            g_stTempStateCtrl.enTempState = TEMP_WAIT_STATE;

            SPY_WARNING_LOG("Spy_CheckTemp: Vote the soft down");

            return SPY_VOTE_SOFTDOWN;
        }
    }
    else
    {
        Spy_CountInit(TEMP_CLOSE_ITEM);
    }

    /*判断是否超过报警门限的条件*/
    if (lSimTemp >= TEMP_ALARM_THRESHOLD)
    {
        g_stTempStateCtrl.ulTempAlarmCount--;

        SPY_WARNING1_LOG("Spy_CheckTemp: The Tempture is Higher then Warning Level, %d", lSimTemp);

        /*SIM卡温度超过报警条件，进入报警状态*/
        if (0 == g_stTempStateCtrl.ulTempAlarmCount)
        {
            Spy_CountInit(TEMP_ALL_ITEM);

            g_stTempStateCtrl.enTempState = TEMP_WAIT_STATE;

            SPY_WARNING_LOG("Spy_CheckTemp: Vote the Qos 384");

            return SPY_VOTE_QOSDOWN;
        }

        Spy_ReportEvent(SPY_EVENT_TEMP_OVER_HIGH, lSimTemp);
    }
    else
    {
        Spy_CountInit(TEMP_ALARM_ITEM);
    }

    /*判断是否低于恢复门限的条件*/
    if (lSimTemp <= TEMP_RESUME_THRESHOLD)
    {
        g_stTempStateCtrl.ulTempResumeCount--;

        SPY_WARNING1_LOG("Spy_CheckTemp: The Tempture is Lower then Resume Level, %d", lSimTemp);

        /*SIM卡温度低于恢复门限，进行恢复操作*/
        if (0 == g_stTempStateCtrl.ulTempResumeCount)
        {
            Spy_CountInit(TEMP_ALL_ITEM);

            SPY_WARNING_LOG("Spy_CheckTemp: Vote the Resume");

            Spy_ReportEvent(SPY_EVENT_TEMP_RESUME, lSimTemp);

            return SPY_VOTE_RESUME;
        }
    }
    else
    {
        Spy_CountInit(TEMP_RESUME_ITEM);
    }

    return SPY_VOTE_NOMAL;
}
VOS_UINT32 Spy_VoteJudgeState(VOS_INT32 lVote)
{
    SPY_STATE_ENUM_UINT32       enState     = SPY_STATE_NORMAL;
    SPY_VOTERESULT_ENUM_UINT32  enVoteResult= SPY_VOTERESULT_NULL;

    switch(lVote)
    {
        case SPY_VOTE_POWEROFF:/*当前的投票结果是关机*/
            enVoteResult    = SPY_VOTERESULT_POWEROFF;
            SPY_NORMAL_LOG("Spy_VoteJudgeState: The Vote is Power OFF");
            break;

        case SPY_VOTE_SOFTDOWN :
            if(TAF_PH_MODE_FULL == g_stSysCurStatus.ulCurPSStatus)/*当前开机状态是正常情况下可以关机*/
            {
                enState         = SPY_STATE_PSOFF;  /*更新当前系统状态*/
                enVoteResult    = SPY_VOTERESULT_PSOFF;
                SPY_NORMAL_LOG("Spy_VoteJudgeState: The Vote is Soft Down");

                Spy_SendAtTempProtectMsg(SPY_STATE_PSOFF, 0);
            }
            else /*如果不能正常关机，则将温度保护模块切换到正常状态*/
            {
                enState = g_enSpyCurState;
                g_stTempStateCtrl.enTempState = TEMP_NORMAL_STATE;
                SPY_NORMAL_LOG("Spy_VoteJudgeState: The Current PS Status is not TAF_PH_MODE_FULL");
            }
            break;

        case SPY_VOTE_QOSDOWN: /*当前的投票结果是调节QOS 384*/
            if (SPY_STATE_NORMAL == g_enSpyCurState)
            {
                enState = SPY_STATE_QOS;
                enVoteResult    = SPY_VOTERESULT_DOWNQOS;  /*更新当前系统状态*/
                SPY_NORMAL_LOG("Spy_VoteJudgeState: The Vote is QOS DOWN");

                Spy_SendAtTempProtectMsg(SPY_STATE_QOS, 0);
            }
            else
            {
                enState = g_enSpyCurState;
                SPY_NORMAL_LOG("Spy_VoteJudgeState: The Vote is NULL");
            }
            break;

        case SPY_VOTE_RESUME:   /*当前需要恢复*/
            if(SPY_STATE_QOS == g_enSpyCurState)/*当前系统状态在调节QOS后才能恢复*/
            {
                enVoteResult    = SPY_VOTERESULT_DEFQOS;
                enState         = SPY_STATE_NORMAL;
                SPY_NORMAL_LOG("Spy_VoteJudgeState: The Vote is Resume Qos");

                Spy_SendAtTempProtectMsg(SPY_STATE_NORMAL, 0);
            }
            else if(TAF_PH_MODE_LOWPOWER == g_stSysCurStatus.ulCurPSStatus)/*当前软关机状态,并且是SPY触发软关机*/
            {
                enVoteResult    = SPY_VOTERESULT_PSON;  /*系统状态是开机*/
                enState         = SPY_STATE_NORMAL;
                SPY_NORMAL_LOG("Spy_VoteJudgeState: The Vote is Turn On the PS");

                Spy_SendAtTempProtectMsg(SPY_STATE_NORMAL, 0);
            }
            else
            {
                SPY_WARNING_LOG("Spy_VoteJudgeState: The Current System do not Need Resume");
            }
            break;

        default:
            SPY_NORMAL_LOG("Spy_VoteJudgeState: The Vote is NULL");
            return SPY_VOTERESULT_NULL;
    }

    g_enSpyCurState = enState;

    return enVoteResult;
}


VOS_UINT32 Spy_PowerDown(VOS_VOID)
{
    OAM_MNTN_SPY_ERROR_EVENT_STRU       stSpyErrorLog;

    /* 在Errorlog文件中记录事件 */
    if(VOS_OK != MNTN_RecordErrorLog(MNTN_SPY_PWOER_DOWN_EVENT, (void *)&stSpyErrorLog,
                                            sizeof(OAM_MNTN_SPY_ERROR_EVENT_STRU)))
    {
        SPY_ERROR_LOG("Spy_PowerDown: Fail to record Power Down event in Errorlog file");
    }

#ifdef SPY_STATISTICS_DEBUG
    Spy_TimeRecord("Power Down");
    g_stSpyStatisticsInfo.ulPowerOffNum++;
#endif

    vos_printf("\r\nSpy_PowerDown SHUT DOWN\r\n");
    DRV_SHUT_DOWN(DRV_SHUTDOWN_TEMPERATURE_PROTECT); /*如果不支持电池也可以调用接口，底软内部打桩*/

    return VOS_OK;
}


VOS_UINT32 Spy_SoftPowerOff(VOS_VOID)
{
#ifndef COMM_ITT
    OAM_MNTN_SPY_ERROR_EVENT_STRU       stSpyErrorLog;

    /*关闭2分钟定时器*/
    if (VOS_NULL_PTR!= g_stSpyFlowCtrlTimer)
    {
        VOS_StopRelTimer(&g_stSpyFlowCtrlTimer);
    }

    /* 在Errorlog文件中记录事件 */
    if(VOS_OK != MNTN_RecordErrorLog(MNTN_SPY_SOFTOFF_EVENT, (void *)&stSpyErrorLog,
                                            sizeof(OAM_MNTN_SPY_ERROR_EVENT_STRU)))
    {
        SPY_ERROR_LOG("Spy_SoftPowerOff: Fail to Write Soft Off event in Errorlog file");
    }

    /* 恢复初速*/
    if (VOS_NULL_PTR != g_pfunRegRestoreProc)
    {
        g_pfunRegRestoreProc();
    }

#ifdef SPY_STATISTICS_DEBUG
    Spy_TimeRecord("Soft Power Off");
    g_stSpyStatisticsInfo.ulSoftPowerOffNum++;
#endif
#if (FEATURE_OFF == FEATURE_MERGE_OM_CHAN)
    OM_SysCtrlCmd(OAM_CLIENT_ID_SPY, TAF_PH_MODE_LOWPOWER, MODEM_ID_0);
#else
    OM_SysCtrlCmd(OM_LOGIC_CHANNEL_IND, OAM_CLIENT_ID_SPY, TAF_PH_MODE_LOWPOWER, MODEM_ID_0);
#endif //(FEATURE_OFF == FEATURE_MERGE_OM_CHAN)

#endif

    return VOS_OK;
}
VOS_UINT32 Spy_SoftPowerOn(VOS_VOID)
{
#ifndef COMM_ITT

#ifdef SPY_STATISTICS_DEBUG
    Spy_TimeRecord("Soft Power On");
    g_stSpyStatisticsInfo.ulSoftPowerOnNum++;
#endif
#if (FEATURE_OFF == FEATURE_MERGE_OM_CHAN)
    OM_SysCtrlCmd(OAM_CLIENT_ID_SPY, TAF_PH_MODE_FULL, MODEM_ID_0);
#else
    OM_SysCtrlCmd(OM_LOGIC_CHANNEL_IND, OAM_CLIENT_ID_SPY, TAF_PH_MODE_FULL, MODEM_ID_0);
#endif //(FEATURE_OFF == FEATURE_MERGE_OM_CHAN)
#endif

    return VOS_OK;
}


VOS_UINT32 Spy_FlowCtrlDown()
{
    /*call L2*/
    SPY_NORMAL_LOG("Spy_FlowCtrlDown: Down Flow Control Start.");

    if (VOS_NULL_PTR != g_stSpyFlowCtrlTimer)
    {
        VOS_StopRelTimer(&g_stSpyFlowCtrlTimer);
    }

#ifdef SPY_STATISTICS_DEBUG
    Spy_TimeRecord("Flow Ctrl Down");
    g_stSpyStatisticsInfo.ulFlowCtrlDownNum++;
#endif

    if (VOS_NULL_PTR != g_pfunRegDowngradeProc)
    {
        /* 如果降速未完成，则需要再启动2分钟定时器*/
        if (SPY_DATA_DOWNGRADE_CONTINUE == g_pfunRegDowngradeProc())
        {
            /*启动2分钟定时器*/
            if (VOS_OK != VOS_StartRelTimer(&g_stSpyFlowCtrlTimer, WUEPS_PID_SPY,
                 SPY_FLOW_CTRL_TIMER_LEN,TIMER_FLOWCTRL,0,VOS_RELTIMER_NOLOOP, VOS_TIMER_NO_PRECISION))
            {
                SPY_ERROR_LOG("Spy_FlowCtrlDown:Start The Check Timer Failed!");
            }
        }
    }

    return VOS_OK;
}
VOS_UINT32 Spy_FlowCtrlUp()
{
    /*call L2*/
    SPY_NORMAL_LOG("Spy_FlowCtrlUp: Up Flow Control Start.");

    if (VOS_NULL_PTR!= g_stSpyFlowCtrlTimer)
    {
        VOS_StopRelTimer(&g_stSpyFlowCtrlTimer);
    }

#ifdef SPY_STATISTICS_DEBUG
    Spy_TimeRecord("Flow Ctrl Up");
    g_stSpyStatisticsInfo.ulFlowCtrlUpNum++;
#endif

    if (VOS_NULL_PTR != g_pfunRegDowngradeProc)
    {
        /* 如果升速未完成，则需要再启动2分钟定时器*/
        if (SPY_DATA_UPGRADE_CONTINUE == g_pfunRegUpgradeProc())
        {
            /*启动2分钟定时器*/
            if (VOS_OK != VOS_StartRelTimer(&g_stSpyFlowCtrlTimer, WUEPS_PID_SPY,
                SPY_FLOW_CTRL_TIMER_LEN,TIMER_FLOWCTRL,0,VOS_RELTIMER_NOLOOP, VOS_TIMER_NO_PRECISION))
            {
                SPY_ERROR_LOG("Spy_FlowCtrlUp:Start The Check Timer Failed!");
            }
        }
    }

    return VOS_OK;
}
VOS_UINT32 Spy_Execute(VOS_UINT32 ulVoteResult)
{
    VOS_UINT32  ulResult = VOS_ERR;

    switch(ulVoteResult)
    {
        case SPY_VOTERESULT_DEFQOS:
            SPY_NORMAL_LOG("Spy_Execute: Need Resume the User Default Qos Value");

            ulResult = Spy_FlowCtrlUp();
            break;

        case SPY_VOTERESULT_PSON:
            SPY_NORMAL_LOG("Spy_Execute: Need Re Start the PS");
            ulResult = Spy_SoftPowerOn();
            break;

        case SPY_VOTERESULT_DOWNQOS:
            SPY_NORMAL_LOG("Spy_Execute: Need Limit To the Qos");

            ulResult = Spy_FlowCtrlDown();
            break;

        case SPY_VOTERESULT_PSOFF:
            SPY_NORMAL_LOG("Spy_Execute: Need Power off the PS");
            ulResult = Spy_SoftPowerOff();
            break;

        case SPY_VOTERESULT_POWEROFF:
            SPY_NORMAL_LOG("Spy_Execute: Need Power off the Platform");
            ulResult = Spy_PowerDown();
            break;

        default:
            SPY_WARNING_LOG("Spy_Execute: The Vote is NULL");
            break;
    }

    if(VOS_ERR == ulResult)
    {
        /*lint -e516 */
        SPY_WARNING1_LOG("Spy_Execute: Excute the Vote %d is Failed",
            (VOS_INT32)ulVoteResult);
        /*lint +e516 */
    }

    return ulResult;
}


VOS_VOID Spy_MainProc(VOS_VOID)
{
    VOS_INT32  lTempVote;
    VOS_UINT32 ulVoteResult = SPY_VOTERESULT_NULL;
    VOS_UINT32 ulResult     = VOS_OK;
    VOS_UINT32 ulTimeLen    = SPY_TIMER_LEN;
    VOS_UINT32 ulPrecision  = VOS_TIMER_NO_PRECISION;

    lTempVote = Spy_CheckTemp();/*获取温度监控的投票*/

    if (SPY_VOTE_NOMAL != lTempVote)
    {
        Spy_ReportEvent(SPY_EVENT_VOTE_RESULT, (VOS_INT)lTempVote);

        ulVoteResult = Spy_VoteJudgeState(lTempVote);    /*需要根据当前的投票结果确定系统状态转换*/

        if(SPY_VOTERESULT_NULL != ulVoteResult)
        {
            ulTimeLen = TEMP_CLOSE_TIMER_LEN * SPY_TIMER_LEN;

            ulResult = Spy_Execute(ulVoteResult);

            if(VOS_OK != ulResult) /*根据投票结果操作*/
            {
                /*lint -e516 */
                SPY_ERROR1_LOG(
                    "Spy_MainProc:Spy_Execute return result is  Failed!,Current State is %d", (VOS_INT32)g_enSpyCurState);
                /*lint +e516 */

                ulTimeLen    = SPY_TIMER_LEN;
            }
        }
    }

    /* 温保软关机后，定时器类型需要修改为32K定时器,确保CCPU下电仍然可以进行温度监控 */
    if (SPY_STATE_PSOFF == g_enSpyCurState)
    {
        ulPrecision = VOS_TIMER_PRECISION_5;
    }

    if(VOS_OK != VOS_StartRelTimer(&g_stSpyTempProtect_Timer, WUEPS_PID_SPY,
                                ulTimeLen,1,0,VOS_RELTIMER_NOLOOP, ulPrecision))
    {
        SPY_ERROR_LOG("Spy_MainProc:Start The Check Timer Failed!");
    }

    return;
}


VOS_VOID Spy_E5_CheckSIMTemp(VOS_VOID)
{
    VOS_UINT32                  ulResult;
    VOS_INT16                   sTemp;
    VOS_INT                     lSimTemp = 0;

    if(BSP_MODULE_UNSUPPORT == DRV_GET_BATTERY_SUPPORT())   /*当前不支持电池*/
    {
        return ;
    }

    /*通过底软接口获取当前SIM卡温度*/
    ulResult = (VOS_UINT32)DRV_HKADC_GET_TEMP(HKADC_TEMP_SIM_CARD, VOS_NULL_PTR, &sTemp, HKADC_CONV_DELAY);

    if(VOS_OK != ulResult)
    {
        SPY_ERROR_LOG("Spy_E5_CheckSIMTemp: DRV_GET_SIM_TEMP Failed");

        return;
    }

    lSimTemp    = sTemp;

    if(lSimTemp >= TEMP_SIM_POWEROFF_THRESHOLD)
    {
        vos_printf("\r\nSpy_E5_CheckSIMTemp SHUT DOWN, SIM Temperature is %d\r\n", lSimTemp);
        DRV_SHUT_DOWN(DRV_SHUTDOWN_TEMPERATURE_PROTECT);
    }

    return;
}


VOS_UINT32 Spy_ReadTempMsgProc(MsgBlock *pMsg)
{
    /* 保留一个空函数以防方案再次发生变化 */
    return VOS_OK;
}
VOS_VOID Spy_TempProtectMsgProc(MsgBlock *pstMsg)
{
    REL_TIMER_MSG                           *pstExpireMsg;
    TAF_OAM_EMERGENCY_CALL_STATUS_STRU      *pstTafEmergencyMsg;

    if(VOS_PID_TIMER == pstMsg->ulSenderPid)
    {
        pstExpireMsg                = (REL_TIMER_MSG*)pstMsg;

        if (TIMER_FLOWCTRL == pstExpireMsg->ulName)
        {
            /*2分钟定时下调流控*/
            if(SPY_STATE_QOS == g_enSpyCurState)
            {
                Spy_FlowCtrlDown();
            }

            /*2分钟定时上调流控*/
            if(SPY_STATE_NORMAL == g_enSpyCurState)
            {
                Spy_FlowCtrlUp();
            }
        }
        else
        {
            Spy_E5_CheckSIMTemp();

            g_stSysCurStatus.ulCurPSStatus = MMA_GetCurrentPhoneMode();

            /* 如果温保软关机失败或用户手动开机，更新温保状态 */
            if ((TAF_PH_MODE_FULL == g_stSysCurStatus.ulCurPSStatus)&&(SPY_STATE_PSOFF == g_enSpyCurState))
            {
                g_enSpyCurState = SPY_STATE_NORMAL;

                SPY_NORMAL_LOG("Spy_TempProtectMsgProc: SPY state change from softoff to normal");

                Spy_SendAtTempProtectMsg(SPY_STATE_NORMAL, 0);
            }

            Spy_MainProc();
        }
    }
    else if(WUEPS_PID_TAF == pstMsg->ulSenderPid)
    {
        pstTafEmergencyMsg = (TAF_OAM_EMERGENCY_CALL_STATUS_STRU *)pstMsg;

        if ( TAF_OAM_EMERGENCY_CALL_STATUS_NOTIFY != pstTafEmergencyMsg->ulMsgId )
        {
            return;
        }

        if ( TAF_OAM_EMERGENCY_CALL_STATUS_BUTT == pstTafEmergencyMsg->enEmergencyCallStatus )
        {
            return;
        }

        g_ucSpyEmergencyCallState = pstTafEmergencyMsg->enEmergencyCallStatus;
    }
    else
    {
        /*lint -e516 */
        SPY_WARNING1_LOG("Spy_TempProtectMsgProc: Receive Unkown Msg, Send PID is %d",(VOS_INT32)pstMsg->ulSenderPid);
        /*lint +e516 */
    }

    return;
}
VOS_UINT32 Spy_FID_Init(enum VOS_INIT_PHASE_DEFINE ip)
{
    VOS_UINT32 ulResult = VOS_OK;

    switch (ip)
    {
        case VOS_IP_LOAD_CONFIG:
            ulResult = VOS_RegisterTaskPrio(WUEPS_FID_SPY, COMM_SPY_TASK_PRIO);

            if (ulResult != VOS_OK)
            {
                LogPrint("Spy_FID_Init: VOS_RegisterTaskPrio Failed!");
                return VOS_ERR;
            }

            ulResult = VOS_RegisterPIDInfo(WUEPS_PID_SPY,
                                            VOS_NULL_PTR,
                                            (Msg_Fun_Type)Spy_TempProtectMsgProc);

            if (ulResult != VOS_OK)
            {
                LogPrint("Spy_FID_Init: VOS_RegisterPIDInfo WUEPS_PID_SPY Failed!");
                return VOS_ERR;
            }

            ulResult = VOS_RegisterPIDInfo(WUEPS_PID_MONITOR,
                                            VOS_NULL_PTR,
                                            (Msg_Fun_Type)Spy_ReadTempMsgProc);

            if( VOS_OK != ulResult )
            {
                LogPrint("Spy_FID_Init: VOS_RegisterPIDInfo WUEPS_PID_MONITOR Failed!");
                return VOS_ERR;
            }

            ulResult = VOS_RegisterPIDInfo(WUEPS_PID_SAR,
                                            VOS_NULL_PTR,
                                            (Msg_Fun_Type)Spy_SarMsgProc);

            if( VOS_OK != ulResult )
            {
                LogPrint("Spy_FID_Init: VOS_RegisterPIDInfo WUEPS_PID_SAR Failed!");
                return VOS_ERR;
            }
            break;

        case VOS_IP_INITIAL:

            /*DRV_USB_STATUS_CALLBACK_REGI(Spy_UsbStatusProc);*/
            Spy_AntenStatusInit();

            ulResult = Spy_TempProtectInit();

            if (ulResult != VOS_OK)
            {
                LogPrint("Spy_FID_Init: Spy_TempProtectInit Failed!");
                return VOS_ERR;
            }

            break;

        default:
            break;
    }

    return ulResult;

}
VOS_UINT32 Spy_SetTempPara(SPY_TEMP_THRESHOLD_PARA_STRU *stTempPara)
{
    SPY_TEMP_PROTECT_NV_STRU stTempProtectNv;
    VOS_UINT32               ulChangeFlag = SPY_TEMP_THRESHOLD_PARA_UNCHANGE;

    if(VOS_NULL_PTR == stTempPara)
    {
        SPY_ERROR_LOG("Spy_SetTempPara: Input VOS_NULL_PTR");

        return VOS_ERR;
    }

    /*读取热保护门限值的NV项*/
    if(NV_OK != NV_Read(en_NV_Item_USIM_TEMP_PROTECT_NEW,
                        (VOS_VOID*)&stTempProtectNv,
                        sizeof(SPY_TEMP_PROTECT_NV_STRU)))
    {
        SPY_ERROR_LOG("Spy_SetTempPara: NV_Read en_NV_Item_USIM_TEMP_PROTECT Fail!");

        return VOS_ERR;
    }

    /*若输入参数有效，则进行赋值*/
    if((SPY_TEMP_THRESHOLD_PARA_INVALID != stTempPara->ulIsEnable)
        &&(stTempProtectNv.ulIsEnable != stTempPara->ulIsEnable))
    {
        stTempProtectNv.ulIsEnable = stTempPara->ulIsEnable;
        ulChangeFlag = SPY_TEMP_THRESHOLD_PARA_CHANGE;
    }

    if((SPY_TEMP_THRESHOLD_PARA_INVALID != (VOS_UINT32)stTempPara->lCloseAdcThreshold)
        &&(stTempProtectNv.lCloseAdcThreshold != stTempPara->lCloseAdcThreshold))
    {
        stTempProtectNv.lCloseAdcThreshold = stTempPara->lCloseAdcThreshold;
        ulChangeFlag = SPY_TEMP_THRESHOLD_PARA_CHANGE;
    }

    if((SPY_TEMP_THRESHOLD_PARA_INVALID != (VOS_UINT32)stTempPara->lAlarmAdcThreshold)
        &&(stTempProtectNv.lAlarmAdcThreshold != stTempPara->lAlarmAdcThreshold))
    {
        stTempProtectNv.lAlarmAdcThreshold = stTempPara->lAlarmAdcThreshold;
        ulChangeFlag = SPY_TEMP_THRESHOLD_PARA_CHANGE;
    }

    if((SPY_TEMP_THRESHOLD_PARA_INVALID != (VOS_UINT32)stTempPara->lResumeAdcThreshold)
        &&(stTempProtectNv.lResumeAdcThreshold != stTempPara->lResumeAdcThreshold))
    {
        stTempProtectNv.lResumeAdcThreshold = stTempPara->lResumeAdcThreshold;
        ulChangeFlag = SPY_TEMP_THRESHOLD_PARA_CHANGE;
    }

    /*判断取值合法性*/
    if((stTempProtectNv.lAlarmAdcThreshold >= stTempProtectNv.lCloseAdcThreshold)
       ||(stTempProtectNv.lResumeAdcThreshold >= stTempProtectNv.lAlarmAdcThreshold))
    {
        SPY_ERROR_LOG("Spy_SetTempPara: NV Value Error!");

        return VOS_ERR;
    }

    /*如果用户的输入与原先的门限值不同才写入NV项*/
    if(SPY_TEMP_THRESHOLD_PARA_CHANGE == ulChangeFlag)
    {
        if (NV_OK != NV_Write(en_NV_Item_USIM_TEMP_PROTECT_NEW,
                             (VOS_VOID*)&stTempProtectNv,
                             sizeof(SPY_TEMP_PROTECT_NV_STRU)))
        {
            SPY_ERROR_LOG("Spy_SetTempPara: NV_Write en_NV_Item_USIM_TEMP_PROTECT Fail!");

            return VOS_ERR;
        }
    }

    return VOS_OK;
}


VOS_VOID Spy_DownGradeRegister(SPY_DOWNGRADE_PROC_FUNC pFnDowngradeProcFunc)
{
    if (VOS_NULL_PTR ==  pFnDowngradeProcFunc)
    {
        SPY_ERROR_LOG("Spy_DownGradeRegister: Input VOS_NULL_PTR");
        return;
    }

    g_pfunRegDowngradeProc = pFnDowngradeProcFunc;

    if (SPY_STATE_QOS == g_enSpyCurState)
    {
        Spy_FlowCtrlDown();
    }

    return;
}


VOS_VOID Spy_UpGradeRegister(SPY_UPGRADE_PROC_FUNC pFnUpgradeProcFunc)
{
    if (VOS_NULL_PTR ==  pFnUpgradeProcFunc)
    {
        SPY_ERROR_LOG("Spy_UpGradeRegister: Input VOS_NULL_PTR");
        return;
    }

    g_pfunRegUpgradeProc = pFnUpgradeProcFunc;

    if (SPY_STATE_NORMAL == g_enSpyCurState)
    {
        Spy_FlowCtrlUp();
    }
    return;
}


VOS_VOID Spy_RestoreRegister(SPY_RESTORE_PROC_FUNC pFnRestoreProcFunc)
{
    if (VOS_NULL_PTR ==  pFnRestoreProcFunc)
    {
        SPY_ERROR_LOG("Spy_RestoreRegister: Input VOS_NULL_PTR");
        return;
    }

    g_pfunRegRestoreProc = pFnRestoreProcFunc;

    return;
}

#ifdef SPY_STATISTICS_DEBUG

VOS_VOID  Spy_ShowCmdHelp(VOS_VOID)
{
    vos_printf("\r\n");
    vos_printf("********************** SPY COMMAND HELP *********************\r\n");
    vos_printf(" Spy_ShowSpyStat [1]\r\n");
    vos_printf(" Spy_ShowSysStat \r\n");
    vos_printf(" Spy_ShowNvItem \r\n");
    vos_printf(" Spy_ShowHkadcTable \r\n");
    vos_printf(" Spy_ShowMonitorStat \r\n");

    vos_printf("*******************************************************************\r\n");
    vos_printf("\r\n");

    return;
}


VOS_VOID Spy_ShowSpyStat(VOS_UINT32 ulIndex)
{
    VOS_UINT32 i;

    vos_printf("Spy current state:      0x%x\r\n",g_enSpyCurState);
    vos_printf("Spy enable flag:        0x%x\r\n",g_stTempStateCtrl.enTempEnable);
    vos_printf("PowerOff count:         %d\r\n",g_stTempStateCtrl.ulTempPowerOffCount);
    vos_printf("Soft close count:       %d\r\n",g_stTempStateCtrl.ulTempCloseCount);
    vos_printf("Alarm count:            %d\r\n",g_stTempStateCtrl.ulTempAlarmCount);
    vos_printf("Resume count:           %d\r\n",g_stTempStateCtrl.ulTempResumeCount);
    vos_printf("Wait time:              %d\r\n",g_stTempStateCtrl.ulTempWaitTimeLen);
    vos_printf("Last temperature:       %d\r\n",g_stTempStateCtrl.lOldTemperature);

    vos_printf("\r\n");
    vos_printf("Spy timer info:\r\n");
    vos_printf("Spy temp protect timer: 0x%x\r\n",g_stSpyTempProtect_Timer);
    vos_printf("Spy flow ctrl timer:    0x%x\r\n",g_stSpyFlowCtrlTimer);

    vos_printf("\r\n");
    vos_printf("Spy statistics info:\r\n");
    vos_printf("Flow control down num:  %d\r\n",g_stSpyStatisticsInfo.ulFlowCtrlDownNum);
    vos_printf("Flow control up num:    %d\r\n",g_stSpyStatisticsInfo.ulFlowCtrlUpNum);
    vos_printf("Soft power off num:     %d\r\n",g_stSpyStatisticsInfo.ulSoftPowerOffNum);
    vos_printf("Soft power on num:      %d\r\n",g_stSpyStatisticsInfo.ulSoftPowerOnNum);
    vos_printf("Power off num:          %d\r\n",g_stSpyStatisticsInfo.ulPowerOffNum);

    if (0 != ulIndex)
    {
        vos_printf("\r\n");
        vos_printf("==========================================================\r\n");
        vos_printf("Time Record: count %d, current position %d\r\n",g_stSpyTimeRecord.ulCount,
            ((0==g_stSpyTimeRecord.ulCount) ? 0 : ((g_stSpyTimeRecord.ulCount-1)%SPY_RECORD_NUM)));
        vos_printf("Num      Type                 Time\r\n");
        for (i=0; i<((g_stSpyTimeRecord.ulCount<SPY_RECORD_NUM) ? (g_stSpyTimeRecord.ulCount) : SPY_RECORD_NUM); i++)
        {
            vos_printf("%3d    %-16s  %10d\r\n",i,g_stSpyTimeRecord.astRecord[i].acType,g_stSpyTimeRecord.astRecord[i].ulTime);
        }
    }
}


VOS_VOID Spy_ShowSysStat(VOS_VOID)
{
    vos_printf("PDP status:          0x%x\r\n",g_stSysCurStatus.ulCurPSStatus);
}


VOS_VOID Spy_ShowNvItem(VOS_VOID)
{
    vos_printf("SPY NV item:\r\n");
    vos_printf("Spy enable flag:        0x%x\r\n",g_stTempProtectNv.ulIsEnable);
    vos_printf("Spy Location:           0x%x\r\n",g_stTempProtectNv.ulLocation);
    vos_printf("PowerOff threshold:     %d\r\n",g_stTempProtectNv.lPowerOffThreshold);
    vos_printf("Soft close threshold:   %d\r\n",g_stTempProtectNv.lCloseAdcThreshold);
    vos_printf("Alarm threshold:        %d\r\n",g_stTempProtectNv.lAlarmAdcThreshold);
    vos_printf("Resume threshold:       %d\r\n",g_stTempProtectNv.lResumeAdcThreshold);
    vos_printf("SIM PowerOff threshold: %d\r\n",g_stTempProtectNv.lSIMPowerOffThreshold);

    vos_printf("Alarm count:            %d\r\n",g_stTempProtectNv.stTempCtrlPara.ucTempOverCount);
    vos_printf("Rusume count:           %d\r\n",g_stTempProtectNv.stTempCtrlPara.ucTempRsumeCount);
    vos_printf("Soft close count:       %d\r\n",g_stTempProtectNv.stTempCtrlPara.ucTempCloseCount);
    vos_printf("PowerOff count:         %d\r\n",g_stTempProtectNv.stTempCtrlPara.ucTempPowerOffCount);
    vos_printf("Wait time:              %d\r\n",g_stTempProtectNv.stTempCtrlPara.ulWaitTimer);

    vos_printf("\r\n");
    vos_printf("AntCtrl NV item:\r\n");
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */




