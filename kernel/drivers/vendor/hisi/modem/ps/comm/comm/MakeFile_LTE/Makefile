# Include COMM_CODE
include $(SRCTOP)/PS_CODE/COMM_CODE/MakeFile_LTE/AutoMake/make_COMM_CODE_LTE.include


# Include PS
include $(SRCTOP)/PS_CODE/COMM_CODE/MakeFile_LTE/AutoMake/make_PS_CODE_LTE.include

# Spawn module file
obj-m  := LPS.o

EXTRA_CFLAGS += $(INCLUDE_FLAG) $(PLAT_FORM_FLAG) -fsigned-char -DVOS_LINUX=4 -DOSA_CPU_ACPU=1 -DVOS_OS_VER=VOS_LINUX -DVOS_OSA_CPU=OSA_CPU_ACPU -D__LOG_RELEASE__ -DBSP_CORE_APP -DGU_MODE -DMSP_SUPPORT_USB_VCOM1 -DMSP_SUPPORT_USB_VCOM2 -DVOS_DOPRA_VER=2 -DNDIS_RELEASE -DBALONG_PHY_V200=2 -DBALONG_PHY_V300=3 -DBALONG_PHY_VER=BALONG_PHY_V300 -DBALONG_CHIP_V200=2 -DBALONG_CHIP_V300=3 -DBALONG_CHIP_V500=4 -DBALONG_CHIP_VER=BALONG_CHIP_V500 -DVERSION_V7R1_C010 -DMSP_GUNAS_AT_UNITE -DVERSION_V3R2

all:
	$(MAKE) -C $(LINUXROOT) M=$(SRCTOP)/PS_CODE/COMM_CODE/MakeFile_LTE PRODUCT_CFG_BOARD_TYPE=V7R1_ASIC PRODUCT_CFG_ASIC_TYPE=V7R1 modules V=$(DISVERBOSE)

clean:
	rm -f *.o
	rm -f *.ko
	rm -f *.mod.c
	rm -f .*.cmd 
	
	$(shell if [ "$(PSCMDOBJ)" ]; then \
		echo rm -f $(PSCMDOBJ); \
	fi;)	
	$(shell if [ "$(PSOBJ)" ]; then \
		echo rm -f $(PSOBJ); \
	fi;)
	
