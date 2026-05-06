# -----------------------------------------------------------------------------
#.SILENT:
VERIF = ${WORKINGDIR}/verif

SRC :=  test_case.c \


DEFINEFLAGS +=  EBUG_RW_MSG \

# This needs to be changed depening on the path to make.include.tests.vip_cortex_A9
include $(VERIF)/C_ENV/make.include.c_env
clean:
	\rm -rf *.bin *.o *.map *.lst image.htm *.out *.mem *.axf *.so objects *.rom *.elf *.hex 
