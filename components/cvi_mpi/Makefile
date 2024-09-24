.PHONY: all clean clean_all
SUB_DIRS := cvi_link cvi_msg 
SUB_DIRS += cvi_mw_audio cvi_mw_bin cvi_mw_gdc cvi_mw_isp cvi_mw_isp_ae cvi_mw_isp_af cvi_mw_isp_algo cvi_mw_isp_awb \
			cvi_mw_isp_bin cvi_mw_ldc cvi_mw_mipi_tx cvi_mw_rgn cvi_mw_sys cvi_mw_vdec cvi_mw_venc cvi_mw_vi cvi_mw_vo cvi_mw_vpss
SUB_DIRS += cvi_osdrv_base cvi_osdrv_cif cvi_osdrv_ipcm cvi_osdrv_ldc cvi_osdrv_rgn cvi_osdrv_sys cvi_osdrv_vcodec \
			cvi_osdrv_vi cvi_osdrv_vo cvi_osdrv_vpss
 
all:
	@echo "########################################################################################################"
	@echo "#"
	@echo "#  Compiling 'module libs' Configs as below ..."
	@echo "#	"CROSS_COMPILE=$(CROSS_COMPILE)
	@echo "#"
	@echo "########################################################################################################"
	@$(foreach dir,$(SUB_DIRS), \
		if test -d $(dir) ; then \
			cd $(dir);$(MAKE) all;cd -; \
		else \
			exit 1; \
		fi;)
 
clean:
	@$(foreach dir,$(SUB_DIRS), \
		if test -d $(dir) ; then \
			cd $(dir);$(MAKE) clean;cd -; \
		else \
			exit 1; \
		fi;)
 
clean_all:
	@$(foreach dir,$(SUB_DIRS), \
		if test -d $(dir) ; then \
			cd $(dir);$(MAKE) clean_all;cd -; \
		else \
			exit 1; \
		fi;)