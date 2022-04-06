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

#include "lib/littlefs/lfs.h"

extern lfs_t lfs;








#define MAX_IDX_HANDLES   128
#define IDX_DELETED 0xFFFFFFFF
#define IDX_CLEAR   NULL

#define PATH_LEN    128

typedef struct
{
    uint32_t parent;
    char * name;
} idx_path_t;

#define IDX_OFFSET      0x80000000

idx_path_t * idx_to_filename[MAX_IDX_HANDLES];
uint32_t idx_new_index = 0;

bool construct_path(char * path, uint32_t idx)
{
    idx &= ~IDX_OFFSET;

    if (idx == 0x7FFFFFFF)
    {
        strcpy(path, "");
        return true;
    }

    if (idx < idx_new_index)
    {
        if (!construct_path(path, idx_to_filename[idx]->parent))
            return false;

        strcat(path, "/");
        strcat(path, idx_to_filename[idx]->name);

        return true;
    }

    return false;
}

uint32_t get_idx(uint32_t parent, char * name)
{
    parent &= ~IDX_OFFSET;

    for (uint32_t i = 0 ; i < idx_new_index; i++)
    {
        if (idx_to_filename[i]->parent == parent)
        {
            if (strcmp(idx_to_filename[i]->name, name) == 0)
            {
                return i | IDX_OFFSET;
            }
        }
    }

    //add new
    idx_path_t * n = (idx_path_t *) ps_malloc(sizeof(idx_path_t));
    n->name = (char *) ps_malloc(strlen(name) + 1);

    strcpy(n->name, name);
    n->parent = parent;

    uint32_t idx = idx_new_index;
    idx_new_index++;
    idx_to_filename[idx] = n;
    ASSERT(idx_new_index < MAX_IDX_HANDLES);

    return idx | IDX_OFFSET;
}

idx_path_t * get_path(uint32_t idx)
{
    idx &= ~IDX_OFFSET;
    ASSERT(idx < MAX_IDX_HANDLES);

    return idx_to_filename[idx];
}

uint32_t get_parent(uint32_t idx)
{
    idx &= ~IDX_OFFSET;
    ASSERT(idx < MAX_IDX_HANDLES);

    return idx_to_filename[idx]->parent | IDX_OFFSET;
}














static uint8_t mtp_buffer[128];

void mtp_activate(void * param)
{
    INFO("mtp_activate %p", param);

    idx_new_index = 0;

    for (uint32_t i = 0; i < MAX_IDX_HANDLES; i++)
    {
        idx_to_filename[i] = IDX_CLEAR;
    }
}

void mtp_deactivate(void * param)
{
    INFO("mtp_deactivate %p", param);

    for (uint32_t i = 0; i < MAX_IDX_HANDLES; i++)
    {
        if (idx_to_filename[i] != IDX_CLEAR)
        {
            ps_free(idx_to_filename[i]->name);
            ps_free(idx_to_filename[i]);
            idx_to_filename[i] = IDX_CLEAR;
        }
    }

    idx_new_index = 0;
}


static UINT mtp_device_reset(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima)
{
    INFO("mtp_device_reset");
    return UX_SUCCESS;
}

typedef struct {
    uint16_t property_code;
    uint16_t datatype;
    uint8_t get_set;
    uint8_t default_value;
    uint8_t current_value;
    uint8_t form;
} device_prop_desc_uint8_t;


static UINT mtp_device_prop_desc_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG device_property, UCHAR **device_prop_dataset, ULONG *device_prop_dataset_length)
{
    INFO("mtp_device_prop_desc_get %04X", device_property);

    switch (device_property)
    {
        case (UX_DEVICE_CLASS_PIMA_DEV_PROP_BATTERY_LEVEL):
        {
            device_prop_desc_uint8_t * payload = (device_prop_desc_uint8_t *)&mtp_buffer;

            payload->property_code = UX_DEVICE_CLASS_PIMA_DEV_PROP_BATTERY_LEVEL;
            payload->datatype = UX_DEVICE_CLASS_PIMA_TYPES_UINT8;
            payload->get_set = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_VALUE_GET;
            payload->default_value = 0;
            payload->current_value = 50; //TODO: add real number
            payload->form = 0x00;

            *device_prop_dataset = (UCHAR *)payload;
            *device_prop_dataset_length = sizeof(device_prop_desc_uint8_t);
        }
        break;

        default:
            WARN("Not handled");
            return UX_ERROR;
    }


    return UX_SUCCESS;
}

static UINT mtp_device_prop_value_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG device_property, UCHAR **device_prop_value, ULONG *device_prop_value_length)
{
    INFO("mtp_device_prop_value_get %04X", device_property);

    switch (device_property)
    {
        case (UX_DEVICE_CLASS_PIMA_DEV_PROP_DEVICE_FRIENDLY_NAME):
            *device_prop_value_length = 0;
        break;

        default:
            WARN("device_property not handled");
            return UX_ERROR;
    }

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
    INFO("mtp_storage_info_get %08X", storage_id);

    uint64_t max_capacity = (uint64_t)lfs.cfg->block_size * (uint64_t)lfs.cfg->block_count;
    uint64_t free_space =  (uint64_t)lfs.cfg->block_size * (uint64_t)(lfs.cfg->block_count - lfs_fs_size(&lfs));

    pima->ux_device_class_pima_storage_free_space_high = (free_space & 0xFFFFFFFF00000000) >> 8;
    pima->ux_device_class_pima_storage_free_space_low = free_space & 0xFFFFFFFF;

    pima->ux_device_class_pima_storage_max_capacity_high = (max_capacity & 0xFFFFFFFF00000000) >> 8;
    pima->ux_device_class_pima_storage_max_capacity_low = max_capacity & 0xFFFFFFFF;

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
    INFO("mtp_object_handles_get %04X", object_handles_association);

    uint32_t count = 0;
    char path[PATH_LEN];

    if (construct_path(path, object_handles_association))
    {
        lfs_dir_t dir;

        INFO(" listing '%s'", path);
        if (lfs_dir_open(&lfs, &dir, path) == LFS_ERR_OK)
        {
            int found;

            do
            {
                struct lfs_info info;
                found = lfs_dir_read(&lfs, &dir, &info);
                if (found)
                {
                    if (info.name[0] == '.')
                        continue;

                    object_handles_array[count + 1] = get_idx(object_handles_association, info.name);
                    INFO("  %08X %s", object_handles_array[count + 1], info.name);
                    count++;
                }

            } while(found);

            lfs_dir_close(&lfs, &dir);
        }
    }
    INFO(" done, count %u", count);
    object_handles_array[0] = count;


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

typedef struct {
    uint16_t property_code;
    uint16_t datatype;
    uint8_t get_set;
    uint64_t default_value;
    uint64_t current_value;
    uint8_t form;
} object_prop_desc_uint64_t;

static UINT mtp_object_prop_desc_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle, ULONG object_property, UCHAR **object_prop_dataset, ULONG *object_prop_dataset_length)
{
    INFO("mtp_object_prop_desc_get %04X %04X", object_handle, object_property);

    switch (object_handle)
    {
        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_SIZE):
        {
            //every object size is 0
            object_prop_desc_uint64_t * payload = (object_prop_desc_uint64_t *)&mtp_buffer;

            payload->property_code = object_property;
            payload->datatype = UX_DEVICE_CLASS_PIMA_TYPES_UINT64;
            payload->get_set = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_VALUE_GET;
            payload->default_value = 0;
            payload->current_value = 0;
            payload->form = 0x00;

            *object_prop_dataset = (UCHAR *)payload;
            *object_prop_dataset_length = sizeof(device_prop_desc_uint8_t);
            break;
        }
        break;

        default:
            WARN("object_handle not handled");
            return UX_ERROR;
    }

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
static UCHAR storage_desc[] = "Internal storage";

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
    parameter->ux_device_class_pima_parameter_storage_free_space_image = 0xFFFFFFFF;
    parameter->ux_device_class_pima_parameter_storage_description = storage_desc;
    parameter->ux_device_class_pima_parameter_storage_volume_label = NULL;

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

    HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_HS, 1024);
    HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 1, 32);
    HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 3, 128);

    UINT status = ux_dcd_stm32_initialize((ULONG)0, (ULONG)&hpcd_USB_OTG_HS);

    HAL_PCD_Start(&hpcd_USB_OTG_HS);
}


