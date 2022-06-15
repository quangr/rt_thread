#include <stdio.h>
#include <string.h>

#include "dev_sign_api.h"
#include "mqtt_api.h"

#ifdef ATM_ENABLED
#include "at_api.h"
#endif
#include <rtthread.h>
#include <at_device.h>
#include <cJSON.h>
#include <rtdevice.h>
#define LED_PIN (0x2D)

#define LED0_PORT (PortE)
#define LED0_PIN (Pin01)

#define LED0_ON() (PORT_SetBits(LED0_PORT, LED0_PIN))
#define LED0_OFF() (PORT_ResetBits(LED0_PORT, LED0_PIN))
static char g_topic_name[CONFIG_MQTT_TOPIC_MAXLEN];

extern void dma_test(void);

void HAL_Printf(const char *fmt, ...);
int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN]);
int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN]);
int HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN]);
uint64_t HAL_UptimeMs(void);
int HAL_Snprintf(char *str, const int len, const char *fmt, ...);
volatile extern int data_ready;
volatile extern uint16_t state_changed;
volatile extern uint32_t senddata[64];
void data_init();
void air720_h_reset(void);
void protect_set(uint16_t line, uint16_t bound_I, uint16_t time_I, uint16_t bound_II, uint16_t time_II);
void timer_set(int line, uint8_t weekday, uint32_t starttime, uint32_t duration);
int datereport = 0;
int refresh_data = 0;
void set_mode(int mode);

#define EXAMPLE_TRACE(fmt, ...)                        \
    do                                                 \
    {                                                  \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__);                \
        HAL_Printf("%s", "\r\n");                      \
    } while (0)
int example_publish(void *handle, char *a, uint8_t *sentdata)
{
    int res = 0;
    iotx_mqtt_topic_info_t topic_msg;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_name[IOTX_DEVICE_NAME_LEN + 1] = {0};
    const char *fmt = "/sys/%s/%s/rrpc/response/%s";
    char *topic = NULL;
    int topic_len = 0;

    HAL_GetProductKey(product_key);
    HAL_GetDeviceName(device_name);

    topic_len = strlen(fmt) + strlen(product_key) + strlen(device_name) + strlen(a) + 1;
    if (topic_len > CONFIG_MQTT_TOPIC_MAXLEN)
    {
        HAL_Printf("topic too long\n");
        return -1;
    }
    topic = g_topic_name;
    memset(topic, 0, CONFIG_MQTT_TOPIC_MAXLEN);
    HAL_Snprintf(topic, topic_len, fmt, product_key, device_name, a);

    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));

    topic_msg.qos = IOTX_MQTT_QOS0;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)sentdata;
    topic_msg.payload_len = strlen(sentdata);

    res = IOT_MQTT_Publish(handle, topic, &topic_msg);
    if (res < 0)
    {
        HAL_Printf("publish failed\n");
        return -1;
    }

    return 0;
}

void example_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_t *topic_info = (iotx_mqtt_topic_info_pt)msg->msg;
    switch (msg->event_type)
    {
    case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
        /* print topic name and topic message */
        HAL_Printf("Message Arrived: \n");
        HAL_Printf("Topic  : %.*s\n", topic_info->topic_len, topic_info->ptopic);
        HAL_Printf("Payload: %.*s\n", topic_info->payload_len, topic_info->payload);
        char string[100] = {0};
        char a[20];
        sscanf(topic_info->ptopic, "/sys/%*[^/]%*1[/]%*[^/]/rrpc/request/%s", a);
        HAL_Printf("I GET %s!!: \n", a);
        char *c = topic_info->payload;
        rt_kprintf("revecive %d,%d,%d,%d\n", c[0], c[1], c[2], c[3]);
        if (c[0] & (1 << 0))
        {
            HAL_Printf("IT'S A ON SERVICE!");
            switch_on(c[1] & 0xf);
            datereport = 1;
        }
        if (c[0] & (1 << 1))
        {
            HAL_Printf("IT'S A OFF SERVICE!");
            switch_off(c[1] & 0xf);
            datereport = 1;
        }
        if (c[0] & (1 << 2))
        {
            HAL_Printf("IT'S A PROTECT SET SERVICE!");
            for (size_t i = 0; i < 4; i++)
            {
                if (c[1] & (0x1 << i))
                {
                    protect_set(i, *((uint16_t *)&c[2 + i * 8]), *((uint16_t *)&c[2 + i * 8 + 2]), *((uint16_t *)&c[2 + i * 8 + 4]), *((uint16_t *)&c[2 + i * 8 + 6]));
                }
            }
            datereport = 1;
        }
        if (c[0] & (1 << 3))
        {
            HAL_Printf("IT'S A TIMER SET SERVICE!");
            for (size_t i = 0; i < 4; i++)
            {
                if (c[1] & (0x1 << i))
                {
                    timer_set(i, c[2 + i * 9], *((uint32_t *)&c[2 + i * 9 + 1]), *((uint32_t *)&c[2 + i * 9 + 5]));
                }
            }
            datereport = 1;
        }
        if (c[0] & (1 << 4))
        {
            HAL_Printf("IT'S A FRESH SERVICE!");
            refresh_data = 1;
            while (refresh_data == 1)
                ;
            datereport = 1;
        }
        // if (c[0] & (1 << 5))
        // {
        //     HAL_Printf("IT'S A MODE CHANGE SERVICE!");
        //     set_mode(c[1]);
        //     datereport = 1;
        // }
        sprintf(string, "c,%d,", senddata[0]);
        example_publish(pclient, a, string);
        rt_thread_mdelay(600);
        break;
    default:
        break;
    }
}

int example_subscribe(void *handle)
{
    int res = 0;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_name[IOTX_DEVICE_NAME_LEN + 1] = {0};
    const char *fmt = "/sys/%s/%s/rrpc/request/+";
    char *topic = NULL;
    int topic_len = 0;

    HAL_GetProductKey(product_key);
    HAL_GetDeviceName(device_name);

    topic_len = strlen(fmt) + strlen(product_key) + strlen(device_name) + 1;
    if (topic_len > CONFIG_MQTT_TOPIC_MAXLEN)
    {
        HAL_Printf("topic too long\n");
        return -1;
    }
    topic = g_topic_name;
    memset(topic, 0, CONFIG_MQTT_TOPIC_MAXLEN);
    HAL_Snprintf(topic, topic_len, fmt, product_key, device_name);

    res = IOT_MQTT_Subscribe(handle, topic, IOTX_MQTT_QOS0, example_message_arrive, NULL);
    if (res < 0)
    {
        HAL_Printf("subscribe failed\n");
        return -1;
    }

    return 0;
}

int e_test_publish(void *handle)
{
    int res = 0;
    iotx_mqtt_topic_info_t topic_msg;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_name[IOTX_DEVICE_NAME_LEN + 1] = {0};
    const char *fmt = "/sys/%s/%s/thing/model/up_raw";
    char *topic = NULL;
    int topic_len = 0;
    char string[1024] = {0};
    sprintf(string, "u,");
    for (size_t i = 0; i < (sizeof(senddata) / sizeof(uint32_t)) - 1; i++)
    {
        sprintf(&string[strlen(string)], "%d,", senddata[i]);
    }

    char *payload = string;
    HAL_GetProductKey(product_key);
    HAL_GetDeviceName(device_name);

    topic_len = strlen(fmt) + strlen(product_key) + strlen(device_name) + 1;
    if (topic_len > CONFIG_MQTT_TOPIC_MAXLEN)
    {
        HAL_Printf("topic too long\n");
        return -1;
    }
    topic = g_topic_name;
    memset(topic, 0, CONFIG_MQTT_TOPIC_MAXLEN);
    HAL_Snprintf(topic, topic_len, fmt, product_key, device_name);

    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    topic_msg.qos = IOTX_MQTT_QOS0;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)payload;
    topic_msg.payload_len = strlen(string);

    res = IOT_MQTT_Publish(handle, topic, &topic_msg);

    if (res < 0)
    {
        HAL_Printf("publish failed\n");
        return -1;
    }

    return 0;
}


//TODO error process
int test_publish(void *handle)
{
    int res = 0;
    iotx_mqtt_topic_info_t topic_msg;
    char product_key[IOTX_PRODUCT_KEY_LEN + 1] = {0};
    char device_name[IOTX_DEVICE_NAME_LEN + 1] = {0};
    const char *fmt = "/sys/%s/%s/thing/model/up_raw";
    char *topic = NULL;
    int topic_len = 0;
    char string[1024] = {0};
    for (size_t i = 0; i < (sizeof(senddata) / sizeof(uint32_t)) - 1; i++)
    {
        sprintf(&string[strlen(string)], "%d,", senddata[i]);
    }
    char *payload = string;

    HAL_GetProductKey(product_key);
    HAL_GetDeviceName(device_name);

    topic_len = strlen(fmt) + strlen(product_key) + strlen(device_name) + 1;
    if (topic_len > CONFIG_MQTT_TOPIC_MAXLEN)
    {
        HAL_Printf("topic too long\n");
        return -1;
    }
    topic = g_topic_name;
    memset(topic, 0, CONFIG_MQTT_TOPIC_MAXLEN);
    HAL_Snprintf(topic, topic_len, fmt, product_key, device_name);

    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    topic_msg.qos = IOTX_MQTT_QOS0;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)payload;
    topic_msg.payload_len = strlen(string);

    res = IOT_MQTT_Publish(handle, topic, &topic_msg);

    if (res < 0)
    {
        HAL_Printf("publish failed\n");
        return -1;
    }

    return 0;
}

void example_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    HAL_Printf("msg->event_type : %d\n", msg->event_type);
}

/*
 *  NOTE: About demo topic of /${productKey}/${deviceName}/user/get
 *
 *  The demo device has been configured in IoT console (https://iot.console.aliyun.com)
 *  so that its /${productKey}/${deviceName}/user/get can both be subscribed and published
 *
 *  We design this to completely demostrate publish & subscribe process, in this way
 *  MQTT client can receive original packet sent by itself
 *
 *  For new devices created by yourself, pub/sub privilege also required to be granted
 *  to its /${productKey}/${deviceName}/user/get to run whole example
 */

int mqtt_test(int argc, char *argv[])
{
    data_init();
    void *pclient = NULL;
    int res = 0;
    int loop_cnt = 0;
    iotx_mqtt_region_types_t region = IOTX_CLOUD_REGION_SHANGHAI;
    iotx_sign_mqtt_t sign_mqtt;
    iotx_dev_meta_info_t meta;
    iotx_mqtt_param_t mqtt_params;

#ifdef ATM_ENABLED
    if (IOT_ATM_Init() < 0)
    {
        HAL_Printf("IOT ATM init failed!\n");
        return -1;
    }
#endif
    HAL_Printf("mqtt example\n");

    memset(&meta, 0, sizeof(iotx_dev_meta_info_t));
    HAL_GetProductKey(meta.product_key);
    HAL_GetDeviceName(meta.device_name);
    HAL_GetDeviceSecret(meta.device_secret);

    memset(&sign_mqtt, 0x0, sizeof(iotx_sign_mqtt_t));

    if (IOT_Sign_MQTT(region, &meta, &sign_mqtt) < 0)
    {
        return -1;
    }

#if 0 /* Uncomment this to show more information */
    HAL_Printf("sign_mqtt.hostname: %s\n", sign_mqtt.hostname);
    HAL_Printf("sign_mqtt.port    : %d\n", sign_mqtt.port);
    HAL_Printf("sign_mqtt.username: %s\n", sign_mqtt.username);
    HAL_Printf("sign_mqtt.password: %s\n", sign_mqtt.password);
    HAL_Printf("sign_mqtt.clientid: %s\n", sign_mqtt.clientid);
#endif

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port = sign_mqtt.port;
    mqtt_params.host = sign_mqtt.hostname;
    mqtt_params.client_id = sign_mqtt.clientid;
    mqtt_params.username = sign_mqtt.username;
    mqtt_params.password = sign_mqtt.password;

    mqtt_params.request_timeout_ms = 10000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.read_buf_size = 1024;
    mqtt_params.write_buf_size = 1024;

    mqtt_params.handle_event.h_fp = example_event_handle;
    mqtt_params.handle_event.pcontext = NULL;
    int waiting_time = 1000;
__construct:
    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient)
    {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("MQTT construct failed");
        struct at_device *netdevice = at_device_get_first_initialized();
        at_device_control(netdevice, AT_DEVICE_CTRL_RESET, RT_NULL);
        if (waiting_time < 60000)
        {
            waiting_time *= 2;
        }
        rt_thread_mdelay(waiting_time);
        goto __construct;
    }
    waiting_time = 1000;
    res = example_subscribe(pclient);
    if (res < 0)
    {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("MQTT construct failed");
        if (waiting_time < 60000)
        {
            waiting_time *= 2;
        }
        rt_thread_mdelay(waiting_time);
        goto __construct;
    }

    rt_thread_t tid = rt_thread_create("dma_thread", dma_test, (void *)RT_NULL,
                                       2048, 0, 20);
    if (tid != RT_NULL)
    {
        HAL_Printf("enable dma");
        rt_thread_startup(tid);
    }

    while (1)
    {
        if (data_ready == 1 || datereport == 1 || state_changed == 1)
        {
            if (state_changed == 0)
            {
                if (test_publish(pclient) == 0)
                {
                    data_ready = 0;
                    datereport = 0;
                }
            }else{
                if (e_test_publish(pclient) == 0)
                {
                    data_ready = 0;
                    datereport = 0;
                    state_changed = 0;
                }

            }
        }
        rt_thread_mdelay(150);
        IOT_MQTT_Yield(pclient, 500);
    }
    IOT_MQTT_Destroy(&pclient);

    return 0;
}

MSH_CMD_EXPORT(mqtt_test, a tcp client sample);
