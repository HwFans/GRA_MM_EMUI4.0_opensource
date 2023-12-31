/*****************************************************************************/
/*                                                                           */
/*                Copyright 1999 - 2003, Huawei Tech. Co., Ltd.              */
/*                           ALL RIGHTS RESERVED                             */
/*                                                                           */
/* FileNmae: \vos\include\vos\v_id.inc                                       */
/*                                                                           */
/* Author: Qin Peifeng                                                       */
/*                                                                           */
/* Version: 1.0                                                              */
/*                                                                           */
/* Date: 2000-04-21                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/* Others:                                                                   */
/*                                                                           */
/* History:                                                                  */
/* 1. Date: 2000-04-21                                                       */
/*    Author: Qin Peifeng                                                    */
/*    Modification: Create this file                                         */
/*                                                                           */
/*****************************************************************************/

#ifndef _V_ID_INC
#define _V_ID_INC

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#include "product_config.h"

extern VOS_UINT32 MOD_RegFidPidRSP( VOS_UINT32 ulPID, VOS_UINT32 ulFID, VOS_INT priority);

/*********************** INNER VARIABLES DEFINITION **************************/
/* PID与FID对应关系表 */
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
BEGIN_DECLARE_PID_BELONG_TO_FID()
    ON_DECLARE(I0_UEPS_PID_DL,                  I0_UEPS_FID_PBD,        VOS_PRIORITY_NULL)
    ON_DECLARE(I0_UEPS_PID_LL,                  I0_UEPS_FID_SL,         VOS_PRIORITY_NULL)
    ON_DECLARE(I0_UEPS_PID_SN,                  I0_UEPS_FID_SL,         VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_BMC,                   UEPS_FID_PBD,           VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_PDCP,                  UEPS_FID_PBD,           VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_RLC,                   UEPS_FID_RM,            VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_MAC,                   UEPS_FID_RM,            VOS_PRIORITY_NULL)
    ON_DECLARE(I0_UEPS_PID_GAS,                 I0_UEPS_FID_RR,         VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_WRR,                   UEPS_FID_RR,            VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_WCOM,                  UEPS_FID_RR,            VOS_PRIORITY_NULL)
    ON_DECLARE(I0_UEPS_PID_GRM,                 I0_UEPS_FID_GRM,        VOS_PRIORITY_NULL)
    ON_DECLARE(UEPS_PID_GHPA_PP,                WUEPS_FID_HPA,          VOS_PRIORITY_NULL)
    ON_DECLARE(UEPS_PID_GHPA_NPP,               WUEPS_FID_HPA,          VOS_PRIORITY_NULL)
    ON_DECLARE(UEPS_PID_WHPA,                   WUEPS_FID_HPA,          VOS_PRIORITY_NULL)
    ON_DECLARE(UEPS_PID_SHPA,                   WUEPS_FID_HPA,          VOS_PRIORITY_NULL)
    ON_DECLARE(I0_WUEPS_PID_MMC,                I0_WUEPS_FID_MM,        VOS_PRIORITY_NULL)
    ON_DECLARE(I0_WUEPS_PID_MM,                 I0_WUEPS_FID_MM,        VOS_PRIORITY_NULL)
    ON_DECLARE(I0_WUEPS_PID_GMM,                I0_WUEPS_FID_MM,        VOS_PRIORITY_NULL)
    ON_DECLARE(I0_WUEPS_PID_MMA,                I0_WUEPS_FID_MM,        VOS_PRIORITY_NULL)
    ON_DECLARE(I0_WUEPS_PID_CC,                 I0_WUEPS_FID_CM,        VOS_PRIORITY_NULL)
    ON_DECLARE(I0_WUEPS_PID_SS,                 I0_WUEPS_FID_CM,        VOS_PRIORITY_NULL)
    ON_DECLARE(I0_WUEPS_PID_SMS,                I0_WUEPS_FID_CM,        VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_SMT,                   WUEPS_FID_CM,           VOS_PRIORITY_NULL)
    ON_DECLARE(I0_WUEPS_PID_TC,                 I0_WUEPS_FID_CM,        VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_CBS,                   WUEPS_FID_CM,           VOS_PRIORITY_NULL)
    ON_DECLARE(I0_WUEPS_PID_RABM,               I0_WUEPS_FID_RABMSM,    VOS_PRIORITY_NULL)
    ON_DECLARE(I0_WUEPS_PID_SM,                 I0_WUEPS_FID_RABMSM,    VOS_PRIORITY_NULL)
    ON_DECLARE(I0_WUEPS_PID_USIM,               I0_MAPS_USIMM_FID,      VOS_PRIORITY_NULL)
    ON_DECLARE(NVIM_WRITE_DATA_PID,             MAPS_USIMM_FID,         VOS_PRIORITY_NULL)
    ON_DECLARE(I0_WUEPS_PID_ADMIN,              I0_WUEPS_FID_TAF,       VOS_PRIORITY_NULL)
    ON_DECLARE(WUEAPP_PID_MUX,                  WUEPS_FID_TAF,          VOS_PRIORITY_NULL)
    ON_DECLARE(I0_WUEPS_PID_TAF,                I0_WUEPS_FID_TAF,       VOS_PRIORITY_NULL)
    ON_DECLARE(I0_WUEPS_PID_VC,                 I0_WUEPS_FID_TAF,       VOS_PRIORITY_NULL)
    ON_DECLARE(I0_WUEPS_PID_DRV_AGENT,          I0_WUEPS_FID_TAF,       VOS_PRIORITY_NULL)
    ON_DECLARE(I0_UEPS_PID_MTA,                 I0_WUEPS_FID_TAF,       VOS_PRIORITY_NULL)
    /*NAS PC工程需要将AT挂在TAF FID*/
    #if (VOS_WIN32 == VOS_OS_VER)
    ON_DECLARE(WUEPS_PID_AT,                    WUEPS_FID_AT,          VOS_PRIORITY_NULL)
    #else
    ON_DECLARE(CCPU_PID_AT,                 WUEPS_FID_TAF,          VOS_PRIORITY_NULL)
    #endif
#if (VOS_WIN32 == VOS_OS_VER)
    ON_DECLARE(ACPU_PID_TAFAGENT,               ACPU_FID_TAFAGENT,      VOS_PRIORITY_NULL)
#endif
    ON_DECLARE(I0_MAPS_STK_PID,                 I0_MAPS_SI_FID,         VOS_PRIORITY_NULL)
    ON_DECLARE(I0_MAPS_PIH_PID,                 I0_MAPS_SI_FID,         VOS_PRIORITY_NULL)
    ON_DECLARE(I0_MAPS_PB_PID,                  I0_MAPS_SI_FID,         VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_OM,                    WUEPS_FID_OM,           VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_AGING,                 WUEPS_FID_OM,           VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_DIAG,                  WUEPS_FID_OM,           VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_RFA,                   WUEPS_FID_OM,           VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_REG,                   WUEPS_FID_OM,           VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_OM_CALLBACK,           WUEPS_FID_OM_CALLBACK,  VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_SPY,                   WUEPS_FID_SPY,          VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_MONITOR,               WUEPS_FID_SPY,          VOS_PRIORITY_NULL)
    ON_DECLARE(MSP_SYS_FTM_PID ,                MSP_FTM_FID,            VOS_PRIORITY_NULL)
    ON_DECLARE(MSP_PID_DIAG_AGENT ,             MSP_FID_DIAG_CCPU,      VOS_PRIORITY_NULL)
    ON_DECLARE(MSP_PID_BBP_AGENT ,              MSP_FID_DIAG_CCPU,      VOS_PRIORITY_NULL)
    ON_DECLARE(MSP_L4_L4A_PID,                  MSP_L4_FID,             VOS_PRIORITY_NULL)
    ON_DECLARE(MSP_L4_RAI_PID,                  MSP_L4_FID,             VOS_PRIORITY_NULL)
    ON_DECLARE(MSP_L4_AT_FW_PID,                MSP_L4_FID,             VOS_PRIORITY_NULL)
    ON_DECLARE(MSP_L4_AT_LTE_PID,               MSP_L4_FID,             VOS_PRIORITY_NULL)
    ON_DECLARE(MSP_L4_AT_GU_PID,                MSP_L4_FID,             VOS_PRIORITY_NULL)
    ON_DECLARE(MSP_PID_HAL_DRA ,                MSP_HAL_FID,            VOS_PRIORITY_NULL)
    ON_DECLARE(CCPU_PID_OMAGENT,                AGENT_FID_OM,           VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_CMMCA,                 WUEPS_FID_CMMCA,        VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_CBPCA,                 WUEPS_FID_CBPCA,        VOS_PRIORITY_NULL)
    ON_DECLARE(I0_WUEPS_PID_CSIMA,              I0_WUEPS_FID_CSIMA,     VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_ERRC   ,                  PS_FID_RRC  ,           VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_ERMM   ,                  PS_FID_RRC  ,           VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_MM     ,                  WUEPS_FID_MM,           VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_ESM    ,                  PS_FID_CM   ,           VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_OM     ,                  PS_FID_OM   ,           VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_RABM   ,                  PS_FID_CM ,             VOS_PRIORITY_NULL)
    /*added by sunbing for VoLTE 20130715 begin*/
    ON_DECLARE(PS_PID_IMSA   ,                  PS_FID_IMSA ,           VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_IMSVA  ,                  PS_FID_IMSVA,           VOS_PRIORITY_NULL)
    /*added by sunbing for VoLTE 20130715 end*/
    ON_DECLARE(PS_PID_MAC_UL ,                  PS_FID_RM_UL,           VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_RLC_UL ,                  PS_FID_RM_UL,           VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_MAC_DL ,                  PS_FID_RM_DL,           VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_RLC_DL ,                  PS_FID_RM_DL,           VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_PDCP_UL,                  PS_FID_RM_UL,           VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_PDCP_DL,                  PS_FID_RM_DL,           VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_HPA    ,                  PS_FID_HPA  ,           VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_MUX    ,                  PS_FID_COMM ,           VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_MUXOM  ,                  PS_FID_COMM ,           VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_TC     ,                  PS_FID_CM ,             VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_IP     ,                  PS_FID_CM ,             VOS_PRIORITY_NULL)
    /* added by hujianbo for PTT 20111201 begin*/
    ON_DECLARE(PS_PID_PTT    ,                  PS_FID_CM ,             VOS_PRIORITY_NULL)
    /* added by hujianbo for PTT 20111201 end */
    ON_DECLARE(UEPS_PID_FLOWCTRL_C,             UEPS_FID_FLOWCTRL_C,    VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_MACDL,                 UEPS_FID_RM_DL,         VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_RLCDL,                 UEPS_FID_RM_DL,         VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_NOSIG,                 WUEPS_FID_NOSIG,        VOS_PRIORITY_NULL)
    ON_DECLARE(UEPS_PID_CST,                    UEPS_FID_PBD,           VOS_PRIORITY_NULL)
#if (FEATURE_ON == FEATURE_UE_MODE_TDS)
/* TDS begin */
    ON_DECLARE(HL1_PID_SNDCMD,                  HL1_FID_SNDCMD,         VOS_PRIORITY_NULL)
    ON_DECLARE(TPS_PID_RLC    ,                 TPS_FID_MAC  ,          VOS_PRIORITY_NULL)
    ON_DECLARE(TPS_PID_MAC  ,                   TPS_FID_MAC ,           VOS_PRIORITY_NULL)
    ON_DECLARE(TPS_PID_PDC     ,                TPS_FID_PDC ,           VOS_PRIORITY_NULL)
/* TDS begin */
    ON_DECLARE(TPS_PID_RRC     ,                PS_FID_RRC ,           VOS_PRIORITY_NULL)
#endif

    /* add for VoLTE start */
    ON_DECLARE(UEPS_PID_IPS_CCORE     ,         UEPS_FID_IPS_CCORE ,    VOS_PRIORITY_NULL)
    /* add for VoLTE end */

#if (VOS_WIN32 == VOS_OS_VER)
    ON_DECLARE(WUEPS_PID_ITTSTUB1,              WUEPS_FID_ITTSTUB,      VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_ITTSTUB2,              WUEPS_FID_ITTSTUB,      VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_WTTFTEST,              WUEPS_FID_ITTSTUB,      VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_OAMTEST,               WUEPS_FID_ITTSTUB,      VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_OM,                     ACPU_FID_OM,            VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_DIAG,                   ACPU_FID_OM,            VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_OMAGENT,                ACPU_FID_OM,            VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_PCVOICE,                ACPU_FID_OM,            VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_PCSC,                   ACPU_FID_OM,            VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_PB,                     ACPU_FID_OM,            VOS_PRIORITY_NULL)
#endif

#ifdef HPA_ITT
    ON_DECLARE(MAPS_STUB_RCVMSG_PID,            MAPS_STUB_FID,          VOS_PRIORITY_NULL)
    ON_DECLARE(MAPS_STUB_SENDMSG_PID,           MAPS_STUB_FID,          VOS_PRIORITY_NULL)
    ON_DECLARE(MAPS_STUB_GTRMSG_PROC_PID,       MAPS_STUB_FID,          VOS_PRIORITY_NULL)
#endif
    ON_DECLARE(UEPS_PID_TTF_MEM_RB_FREE,        UEPS_FID_TTF_MEM_RB_FREE, VOS_PRIORITY_NULL)
    ON_DECLARE(UEPS_PID_CDS,                    UEPS_FID_CDS,           VOS_PRIORITY_NULL)
    ON_DECLARE(UEPS_PID_NDCLIENT,               UEPS_FID_NDCLIENT,      VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_SAR,                   WUEPS_FID_SPY,          VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_NVIM_FLUSH,            WUEPS_FID_NVIM_FLUSH,   VOS_PRIORITY_NULL)
    ON_DECLARE(WUEPS_PID_SLEEP,                 WUEPS_FID_SLEEP,        VOS_PRIORITY_NULL)

#ifdef FEATURE_DSP2ARM
    ON_DECLARE(I0_DSP_PID_APM,                 I0_DSP_FID_APM,          VOS_PRIORITY_NULL)
    ON_DECLARE(DSP_PID_UPA,                    DSP_FID_UPA,             VOS_PRIORITY_NULL)
    ON_DECLARE(DSP_PID_WPHY,                   DSP_FID_WPHY,            VOS_PRIORITY_NULL)
    ON_DECLARE(I0_DSP_PID_GPHY,                I0_DSP_FID_GPHY,         VOS_PRIORITY_NULL)
    ON_DECLARE(I0_DSP_PID_DRX,                 I0_DSP_FID_DRX,          VOS_PRIORITY_NULL)
    ON_DECLARE(I0_DSP_PID_IDLE,                I0_DSP_FID_IDLE,         VOS_PRIORITY_NULL)
    ON_DECLARE(DSP_PID_WMEAS,                  DSP_FID_WMEAS,           VOS_PRIORITY_NULL)
    ON_DECLARE(DSP_PID_RCM,                    DSP_FID_RCM,             VOS_PRIORITY_NULL)
#endif
#if ( FEATURE_MULTI_MODEM == FEATURE_ON )
    ON_DECLARE(I1_WUEPS_PID_USIM,               I1_MAPS_USIMM_FID,      VOS_PRIORITY_NULL)
    ON_DECLARE(I1_MAPS_STK_PID,                 I1_MAPS_SI_FID,         VOS_PRIORITY_NULL)
    ON_DECLARE(I1_MAPS_PIH_PID,                 I1_MAPS_SI_FID,         VOS_PRIORITY_NULL)
    ON_DECLARE(I1_MAPS_PB_PID,                  I1_MAPS_SI_FID,         VOS_PRIORITY_NULL)
    ON_DECLARE(I1_WUEPS_PID_SLEEP,              I1_WUEPS_FID_SLEEP,     VOS_PRIORITY_NULL)
    ON_DECLARE(I1_UEPS_PID_DL,                  I1_UEPS_FID_PBD,        VOS_PRIORITY_NULL)
    ON_DECLARE(I1_UEPS_PID_LL,                  I1_UEPS_FID_SL,         VOS_PRIORITY_NULL)
    ON_DECLARE(I1_UEPS_PID_SN,                  I1_UEPS_FID_SL,         VOS_PRIORITY_NULL)
    ON_DECLARE(I1_UEPS_PID_GRM,                 I1_UEPS_FID_GRM,        VOS_PRIORITY_NULL)
    ON_DECLARE(I1_DSP_PID_GPHY,                 I1_DSP_FID_GPHY,        VOS_PRIORITY_NULL)
    ON_DECLARE(I1_DSP_PID_DRX,                  I1_DSP_FID_DRX,         VOS_PRIORITY_NULL)
    ON_DECLARE(I1_DSP_PID_IDLE,                 I1_DSP_FID_IDLE,        VOS_PRIORITY_NULL)
    ON_DECLARE(I1_DSP_PID_APM,                  I1_DSP_FID_APM,         VOS_PRIORITY_NULL)
    ON_DECLARE(I1_UEPS_PID_GAS,                 I1_UEPS_FID_RR,         VOS_PRIORITY_NULL)
    ON_DECLARE(I1_WUEPS_PID_MM,                 I1_WUEPS_FID_MM,        VOS_PRIORITY_NULL)
    ON_DECLARE(I1_WUEPS_PID_MMC,                I1_WUEPS_FID_MM,        VOS_PRIORITY_NULL)
    ON_DECLARE(I1_WUEPS_PID_GMM,                I1_WUEPS_FID_MM,        VOS_PRIORITY_NULL)
    ON_DECLARE(I1_WUEPS_PID_MMA,                I1_WUEPS_FID_MM,        VOS_PRIORITY_NULL)
    ON_DECLARE(I1_WUEPS_PID_CC,                 I1_WUEPS_FID_CM,        VOS_PRIORITY_NULL)
    ON_DECLARE(I1_WUEPS_PID_SS,                 I1_WUEPS_FID_CM,        VOS_PRIORITY_NULL)
    ON_DECLARE(I1_WUEPS_PID_TC,                 I1_WUEPS_FID_CM,        VOS_PRIORITY_NULL)
    ON_DECLARE(I1_WUEPS_PID_SMS,                I1_WUEPS_FID_CM,        VOS_PRIORITY_NULL)
    ON_DECLARE(I1_WUEPS_PID_RABM,               I1_WUEPS_FID_RABMSM,    VOS_PRIORITY_NULL)
    ON_DECLARE(I1_WUEPS_PID_SM,                 I1_WUEPS_FID_RABMSM,    VOS_PRIORITY_NULL)
    ON_DECLARE(I1_WUEPS_PID_ADMIN,              I1_WUEPS_FID_TAF,       VOS_PRIORITY_NULL)
    ON_DECLARE(I1_WUEPS_PID_TAF,                I1_WUEPS_FID_TAF,       VOS_PRIORITY_NULL)
    ON_DECLARE(I1_WUEPS_PID_VC,                 I1_WUEPS_FID_TAF,       VOS_PRIORITY_NULL)
    ON_DECLARE(I1_UEPS_PID_MTA,                 I1_WUEPS_FID_TAF,       VOS_PRIORITY_NULL)
    ON_DECLARE(I1_WUEPS_PID_DRV_AGENT,          I1_WUEPS_FID_TAF,       VOS_PRIORITY_NULL)
    ON_DECLARE(UEPS_PID_MTC,                    UEPS_FID_MTC,           VOS_PRIORITY_NULL)

    ON_DECLARE(UEPS_PID_RRM,                    UEPS_FID_RRM,           VOS_PRIORITY_NULL)
    ON_DECLARE(I1_WUEPS_PID_CSIMA,              I1_WUEPS_FID_CSIMA,     VOS_PRIORITY_NULL)
#endif
#if (FEATURE_ON == FEATURE_BASTET)
    ON_DECLARE(UEPS_PID_BASTET,                 UEPS_FID_BASTET,        VOS_PRIORITY_NULL)
#endif

    /* Added by x00312156 for TLPHY CHR, 2015-01-28, Begin */
    ON_DECLARE(TLPHY_PID_RTTAGENT,              TLPHY_FID_RTTAGENT,     VOS_PRIORITY_NULL)
    /* Added by x00312156 for TLPHY CHR, 2015-01-28, End */

END_DECLARE_PID_BELONG_TO_FID()

#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
BEGIN_DECLARE_PID_BELONG_TO_FID()
    #if (VOS_WIN32 != VOS_OS_VER)
    ON_DECLARE(WUEPS_PID_AT,                    MSP_APP_DS_FID,         VOS_PRIORITY_NULL)
    #endif
    ON_DECLARE(PS_PID_APP_NDIS,                 PS_FID_APP_NDIS_PPP_DIPC,   VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_APP_PPP,                  PS_FID_APP_NDIS_PPP_DIPC,   VOS_PRIORITY_NULL)

    ON_DECLARE(UEPS_PID_NDSERVER,               PS_FID_APP_NDIS_PPP_DIPC,   VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_APP_DIPC,                 PS_FID_APP_NDIS_PPP_DIPC,   VOS_PRIORITY_NULL)
    ON_DECLARE(PS_PID_APP_MUX,                  PS_FID_APP_NDIS_PPP_DIPC,   VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_OM,                     ACPU_FID_OM,            VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_DIAG,                   ACPU_FID_OM,            VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_OMAGENT,                ACPU_FID_OM,            VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_PCVOICE,                ACPU_FID_OM,            VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_PCSC,                   ACPU_FID_OM,            VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_PB,                     ACPU_FID_OM,            VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_OM_CALLBACK,            ACPU_FID_OM_CALLBACK,   VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_ADS_UL,                 ACPU_FID_ADS_UL,        VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_ADS_DL,                 ACPU_FID_ADS_DL,        VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_CSD,                    ACPU_FID_CSD,           VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_TAFAGENT,               ACPU_FID_TAFAGENT,      VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_RNIC,                   ACPU_FID_RNIC,          VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_NFEXT,                  ACPU_FID_NFEXT,         VOS_PRIORITY_NULL)
    ON_DECLARE(UEPS_PID_FLOWCTRL_A,             UEPS_FID_FLOWCTRL_A,    VOS_PRIORITY_NULL)
    ON_DECLARE(ACPU_PID_CPULOAD,                ACPU_FID_CPULOAD,       VOS_PRIORITY_NULL)
    ON_DECLARE(UEPS_PID_IMM_RB_FREE,            UEPS_FID_IMM_RB_FREE,   VOS_PRIORITY_NULL)
#if (FEATURE_VOICE_UP == FEATURE_ON)
    /* Voice in ACPU start */
    ON_DECLARE(DSP_PID_VOICE_RT,                ACPU_FID_MED_RT,        VOS_PRIORITY_NULL)
    ON_DECLARE(DSP_PID_VOICE,                   ACPU_FID_MED,           VOS_PRIORITY_NULL)
    /* Voice in ACPU end */
#endif
#if (RAT_MODE != RAT_GU)
    ON_DECLARE(MSP_PID_DIAG_FW,                 MSP_FID_DIAG_ACPU,      VOS_PRIORITY_NULL)
    ON_DECLARE(MSP_PID_DIAG_APP_AGENT,          MSP_FID_DIAG_ACPU,      VOS_PRIORITY_NULL)
#endif

#if (FEATURE_ON == FEATURE_BASTET)
    ON_DECLARE(ACPU_PID_BASTET_COMM,            ACPU_FID_BASTET_COMM,   VOS_PRIORITY_NULL)
#endif

END_DECLARE_PID_BELONG_TO_FID()
#endif

#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/* _V_ID_INC */
