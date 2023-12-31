
/*lint --e{740,958,537,767,718,746,713}*/
#include "usimmdl.h"
#include "usimmt1dl.h"
#include "usimmbase.h"
#include "DrvInterface.h"
#include "v_iddef.h"
#include "om.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/

#define    THIS_FILE_ID        PS_FILE_ID_USIMM_DLMODULE_C



VOS_VOID USIMM_DLJudgeCmdCase(USIMM_APDU_ST* pstAPDUData)
{
        /* 判断当前的卡命令类型 */
    if(0 == pstAPDUData->ulLcValue)
    {
        if(0 == pstAPDUData->ulLeValue) /* Lc,Le都不存在 */
        {
            gstUSIMMDLControl.enCMDCase = USIMM_CMD_CASE1;
        }
        else                            /* Lc不存在,Le存在 */
        {
            gstUSIMMDLControl.enCMDCase = USIMM_CMD_CASE2;
        }
    }
    else
    {
        if(0 == pstAPDUData->ulLeValue) /* Lc存在,Le不存在 */
        {
            gstUSIMMDLControl.enCMDCase = USIMM_CMD_CASE3;
        }
        else                            /* Lc,Le都存在 */
        {
            gstUSIMMDLControl.enCMDCase = USIMM_CMD_CASE4;
        }
    }

    return;
}
VOS_VOID USIMM_DLGetCardStatusTimerHandler(VOS_UINT32 ulParam, VOS_UINT32 ulTimerName)
{
    VOS_SmV(g_ulUSIMMGetCardStatSemID);

    return;
}


VOS_UINT32 USIMM_DLResetCard(VOS_UINT32 ulResetType, VOS_INT32 *plVoltageSwitchRst)
{
    VOS_INT32                           lSCIResult = VOS_ERR;
    VOS_UINT32                          i;
    NV_USIMM_T1_CTRL_PARA_STRU          stT1CtrlPara;

    if (USIMM_RESET_CARD == ulResetType)
    {
        OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_1, WUEPS_PID_USIM, WUEPS_PID_USIM, USIMMDL_DRV_USIMMSCI_RST);
        DRV_USIMMSCI_RST(COLD_RESET);
        OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);

        if(USIMM_DL_T_MODE_T1 == g_enUSIMMTMode)
        {
            /* 更新变量 */
            if(VOS_OK != USIMM_T1DLUpdateParaByATR(g_stUSIMMT1Para.enErrDetectMode, g_stUSIMMT1Para.ulIFSC, g_stUSIMMT1Para.ulIFSD))
            {
                USIMM_ERROR_LOG("USIMM_DLResetCard: USIMM_T1DLUpdateParaByATR fail!");
                return VOS_ERR;
            }

            VOS_MemSet(&stT1CtrlPara, 0, sizeof(stT1CtrlPara));

            /* 读NV项 */
            if(VOS_OK != NV_Read(en_NV_Item_Usimm_T1_Ctrl_PARA, &stT1CtrlPara, sizeof(stT1CtrlPara)))
            {
                /* 打印错误 */
                USIMM_ERROR_LOG("USIMM_DecideTMode:USIM Read T1 PARA NV error");

                /* NV读取失败，显式赋值为T0 */
                stT1CtrlPara.enTMode = USIMM_DL_T_MODE_T0;

            }

            /* 发送IFS命令,告诉卡UE接收能力 */
            if(VOS_OK != USIMM_T1SendIFSD_APDU((VOS_UINT8)stT1CtrlPara.ulDefaultIFSD))
            {
                USIMM_ERROR_LOG("USIMM_DLResetCard: USIMM_T1SendIFSD_APDU fail!");
                return VOS_ERR;
            }

            OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_1, WUEPS_PID_USIM, WUEPS_PID_USIM, USIMMDL_DRV_USIMMSCI_T1_PRTCL_SWITCH_1);
            if(VOS_OK != DRV_USIMMSCI_PROTOCOL_SWITCH((PROTOCOL_MODE)g_enUSIMMTMode))
            {
                /* 可维可测信息统计 */
                g_stT1StatisticInfo.ulProtocolSwitchFailCnt++;

                USIMM_WARNING_LOG("USIMM_DLResetCard:SET T MODE FAILED");
            }
            OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);
        }
    }
    else
    {
        OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_1, WUEPS_PID_USIM, WUEPS_PID_USIM, USIMMDL_DRV_USIMMSCI_CLASS_SWITCH);
        lSCIResult = DRV_USIMMSCI_CLASS_SWITCH();
        OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);

        *plVoltageSwitchRst = lSCIResult;

        if (USIMM_SCI_SUCCESS == lSCIResult)      /*电压切换成功后需要延迟*/
        {
            USIMM_NORMAL_LOG("USIMM_DLResetCard: SIM Card Change the Voltage Successful");
        }
        else if (USIMM_SCI_NONEED_CHANGEVCC == lSCIResult)
        {
            USIMM_NORMAL_LOG("USIMM_DLResetCard: Card needn't Change the Voltage");
        }
        else                     /*电压切换失败按照无卡处理*/
        {
            gastUSIMMCardAppInfo[USIMM_UICC_USIM].enCardType = USIMM_CARD_NOCARD;

            return VOS_ERR;
        }
    }

    for (i = 0; i < USIMM_GET_SCISTATUS_MAX; i++)        /*循环判断当前的卡复位情况*/
    {
        OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_1, WUEPS_PID_USIM, WUEPS_PID_USIM, USIMMDL_DRV_USIMMSCI_GET_CARD_STAU);
        lSCIResult = DRV_USIMMSCI_GET_CARD_STAU(); /*如果复位成功则返回成功*/
        OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);

        if (SCI_CARD_STATE_BUSY != lSCIResult)
        {
            break;
        }

        /*等待底软接收完成ATR和PPS过程*/
        if(VOS_OK != VOS_StartCallBackRelTimer(&g_ulUSIMMGetCardStatTimerID,
                                  WUEPS_PID_USIM,
                                  USIMM_CB_TIMER_LEN_GET_CARD_STATUS,
                                  USIMM_CB_TIMER_NAME_GET_CARD_STATUS,
                                  0,
                                  VOS_RELTIMER_NOLOOP,
                                  USIMM_DLGetCardStatusTimerHandler,
                                  VOS_TIMER_PRECISION_5))
        {
            USIMM_ERROR_LOG("USIMM_DLResetCard: USIMM Could Not Find Any SIM Card fail!");
            continue;
        }

        if(VOS_OK != VOS_SmP(g_ulUSIMMGetCardStatSemID, USIMM_GET_CARD_STATUS_SEM_LEN))
        {
            USIMM_ERROR_LOG("USIMM_DLResetCard: VOS_SmP fail!");
            /* 等待信号量超时，按照无卡处理 */
            lSCIResult = SCI_CARD_STATE_NOCARD;
            break;
        }
    }

    if (SCI_CARD_STATE_NOCARD == lSCIResult)
    {
        USIMM_ERROR_LOG("USIMM_DLResetCard: USIMM Could Not Find Any SIM Card!");

        gastUSIMMCardAppInfo[USIMM_UICC_USIM].enCardType = USIMM_CARD_NOCARD;
        
        if (USIMM_RESET_CARD == ulResetType)
        {   
            DRV_USIMMSCI_RECORD_DATA_SAVE(SCI_LOG_RESET_FAIL);
        }
        else
        {
            DRV_USIMMSCI_RECORD_DATA_SAVE(SCI_LOG_VOL_SWITCH_FAIL);
        }

        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_VOID USIMM_SciDataHook(VOS_UINT32 ulLength, VOS_UINT8 *pucData)
{
    MsgBlock     *pMsg;
    VOS_UINT32   ulResult;

#ifdef USIMM_DEBUG_PEINT
    VOS_UINT32   i;

    vos_printf("\r\nSciDataHook Length is :%d\r\n",ulLength);

    for(i = 0;i<ulLength; i++)
    {
        vos_printf("0x%02x,",pucData[i]);
    }

    vos_printf("\r\nend\n");
#endif

    USIMM_LogDataSave(pucData, (VOS_UINT8)ulLength);

    USIMM_ExcLogDataSave(pucData, (VOS_UINT8)ulLength);

    if(VOS_NULL_PTR == vos_MsgHook)
    {
        USIMM_WARNING_LOG("USIMM_SciDataHook:WARNING:vos_MsgHook is Null.\n");

        return;
    }

    pMsg = (MsgBlock*)VOS_AllocMsg(WUEPS_PID_USIM, ulLength);

    if(VOS_NULL_PTR == pMsg)
    {
        USIMM_WARNING_LOG("USIMM_SciDataHook:WARNING:alloc msg failed\n");

        return;
    }

    pMsg->ulReceiverPid   = WUEPS_PID_USIM;

    pMsg->ulLength        = ulLength;

	/*lint --e{831,669} */
    VOS_MemCpy(pMsg->aucValue, pucData, ulLength);

    ulResult = (vos_MsgHook)((VOS_VOID *)(pMsg));

    if(VOS_OK != ulResult)
    {
        USIMM_WARNING_LOG("USIMM_SciDataHook: vos_MsgHook is Failed");
    }

    VOS_FreeMsg(WUEPS_PID_USIM, pMsg);

    return;
}
VOS_VOID USIMM_DLStateStar(USIMM_APDU_ST* pstAPDUData)
{
    USIMM_DLJudgeCmdCase(pstAPDUData);

    /*发送命令头，初始化相关值*/
    gstUSIMMDLControl.ulSendDataLen = sizeof(pstAPDUData->aucAPDU);

    gstUSIMMDLControl.ulRecDataLen  = sizeof(pstAPDUData->ucACK);

    gstUSIMMDLControl.enDLError     = USIMMDL_NOERROR;

    gstUSIMMDLControl.enDLState     = USIMMDL_STATE_SENDAPDU;

    return;
}
VOS_INT32 USIMM_DLSendDataOneByOne(VOS_UINT8 ucINS,VOS_UINT32 ulDataLen, VOS_UINT8* pucData)
{
    VOS_UINT32                          i;
    VOS_UINT8                           ucRecData = 0;
    VOS_UINT8                           ucSendData;
    VOS_INT32                           lSCIResult = 0;
    VOS_UINT32                          ulRemainLen = ulDataLen;
    USIMMDL_STATE_ENUM_UINT32   enSendState = USIMMDL_STATE_SENDDATA;

    for(i=0; i<ulDataLen; )
    {
        if(enSendState == USIMMDL_STATE_SENDDATA)
        {
            ucSendData = pucData[i];

            OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_1, WUEPS_PID_USIM, WUEPS_PID_USIM, USIMMDL_DRV_USIMMSCI_SND_DATA1);
            lSCIResult = DRV_USIMMSCI_SND_DATA(sizeof(VOS_UINT8), &ucSendData);
            OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);

            if(USIMM_SCI_SUCCESS != lSCIResult)      /*发送一个字节*/
            {
                USIMM_ERROR_LOG("USIMM_DLSendDataOneByOne: USIMMSCISendData Error");

                return VOS_ERR;
            }

            ulRemainLen = ((ulDataLen - i) - 1);

            if(ulRemainLen == 0x00) /*逐字节发送的最后一个字节不用接收INS*/
            {
                return VOS_OK;
            }
        }


        OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_1, WUEPS_PID_USIM, WUEPS_PID_USIM, USIMMDL_DRV_USIMMSCI_RCV1);
        lSCIResult = DRV_USIMMSCI_RCV(sizeof(VOS_UINT8), &ucRecData);
        OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);

        if(USIMM_SCI_SUCCESS != lSCIResult)  /*接收过程字节*/
        {
            USIMM_ERROR_LOG("USIMM_DLSendDataOneByOne: USIMMSCIReceived Error");

            return VOS_ERR;
        }

        if(ucRecData == ucINS)  /*如果字节和INS相等，则需要把剩余字节一次性发送*/
        {

            OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_1, WUEPS_PID_USIM, WUEPS_PID_USIM, USIMMDL_DRV_USIMMSCI_SND_DATA2);
            lSCIResult = DRV_USIMMSCI_SND_DATA(ulRemainLen, &pucData[i+1]);
            OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);

            return ((USIMM_SCI_SUCCESS == lSCIResult)?VOS_OK:VOS_ERR);
        }
        else if(ucRecData == 0x60)
        {
            USIMM_WARNING_LOG("USIMM_DLSendDataOneByOne: Receive NULL Byte");

            enSendState = USIMMDL_STATE_WAITACK;
        }
        else
        {
            enSendState = USIMMDL_STATE_SENDDATA;

            i++;
        }
    }

    return VOS_OK;
}


VOS_UINT32 USIMM_DLIdle(USIMM_APDU_ST *pstAPDUData)
{
    USIMM_ERROR_LOG("USIMM_DLIdle: Usimm DL Run into Error State");

    gstUSIMMDLControl.enDLError = USIMMDL_STATE_ERROR;

    return VOS_ERR;
}


VOS_UINT32 USIMM_DLSendAPDU(USIMM_APDU_ST *pstAPDUData)
{
    VOS_INT32                           lSCIResult;
    VOS_UINT8                          *pucSendData;

    OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_1, WUEPS_PID_USIM, WUEPS_PID_USIM, USIMMDL_DRV_USIMMSCI_SND_DATA3);

    if (VOS_FALSE == gstUSIMMDLControl.bSendRep)
    {
        pucSendData = pstAPDUData->aucAPDU;
    }
    else
    {
        pucSendData = pstAPDUData->aucGetRepAPDU;
    }

    lSCIResult = DRV_USIMMSCI_SND_DATA(gstUSIMMDLControl.ulSendDataLen, pucSendData);

    OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);

    USIMM_SciDataHook(gstUSIMMDLControl.ulSendDataLen, pucSendData);

    if (USIMM_SCI_SUCCESS == lSCIResult)             /*命令数据发送成功*/
    {
        gstUSIMMDLControl.enDLState = USIMMDL_STATE_WAITACK;
    }
    else                                /*命令发送失败*/
    {
        USIMM_ERROR_LOG("USIMM_DLSendAPDU: USIMMSCISendData APDU Error");

        gstUSIMMDLControl.enDLError = USIMMDL_SCISENDAPDU_ERROR;
    }

    gstUSIMMDLControl.ulSendAPDUTime++;

    return ((USIMM_SCI_SUCCESS == lSCIResult)?VOS_OK:VOS_ERR);/* [false alarm]: 屏蔽Fortify 错误 */
}
VOS_UINT32 USIMM_DLSendData(USIMM_APDU_ST *pstAPDUData)
{
    VOS_INT32                           lSCIResult;
    VOS_UINT8                           aucSendBuf[USIMM_APDU_MAXLEN];

    if(pstAPDUData->ucACK == pstAPDUData->aucAPDU[INS])/*一次性发送*/
    {
        OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_1, WUEPS_PID_USIM, WUEPS_PID_USIM, USIMMDL_DRV_USIMMSCI_SND_DATA4);
        lSCIResult = DRV_USIMMSCI_SND_DATA(gstUSIMMDLControl.ulSendDataLen, pstAPDUData->aucSendBuf);
        OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);
    }
    else if(pstAPDUData->ucACK == (VOS_UINT8)(~pstAPDUData->aucAPDU[INS]))/*逐一发送*/
    {
        lSCIResult = USIMM_DLSendDataOneByOne(pstAPDUData->aucAPDU[INS],
                                            gstUSIMMDLControl.ulSendDataLen,
                                            pstAPDUData->aucSendBuf);
    }
    else                                                                                    /*ACK内容错误*/
    {
        USIMM_ERROR_LOG("USIMM_DLSendData: The INS is Wrong");/*打印错误*/

        gstUSIMMDLControl.enDLError = USIMMDL_SCIRECEIVEACK_ERROR;

        return VOS_ERR;
    }

    if ((USIMM_PINCMD_VERIFY == pstAPDUData->aucAPDU[INS])
        || (USIMM_PINCMD_CHANGE == pstAPDUData->aucAPDU[INS])
        || (USIMM_PINCMD_DISABLE == pstAPDUData->aucAPDU[INS])
        || (USIMM_PINCMD_ENABLE == pstAPDUData->aucAPDU[INS])
        || (USIMM_PINCMD_UNBLOCK == pstAPDUData->aucAPDU[INS]))
    {
        VOS_MemSet(aucSendBuf, (VOS_CHAR)0xFF, sizeof(aucSendBuf));
        USIMM_SciDataHook(gstUSIMMDLControl.ulSendDataLen, aucSendBuf);
    }
    else
    {
        USIMM_SciDataHook(gstUSIMMDLControl.ulSendDataLen, pstAPDUData->aucSendBuf);
    }

    if(USIMM_SCI_SUCCESS == lSCIResult)     /*命令数据发送成功*/
    {
        gstUSIMMDLControl.ulRecDataLen = 1; /*接收SW1,由于60的出现所以SW1,SW2分开接受*/

        gstUSIMMDLControl.enDLState    = USIMMDL_STATE_WAITACK;/*收发转向的时候需要先接受ACK*/
    }
    else                                /*命令发送失败*/
    {
        USIMM_ERROR_LOG("USIMM_DLSendData: SCI Send Data Error");

        gstUSIMMDLControl.enDLError = USIMMDL_SCISENDLC_ERROR;
    }

    return ((USIMM_SCI_SUCCESS == lSCIResult)?VOS_OK:VOS_ERR);/* [false alarm]: 屏蔽Fortify 错误 */
}
VOS_UINT32 USIMM_DLReceiveACK(USIMM_APDU_ST *pstAPDUData)
{
    VOS_UINT8                           aucRecData[2] = {0};
    VOS_INT32                           lSCIesult = 0;
    VOS_UINT8                           ucINS;

    OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_1, WUEPS_PID_USIM, WUEPS_PID_USIM, USIMMDL_DRV_USIMMSCI_RCV2);
    lSCIesult = DRV_USIMMSCI_RCV(1, aucRecData);
    OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);

    if(VOS_OK != lSCIesult)/*接收错误*/
    {
        USIMM_ERROR_LOG("USIMM_DLReceiveACK: USIMMSCIReceived ACK Error");

        gstUSIMMDLControl.enDLError = USIMMDL_SCIRECEIVEACK_ERROR;

        return VOS_ERR;
    }

    if(aucRecData[0] == 0x60)   /*跳过0x60字节，状态不变再进此函数收ACK*/
    {
        USIMM_WARNING_LOG("USIMM_DLReceiveACK:receive 0x60 byte");

        USIMM_SciDataHook(1, aucRecData);    /*接收成功*/

        return VOS_OK;
    }
    else if(((aucRecData[0]&0xF0) == 0x60)||((aucRecData[0]&0xF0) == 0x90))/*接收为SW1字节*/
    {
        gstUSIMMDLControl.enDLState    = USIMMDL_STATE_WAITSW;     /*准备接收SW2字节*/

        gstUSIMMDLControl.ulRecDataLen = 1;

        pstAPDUData->ucSW1             = aucRecData[0];
    }
    else                                    /*接收为ACK内容*/
    {
        USIMM_SciDataHook(gstUSIMMDLControl.ulRecDataLen, aucRecData);    /*接收成功*/

        pstAPDUData->ucACK = aucRecData[0];

        if(USIMM_CMD_CASE1 == gstUSIMMDLControl.enCMDCase) /*准备接收SW */
        {
            if (VOS_FALSE == gstUSIMMDLControl.bSendRep)
            {
                ucINS = pstAPDUData->aucAPDU[INS];
            }
            else
            {
                ucINS = pstAPDUData->aucGetRepAPDU[INS];
            }
            
            if (pstAPDUData->ucACK == ucINS)
            {
                USIMM_INFO_LOG("USIMM_DLReceiveACK: Case 1 To Case 2 Command, Receive Data");

                gstUSIMMDLControl.enCMDCase    = USIMM_CMD_CASE2;

                gstUSIMMDLControl.ulRecDataLen = USIMM_APDU_MAXLEN;

                pstAPDUData->ulLeValue         = USIMM_APDU_MAXLEN;

                gstUSIMMDLControl.enDLState    = USIMMDL_STATE_WAITDATA;
            }
            else    /*当前字节不识别，尝试接收2字节数据作为SW*/
            {
                USIMM_INFO_LOG("USIMM_DLReceiveACK: Case 1 Command, Receive SW");

                gstUSIMMDLControl.ulRecDataLen = 2; /*准备接收SW1,SW2*/

                gstUSIMMDLControl.enDLState    = USIMMDL_STATE_WAITSW;
            }
        }
        else if(USIMM_CMD_CASE2 == gstUSIMMDLControl.enCMDCase)/*需要接收命令内容Le*/
        {
            USIMM_INFO_LOG("USIMM_DLReceiveACK: Case 2 Command, Receive Cmd Data");

            gstUSIMMDLControl.ulRecDataLen = pstAPDUData->ulLeValue;

            gstUSIMMDLControl.enDLState    = USIMMDL_STATE_WAITDATA;
        }
        else if(USIMM_CMD_CASE3 == gstUSIMMDLControl.enCMDCase)/*需要发送命令内容Lc*/
        {
            USIMM_INFO_LOG("USIMM_DLReceiveACK: Case 3 Command, Send Command Data");

            gstUSIMMDLControl.ulSendDataLen  = pstAPDUData->ulLcValue;

            gstUSIMMDLControl.enDLState      = USIMMDL_STATE_SENDDATA;
        }
        else
        {
            USIMM_ERROR_LOG("USIMM_DLReceiveACK: Case 4 Command, Can Not Handler");

            gstUSIMMDLControl.enDLError = USIMMDL_STATE_ERROR;

            return VOS_ERR;
        }
    }

    return VOS_OK;
}

VOS_UINT32 USIMM_DLReceiveData(USIMM_APDU_ST *pstAPDUData)
{
    VOS_INT32                           lSCIResult = 0;
    VOS_UINT8                           aucRecData[256] = {0};

    OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_1, WUEPS_PID_USIM, WUEPS_PID_USIM, USIMMDL_DRV_USIMMSCI_RCV3);
    lSCIResult = DRV_USIMMSCI_RCV(gstUSIMMDLControl.ulRecDataLen, aucRecData);
    OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);

    if(USIMM_SCI_SUCCESS == lSCIResult)                           /*接收成功准备接收SW字节*/
    {
        USIMM_SciDataHook(gstUSIMMDLControl.ulRecDataLen, aucRecData);

        if((pstAPDUData->ulRecDataLen+gstUSIMMDLControl.ulRecDataLen) > sizeof(pstAPDUData->aucRecvBuf))   /*多次获取的内容已经超长，返回错误，不支持该情况*/
        {
            USIMM_WARNING_LOG("USIMM_GetResponse_APDU: The Receive Data Too long to the aucRecvBuf");
        }
        else
        {
            VOS_MemCpy(pstAPDUData->aucRecvBuf+pstAPDUData->ulRecDataLen, aucRecData, gstUSIMMDLControl.ulRecDataLen);   /*多次获取的内容需要保存在局部变量中*/

            pstAPDUData->ulRecDataLen += gstUSIMMDLControl.ulRecDataLen;                         /*记录总共保存的数据长度*/
        }
#if (VOS_WIN32 == VOS_OS_VER)
        gstUSIMMDLControl.enDLState    = USIMMDL_STATE_WAITSW;

        gstUSIMMDLControl.ulRecDataLen = 2;         /*接受全部的SW1,SW2*/
#else
         /*接受下一个过程字节，卡有可能回复的是60,而不是SW*/
        gstUSIMMDLControl.enDLState    = USIMMDL_STATE_WAITACK;
        gstUSIMMDLControl.ulRecDataLen = 1;
#endif
    }
    else                                            /*接收失败*/
    {
        USIMM_ERROR_LOG("USIMM_DLReceiveData: USIMMSCIReceived Data Error");

        gstUSIMMDLControl.enDLError = USIMMDL_SCIRECEIVELE_ERROR;
    }

    return ((USIMM_SCI_SUCCESS == lSCIResult)?VOS_OK:VOS_ERR);/* [false alarm]: 屏蔽Fortify 错误 */
}


VOS_UINT32 USIMM_DLReceiveSW(USIMM_APDU_ST *pstAPDUData)
{
    VOS_INT32                           lSCIResult = 0;
    VOS_UINT32                          ulResult = 0;
    VOS_UINT8                           aucRecData[2] = {0};

    OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_1, WUEPS_PID_USIM, WUEPS_PID_USIM, USIMMDL_DRV_USIMMSCI_RCV4);
    lSCIResult = DRV_USIMMSCI_RCV(gstUSIMMDLControl.ulRecDataLen, aucRecData);
    OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);

    if(USIMM_SCI_SUCCESS == lSCIResult)                      /*接收成功*/
    {
        if (1 == gstUSIMMDLControl.ulRecDataLen) /*当前接收的SW2*/
        {
            pstAPDUData->ucSW2 = aucRecData[0];
        }
        else
        {
            pstAPDUData->ucSW1 = aucRecData[0];    /*保存SW内容*/

            pstAPDUData->ucSW2 = aucRecData[1];
        }

        if (1 == gstUSIMMDLControl.ulSendAPDUTime)   /*保存第一次发送返回SW内容*/
        {
            pstAPDUData->ucApduSW1 = pstAPDUData->ucSW1;

            pstAPDUData->ucApduSW2 = pstAPDUData->ucSW2;
        }

        gstUSIMMDLControl.enDLError = USIMMDL_NOERROR;

        USIMM_SciDataHook(2, &pstAPDUData->ucSW1);
    }
    else                                        /*接收失败*/
    {
        USIMM_ERROR_LOG("USIMM_DLReceiveSW: USIMMSCIReceived SW Error");

        gstUSIMMDLControl.enDLError = USIMMDL_SCIRECEIVESW_ERROR;

        return VOS_ERR;
    }

    ulResult = USIMM_CheckSW(pstAPDUData);

    if (USIMM_SW_P3_ERROR == ulResult)   /*由于P3字节错误，改变之后重新进入新的DL流程发送*/
    {
        USIMM_WARNING_LOG("USIMM_DLReceiveSW:Receive 6C or 67 Byte,Resend APDU Command");

        if ((VOS_TRUE == gstUSIMMDLControl.bSendRep)&&(CMD_INS_GET_RESPONSE == pstAPDUData->aucGetRepAPDU[INS]))
        {
            pstAPDUData->ulLeValue  = pstAPDUData->ucSW2;/*更改对应的Le的长度*/

            pstAPDUData->aucGetRepAPDU[P3]= pstAPDUData->ucSW2;/*更新P3字节的错误*/
        }
        else
        {
            if (CMD_INS_STATUS == pstAPDUData->aucAPDU[INS])/*STATUS命令需要更新对应的全局变量内容*/
            {
                pstAPDUData->ulLeValue  = pstAPDUData->ucSW2;/*更改对应的Le的长度*/
            }
            else
            {
                if (pstAPDUData->ulLcValue != 0)
                {
                    pstAPDUData->ulLcValue  = pstAPDUData->ucSW2;/*更改对应的Lc的长度*/
                }
                else
                {
                    pstAPDUData->ulLeValue  = pstAPDUData->ucSW2;/*更改对应的Le的长度*/
                }
            }

            pstAPDUData->aucAPDU[P3]= pstAPDUData->ucSW2;/*更新P3字节的错误*/
        }

        USIMM_DLStateStar(pstAPDUData);              /*重新开始DL层的初始化*/

        return VOS_OK;
    }

    if (USIMM_SW_OK_WITH_RESOPNSE == ulResult)
    {
        gstUSIMMDLControl.enDLState = USIMMDL_STATE_GETRESPONSE;

        return VOS_OK;
    }

    /* 参考7816-4 8.6.1 */
    if (USIMM_SW_WARNING == ulResult)
    {
        gstUSIMMDLControl.enDLState = USIMMDL_STATE_GETRESPONSE;
    }
    else
    {
        gstUSIMMDLControl.enDLState = USIMMDL_STATE_IDLE;
    }

    return VOS_OK;
}
VOS_UINT32 USIMM_DLGetResponse(USIMM_APDU_ST* pstAPDUData)
{
    if (0x80 == pstAPDUData->aucAPDU[CLA])       /* Envelope Command 需要获取响应值 */
    {
        pstAPDUData->aucGetRepAPDU[CLA]   = 0x00;
    }
    else
    {
        pstAPDUData->aucGetRepAPDU[CLA]   = pstAPDUData->aucAPDU[CLA];
    }

    pstAPDUData->aucGetRepAPDU[INS]   = CMD_INS_GET_RESPONSE;

    pstAPDUData->aucGetRepAPDU[P1]    = 0x00;

    pstAPDUData->aucGetRepAPDU[P2]    = 0x00;

    if((0x61 == pstAPDUData->ucSW1)||(0x9F == pstAPDUData->ucSW1))
    {
        pstAPDUData->aucGetRepAPDU[P3] = pstAPDUData->ucSW2; /*填入上一次命令状态字61XX中SW2的值*/

        if(0x00 == pstAPDUData->aucGetRepAPDU[P3])
        {
            pstAPDUData->ulLeValue  = 0x100;
        }
        else
        {
            pstAPDUData->ulLeValue  = pstAPDUData->aucGetRepAPDU[P3];
        }
    }
    else
    {
        pstAPDUData->aucGetRepAPDU[P3]= 0x00;             /*如果是62或者63*/

        pstAPDUData->ulLeValue  = 0x00;
    }

    pstAPDUData->ulLcValue      = 0x00;

    pstAPDUData->ucSW1          = 0x00;

    pstAPDUData->ucSW2          = 0x00;

    pstAPDUData->ucACK          = 0x00;

    gstUSIMMDLControl.bSendRep  = VOS_TRUE;

    USIMM_DLStateStar(pstAPDUData);

    return VOS_OK;
}
static USIMMDL_FUNC_STATUS gastUSIMMDLProc[] =
{
    {USIMMDL_STATE_IDLE,        USIMM_DLIdle},
    {USIMMDL_STATE_SENDAPDU,    USIMM_DLSendAPDU},
    {USIMMDL_STATE_SENDDATA,    USIMM_DLSendData},
    {USIMMDL_STATE_WAITACK,     USIMM_DLReceiveACK},
    {USIMMDL_STATE_WAITDATA,    USIMM_DLReceiveData},
    {USIMMDL_STATE_WAITSW,      USIMM_DLReceiveSW},
    {USIMMDL_STATE_GETRESPONSE, USIMM_DLGetResponse}
};

VOS_UINT32 USIMM_T0DLHandle(USIMM_APDU_ST* pstAPDUData)
{
    VOS_UINT32  ulResult = 0;           /* 操作结果 */
    VOS_UINT32  i;

    if(USIMMDL_STATE_IDLE != gstUSIMMDLControl.enDLState)
    {
        USIMM_WARNING_LOG("USIMM_T0DLHandle: DL State is Wrong");/* 打印警告 */
    }

    USIMM_ExcLogDataBK();

    pstAPDUData->ulRecDataLen  = 0;     /*存在多次接受数据，因此DL层开始的时候才能清零*/

    VOS_MemSet(&gstUSIMMDLControl, 0, sizeof(gstUSIMMDLControl));   /* 初始化当前状态 */

    VOS_MemSet(pstAPDUData->aucGetRepAPDU, 0, USIMM_APDU_HEADLEN);

    USIMM_DLStateStar(pstAPDUData);

    if(USIMM_CMD_CASE4 == gstUSIMMDLControl.enCMDCase)
    {
        /* T=0协议，没有case4的情况，是通过case3+case2实现的case4 */
        gstUSIMMDLControl.enCMDCase = USIMM_CMD_CASE3;

        pstAPDUData->ulLeValue = 0;
    }

    for(i=0; i<(sizeof(gastUSIMMDLProc)/sizeof(USIMMDL_FUNC_STATUS)); i++)
    {
        if(gstUSIMMDLControl.enDLState == gastUSIMMDLProc[i].enStateId)
        {
            ulResult = gastUSIMMDLProc[i].pProcFun(pstAPDUData);

            /*当出现错误或者完成DL过程后退出*/
            if((VOS_OK != ulResult)||(USIMMDL_STATE_IDLE == gstUSIMMDLControl.enDLState))
            {
                break;
            }

            i = 0;
        }
    }

    gstUSIMMDLControl.enDLState = USIMMDL_STATE_IDLE;

    /*根据错误信息返回函数操作结果*/
    if(USIMMDL_NOERROR == gstUSIMMDLControl.enDLError)
    {
        USIMM_INFO_LOG("USIMM_T0DLHandle:DL Complete the Step Right");

        ulResult = USIMM_DL_SUCESS;

        USIMM_ExcLogDataRE();
    }
    else if(gstUSIMMDLControl.enDLError > USIMMDL_SCISENDLC_ERROR)
    {
        USIMM_ERROR_LOG("USIMM_T0DLHandle:DL Complete the Step With Receive Data Error");

        ulResult = USIMM_DL_RECEIVE_ERROR;
    }
    else
    {
        USIMM_ERROR_LOG("USIMM_T0DLHandle:DL Complete the Step With Send Data Error");

        ulResult = USIMM_DL_SEND_ERROR;
    }

    gstUSIMMDLControl.enDLState = USIMMDL_STATE_IDLE;   /*DL过程结束*/

    return ulResult;
}


VOS_UINT32 USIMM_DLHandle(USIMM_APDU_ST* pstAPDUData)
{
    VOS_UINT32      ulResult;           /* 操作结果 */

    if(USIMM_DL_T_MODE_T0 == g_enUSIMMTMode)
    {
        /* 当前支持T=0 */
        ulResult = USIMM_T0DLHandle(pstAPDUData);
    }
    else if(USIMM_DL_T_MODE_T1 == g_enUSIMMTMode)
    {
        /* 当前支持T=1 */
        ulResult = USIMM_T1DLHandle(pstAPDUData);
    }
    else
    {
        USIMM_ERROR_LOG("USIMM_DLHandle: USIMM T mode is BUTT");

        ulResult = VOS_ERR;
    }

    return ulResult;
}
VOS_UINT32 USIMM_DLCglaReceiveSW(USIMM_APDU_ST *pstAPDUData)
{
    VOS_INT32                           lSCIResult = 0;
    VOS_UINT32                          ulResult = 0;
    VOS_UINT8                           aucRecData[2] = {0};

    OM_RecordInfoStart(VOS_EXC_DUMP_MEM_NUM_1, WUEPS_PID_USIM, WUEPS_PID_USIM, USIMMDL_DRV_USIMMSCI_RCV4);
    lSCIResult = DRV_USIMMSCI_RCV(gstUSIMMDLControl.ulRecDataLen, aucRecData);
    OM_RecordInfoEnd(VOS_EXC_DUMP_MEM_NUM_1);

    if(USIMM_SCI_SUCCESS == lSCIResult)                      /*接收成功*/
    {
        if(1 == gstUSIMMDLControl.ulRecDataLen) /*当前接收的SW2*/
        {
            pstAPDUData->ucSW2 = aucRecData[0];
        }
        else
        {
            pstAPDUData->ucSW1 = aucRecData[0];    /*保存SW内容*/

            pstAPDUData->ucSW2 = aucRecData[1];
        }

        gstUSIMMDLControl.enDLError = USIMMDL_NOERROR;

        USIMM_SciDataHook(2, &pstAPDUData->ucSW1);
    }
    else                                        /*接收失败*/
    {
        USIMM_ERROR_LOG("USIMM_DLCglaReceiveSW: USIMMSCIReceived SW Error");

        gstUSIMMDLControl.enDLError = USIMMDL_SCIRECEIVESW_ERROR;

        return VOS_ERR;
    }

    ulResult = USIMM_CheckSW(pstAPDUData);

    /* 保存第一次发送返回SW内容和Result值 */
    if (1 == gstUSIMMDLControl.ulSendAPDUTime)
    {
        pstAPDUData->ucApduSW1 = pstAPDUData->ucSW1;
        pstAPDUData->ucApduSW2 = pstAPDUData->ucSW2;
        pstAPDUData->ucResult  = (VOS_UINT8)ulResult;
    }

    /* 中移测试有发送INS=0x09，需要直接返回 */
    if((USIMM_SW_P3_ERROR == ulResult)&&(0x09 != pstAPDUData->aucAPDU[INS]))   /*由于P3字节错误，改变之后重新进入新的DL流程发送*/
    {
        USIMM_WARNING_LOG("USIMM_DLCglaReceiveSW:Receive 6C or 67 Byte,Resend APDU Command");

        if ((VOS_TRUE == gstUSIMMDLControl.bSendRep)&&(CMD_INS_GET_RESPONSE == pstAPDUData->aucGetRepAPDU[INS]))
        {
            pstAPDUData->ulLeValue  = pstAPDUData->ucSW2;/*更改对应的Le的长度*/

            pstAPDUData->aucGetRepAPDU[P3]= pstAPDUData->ucSW2;/*更新P3字节的错误*/
        }
        else
        {
            if (CMD_INS_STATUS == pstAPDUData->aucAPDU[INS])/*STATUS命令需要更新对应的全局变量内容*/
            {
                pstAPDUData->ulLeValue  = pstAPDUData->ucSW2;/*更改对应的Le的长度*/
            }
            else
            {
                if (pstAPDUData->ulLcValue != 0)
                {
                    pstAPDUData->ulLcValue  = pstAPDUData->ucSW2;/*更改对应的Lc的长度*/
                }
                else
                {
                    pstAPDUData->ulLeValue  = pstAPDUData->ucSW2;/*更改对应的Le的长度*/
                }
            }

            pstAPDUData->aucAPDU[P3]= pstAPDUData->ucSW2;/*更新P3字节的错误*/
        }

        USIMM_DLStateStar(pstAPDUData);              /*重新开始DL层的初始化*/

        return VOS_OK;
    }

    if(USIMM_SW_OK_WITH_RESOPNSE == ulResult)
    {
        gstUSIMMDLControl.enDLState = USIMMDL_STATE_GETRESPONSE;

        return VOS_OK;
    }

    gstUSIMMDLControl.enDLState = USIMMDL_STATE_IDLE;

    /* 参考7816-4 8.6.1 */
    if (USIMM_SW_WARNING == ulResult)
    {
        if (VOS_TRUE == g_stUsimmFeatureCfg.unCfg.stFeatureCfg.ulCglaGetRsp)
        {
            gstUSIMMDLControl.enDLState = USIMMDL_STATE_GETRESPONSE;
        }
    }

    return VOS_OK;
}


VOS_UINT32 USIMM_DLCglaGetResponse(USIMM_APDU_ST* pstAPDUData)
{
    VOS_UINT32                      ulTempLen;

    if (USIMM_SIM_CLA != pstAPDUData->aucAPDU[CLA])
    {
        /* 参照102.221协议,GET RESPONSE的INS字段支持'0X' or '4X' or '6X' */
        pstAPDUData->aucGetRepAPDU[CLA] = pstAPDUData->aucAPDU[CLA] & 0x6F;
    }
    else
    {
        pstAPDUData->aucGetRepAPDU[CLA] = USIMM_SIM_CLA;
    }

    pstAPDUData->aucGetRepAPDU[INS]   = CMD_INS_GET_RESPONSE;

    pstAPDUData->aucGetRepAPDU[P1]    = 0x00;

    pstAPDUData->aucGetRepAPDU[P2]    = 0x00;

    if((0x61 == pstAPDUData->ucSW1)||(0x9F == pstAPDUData->ucSW1))
    {
        pstAPDUData->aucGetRepAPDU[P3] = pstAPDUData->ucSW2; /*填入上一次命令状态字61XX中SW2的值*/

        if(0x00 == pstAPDUData->aucGetRepAPDU[P3])
        {
            ulTempLen = 0x100;
        }
        else
        {
            ulTempLen = pstAPDUData->aucGetRepAPDU[P3];
        }

        if ((pstAPDUData->ulRecDataLen + ulTempLen) > USIMM_APDU_MAXLEN)
        {
            gstUSIMMDLControl.enDLState = USIMMDL_STATE_IDLE;

            return VOS_OK;
        }

        pstAPDUData->ulLeValue  = ulTempLen;
    }
    else
    {
        pstAPDUData->aucGetRepAPDU[P3]= 0x00;             /*如果是62或者63*/

        pstAPDUData->ulLeValue  = 0x00;
    }

    pstAPDUData->ulLcValue      = 0x00;

    pstAPDUData->ucSW1          = 0x00;

    pstAPDUData->ucSW2          = 0x00;

    pstAPDUData->ucACK          = 0x00;

    gstUSIMMDLControl.bSendRep  = VOS_TRUE;

    USIMM_DLStateStar(pstAPDUData);

    return VOS_OK;
}
static USIMMDL_FUNC_STATUS gastUSIMMCglaDLProc[] =
{
    {USIMMDL_STATE_IDLE,        USIMM_DLIdle},
    {USIMMDL_STATE_SENDAPDU,    USIMM_DLSendAPDU},
    {USIMMDL_STATE_SENDDATA,    USIMM_DLSendData},
    {USIMMDL_STATE_WAITACK,     USIMM_DLReceiveACK},
    {USIMMDL_STATE_WAITDATA,    USIMM_DLReceiveData},
    {USIMMDL_STATE_WAITSW,      USIMM_DLCglaReceiveSW},
    {USIMMDL_STATE_GETRESPONSE, USIMM_DLCglaGetResponse}
};

VOS_UINT32 USIMM_T0CglaDLHandle(USIMM_APDU_ST* pstAPDUData)
{
    VOS_UINT32  ulResult = 0;           /* 操作结果 */
    VOS_UINT32  i;

    if(USIMMDL_STATE_IDLE != gstUSIMMDLControl.enDLState)
    {
        USIMM_WARNING_LOG("USIMM_T0CglaDLHandle: DL State is Wrong");/* 打印警告 */

        gstUSIMMDLControl.enDLState = USIMMDL_STATE_IDLE;/* 初始化当前状态 */
    }

    pstAPDUData->ulRecDataLen  = 0;     /*存在多次接受数据，因此DL层开始的时候才能清零*/

    VOS_MemSet(&gstUSIMMDLControl, 0, sizeof(gstUSIMMDLControl));   /* 初始化当前状态 */

    VOS_MemSet(pstAPDUData->aucGetRepAPDU, 0, USIMM_APDU_HEADLEN);

    USIMM_DLStateStar(pstAPDUData);

    if(USIMM_CMD_CASE4 == gstUSIMMDLControl.enCMDCase)
    {
        /* T=0协议，没有case4的情况，是通过case3+case2实现的case4 */
        gstUSIMMDLControl.enCMDCase = USIMM_CMD_CASE3;

        pstAPDUData->ulLeValue = 0;
    }

    for(i=0; i<(sizeof(gastUSIMMCglaDLProc)/sizeof(USIMMDL_FUNC_STATUS)); i++)
    {
        if(gstUSIMMDLControl.enDLState == gastUSIMMCglaDLProc[i].enStateId)
        {
            ulResult = gastUSIMMCglaDLProc[i].pProcFun(pstAPDUData);

            /*当出现错误或者完成DL过程后退出*/
            if((VOS_OK != ulResult)||(USIMMDL_STATE_IDLE == gstUSIMMDLControl.enDLState))
            {
                break;
            }

            i = 0;
        }
    }

    gstUSIMMDLControl.enDLState = USIMMDL_STATE_IDLE;

    /*根据错误信息返回函数操作结果*/
    if(USIMMDL_NOERROR == gstUSIMMDLControl.enDLError)
    {
        USIMM_INFO_LOG("USIMM_T0CglaDLHandle:DL Complete the Step Right");

        ulResult = USIMM_DL_SUCESS;
    }
    else if(gstUSIMMDLControl.enDLError > USIMMDL_SCISENDLC_ERROR)
    {
        USIMM_ERROR_LOG("USIMM_T0CglaDLHandle:DL Complete the Step With Receive Data Error");

        ulResult = USIMM_DL_RECEIVE_ERROR;
    }
    else
    {
        USIMM_ERROR_LOG("USIMM_T0CglaDLHandle:DL Complete the Step With Send Data Error");

        ulResult = USIMM_DL_SEND_ERROR;
    }

    gstUSIMMDLControl.enDLState = USIMMDL_STATE_IDLE;   /*DL过程结束*/

    return ulResult;
}
VOS_UINT32 USIMM_CglaDLHandle(USIMM_APDU_ST* pstAPDUData)
{

    VOS_UINT32  ulResult = 0;           /* 操作结果 */

    if(USIMM_DL_T_MODE_T0 == g_enUSIMMTMode)
    {
        /* 当前支持T=0 */
        ulResult = USIMM_T0CglaDLHandle(pstAPDUData);
    }
    else if(USIMM_DL_T_MODE_T1 == g_enUSIMMTMode)
    {
        /* 当前支持T=1 */
        ulResult = USIMM_T1DLHandle(pstAPDUData);
    }
    else
    {
        USIMM_ERROR_LOG("USIMM_CglaDLHandle: USIMM T mode is BUTT");

        ulResult = VOS_ERR;
    }

    return ulResult;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */




