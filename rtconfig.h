
#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__
#include "user.h"

/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Project Configuration */

/* RT-Thread Kernel */

#define RT_NAME_MAX 8
#define RT_ALIGN_SIZE 4
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 100
#define RT_USING_OVERFLOW_CHECK
#define RT_USING_HOOK
#define RT_USING_IDLE_HOOK
#define RT_IDLE_HOOK_LIST_SIZE 4
#define IDLE_THREAD_STACK_SIZE 256
#define RT_USING_TIMER_SOFT
#define RT_TIMER_THREAD_PRIO 4
#define RT_TIMER_THREAD_STACK_SIZE 512

/* kservice optimization */

#define RT_DEBUG

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_SMALL_MEM
#define RT_USING_MEMTRACE
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 256
#define RT_CONSOLE_DEVICE_NAME "uart1"
#define RT_VER_NUM 0x40004
#define ARCH_ARM
#define RT_USING_CPU_FFS
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M4

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 2048
#define RT_MAIN_THREAD_PRIORITY 10

/* C++ features */

#define RT_USING_CPLUSPLUS

/* Command shell */

#define RT_USING_FINSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 4096
#define FINSH_CMD_SIZE 80
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT
#define FINSH_ARG_MAX 10

/* Device virtual file system */

#define RT_USING_DFS
#define DFS_USING_WORKDIR
#define DFS_FILESYSTEMS_MAX 4
#define DFS_FILESYSTEM_TYPES_MAX 4
#define DFS_FD_MAX 16
#define RT_USING_DFS_DEVFS

/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_PIPE_BUFSZ 512
#define RT_USING_SYSTEM_WORKQUEUE
#define RT_SYSTEM_WORKQUEUE_STACKSIZE 2048
#define RT_SYSTEM_WORKQUEUE_PRIORITY 23
#define RT_USING_SERIAL
#define RT_USING_SERIAL_V1
#define RT_SERIAL_USING_DMA
#define RT_SERIAL_RB_BUFSZ 512
#define RT_USING_I2C
#define RT_USING_I2C_BITOPS
#define RT_USING_PIN
#define RT_USING_RTC

/* Using USB */


/* POSIX layer and C standard library */

#define RT_USING_LIBC
#define RT_USING_POSIX
#define RT_LIBC_FIXED_TIMEZONE 8

/* Network */

/* Socket abstraction layer */

#define RT_USING_SAL

/* protocol stack implement */

#define SAL_USING_AT
#define SAL_USING_POSIX

/* Network interface device */

#define RT_USING_NETDEV
#define NETDEV_USING_IFCONFIG
#define NETDEV_USING_NETSTAT
#define NETDEV_USING_AUTO_DEFAULT
#define NETDEV_IPV4 1
#define NETDEV_IPV6 0

/* light weight TCP/IP stack */


/* AT commands */

#define RT_USING_AT
#define AT_DEBUG
#define AT_USING_CLIENT
#define AT_CLIENT_NUM_MAX 1
#define AT_USING_SOCKET
#define AT_USING_CLI
#define AT_PRINT_RAW_CMD
#define AT_CMD_MAX_LEN 1024
#define AT_SW_VERSION_NUM 0x10301

/* VBUS(Virtual Software BUS) */


/* Utilities */


/* RT-Thread online packages */

/* IoT - internet of things */


/* Wi-Fi */

/* Marvell WiFi */


/* Wiced WiFi */

#define PKG_USING_AT_DEVICE
#ifdef AIR720
#define AT_DEVICE_USING_AIR720
#define AT_DEVICE_AIR720_SAMPLE
#define AIR720_SAMPLE_POWER_PIN -1
#define AIR720_SAMPLE_STATUS_PIN -1
#define AIR720_SAMPLE_CLIENT_NAME "uart2"
#define AIR720_SAMPLE_RECV_BUFF_LEN 512
#define AT_DEVICE_AIR720_INIT_ASYN
#endif

#ifdef EC200X
#define AT_DEVICE_USING_EC200X
#define AT_DEVICE_EC200X_SAMPLE
#define EC200X_SAMPLE_POWER_PIN -1
#define EC200X_SAMPLE_STATUS_PIN -1
#define EC200X_SAMPLE_WAKEUP_PIN -1
#define EC200X_SAMPLE_CLIENT_NAME "uart2"
#define EC200X_SAMPLE_RECV_BUFF_LEN 512
#define AT_DEVICE_EC200X_INIT_ASYN
#endif



#define PKG_USING_AT_DEVICE_V204
#define PKG_AT_DEVICE_VER_NUM 0x20004


/* IoT Cloud */


/* security packages */


/* language packages */


/* multimedia packages */


/* tools packages */


/* system packages */

/* acceleration: Assembly language or algorithmic acceleration packages */


/* Micrium: Micrium software products porting for RT-Thread */

#define PKG_USING_RT_KPRINTF_THREADSAFE
#define PKG_USING_RT_KPRINTF_THREADSAFE_LATEST_VERSION

/* peripheral libraries and drivers */

// #define PKG_USING_U8G2

/* U8G2 Examples */

// #define PKG_USING_U8G2_V300

/* C     version: v1.x.x */

/* C++   version: v2.x.x */

/* C/C++ version: v3.x.x */

// #define PKG_U8G2_VER_NUM 0x30000
#define PKG_USING_AT24CXX
#define PKG_USING_AT24CXX_LATEST_VERSION

/* AI packages */


/* miscellaneous packages */


/* samples: kernel and components samples */


/* entertainment: terminal games and other interesting software packages */


/* Hardware Drivers Config */

#define MCU_HC32F460

/* Onboard Peripheral Drivers */

/* On-chip Peripheral Drivers */

#define BSP_USING_GPIO
#define BSP_USING_UART
#define BSP_USING_UART1
#define BSP_UART1_RX_USING_DMA
#define BSP_UART1_TX_USING_DMA
#define BSP_UART1_TX_BUFSIZE 512
#define BSP_UART1_RX_BUFSIZE 512
#define BSP_USING_UART2
#define BSP_UART2_RX_USING_DMA
#define BSP_UART2_TX_USING_DMA
#define BSP_UART2_TX_BUFSIZE 512
#define BSP_UART2_RX_BUFSIZE 512

/* Board extended module Drivers */


#endif
