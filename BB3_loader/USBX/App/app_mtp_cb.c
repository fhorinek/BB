/*
 * app_mtp_cb.c
 *
 *  Created on: Apr 5, 2022
 *      Author: horinek
 */

#include "app_mtp_cb.h"
#include "common.h"

#include "usb_otg.h"
#include "ux_dcd_stm32.h"


void mtp_activate(void * param)
{
    INFO("mtp_activate %p", param);
}

void mtp_deactivate(void * param)
{
    INFO("mtp_deactivate %p", param);
}


static UINT mtp_device_reset(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima)
{
    INFO("mtp_device_reset");
    return UX_SUCCESS;
}

static UINT mtp_device_prop_desc_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG device_property, UCHAR **device_prop_dataset, ULONG *device_prop_dataset_length)
{
    INFO("mtp_device_prop_desc_get");
    return UX_SUCCESS;
}

static UINT mtp_device_prop_value_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG device_property, UCHAR **device_prop_value, ULONG *device_prop_value_length)
{
    INFO("mtp_device_prop_value_get");
    return UX_SUCCESS;
}

static UINT mtp_device_prop_value_set(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG device_property, UCHAR *device_prop_value, ULONG device_prop_value_length)
{
    INFO("mtp_device_prop_value_set");
    return UX_SUCCESS;
}

static UINT mtp_storage_format(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG storage_id)
{
    INFO("mtp_storage_format");
    return UX_SUCCESS;
}

static UINT mtp_storage_info_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG storage_id)
{
    INFO("mtp_storage_info_get");
    return UX_SUCCESS;
}

static UINT mtp_object_number_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_format_code, ULONG object_association, ULONG *object_number)
{
    INFO("mtp_object_number_get");
    return UX_SUCCESS;
}

static UINT mtp_object_handles_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handles_format_code,
        ULONG object_handles_association,
        ULONG *object_handles_array,
        ULONG object_handles_max_number)
{
    INFO("mtp_object_handles_get");
    return UX_SUCCESS;
}

static UINT mtp_object_info_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle, UX_SLAVE_CLASS_PIMA_OBJECT **object)
{
    INFO("mtp_object_info_get");
    return UX_SUCCESS;
}

static UINT mtp_object_data_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle, UCHAR *object_buffer, ULONG object_offset,
        ULONG object_length_requested, ULONG *object_actual_length)
{
    INFO("mtp_object_data_get");
    return UX_SUCCESS;
}

static UINT mtp_object_info_send(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, UX_SLAVE_CLASS_PIMA_OBJECT *object, ULONG storage_id, ULONG parent_object_handle, ULONG *object_handle)
{
    INFO("mtp_object_info_send");
    return UX_SUCCESS;
}

static UINT mtp_object_data_send(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle, ULONG phase, UCHAR *object_buffer, ULONG object_offset,
        ULONG object_length)
{
    INFO("mtp_object_data_send");
    return UX_SUCCESS;
}

static UINT mtp_object_delete(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle)
{
    INFO("mtp_object_delete");
    return UX_SUCCESS;
}

static UINT mtp_object_prop_desc_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle, ULONG object_property, UCHAR **object_prop_dataset, ULONG *object_prop_dataset_length)
{
    INFO("mtp_object_prop_desc_get");
    return UX_SUCCESS;
}

static UINT mtp_object_prop_value_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle, ULONG object_property, UCHAR **object_prop_value, ULONG *object_prop_value_length)
{
    INFO("mtp_object_prop_value_get");
    return UX_SUCCESS;
}

static UINT mtp_object_prop_value_set(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle, ULONG object_property, UCHAR *object_prop_value, ULONG object_prop_value_length)
{
    INFO("mtp_object_prop_value_set");
    return UX_SUCCESS;
}

static UINT mtp_object_references_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle, UCHAR **object_handle_array, ULONG *object_handle_array_length)
{
    INFO("mtp_object_references_get");
    return UX_SUCCESS;
}

static UINT mtp_object_references_set(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle, UCHAR *object_handle_array, ULONG object_handle_array_length)
{
    INFO("mtp_object_references_set");
    return UX_SUCCESS;
}

static UCHAR manufacturer[] = "SkyBean";
static UCHAR model[] = "Strato";
static UCHAR serial[] = "TODO";
static UCHAR version[] = "1.0";
static UCHAR storage_desc[] = "";
static UCHAR storage_label[] = "Internal storage";

void mtp_assign_parameters(UX_SLAVE_CLASS_PIMA_PARAMETER * parameter)
{
    parameter->ux_device_class_pima_instance_activate = mtp_activate;
    parameter->ux_device_class_pima_instance_deactivate = mtp_deactivate;

    parameter->ux_device_class_pima_parameter_manufacturer = manufacturer;
    parameter->ux_device_class_pima_parameter_model = model;
    parameter->ux_device_class_pima_parameter_serial_number = serial;
    parameter->ux_device_class_pima_parameter_device_version = version;
    parameter->ux_device_class_pima_parameter_storage_id = 0x00010001;
    parameter->ux_device_class_pima_parameter_storage_type = UX_DEVICE_CLASS_PIMA_STC_FIXED_RAM;
    parameter->ux_device_class_pima_parameter_storage_file_system_type = UX_DEVICE_CLASS_PIMA_FSTC_GENERIC_HIERARCHICAL;
    parameter->ux_device_class_pima_parameter_storage_access_capability = UX_DEVICE_CLASS_PIMA_AC_READ_WRITE;
    parameter->ux_device_class_pima_parameter_storage_max_capacity_low = 0;
    parameter->ux_device_class_pima_parameter_storage_max_capacity_high = 0;
    parameter->ux_device_class_pima_parameter_storage_free_space_low = 0;
    parameter->ux_device_class_pima_parameter_storage_free_space_high = 0;
    parameter->ux_device_class_pima_parameter_storage_free_space_image = 0;
    parameter->ux_device_class_pima_parameter_storage_description = storage_desc;
    parameter->ux_device_class_pima_parameter_storage_volume_label = storage_label;

    //lists
    parameter->ux_device_class_pima_parameter_device_properties_list = NULL;
    parameter->ux_device_class_pima_parameter_supported_capture_formats_list = NULL;
    parameter->ux_device_class_pima_parameter_supported_image_formats_list = NULL;
    parameter->ux_device_class_pima_parameter_object_properties_list = NULL;

    //cb
    parameter->ux_device_class_pima_parameter_device_reset = mtp_device_reset;
    parameter->ux_device_class_pima_parameter_device_prop_desc_get = mtp_device_prop_desc_get;
    parameter->ux_device_class_pima_parameter_device_prop_value_get = mtp_device_prop_value_get;
    parameter->ux_device_class_pima_parameter_device_prop_value_set = mtp_device_prop_value_set;
    parameter->ux_device_class_pima_parameter_storage_format = mtp_storage_format;
    parameter->ux_device_class_pima_parameter_storage_info_get = mtp_storage_info_get;
    parameter->ux_device_class_pima_parameter_object_number_get = mtp_object_number_get;
    parameter->ux_device_class_pima_parameter_object_handles_get = mtp_object_handles_get;
    parameter->ux_device_class_pima_parameter_object_info_get = mtp_object_info_get;
    parameter->ux_device_class_pima_parameter_object_data_get = mtp_object_data_get;
    parameter->ux_device_class_pima_parameter_object_info_send = mtp_object_info_send;
    parameter->ux_device_class_pima_parameter_object_data_send = mtp_object_data_send;
    parameter->ux_device_class_pima_parameter_object_delete = mtp_object_delete;
    parameter->ux_device_class_pima_parameter_object_prop_desc_get = mtp_object_prop_desc_get;
    parameter->ux_device_class_pima_parameter_object_prop_value_get = mtp_object_prop_value_get;
    parameter->ux_device_class_pima_parameter_object_prop_value_set = mtp_object_prop_value_set;
    parameter->ux_device_class_pima_parameter_object_references_get = mtp_object_references_get;
    parameter->ux_device_class_pima_parameter_object_references_set = mtp_object_references_set;

    parameter->ux_device_class_pima_parameter_application = NULL;
}

void app_mtp_thread_entry(ULONG arg)
{
    HAL_PWREx_EnableUSBVoltageDetector();

    MX_USB_OTG_HS_PCD_Init();

    HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_HS, 0x200);
    HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 0, 0x80);
    HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 1, 0x174);

    UINT status = ux_dcd_stm32_initialize((ULONG)0, (ULONG)&hpcd_USB_OTG_HS);

    HAL_PCD_Start(&hpcd_USB_OTG_HS);
}


