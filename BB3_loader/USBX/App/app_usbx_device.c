/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    app_usbx_device.c
 * @author  MCD Application Team
 * @brief   USBX Device applicative file
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2020-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_usbx_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_azure_rtos_config.h"
#include "tx_api.h"

#include "app_mtp_cb.h"
#include "drivers/rev.h"

#include "debug.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

#define LOBYTE(x)  ((uint8_t)((x) & 0x00FFU))
#define HIBYTE(x)  ((uint8_t)(((x) & 0xFF00U) >> 8U))

#define SPLIT16(x)    LOBYTE(x), HIBYTE(x)

#define USBD_VID                        0x0001
#define USBD_PID                        0x0001

UCHAR device_framework_high_speed[] =
        {
                /* Device descriptor */
                18, /*bLength */
                0x01, /*bDescriptorType*/
                SPLIT16(0x0200), /*bcdUSB 2.00*/
                0x00, /*bDeviceClass*/
                0x00, /*bDeviceSubClass*/
                0x00, /*bDeviceProtocol*/
                64, /*bMaxPacketSize*/
                SPLIT16(USBD_VID), /*idVendor*/
                SPLIT16(USBD_PID), /*idProduct*/
                SPLIT16(0x0200), /*bcdDevice rel. 2.00*/
                0x01, /*Index of manufacturer  string*/
                0x02, /*Index of product string*/
                0x03, /*Index of serial number string*/
                0x01, /*bNumConfigurations*/

                /* Configuration descriptor */
                9, /* bLength: Configuration Descriptor size */
                0x02, /* bDescriptorType: Configuration */
                SPLIT16(39), /* wTotalLength: Total size of the Config descriptor */
                0x01, /* bNumInterfaces: 1 interface */
                0x01, /* bConfigurationValue: Configuration value */
                0x04, /* iConfiguration: Index of string descriptor describing the configuration */
                0x80, /* bmAttributes: Bus Powered according to user configuration */
                250, /* MaxPower (mA) */

                /* Interface descriptor */
                9, /* bLength: Interface Descriptor size */
                0x04, /* bDescriptorType: Interface descriptor type */
                0x00, /* bInterfaceNumber: Number of Interface */
                0x00, /* bAlternateSetting: Alternate setting */
                0x03, /* bNumEndpoints:  */
                0x06, /* bInterfaceClass: bInterfaceClass: user's interface for MTP */
                0x01, /* bInterfaceSubClass:Abstract Control Model */
                0x01, /* bInterfaceProtocol: Common AT commands */
                0x05, /* iInterface: */

                /* Endpoint descriptor (Bulk Out) */
                7, /* Endpoint descriptor length = 7 */
                0x05, /* Endpoint descriptor type */
                0x81, /* Endpoint address (IN, address 1) */
                0x02, /* Bulk endpoint type */
                SPLIT16(64), /* wMaxPacketSize */
                0x00, /* Polling interval in milliseconds */

                /* Endpoint descriptor (Bulk In) */
                7, /* Endpoint descriptor length = 7 */
                0x05, /* Endpoint descriptor type */
                0x02, /* Endpoint address (OUT, address 2) */
                0x02, /* Bulk endpoint type */
                SPLIT16(64), /* wMaxPacketSize */
                0x00, /* Polling interval in milliseconds */

                /* Endpoint descriptor (Interrupt) */
                7, /* bLength: Endpoint Descriptor size */
                0x05, /* bDescriptorType:*/
                0x83, /* bEndpointAddress: Endpoint Address (IN) */
                0x03, /* bmAttributes: Interrupt endpoint */
                SPLIT16(8), /* wMaxPacketSize */
                0x20 /* Polling interval in milliseconds */
        };

UCHAR string_framework[] = {
        /* Manufacturer string descriptor: Index 1 */
        0x09, 0x04, 0x01,
        7,
        'S', 'k', 'y', 'B', 'e', 'a', 'n',

        /* Product string descriptor: Index 2 */
        0x09, 0x04, 0x02,
        6,
        'S', 't', 'r', 'a', 't', 'o',

        /* Serial Number string descriptor: Index 3 */
        0x09, 0x04, 0x03,
        8,
        'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',

        /* Serial Number string descriptor: Index 4 */
        0x09, 0x04, 0x04,
        3,
        'm', 't', 'p',

        /* Serial Number string descriptor: Index 5 */
        0x09, 0x04, 0x05,
        3,
        'M', 'T', 'P'
};

UCHAR language_id_framework[] = {
        /* English. */
        0x09, 0x04
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void ux_error_cb(UINT system_level, UINT system_context, UINT error_code)
{
    ERR("ux_error_cb %u, %u, %X", system_level, system_context, error_code);
    if (error_code == UX_MEMORY_INSUFFICIENT || error_code == 8193)
        while(1);
}
/* USER CODE END PFP */
/**
  * @brief  Application USBX Device Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT MX_USBX_Device_Init(VOID *memory_ptr)
{
  UINT ret = UX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

  /* USER CODE BEGIN MX_USBX_Device_MEM_POOL */

  /* USER CODE END MX_USBX_Device_MEM_POOL */

  /* USER CODE BEGIN MX_USBX_Device_Init */
    void *allocated_memory;

    //assign serial number
    sprintf(strstr((char *)string_framework, "XXXXXXXX"), "%08lX", rev_get_short_id());

    //allocate memory
    UINT status = tx_byte_allocate(byte_pool, (VOID**) &allocated_memory, 230 * 1024, TX_NO_WAIT);
    ASSERT_MSG(status == UX_SUCCESS, "tx_byte_allocate, status %02X", status);

    status = ux_system_initialize(allocated_memory, 230 * 1024, UX_NULL, 0);
    ASSERT_MSG(status == UX_SUCCESS, "ux_system_initialize, status %02X", status);

    ux_utility_error_callback_register(ux_error_cb);

    //init stack
    status = ux_device_stack_initialize(device_framework_high_speed, sizeof(device_framework_high_speed),
            device_framework_high_speed, sizeof(device_framework_high_speed),
            string_framework, sizeof(string_framework),
            language_id_framework, sizeof(language_id_framework),
            UX_NULL);
    ASSERT_MSG(status == UX_SUCCESS, "ux_device_stack_initialize, status %02X", status);

    //register MTP
    UX_SLAVE_CLASS_PIMA_PARAMETER parameter;
    mtp_assign_parameters(&parameter);

    status = ux_device_stack_class_register(_ux_system_slave_class_pima_name,
            ux_device_class_pima_entry,
            1,
            0,
            &parameter);
    ASSERT_MSG(status == UX_SUCCESS, "ux_device_stack_class_register, status %02X", status);

    //start usb
    status = tx_byte_allocate(byte_pool, (VOID**) &allocated_memory, 2 * 1024, TX_NO_WAIT);
    ASSERT_MSG(status == UX_SUCCESS, "tx_byte_allocate, status %02X", status);

    static TX_THREAD ux_app_thread;
    status = tx_thread_create(&ux_app_thread, "app_mtp_thread_entry",
            app_mtp_thread_entry, 0,
            allocated_memory, 2 * 1024,
            10, 10,
            TX_NO_TIME_SLICE, TX_AUTO_START);
    ASSERT_MSG(status == UX_SUCCESS, "tx_thread_create app_mtp_thread_entry, status %02X", status);

    INFO("ux_app_thread %p", &ux_app_thread);

  /* USER CODE END MX_USBX_Device_Init */

  return ret;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
