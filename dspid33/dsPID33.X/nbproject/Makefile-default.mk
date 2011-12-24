#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
include Makefile

# Environment
# Adding MPLAB X bin directory to path
PATH:=/Applications/microchip/mplabx/mplab_ide.app/Contents/Resources/mplab_ide/mplab_ide/modules/../../bin/:$(PATH)
MKDIR=mkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/dsPID33.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/dsPID33.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/1360937237/traps.o ${OBJECTDIR}/_ext/1360937237/dsPID33.o ${OBJECTDIR}/_ext/1360937237/dsPid33_settings.o "${OBJECTDIR}/_ext/1360937237/DEE Emulation 16-bit.o" "${OBJECTDIR}/_ext/1360937237/Flash Operations.o" ${OBJECTDIR}/_ext/1360937237/com.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/1360937237/traps.o.d ${OBJECTDIR}/_ext/1360937237/dsPID33.o.d ${OBJECTDIR}/_ext/1360937237/dsPid33_settings.o.d "${OBJECTDIR}/_ext/1360937237/DEE Emulation 16-bit.o.d" "${OBJECTDIR}/_ext/1360937237/Flash Operations.o.d" ${OBJECTDIR}/_ext/1360937237/com.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/1360937237/traps.o ${OBJECTDIR}/_ext/1360937237/dsPID33.o ${OBJECTDIR}/_ext/1360937237/dsPid33_settings.o ${OBJECTDIR}/_ext/1360937237/DEE\ Emulation\ 16-bit.o ${OBJECTDIR}/_ext/1360937237/Flash\ Operations.o ${OBJECTDIR}/_ext/1360937237/com.o


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

# Path to java used to run MPLAB X when this makefile was created
MP_JAVA_PATH="/System/Library/Java/JavaVirtualMachines/1.6.0.jdk/Contents/Home/bin/"
OS_CURRENT="$(shell uname -s)"
############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
MP_CC="/Applications/microchip/mplabc30/v3.30c/bin/pic30-gcc"
# MP_BC is not defined
MP_AS="/Applications/microchip/mplabc30/v3.30c/bin/pic30-as"
MP_LD="/Applications/microchip/mplabc30/v3.30c/bin/pic30-ld"
MP_AR="/Applications/microchip/mplabc30/v3.30c/bin/pic30-ar"
DEP_GEN=${MP_JAVA_PATH}java -jar "/Applications/microchip/mplabx/mplab_ide.app/Contents/Resources/mplab_ide/mplab_ide/modules/../../bin/extractobjectdependencies.jar" 
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps
MP_CC_DIR="/Applications/microchip/mplabc30/v3.30c/bin"
# MP_BC_DIR is not defined
MP_AS_DIR="/Applications/microchip/mplabc30/v3.30c/bin"
MP_LD_DIR="/Applications/microchip/mplabc30/v3.30c/bin"
MP_AR_DIR="/Applications/microchip/mplabc30/v3.30c/bin"
# MP_BC_DIR is not defined

.build-conf:  ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/dsPID33.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33FJ128MC802
MP_LINKER_FILE_OPTION=,-Tp33FJ128MC802.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1360937237/Flash\ Operations.o: ../src/Flash\ Operations.s  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1360937237 
	@${RM} ${OBJECTDIR}/_ext/1360937237/Flash\ Operations.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/Flash\ Operations.o.ok ${OBJECTDIR}/_ext/1360937237/Flash\ Operations.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1360937237/Flash Operations.o.d" $(SILENT) -c ${MP_AS} $(MP_EXTRA_AS_PRE)  "../src/Flash Operations.s" -o "${OBJECTDIR}/_ext/1360937237/Flash Operations.o" -omf=elf -p=$(MP_PROCESSOR_OPTION) --defsym=__MPLAB_BUILD=1 --defsym=__MPLAB_DEBUG=1 --defsym=__ICD2RAM=1 --defsym=__DEBUG=1 --defsym=__MPLAB_DEBUGGER_ICD3=1 -g  -MD "${OBJECTDIR}/_ext/1360937237/Flash Operations.o.d" -I"../../inc" --keep-locals -ahl="$(BINDIR_)$(INFILEBASE).lst" -g $(MP_EXTRA_AS_POST)
	
else
${OBJECTDIR}/_ext/1360937237/Flash\ Operations.o: ../src/Flash\ Operations.s  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1360937237 
	@${RM} ${OBJECTDIR}/_ext/1360937237/Flash\ Operations.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/Flash\ Operations.o.ok ${OBJECTDIR}/_ext/1360937237/Flash\ Operations.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1360937237/Flash Operations.o.d" $(SILENT) -c ${MP_AS} $(MP_EXTRA_AS_PRE)  "../src/Flash Operations.s" -o "${OBJECTDIR}/_ext/1360937237/Flash Operations.o" -omf=elf -p=$(MP_PROCESSOR_OPTION) --defsym=__MPLAB_BUILD=1 -g  -MD "${OBJECTDIR}/_ext/1360937237/Flash Operations.o.d" -I"../../inc" --keep-locals -ahl="$(BINDIR_)$(INFILEBASE).lst" -g $(MP_EXTRA_AS_POST)
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/1360937237/traps.o: ../src/traps.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1360937237 
	@${RM} ${OBJECTDIR}/_ext/1360937237/traps.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/traps.o.ok ${OBJECTDIR}/_ext/1360937237/traps.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1360937237/traps.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -fno-short-double -Wall -I"../C:/Programmi/Microchip/MPLAB C30/include" -I"../src" -mlarge-data -mlarge-scalar -MMD -MF "${OBJECTDIR}/_ext/1360937237/traps.o.d" -o ${OBJECTDIR}/_ext/1360937237/traps.o ../src/traps.c  -fast-math -legacy-libc
	
${OBJECTDIR}/_ext/1360937237/dsPID33.o: ../src/dsPID33.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1360937237 
	@${RM} ${OBJECTDIR}/_ext/1360937237/dsPID33.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/dsPID33.o.ok ${OBJECTDIR}/_ext/1360937237/dsPID33.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1360937237/dsPID33.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -fno-short-double -Wall -I"../C:/Programmi/Microchip/MPLAB C30/include" -I"../src" -mlarge-data -mlarge-scalar -MMD -MF "${OBJECTDIR}/_ext/1360937237/dsPID33.o.d" -o ${OBJECTDIR}/_ext/1360937237/dsPID33.o ../src/dsPID33.c  -fast-math -legacy-libc
	
${OBJECTDIR}/_ext/1360937237/dsPid33_settings.o: ../src/dsPid33_settings.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1360937237 
	@${RM} ${OBJECTDIR}/_ext/1360937237/dsPid33_settings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/dsPid33_settings.o.ok ${OBJECTDIR}/_ext/1360937237/dsPid33_settings.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1360937237/dsPid33_settings.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -fno-short-double -Wall -I"../C:/Programmi/Microchip/MPLAB C30/include" -I"../src" -mlarge-data -mlarge-scalar -MMD -MF "${OBJECTDIR}/_ext/1360937237/dsPid33_settings.o.d" -o ${OBJECTDIR}/_ext/1360937237/dsPid33_settings.o ../src/dsPid33_settings.c  -fast-math -legacy-libc
	
${OBJECTDIR}/_ext/1360937237/DEE\ Emulation\ 16-bit.o: ../src/DEE\ Emulation\ 16-bit.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1360937237 
	@${RM} ${OBJECTDIR}/_ext/1360937237/DEE\ Emulation\ 16-bit.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/DEE\ Emulation\ 16-bit.o.ok ${OBJECTDIR}/_ext/1360937237/DEE\ Emulation\ 16-bit.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1360937237/DEE Emulation 16-bit.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -fno-short-double -Wall -I"../C:/Programmi/Microchip/MPLAB C30/include" -I"../src" -mlarge-data -mlarge-scalar -MMD -MF "${OBJECTDIR}/_ext/1360937237/DEE Emulation 16-bit.o.d" -o "${OBJECTDIR}/_ext/1360937237/DEE Emulation 16-bit.o" "../src/DEE Emulation 16-bit.c"  -fast-math -legacy-libc
	
${OBJECTDIR}/_ext/1360937237/com.o: ../src/com.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1360937237 
	@${RM} ${OBJECTDIR}/_ext/1360937237/com.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/com.o.ok ${OBJECTDIR}/_ext/1360937237/com.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1360937237/com.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -fno-short-double -Wall -I"../C:/Programmi/Microchip/MPLAB C30/include" -I"../src" -mlarge-data -mlarge-scalar -MMD -MF "${OBJECTDIR}/_ext/1360937237/com.o.d" -o ${OBJECTDIR}/_ext/1360937237/com.o ../src/com.c  -fast-math -legacy-libc
	
else
${OBJECTDIR}/_ext/1360937237/traps.o: ../src/traps.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1360937237 
	@${RM} ${OBJECTDIR}/_ext/1360937237/traps.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/traps.o.ok ${OBJECTDIR}/_ext/1360937237/traps.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1360937237/traps.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -fno-short-double -Wall -I"../C:/Programmi/Microchip/MPLAB C30/include" -I"../src" -mlarge-data -mlarge-scalar -MMD -MF "${OBJECTDIR}/_ext/1360937237/traps.o.d" -o ${OBJECTDIR}/_ext/1360937237/traps.o ../src/traps.c  -fast-math -legacy-libc
	
${OBJECTDIR}/_ext/1360937237/dsPID33.o: ../src/dsPID33.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1360937237 
	@${RM} ${OBJECTDIR}/_ext/1360937237/dsPID33.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/dsPID33.o.ok ${OBJECTDIR}/_ext/1360937237/dsPID33.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1360937237/dsPID33.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -fno-short-double -Wall -I"../C:/Programmi/Microchip/MPLAB C30/include" -I"../src" -mlarge-data -mlarge-scalar -MMD -MF "${OBJECTDIR}/_ext/1360937237/dsPID33.o.d" -o ${OBJECTDIR}/_ext/1360937237/dsPID33.o ../src/dsPID33.c  -fast-math -legacy-libc
	
${OBJECTDIR}/_ext/1360937237/dsPid33_settings.o: ../src/dsPid33_settings.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1360937237 
	@${RM} ${OBJECTDIR}/_ext/1360937237/dsPid33_settings.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/dsPid33_settings.o.ok ${OBJECTDIR}/_ext/1360937237/dsPid33_settings.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1360937237/dsPid33_settings.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -fno-short-double -Wall -I"../C:/Programmi/Microchip/MPLAB C30/include" -I"../src" -mlarge-data -mlarge-scalar -MMD -MF "${OBJECTDIR}/_ext/1360937237/dsPid33_settings.o.d" -o ${OBJECTDIR}/_ext/1360937237/dsPid33_settings.o ../src/dsPid33_settings.c  -fast-math -legacy-libc
	
${OBJECTDIR}/_ext/1360937237/DEE\ Emulation\ 16-bit.o: ../src/DEE\ Emulation\ 16-bit.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1360937237 
	@${RM} ${OBJECTDIR}/_ext/1360937237/DEE\ Emulation\ 16-bit.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/DEE\ Emulation\ 16-bit.o.ok ${OBJECTDIR}/_ext/1360937237/DEE\ Emulation\ 16-bit.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1360937237/DEE Emulation 16-bit.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -fno-short-double -Wall -I"../C:/Programmi/Microchip/MPLAB C30/include" -I"../src" -mlarge-data -mlarge-scalar -MMD -MF "${OBJECTDIR}/_ext/1360937237/DEE Emulation 16-bit.o.d" -o "${OBJECTDIR}/_ext/1360937237/DEE Emulation 16-bit.o" "../src/DEE Emulation 16-bit.c"  -fast-math -legacy-libc
	
${OBJECTDIR}/_ext/1360937237/com.o: ../src/com.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1360937237 
	@${RM} ${OBJECTDIR}/_ext/1360937237/com.o.d 
	@${RM} ${OBJECTDIR}/_ext/1360937237/com.o.ok ${OBJECTDIR}/_ext/1360937237/com.o.err 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1360937237/com.o.d" $(SILENT) -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -fno-short-double -Wall -I"../C:/Programmi/Microchip/MPLAB C30/include" -I"../src" -mlarge-data -mlarge-scalar -MMD -MF "${OBJECTDIR}/_ext/1360937237/com.o.d" -o ${OBJECTDIR}/_ext/1360937237/com.o ../src/com.c  -fast-math -legacy-libc
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/dsPID33.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -omf=elf -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -o dist/${CND_CONF}/${IMAGE_TYPE}/dsPID33.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}   /Applications/microchip/mplabc30/v3.30c/lib/libdsp-elf.a /Applications/microchip/mplabc30/v3.30c/lib/libq-elf.a /Applications/microchip/mplabc30/v3.30c/lib/libfastm-elf.a    -fast-math -legacy-libc -Wl,--defsym=__MPLAB_BUILD=1,--heap=512,-L"../C:/Programmi/Microchip/MPLAB C30/lib",-Map="${DISTDIR}/dsPID33.X.${IMAGE_TYPE}.map",--report-mem,--cref$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__ICD2RAM=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_ICD3=1
else
dist/${CND_CONF}/${IMAGE_TYPE}/dsPID33.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -omf=elf -mcpu=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/dsPID33.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}   /Applications/microchip/mplabc30/v3.30c/lib/libdsp-elf.a /Applications/microchip/mplabc30/v3.30c/lib/libq-elf.a /Applications/microchip/mplabc30/v3.30c/lib/libfastm-elf.a    -fast-math -legacy-libc -Wl,--defsym=__MPLAB_BUILD=1,--heap=512,-L"../C:/Programmi/Microchip/MPLAB C30/lib",-Map="${DISTDIR}/dsPID33.X.${IMAGE_TYPE}.map",--report-mem,--cref$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION)
	${MP_CC_DIR}/pic30-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/dsPID33.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -omf=elf
endif


# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell "/Applications/microchip/mplabx/mplab_ide.app/Contents/Resources/mplab_ide/mplab_ide/modules/../../bin/"mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
