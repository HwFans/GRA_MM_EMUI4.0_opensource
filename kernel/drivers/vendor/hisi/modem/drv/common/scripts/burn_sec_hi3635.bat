fastboot flash xloader            sec_xloader.img
fastboot flash ptable             ptable.img
fastboot flash fastboot           sec_fastboot.img
fastboot -i 0x12d1 reboot-bootloader
ping -n 10 127.0.0.1>nul
fastboot flash dtimage            dt.img
fastboot flash mcuimage           sec_lpm3.img
fastboot flash sensorhub          sec_sensorhub_asic.img
fastboot flash hifi               sec_hifi_k3v3.img
fastboot flash teeos              sec_trustedcore.img
fastboot flash recovery           sec_recovery.img
fastboot flash cache              cache.img
fastboot flash boot               sec_boot.img
fastboot flash nvme               nvme.img
fastboot flash trustfirmware      sec_bl31.bin
fastboot flash modem              sec_balong_modem.bin 
fastboot flash modemnvm_update    nv.bin
fastboot flash modem_dsp          lphy.bin
fastboot flash userdata           userdata.img
fastboot flash system             system.img
::第一次单板升级或者分区表修改，需打开并烧写以下两个镜像
::fastboot flash modem_om               modem_fs.img
::fastboot flash modemnvm_img               modem_nv.img
pause
