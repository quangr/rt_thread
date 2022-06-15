/**
 * NOTE:
 *
 * HAL_TCP_xxx API reference implementation: wrappers/os/ubuntu/HAL_TCP_linux.c
 *
 */
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"
#include "wrappers_defs.h"
#include "at_wrapper.h"
#include "stdarg.h"
#include "at_mqtt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <at_device.h>
#include <at_api.h>
#include <at_log.h>

static void urc_close_func(struct at_client *client, const char *data, rt_size_t size);
static void urc_send_func(struct at_client *client, const char *data, rt_size_t size);
static void urc_recv_func(struct at_client *client, const char *data, rt_size_t size);

void air720_h_reset(void);

static char log_buf[RT_CONSOLEBUF_SIZE];
static void urc_connect_func(struct at_client *client, const char *data, rt_size_t size);
iotx_mc_state_t mqtt_state = IOTX_MC_STATE_INVALID;
uint16_t port = -1;				/* Specify MQTT broker port */
char *host = RT_NULL;			/* Specify MQTT broker host */
char *client_id = RT_NULL;		/* Specify MQTT connection client id*/
char *username = RT_NULL;		/* Specify MQTT user name */
char *password = RT_NULL;		/* Specify MQTT password */
char *customize_info = RT_NULL; /* Specify User custom information */
/* Specify MQTT transport channel and key.
 * If the value is NULL, it means that use TCP channel,
 * If the value is NOT NULL, it means that use SSL/TLS channel and
 *   @pub_key point to the CA certification */
char *pub_key = RT_NULL;
uint8_t clean_session = -1;			 /* Specify MQTT clean session or not*/
uint32_t request_timeout_ms = -1;	 /* Specify timeout of a MQTT request in millisecond */
uint32_t keepalive_interval_ms = -1; /* Specify MQTT keep-alive interval in millisecond */
uint32_t write_buf_size = -1;		 /* Specify size of write-buffer in byte */
uint32_t read_buf_size = -1;		 /* Specify size of read-buffer in byte */
rt_mutex_t lock = RT_NULL, reclock = RT_NULL;
static void reconnect_entry(void *parameter);

#define productKey "gdimTP7qv4n"
#define productSecret "lCdNWaUdp0c0AMK8"

rt_event_t mqtt_event = RT_NULL;
static char *_product_key = productKey;
static char *_product_secret = productSecret;
static char *_device_name = deviceName;
static char *_device_secret = deviceSecret;
static char *_version = "1.1";

const char *subtopic = "/sys/" productKey "/" deviceName "/rrpc/request/+";

int needcheck = 0;
uint32_t checktime = 2000 * 24 * 60 * 2-100;

static int mqtt_event_send(rt_event_t rtevent, uint32_t event)
{
	return (int)rt_event_send(rtevent, event);
}
int statusc = 0;
int errorn = 0, last_err = 0;

static int mqtt_event_recv(rt_event_t rtevent, uint32_t event, uint32_t timeout, rt_uint8_t option)
{
	int result = RT_EOK;
	rt_uint32_t recved;

	result = rt_event_recv(rtevent, event, option | RT_EVENT_FLAG_CLEAR, timeout, &recved);
	if (result != RT_EOK)
	{
		return -RT_ETIMEOUT;
	}

	return recved;
}
#define MQTT_EVENT_CONN_OK (1L << 0)
#define MQTT_EVNET_CLOSE_OK (1L << 1)
#define MQTT_EVENT_CONN_FAIL (1L << 2)
#define MQTT_EVENT_PUBACK (1L << 3)
#define MQTT_EVENT_CONNACK (1L << 4)
#define MQTT_EVENT_SUBACK (1L << 5)

/**
 *
 * 函数 HAL_AT_MQTT_Connect() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_AT_MQTT_Connect() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_AT_MQTT_Connect(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_AT_MQTT_Connect, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * Setup MQTT Connection
 *
 * @param[in] product key (optional).
 * @param[in] device name (optional).
 * @param[in] device Secret (optional).
 *
 * @return  0 - success, -1 - failure
 */

/**
 *
 * 函数 HAL_AT_MQTT_Deinit() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_AT_MQTT_Deinit() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_AT_MQTT_Deinit(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_AT_MQTT_Deinit, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * Deinit low layer
 *
 * @return  0 - success, -1 - failure
 */
int HAL_AT_MQTT_Deinit(void)
{
	at_client_send("AT+MIPCLOSE\r\n", 13);
	if (mqtt_event != RT_NULL)
	{
		rt_event_delete(mqtt_event);
		mqtt_event = RT_NULL;
	}
	if (host != RT_NULL)
	{
		rt_free(host);
	}
	if (client_id != RT_NULL)
	{
		rt_free(client_id);
	}
	if (password != RT_NULL)
	{
		rt_free(password);
	}
	if (username != RT_NULL)
	{
		rt_free(username);
	}
	if (customize_info != RT_NULL)
	{
		rt_free(customize_info);
	}
	if (pub_key != RT_NULL)
	{
		rt_free(pub_key);
	}
	if (lock != RT_NULL)
	{
		rt_mutex_delete(lock);
		lock = RT_NULL;
	}
	if (reclock != RT_NULL)
	{
		rt_mutex_delete(reclock);
		lock = RT_NULL;
	}
	errorn = 0;
	return 0;
}

/**
 *
 * 函数 HAL_AT_MQTT_Disconnect() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_AT_MQTT_Disconnect() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_AT_MQTT_Disconnect(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_AT_MQTT_Disconnect, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * Close MQTT connection
 *
 * @return  0 - success, -1 - failure
 */
int HAL_AT_MQTT_Disconnect(void)
{

	at_set_urc_table(RT_NULL, 0);
	mqtt_state = IOTX_MC_STATE_DISCONNECTED;
	at_response_t resp = RT_NULL;
	int result = RT_EOK;
	rt_mutex_take(lock, RT_WAITING_FOREVER);
	resp = at_create_resp(128, 0, 10 * RT_TICK_PER_SECOND);
	if (resp == RT_NULL)
	{
		LOG_E("no memory for air720 device response structure.");
		return -RT_ENOMEM;
	}

	if (at_exec_cmd(resp,
					"AT+MDISCONNECT") < 0)
	{
		result = -RT_ERROR;
		goto __exit;
	}

	if (at_exec_cmd(resp,
					"AT+MIPCLOSE") < 0)
	{
		result = -RT_ERROR;
		goto __exit;
	}
	rt_mutex_release(lock);

__exit:
	if (resp)
	{
		at_delete_resp(resp);
	}
	HAL_AT_MQTT_Deinit();
	if (result != RT_EOK)
	{
		return -1;
	}
	else
	{
		return 0;
	}

	return (int)1;
}

/**
 *
 * 函数 HAL_AT_MQTT_Init() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_AT_MQTT_Init() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_AT_MQTT_Init(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_AT_MQTT_Init, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * Initialize low layer
 *
 * @param[in]  iotx_mqtt_param_t a struct contain usename, password, etc.
 *
 * @return  0 - success, -1 - failure
 */

/**
 *
 * 函数 HAL_AT_MQTT_Publish() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_AT_MQTT_Publish() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_AT_MQTT_Publish(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_AT_MQTT_Publish, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * Publish message
 *
 * @param[in] topic
 * @param[in] qos
 * @param[in] message
 * @param[in] msg_len
 *
 * @return  0 - success, -1 - failure
 */
int HAL_AT_MQTT_Publish(const char *topic, int qos, const char *message, unsigned int msg_len)
{
	at_response_t resp = RT_NULL;
	int result = RT_EOK;
	char *msg1 = RT_NULL, *t, *t2;
	t2 = message;
	// msg1 = rt_malloc(3 * msg_len);
	// t2 = msg1;
	// t = (char *)message;
	// while (t <= message + msg_len)
	// {
	// 	if (*t == '\"')
	// 	{
	// 		*msg1 = '\\';
	// 		msg1++;
	// 		*msg1 = '2';
	// 		msg1++;
	// 		*msg1 = '2';
	// 		msg1++;
	// 	}
	// 	else
	// 	{
	// 		*msg1 = *t;
	// 		msg1++;
	// 	}
	// 	t++;
	// }
	// RT_ASSERT(msg1 <= t2 + 3 * msg_len);
	if (rt_mutex_take(lock, rt_tick_from_millisecond(request_timeout_ms)) != RT_EOK)
	{
		return -1;
	};
	int pubres = 0;
	switch (qos)
	{
		resp = at_create_resp(128, 0, rt_tick_from_millisecond(request_timeout_ms));
		if (resp == RT_NULL)
		{
			LOG_E("no memory for air720 device response structure.");
			result = -RT_ENOMEM;
			goto __exit;
		}
	case 0:
		pubres = at_exec_cmd(resp, "AT+MPUB=\"%s\",%d,0,\"%s\"", topic, qos, t2);
		if (pubres < 0)
		{
			if (pubres == -RT_ERROR)
			{
				errorn += 3;
			}
			result = -RT_ERROR;
			goto __exit;
		}
		break;
	case 1:
		resp = at_create_resp(128, 0, 50 * RT_TICK_PER_SECOND);
		if (resp == RT_NULL)
		{
			LOG_E("no memory for air720 device response structure.");
			result = -RT_ENOMEM;
			goto __exit;
		}
		mqtt_event_recv(mqtt_event, MQTT_EVENT_PUBACK, 0, RT_EVENT_FLAG_OR);
		pubres = at_exec_cmd(resp, "AT+MPUB=\"%s\",%d,0,\"%s\"", topic, qos, t2);
		if (pubres < 0)
		{
			if (pubres == -RT_ERROR)
			{
				errorn += 3;
			}
			result = -RT_ERROR;
			goto __exit;
		}
		int event_result = mqtt_event_recv(mqtt_event, MQTT_EVENT_PUBACK, rt_tick_from_millisecond(request_timeout_ms), RT_EVENT_FLAG_OR);
		if (event_result == -RT_ETIMEOUT)
		{
			LOG_E("time out");
			result = -RT_ERROR;
			goto __exit;
		}

		break;

	case 2:
		resp = at_create_resp(128, 6, 50 * RT_TICK_PER_SECOND);
		if (resp == RT_NULL)
		{
			LOG_E("no memory for air720 device response structure.");
			return -RT_ENOMEM;
		}

		if (at_exec_cmd(resp,
						"AT+MPUB=\"%s\",%d,0,\"%s\"", topic, qos, t2) < 0)
		{
			result = -RT_ERROR;
			goto __exit;
		}

		if (at_resp_get_line_by_kw(resp, "PUBREC") != NULL & at_resp_get_line_by_kw(resp, "PUBCOMP") != NULL)
		{
		}
		else
		{
			result = -RT_ERROR;
			goto __exit;
		}

		break;
	}

__exit:
	if (resp)
	{
		at_delete_resp(resp);
	}
	// rt_free(t2);
	rt_mutex_release(lock);

	if (result != RT_EOK)
	{
		needcheck = 1;
		return -1;
	}
	else
	{
		last_err = 0;
		return 0;
	}
}

/**
 *
 * 函数 HAL_AT_MQTT_State() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_AT_MQTT_State() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_AT_MQTT_State(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_AT_MQTT_State, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * Check connection status
 *
 * @return  0 - invalid, 1 - initialized, 2 - connected, 3 - disconnected, 4 - disconnect-reconnecting
 */
int HAL_AT_MQTT_State(void)
{
	statusc++;
	checktime++;
	if (checktime > 2000 * 24 * 60 * 2)
	{
		at_response_t resp = RT_NULL;
		int parsed_data[7] = {-1};
		resp = at_create_resp(128, 4, 5 * RT_TICK_PER_SECOND);
		if (at_exec_cmd(resp, "AT+CCLK?") == 0)
		{
			at_resp_parse_line_args_by_kw(resp, "+CCLK:", "+CCLK: \"%d/%d/%d,%d:%d:%d+%d\"",parsed_data,parsed_data+1,parsed_data+2,parsed_data+3,parsed_data+4,parsed_data+5,parsed_data+6);
			set_date(2000+parsed_data[0],parsed_data[1],parsed_data[2]);
			set_time(parsed_data[3],parsed_data[4],parsed_data[5]);
			checktime = 0;
		}
		if (resp)
		{
			at_delete_resp(resp);
		}
	}
	if (statusc == 2000 || needcheck == 1)
	{
		if (mqtt_state == IOTX_MC_STATE_CONNECTED || needcheck == 1)
		{
			needcheck = 0;
			int result = RT_EOK;
			at_response_t resp = RT_NULL;
			int parsed_data = -1;
			resp = at_create_resp(128, 4, 5 * RT_TICK_PER_SECOND);
			if (at_exec_cmd(resp, "AT+MQTTSTATU") < 0)
			{
				urc_close_func(RT_NULL, RT_NULL, 0);
			}
			else
			{
				at_resp_parse_line_args_by_kw(resp, "+MQTTSTATU", "+MQTTSTATU :%d", &parsed_data);
				if (parsed_data != 1)
				{
					urc_close_func(RT_NULL, RT_NULL, 0);
				}
			}
			if (resp)
			{
				at_delete_resp(resp);
			}
		}
		statusc = 0;
	}

	return mqtt_state;
}

/**
 *
 * 函数 HAL_AT_MQTT_Subscribe() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_AT_MQTT_Subscribe() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_AT_MQTT_Subscribe(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_AT_MQTT_Subscribe, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * Subscribe topic
 *
 * @param[in]  topic
 * @param[in]  qos
 * @param[in]  mqtt_packet_id
 * @param[out] mqtt_status
 * @param[in]  timeout_ms
 *
 * @return  0 - success, -1 - failure
 */
int HAL_AT_MQTT_Subscribe(const char *topic, int qos, unsigned int *mqtt_packet_id, int *mqtt_status, int timeout_ms)
{
	at_response_t resp = RT_NULL;
	int result = RT_EOK;
	resp = at_create_resp(128, 0, rt_tick_from_millisecond(request_timeout_ms));
	if (resp == RT_NULL)
	{
		LOG_E("no memory for air720 device response structure.");
		return -RT_ENOMEM;
	}
	mqtt_event_recv(mqtt_event, MQTT_EVENT_SUBACK, 0, RT_EVENT_FLAG_OR);
	if (rt_mutex_take(lock, rt_tick_from_millisecond(request_timeout_ms)) != RT_EOK)
	{
		if (resp)
		{
			at_delete_resp(resp);
		}

		return -1;
	};
	if (at_exec_cmd(resp,
					"AT+MSUB=\"%s\",%d", topic, qos) < 0)
	{
		result = -RT_ERROR;
		goto __exit;
	}
	int event_result = mqtt_event_recv(mqtt_event, MQTT_EVENT_SUBACK, rt_tick_from_millisecond(request_timeout_ms), RT_EVENT_FLAG_OR);
	if (event_result == -RT_ETIMEOUT)
	{
		LOG_E("time out");
		result = -RT_ERROR;
		goto __exit;
	}
	rt_kprintf("done");
__exit:
	if (resp)
	{
		at_delete_resp(resp);
	}
	rt_mutex_release(lock);
	if (result != RT_EOK)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

/**
 *
 * 函数 HAL_AT_MQTT_Unsubscribe() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_AT_MQTT_Unsubscribe() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_AT_MQTT_Unsubscribe(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_AT_MQTT_Unsubscribe, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * Unsubscribe topic
 *
 * @param[in]  topic
 * @param[in]  mqtt_packet_id
 * @param[out] mqtt_status
 *
 * @return  0 - success, -1 - failure
 */
int HAL_AT_MQTT_Unsubscribe(const char *topic, unsigned int *mqtt_packet_id, int *mqtt_status)
{
	at_response_t resp = RT_NULL;
	int result = RT_EOK;
	resp = at_create_resp(128, 0, 50 * RT_TICK_PER_SECOND);
	if (resp == RT_NULL)
	{
		LOG_E("no memory for air720 device response structure.");
		return -RT_ENOMEM;
	}
	if (rt_mutex_take(lock, rt_tick_from_millisecond(request_timeout_ms)) != RT_EOK)
	{
		return -1;
	};

	if (at_exec_cmd(resp,
					"AT+MUNSUB=\"%s\",%d", topic, 1) < 0)
	{
		result = -RT_ERROR;
		goto __exit;
	}

	if (at_resp_get_line_by_kw(resp, "UNSUBACK"))
	{
	}
	else
	{
		result = -RT_ERROR;
		goto __exit;
	}

__exit:
	if (resp)
	{
		at_delete_resp(resp);
	}
	rt_mutex_release(lock);
	if (result != RT_EOK)
	{
		return -1;
	}
	else
	{
		return 0;
	}

	return (int)1;
}

/**
 *
 * 函数 HAL_Free() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_Free() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_Free(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_Free, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * @brief Deallocate memory block
 *
 * @param[in] ptr @n Pointer to a memory block previously allocated with platform_malloc.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_Free(void *ptr)
{
	rt_free(ptr);
}

/**
 *
 * 函数 HAL_GetDeviceName() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_GetDeviceName() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_GetDeviceName(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_GetDeviceName, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * @brief Get device name from user's system persistent storage
 *
 * @param [ou] device_name: array to store device name, max length is IOTX_DEVICE_NAME_LEN
 * @return the actual length of device name
 */
int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN])
{
	memset(device_name, 0x0, IOTX_DEVICE_NAME_LEN + 1);
	strncpy(device_name, _device_name, IOTX_DEVICE_NAME_LEN);

	return strlen(device_name);
}

/**
 *
 * 函数 HAL_GetDeviceSecret() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_GetDeviceSecret() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_GetDeviceSecret(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_GetDeviceSecret, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * @brief Get device secret from user's system persistent storage
 *
 * @param [ou] device_secret: array to store device secret, max length is IOTX_DEVICE_SECRET_LEN
 * @return the actual length of device secret
 */
int HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN])
{
	memset(device_secret, 0x0, IOTX_DEVICE_SECRET_LEN + 1);
	strncpy(device_secret, _device_secret, IOTX_DEVICE_SECRET_LEN);

	return strlen(device_secret);
}

/**
 *
 * 函数 HAL_GetFirmwareVersion() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_GetFirmwareVersion() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_GetFirmwareVersion(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_GetFirmwareVersion, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * @brief Get firmware version
 *
 * @param [ou] version: array to store firmware version, max length is IOTX_FIRMWARE_VER_LEN
 * @return the actual length of firmware version
 */
int HAL_GetFirmwareVersion(char *version)
{
	memset(version, 0x0, IOTX_FIRMWARE_VER_LEN + 1);
	strncpy(version, _version, IOTX_FIRMWARE_VER_LEN);
	return strlen(version);
}

/**
 *
 * 函数 HAL_GetProductKey() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_GetProductKey() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_GetProductKey(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_GetProductKey, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * @brief Get product key from user's system persistent storage
 *
 * @param [ou] product_key: array to store product key, max length is IOTX_PRODUCT_KEY_LEN
 * @return  the actual length of product key
 */
int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN])
{
	memset(product_key, 0x0, IOTX_PRODUCT_KEY_LEN + 1);
	strncpy(product_key, _product_key, IOTX_PRODUCT_KEY_LEN);
	return strlen(product_key);
}

/**
 *
 * 函数 HAL_GetProductSecret() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_GetProductSecret() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_GetProductSecret(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_GetProductSecret, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
int HAL_GetProductSecret(char *product_secret)
{
	memset(product_secret, 0x0, IOTX_PRODUCT_SECRET_LEN + 1);
	strncpy(product_secret, _product_secret, IOTX_PRODUCT_SECRET_LEN);

	return strlen(product_secret);
}

/**
 *
 * 函数 HAL_Malloc() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_Malloc() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_Malloc(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_Malloc, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * @brief Allocates a block of size bytes of memory, returning a pointer to the beginning of the block.
 *
 * @param [in] size @n specify block size in bytes.
 * @return A pointer to the beginning of the block.
 * @see None.
 * @note Block value is indeterminate.
 */
extern void *HAL_Malloc(uint32_t size)
{
	return rt_malloc(size);
}

/**
 *
 * 函数 HAL_MutexCreate() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_MutexCreate() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_MutexCreate(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_MutexCreate, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * @brief Create a mutex.
 *
 * @retval NULL : Initialize mutex failed.
 * @retval NOT_NULL : The mutex handle.
 * @see None.
 * @note None.
 */
extern void *HAL_MutexCreate(void)
{
	rt_mutex_t mutex = rt_mutex_create("ali_ld_mutex", RT_IPC_FLAG_FIFO);
	return mutex;
}

/**
 *
 * 函数 HAL_MutexDestroy() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_MutexDestroy() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_MutexDestroy(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_MutexDestroy, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * @brief Destroy the specified mutex object, it will release related resource.
 *
 * @param [in] mutex @n The specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
extern void HAL_MutexDestroy(void *mutex)
{
	int err_num;

	if (0 != (err_num = rt_mutex_delete((rt_mutex_t)mutex)))
	{
		LOG_E("destroy mutex failed, err num: %d", err_num);
	}
}

/**
 *
 * 函数 HAL_MutexLock() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_MutexLock() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_MutexLock(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_MutexLock, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * @brief Waits until the specified mutex is in the signaled state.
 *
 * @param [in] mutex @n the specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
extern void HAL_MutexLock(void *mutex)
{
	int err_num;

	if (0 != (err_num = rt_mutex_take((rt_mutex_t)mutex, RT_WAITING_FOREVER)))
	{
		LOG_E("lock mutex failed, err num: %d", err_num);
	}
}
/**
 *
 * 函数 HAL_MutexUnlock() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_MutexUnlock() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_MutexUnlock(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_MutexUnlock, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * @brief Releases ownership of the specified mutex object..
 *
 * @param [in] mutex @n the specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
extern void HAL_MutexUnlock(void *mutex)
{
	int err_num;

	if (0 != (err_num = rt_mutex_release((rt_mutex_t)mutex)))
	{
		LOG_E("unlock mutex failed, err num: %d", err_num);
	}
}

/**
 *
 * 函数 HAL_Printf() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_Printf() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_Printf(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_Printf, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * @brief Writes formatted data to stream.
 *
 * @param [in] fmt: @n String that contains the text to be written, it can optionally contain embedded format specifiers
     that specifies how subsequent arguments are converted for output.
 * @param [in] ...: @n the variable argument list, for formatted and inserted in the resulting string replacing their respective specifiers.
 * @return None.
 * @see None.
 * @note None.
 */
extern void HAL_Printf(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	rt_vsnprintf(log_buf, RT_CONSOLEBUF_SIZE, fmt, args);
	va_end(args);
	rt_kprintf("%s", log_buf);
}

/**
 *
 * 函数 HAL_Random() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_Random() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_Random(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_Random, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
uint32_t HAL_Random(uint32_t region)
{
	return (region > 0) ? (rand() % region) : 0;
}

/**
 *
 * 函数 HAL_SetDeviceName() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_SetDeviceName() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_SetDeviceName(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_SetDeviceName, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
int HAL_SetDeviceName(char *device_name)
{
	return (int)1;
}

/**
 *
 * 函数 HAL_SetDeviceSecret() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_SetDeviceSecret() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_SetDeviceSecret(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_SetDeviceSecret, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
int HAL_SetDeviceSecret(char *device_secret)
{
	return (int)1;
}

/**
 *
 * 函数 HAL_SetProductKey() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_SetProductKey() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_SetProductKey(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_SetProductKey, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
int HAL_SetProductKey(char *product_key)
{
	return (int)1;
}

/**
 *
 * 函数 HAL_SetProductSecret() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_SetProductSecret() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_SetProductSecret(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_SetProductSecret, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
int HAL_SetProductSecret(char *product_secret)
{
	return (int)1;
}

/**
 *
 * 函数 HAL_SleepMs() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_SleepMs() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_SleepMs(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_SleepMs, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * @brief Sleep thread itself.
 *
 * @param [in] ms @n the time interval for which execution is to be suspended, in milliseconds.
 * @return None.
 * @see None.
 * @note None.
 */
extern void HAL_SleepMs(uint32_t ms)
{
	rt_thread_mdelay(ms);
}

/**
 *
 * 函数 HAL_Snprintf() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_Snprintf() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_Snprintf(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_Snprintf, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * @brief Writes formatted data to string.
 *
 * @param [out] str: @n String that holds written text.
 * @param [in] len: @n Maximum length of character will be written
 * @param [in] fmt: @n Format that contains the text to be written, it can optionally contain embedded format specifiers
     that specifies how subsequent arguments are converted for output.
 * @param [in] ...: @n the variable argument list, for formatted and inserted in the resulting string replacing their respective specifiers.
 * @return bytes of character successfully written into string.
 * @see None.
 * @note None.
 */
int HAL_Snprintf(char *str, const int len, const char *fmt, ...)
{
	va_list args;
	int rc;

	va_start(args, fmt);
	rc = rt_vsnprintf(str, len, fmt, args);
	va_end(args);

	return rc;
}

/**
 *
 * 函数 HAL_Srandom() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_Srandom() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_Srandom(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_Srandom, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
void HAL_Srandom(uint32_t seed)
{
	srand(seed);
}

/**
 *
 * 函数 HAL_UptimeMs() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_UptimeMs() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_UptimeMs(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_UptimeMs, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
/**
 * @brief Retrieves the number of milliseconds that have elapsed since the system was boot.
 *
 * @return the number of milliseconds.
 * @see None.
 * @note None.
 */
uint64_t HAL_UptimeMs(void)
{

#if (RT_TICK_PER_SECOND == 1000)
	return (uint64_t)rt_tick_get();
#else

	uint64_t tick;
	tick = rt_tick_get();

	tick = tick * 1000;

	return (tick + RT_TICK_PER_SECOND - 1) / RT_TICK_PER_SECOND;
#endif
}

/**
 *
 * 函数 HAL_Vsnprintf() 需要SDK的使用者针对SDK将运行的硬件平台填充实现, 供SDK调用
 * ---
 * Interface of HAL_Vsnprintf() requires to be implemented by user of SDK, according to target device platform
 *
 * 如果需要参考如何实现函数 HAL_Vsnprintf(), 可以查阅SDK移植到 Ubuntu Linux 上时的示例代码
 * ---
 * If you need guidance about how to implement HAL_Vsnprintf, you can check its reference implementation for Ubuntu platform
 *
 * https://code.aliyun.com/linkkit/c-sdk/blob/v3.0.1/wrappers/os/ubuntu/HAL_OS_linux.c
 *
 *
 * 注意! HAL_XXX() 系列的函数虽然有阿里提供的对应参考实现, 但不建议您不做任何修改/检视的应用于您的商用设备!
 * 
 * 注意! 参考示例实现仅用于解释各个 HAL_XXX() 系列函数的语义!
 * 
 */
int HAL_Vsnprintf(char *str, const int len, const char *format, va_list ap)
{
	return rt_vsnprintf(str, len, format, ap);
}

static const struct at_urc urc_table[] =
	{
		{"", "CONNECT OK\r\n", urc_connect_func},
		{"", "CONNECT FAIL\r\n", urc_connect_func},
		{"", "ALREADY CONNECT\r\n", urc_connect_func},
		{"", "CLOSE OK\r\n", urc_close_func},
		{"", "CLOSED\r\n", urc_close_func},
		{"", "MQTT HEART NO ACK\r\n", urc_close_func},
		// TODO reconnect function
		//        {"ECEIVE,", "\r\n", urc_recv_func},
		{"", "CONNACK OK\r\n", urc_connect_func},
		{"", "PUBACK\r\n", urc_send_func},
		{"", "SUBACK\r\n", urc_send_func},
		{"+MSUB:", "\r\n", urc_recv_func},
		//        {"DATA ACCEPT:", "\r\n", urc_dataaccept_func},
};
static void reconnect_entry(void *parameter)
{
	if (rt_mutex_take(reclock, rt_tick_from_millisecond(5000)) != RT_EOK)
	{
		return;
	}
	int wating_time = 2000;
	int result = RT_EOK;
	at_response_t resp = RT_NULL;
	int parsed_data = -1;
	resp = at_create_resp(128, 4, 10 * RT_TICK_PER_SECOND);
	if (at_exec_cmd(resp, "AT+MQTTSTATU") < 0)
	{
		goto __reconnect;
	}
	at_resp_parse_line_args_by_kw(resp, "+MQTTSTATU", "+MQTTSTATU :%d", &parsed_data);
	rt_kprintf("state: %d \n", parsed_data);
	if (parsed_data != 1)
	{
		goto __reconnect;
		// goto __reconnect_withoutreset;
	}
	else
	{
		at_set_urc_table(urc_table, sizeof(urc_table) / sizeof(urc_table[0]));
		goto __exit;
	}
__reconnect_withoutreset:
	at_client_send("AT+MIPCLOSE\r\n", 13);
	rt_thread_mdelay(1000);
	if (HAL_AT_MQTT_Connect(RT_NULL, RT_NULL, RT_NULL) == 0)
	{
		goto __exit;
	}

__reconnect:
	//TODO add reset for first connect
	do
	{
		for (size_t i = 0; i < 5; i++)
		{
			if ((HAL_AT_MQTT_Connect(RT_NULL, RT_NULL, RT_NULL) == 0))
			{
				goto __exit;
			}
			rt_thread_mdelay(2000);
		}
		struct at_device *netdevice = at_device_get_first_initialized();
		at_device_control(netdevice, AT_DEVICE_CTRL_RESET, RT_NULL);
		rt_thread_mdelay(wating_time);
		if (wating_time < 60000)
		{
			wating_time *= 2;
		}

	} while (HAL_AT_MQTT_Connect(RT_NULL, RT_NULL, RT_NULL) != 0);
	goto __exit;

__exit:
	if (HAL_AT_MQTT_Subscribe(subtopic, 0, 0, 0, 5000) != 0)
	{
		goto __reconnect_withoutreset;
	}
	if (resp)
	{
		at_delete_resp(resp);
	}
	mqtt_state = IOTX_MC_STATE_CONNECTED;
	rt_mutex_release(reclock);
}

static void urc_connect_func(struct at_client *client, const char *data, rt_size_t size)
{
	if (mqtt_event == RT_NULL)
	{
		LOG_E("get event failed.");
		return;
	}

	/* get the current socket by receive data */
	if (strstr(data, "CONNECT OK") != 0 | strstr(data, "ALREADY CONNECT") != 0)
	{
		mqtt_event_send(mqtt_event, MQTT_EVENT_CONN_OK);
	}
	else if (strstr(data, "CONNECT FAIL"))
	{
		mqtt_event_send(mqtt_event, MQTT_EVENT_CONN_FAIL);
	}
	else if (strstr(data, "CONNACK OK"))
	{
		mqtt_event_send(mqtt_event, MQTT_EVENT_CONNACK);
	}
}

static void urc_close_func(struct at_client *client, const char *data, rt_size_t size)
{

	if (mqtt_state == IOTX_MC_STATE_INITIALIZED || mqtt_state == IOTX_MC_STATE_DISCONNECTED_RECONNECTING)
	{
		mqtt_event_send(mqtt_event, MQTT_EVENT_CONN_FAIL);
	}
	if (mqtt_state == IOTX_MC_STATE_CONNECTED)
	{
		at_set_urc_table(RT_NULL, 0);
		mqtt_state = IOTX_MC_STATE_DISCONNECTED_RECONNECTING;
		rt_thread_t tid = rt_thread_create("reconnect_thread", reconnect_entry, (void *)RT_NULL,
										   2048, 1, 20);
		if (tid != RT_NULL)
		{
			HAL_Printf("START RECONNECT!");
			rt_thread_startup(tid);
		}
		mqtt_state == IOTX_MC_STATE_DISCONNECTED_RECONNECTING;
	}
}

static void urc_send_func(struct at_client *client, const char *data, rt_size_t size)
{
	if (mqtt_event == RT_NULL)
	{
		LOG_E("get event failed.");
		return;
	}

	/* get the current socket by receive data */
	if (strstr(data, "PUBACK"))
	{
		mqtt_event_send(mqtt_event, MQTT_EVENT_PUBACK);
	}
	else if (strstr(data, "SUBACK"))
	{
		mqtt_event_send(mqtt_event, MQTT_EVENT_SUBACK);
	}
	else if (strstr(data, "CONNACK OK"))
	{
		mqtt_event_send(mqtt_event, MQTT_EVENT_CONNACK);
	}
}
static void urc_recv_func(struct at_client *client, const char *data, rt_size_t size)
{
	rt_kprintf("i'm here");
	if (mqtt_state == IOTX_MC_STATE_CONNECTED)
	{
		rt_kprintf("entering");
		int device_socket = 0;
		rt_int32_t timeout;
		rt_size_t bfsz = 0, temp_size = 0;
		char *recv_buf = RT_NULL, topic[256] = {0};
		struct at_socket *socket = RT_NULL;
		struct at_device *device = RT_NULL;
		char *client_name = client->device->parent.name;

		RT_ASSERT(data && size);

		struct at_mqtt_input param;
		//	rt_kprintf("we get : %s",data);
		int tt;
		sscanf(data, "+MSUB: \"%[^\"]\",%d byte,%n%*s", topic, (int *)&bfsz, &tt);
		rt_kprintf("we get : %s;;%d", topic, bfsz);
		timeout = bfsz;

		if (bfsz == 0)
		{
			return;
		}
		recv_buf = (char *)rt_calloc(1, bfsz + 1);
		if (recv_buf == RT_NULL)
		{
			LOG_E("no memory");
			return;
		}
		memcpy(recv_buf, data + tt, bfsz);
		//	rt_kprintf("we get:%s&&%s",recv_buf,topicb);
		param.topic = topic;
		param.topic_len = strlen(topic) + 1;
		param.message = recv_buf;
		param.msg_len = bfsz + 1;
		if (IOT_ATM_Input(&param) != 0)
		{
			rt_kprintf("hand data to uplayer fail!\n");
		}
		rt_free(recv_buf);
	}
}

int HAL_AT_MQTT_Init(iotx_mqtt_param_t *pInitParams)
{
	uint32_t event = 0;
	rt_bool_t retryed = RT_FALSE;
	at_response_t resp = RT_NULL;
	int result = RT_EOK, event_result = 0;

	mqtt_event = rt_event_create("mqtt", RT_IPC_FLAG_FIFO);
	lock = rt_mutex_create("mqtt_lock", RT_IPC_FLAG_FIFO);
	reclock = rt_mutex_create("mqtt_rec_lock", RT_IPC_FLAG_FIFO);
	host = rt_malloc(strlen(pInitParams->host) + 1);
	RT_ASSERT(host != NULL);
	memset(host, 0, strlen(pInitParams->host) + 1);
	strncpy(host, pInitParams->host, strlen(pInitParams->host));
	rt_kprintf("host: %s", host);

	client_id = rt_malloc(strlen(pInitParams->client_id) + 1);
	RT_ASSERT(client_id != NULL);
	memset(client_id, 0, strlen(pInitParams->client_id) + 1);
	strncpy(client_id, pInitParams->client_id, strlen(pInitParams->client_id));
	rt_kprintf("client_id: %s", client_id);

	username = rt_malloc(strlen(pInitParams->username) + 1);
	RT_ASSERT(username != NULL);
	memset(username, 0, strlen(pInitParams->username) + 1);
	strncpy(username, pInitParams->username, strlen(pInitParams->username));
	rt_kprintf("username: %s", username);

	password = rt_malloc(strlen(pInitParams->password) + 1);
	RT_ASSERT(password != NULL);
	memset(password, 0, strlen(pInitParams->password) + 1);
	strncpy(password, pInitParams->password, strlen(pInitParams->password));
	rt_kprintf("password: %s", password);

	customize_info = rt_malloc(strlen(pInitParams->customize_info) + 1);
	RT_ASSERT(customize_info != NULL);
	memset(customize_info, 0, strlen(pInitParams->customize_info) + 1);
	strncpy(customize_info, pInitParams->customize_info, strlen(pInitParams->customize_info));

	// pub_key = rt_malloc(strlen(pInitParams->pub_key));
	// RT_ASSERT(pub_key != NULL);
	// memset(pub_key, 0, strlen(pInitParams->pub_key) + 1);
	// strncpy(pub_key, pInitParams->pub_key, strlen(pInitParams->pub_key));

	port = pInitParams->port;
	clean_session = pInitParams->clean_session;
	request_timeout_ms = pInitParams->request_timeout_ms;
	keepalive_interval_ms = pInitParams->keepalive_interval_ms;
	write_buf_size = pInitParams->write_buf_size;
	read_buf_size = pInitParams->read_buf_size;
	mqtt_state = IOTX_MC_STATE_INITIALIZED;
	errorn = 0;
	return (int)0;
}
int HAL_AT_MQTT_Connect(char *proKey, char *devName, char *devSecret)
{

	at_set_urc_table(urc_table, sizeof(urc_table) / sizeof(urc_table[0]));

	int result = RT_EOK;
	at_response_t resp = at_create_resp(128, 0, rt_tick_from_millisecond(request_timeout_ms));
	if (resp == RT_NULL)
	{
		LOG_E("no memory for air720 device response structure.");
		return -1;
	}
	int retry_n = 3;
	while (retry_n--)
	{
		if (at_exec_cmd(resp, "AT+MQTTMSGSET=0") < 0)
		{
			result = -RT_ERROR;
			goto __exit;
		}
		if (at_exec_cmd(resp, "AT+MQTTMODE=0") < 0)
		{
			result = -RT_ERROR;
			goto __exit;
		}

		mqtt_event_recv(mqtt_event, MQTT_EVENT_CONN_OK | MQTT_EVENT_CONN_FAIL, 0, RT_EVENT_FLAG_OR);
		struct hostent *host0 = gethostbyname(host);
		if (host0 == RT_NULL)
		{
			result = -RT_ERROR;
			rt_thread_mdelay(1000);
			continue;
		}
		if (strlen(username) == 0)
		{
			if (at_exec_cmd(resp,
							"AT+MCONFIG=\"%s\"", client_id) < 0)
			{
				result = -RT_ERROR;
				goto __exit;
			}
		}
		else
		{
			if (at_exec_cmd(resp,
							"AT+MCONFIG=\"%s\",\"%s\",\"%s\"", client_id, username, password) < 0)
			{
				result = -RT_ERROR;
				goto __exit;
			}
		}
		rt_thread_mdelay(1000);
		/* clear socket connect event */
		int mipresult = at_exec_cmd(resp, "AT+MIPSTART=\"%s\",%d", inet_ntoa(*(struct in_addr *)(host0->h_addr)), port);
		if (mipresult == -RT_ERROR)
		{
			result = -RT_ERROR;
			goto __exit;
		}
		if (mipresult == -RT_ETIMEOUT)
		{
			LOG_E("mip start time out");
			result = -RT_ERROR;
			rt_thread_mdelay(2000);
			continue;
		}
		/* waiting result event from AT URC, the device default connection timeout is 75 seconds, but it set to 10 seconds is convenient to use */
		int event_result = mqtt_event_recv(mqtt_event, MQTT_EVENT_CONN_OK | MQTT_EVENT_CONN_FAIL, rt_tick_from_millisecond(request_timeout_ms), RT_EVENT_FLAG_OR);
		if (event_result == -RT_ETIMEOUT)
		{
			LOG_E("time out");
			result = -RT_ERROR;
			rt_thread_mdelay(2000);
			continue;
		}
		if (event_result & MQTT_EVENT_CONN_FAIL)
		{
			LOG_E("tcp connect failed.");
			result = -RT_ERROR;
			rt_thread_mdelay(2000);
			continue;
		}
		mqtt_event_recv(mqtt_event, MQTT_EVENT_CONNACK | MQTT_EVENT_CONN_FAIL, 0, RT_EVENT_FLAG_OR);
		rt_thread_mdelay(500);
		mipresult = at_exec_cmd(resp, "AT+MCONNECT=%d,%d", clean_session, keepalive_interval_ms / 1000);
		if (mipresult == -RT_ERROR)
		{
			result = -RT_ERROR;
			goto __exit;
		}
		if (mipresult == -RT_ETIMEOUT)
		{
			LOG_E("mip start time out");
			result = -RT_ERROR;
			rt_thread_mdelay(2000);
			continue;
		}
		event_result = mqtt_event_recv(mqtt_event, MQTT_EVENT_CONNACK | MQTT_EVENT_CONN_FAIL, rt_tick_from_millisecond(request_timeout_ms), RT_EVENT_FLAG_OR);
		if (event_result == -RT_ETIMEOUT)
		{
			LOG_E("time out");
			result = -RT_ERROR;
			rt_thread_mdelay(2000);
			continue;
		}
		if (event_result & MQTT_EVENT_CONN_FAIL)
		{
			LOG_E("mqtt connect illegal.");
			result = -RT_ERROR;
			rt_thread_mdelay(2000);
			continue;
		}
		if (event_result & MQTT_EVENT_CONNACK)
		{
			LOG_E("mqtt connect success.");
			result = RT_EOK;
			goto __exit;
		}
	}

__exit:
	if (resp)
	{
		at_delete_resp(resp);
	}

	if (result != RT_EOK)
	{
		return -1;
		mqtt_state = IOTX_MC_STATE_INITIALIZED;
	}
	else
	{
		mqtt_state = IOTX_MC_STATE_CONNECTED;
		return 0;
	}
}
