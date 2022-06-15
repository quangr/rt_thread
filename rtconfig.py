import os

# toolchains options
ARCH='arm'
CPU='cortex-m4'
CROSS_TOOL='iar'

print "############rtconfig##############"

if os.getenv('RTT_CC'):
    CROSS_TOOL = os.getenv('RTT_CC')
if os.getenv('Renode'):
    RENODE = True
else:
    RENODE = False


print "CROSS_TOOL: " + CROSS_TOOL

# cross_tool provides the cross compiler
# EXEC_PATH is the compiler execute path, for example, CodeSourcery, Keil MDK, IAR
if  CROSS_TOOL == 'gcc':
    PLATFORM    = 'gcc'
    EXEC_PATH   = r'C:\\Program Files (x86)\\GNU Arm Embedded Toolchain\\10 2020-q4-major\\bin'
elif CROSS_TOOL == 'keil':
    PLATFORM    = 'armcc'
    EXEC_PATH   = r'C:\Keil_v5'
elif CROSS_TOOL == 'iar':
    PLATFORM    = 'iar'
    EXEC_PATH   = r'D:\03_software\Program Files\IAR Systems\Embedded Workbench 7.5'

if os.getenv('RTT_EXEC_PATH'):
    EXEC_PATH = os.getenv('RTT_EXEC_PATH')

BUILD = 'debug'
MCU_TYPE = 'HC32F460'

if PLATFORM == 'gcc':
    # toolchains
    PREFIX = 'arm-none-eabi-'
    CC = PREFIX + 'gcc'
    CXX = PREFIX + 'g++'
    AS = PREFIX + 'gcc'
    AR = PREFIX + 'ar'
    LINK = PREFIX + 'gcc'
    TARGET_EXT = 'elf'
    SIZE = PREFIX + 'size'
    OBJDUMP = PREFIX + 'objdump'
    OBJCPY = PREFIX + 'objcopy'

    DEVICE = '  -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections'
    # CFLAGS = DEVICE + ' -g -Wall -D HC32F460 -D __DEBUG -D USE_DDL_DRIVER -D __ASSEMBLY__ -D ARM_MATH_LOOPUNROLL -D ARM_MATH_DSP -D USE_DEVICE_DRIVER_LIB -D __RTTHREAD__ -D __CLK_TCK=RT_TICK_PER_SECOND'
    CFLAGS = DEVICE + ' -g -Wall -D HC32F460 -D __DEBUG -D USE_DDL_DRIVER -D __ASSEMBLY__ -D ARM_MATH_LOOPUNROLL -D ARM_MATH_DSP -D USE_DEVICE_DRIVER_LIB -D __RTTHREAD__ -D __CLK_TCK=RT_TICK_PER_SECOND'+(" -D RENODE" if RENODE else "")
    CXXFLAGS = DEVICE + ' -g -Wall'
    AFLAGS = ' -c' + DEVICE + ' -x assembler-with-cpp -Wa,-mimplicit-it=thumb '
    LFLAGS = DEVICE + ' -lm -lgcc -lc' + ' -nostartfiles -Wl,--gc-sections,-u,Reset_Handler,-Map=rtthread.map -T board/link.lds'

    CPATH = ''
    LPATH = ''

    if BUILD == 'debug':
        CFLAGS += ' -O0 -gdwarf-2'
        AFLAGS += ' -gdwarf-2'
    else:
        CFLAGS += ' -O2'

    POST_ACTION = OBJCPY + ' -O ihex $TARGET rtthread.hex\n' + SIZE + ' $TARGET \n'

elif PLATFORM == 'armcc':
    # toolchains
    CC = 'armcc'
    AS = 'armasm'
    AR = 'armar'
    LINK = 'armlink'
    TARGET_EXT = 'axf'

    DEVICE = ' --cpu=cortex-m4.fp'
    CFLAGS = DEVICE + ' --apcs=interwork -DUSE_DDL_DRIVER -DHC32F4A0 -D__DEBUG'
    AFLAGS = DEVICE
    LFLAGS = DEVICE + ' --info sizes --info totals --info unused --info veneers --list rtthread.map --scatter "board/linker_scripts/link.sct"'

    CFLAGS += ' -I' + EXEC_PATH + '/ARM/RV31/INC'
    LFLAGS += ' --libpath ' + EXEC_PATH + '/ARM/RV31/LIB'

    EXEC_PATH += '/arm/bin40/'

    if BUILD == 'debug':
        CFLAGS += ' -g -O0'
        AFLAGS += ' -g'
    else:
        CFLAGS += ' -O2'

    POST_ACTION = 'fromelf --bin $TARGET --output rtthread.bin \nfromelf -z $TARGET'

elif PLATFORM == 'iar':
    # toolchains
    CC = 'iccarm'
    AS = 'iasmarm'
    AR = 'iarchive'
    LINK = 'ilinkarm'
    TARGET_EXT = 'out'

    DEVICE = ' -D __DEBUG'  + ' -D USE_DDL_DRIVER' + ' -D HC32F4A0'

    CFLAGS = DEVICE
    CFLAGS += ' --diag_suppress Pa050'
    CFLAGS += ' --no_cse' 
    CFLAGS += ' --no_unroll'
    CFLAGS += ' --no_inline'
    CFLAGS += ' --no_code_motion'
    CFLAGS += ' --no_tbaa' 
    CFLAGS += ' --no_clustering'
    CFLAGS += ' --no_scheduling'
    CFLAGS += ' --endian=little'
    CFLAGS += ' --cpu=Cortex-M4'
    CFLAGS += ' -e' 
    CFLAGS += ' --fpu=None'
    CFLAGS += ' --dlib_config "' + EXEC_PATH + '/arm/INC/c/DLib_Config_Normal.h"'
    CFLAGS += ' -Ol'
    CFLAGS += ' --use_c++_inline'
        
    AFLAGS = ''
    AFLAGS += ' -s+'
    AFLAGS += ' -w+'
    AFLAGS += ' -r'
    AFLAGS += ' --cpu Cortex-M4'
    AFLAGS += ' --fpu None'
    if BUILD == 'debug':
        CFLAGS += ' --debug'
        CFLAGS += ' -On'
    else:
        CFLAGS += ' -Oh'

    LFLAGS = ' --config "board/linker_scripts/link.icf"'
    LFLAGS += ' --redirect _Printf=_PrintfTiny'
    LFLAGS += ' --redirect _Scanf=_ScanfSmall'
    LFLAGS += ' --entry __iar_program_start'

    EXEC_PATH = EXEC_PATH + '/arm/bin/'
    POST_ACTION = ''
