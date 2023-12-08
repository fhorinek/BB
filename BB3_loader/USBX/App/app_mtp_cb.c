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

#include "drivers/rev.h"
#include "pwr_mng.h"

#include "ux_api.h"

#undef DBG
#define DBG(...)

typedef struct {
    uint16_t property_code;
    uint16_t datatype;
    uint8_t get_set;
    uint64_t default_value[2];
    uint32_t group_code;
    uint8_t form;
} object_prop_desc_uint128_t;

typedef struct {
    uint16_t property_code;
    uint16_t datatype;
    uint8_t get_set;
    uint64_t default_value;
    uint32_t group_code;
    uint8_t form;
} object_prop_desc_uint64_t;

typedef struct {
    uint16_t property_code;
    uint16_t datatype;
    uint8_t get_set;
    uint32_t default_value;
    uint32_t group_code;
    uint8_t form;
} object_prop_desc_uint32_t;

typedef struct {
    uint16_t property_code;
    uint16_t datatype;
    uint8_t get_set;
    uint16_t default_value;
    uint32_t group_code;
    uint8_t form;
} object_prop_desc_uint16_t;

typedef struct {
    uint16_t property_code;
    uint16_t datatype;
    uint8_t get_set;
    uint8_t default_value;
    uint32_t group_code;
    uint8_t form;
} object_prop_desc_str_t;

typedef struct {
    uint16_t property_code;
    uint16_t datatype;
    uint8_t get_set;
    uint8_t default_value;
    uint8_t current_value;
    uint8_t form;
    uint8_t form_min;
    uint8_t form_max;
    uint8_t form_step;
} device_prop_desc_uint8_t;

#define MAX_HANDLES     (1024 * 50)

#define HANDLE_DELETED  0xFFFFFFFF
#define HANDLE_CLEAR    NULL

#define PATH_LEN        128

typedef struct
{
    uint32_t parent;
    char * name;
} handle_node_t;

static char file_deleted[] = "";

static handle_node_t * handle_to_filename[MAX_HANDLES];

static uint32_t handle_new_index = 1;

static int32_t mtp_file = 0;
static uint32_t mtp_file_read_handle = 0;
static uint32_t mtp_file_write_handle = 0;

static uint32_t mtp_new_object_handle = 0;
static uint32_t mtp_new_object_size = 0;

static bool mtp_cancel = false;


bool mtp_construct_path(char * path, uint32_t handle)
{

    if (handle == 0)
    {
        strcpy(path, "");
        return true;
    }

    if (handle < handle_new_index)
    {
        if (!mtp_construct_path(path, handle_to_filename[handle]->parent))
            return false;

        strcat(path, "/");
        strcat(path, handle_to_filename[handle]->name);

        return true;
    }

    return false;
}

uint32_t mtp_get_handle(uint32_t parent, char * name)
{
    for (uint32_t i = 1 ; i < handle_new_index; i++)
    {
        if (handle_to_filename[i]->parent == parent)
        {
            if (strcmp(handle_to_filename[i]->name, name) == 0)
            {
                return i;
            }
        }
    }

    //add new
    handle_node_t * n = (handle_node_t *) ps_malloc(sizeof(handle_node_t));
    n->name = (char *) ps_malloc(strlen(name) + 1);

    strcpy(n->name, name);
    n->parent = parent;

    uint32_t handle = handle_new_index;
    handle_to_filename[handle] = n;

    handle_new_index++;

    ASSERT(handle_new_index < MAX_HANDLES);

    return handle;
}

handle_node_t * mtp_handle_get_node(uint32_t handle)
{
    ASSERT(handle < MAX_HANDLES);

    return handle_to_filename[handle];
}

void mtp_rename_handle(uint32_t handle, char * name)
{
    handle_node_t * n = mtp_handle_get_node(handle);
    ps_free(n->name);
    n->name = (char *) ps_malloc(strlen(name) + 1);
    strcpy(n->name, name);
}

uint32_t mtp_handle_get_parent(uint32_t handle)
{
    ASSERT(handle < MAX_HANDLES);

    return handle_to_filename[handle]->parent;
}

//handle should not be reused if the object is removed
void mtp_handle_delete(uint32_t handle)
{
    ASSERT(handle < MAX_HANDLES);

    if (handle_to_filename[handle]->name != file_deleted)
        ps_free(handle_to_filename[handle]->name);

    handle_to_filename[handle]->name = file_deleted;
}


struct UX_SLAVE_CLASS_PIMA_STRUCT * mtp_pima = NULL;
static uint8_t mtp_buffer[1024];

bool mtp_session_is_open()
{
    if (mtp_pima != NULL)
    {
        return mtp_pima->ux_device_class_pima_session_id != 0;
    }
    return false;
}

void mtp_session_close_force()
{
    mtp_pima = NULL;
}

void mtp_activate(void * param)
{
    INFO("mtp_activate %p", param);
    mtp_pima = NULL;

    handle_new_index = 1;

    for (uint32_t i = 0; i < MAX_HANDLES; i++)
    {
        handle_to_filename[i] = HANDLE_CLEAR;
    }


}

void mtp_deactivate(void * param)
{
    INFO("mtp_deactivate %p", param);
    mtp_pima = NULL;

    for (uint32_t i = 0; i < MAX_HANDLES; i++)
    {
        if (handle_to_filename[i] != HANDLE_CLEAR)
        {
            if (handle_to_filename[i]->name != file_deleted)
                ps_free(handle_to_filename[i]->name);

            ps_free(handle_to_filename[i]);
            handle_to_filename[i] = HANDLE_CLEAR;
        }
    }

    handle_new_index = 1;
}


static UINT mtp_device_reset(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima)
{
    INFO("mtp_device_reset");
    return UX_SUCCESS;
}

static UINT mtp_device_cancel(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima)
{
    INFO("mtp_device_cancel");

    mtp_cancel = true;
    mtp_new_object_handle = 0;

    return UX_SUCCESS;
}


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
            payload->current_value = pwr.fuel_gauge.battery_percentage;
            payload->form = 0x01;
            payload->form_min = 0x00;
            payload->form_max = 0x64;
            payload->form_step = 1;

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
        case (UX_DEVICE_CLASS_PIMA_DEV_PROP_BATTERY_LEVEL):
        {
            uint8_t * bat = (uint8_t *)mtp_buffer;

            *bat = pwr.fuel_gauge.battery_percentage;
            *device_prop_value = (UCHAR *)bat;
            *device_prop_value_length = sizeof(uint8_t);
        }
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
    return UX_ERROR;
}

static UINT mtp_storage_format(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG storage_id)
{
    INFO("mtp_storage_format");

//    if (lfs_unmount(&lfs) != LFS_ERR_OK)
//        return UX_ERROR;
//
//    if (lfs_format(&lfs, lfs.cfg) != LFS_ERR_OK)
//        return UX_ERROR;
//
//    if (lfs_mount(&lfs, lfs.cfg) != LFS_ERR_OK)
//        return UX_ERROR;

    return UX_SUCCESS;
}

static UINT mtp_storage_info_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG storage_id)
{
    INFO("mtp_storage_info_get %08X", storage_id);

    if (mtp_pima == NULL)
    {
        mtp_pima = pima;
    }

    REDSTATFS stat;

    red_statvfs("", &stat);

    uint64_t max_capacity = stat.f_blocks * (uint64_t)stat.f_bsize;
    uint64_t free_space = stat.f_bfree * (uint64_t)stat.f_bsize;

    pima->ux_device_class_pima_storage_free_space_high = (free_space & 0xFFFFFFFF00000000) >> 32;
    pima->ux_device_class_pima_storage_free_space_low = free_space & 0xFFFFFFFF;

    pima->ux_device_class_pima_storage_max_capacity_high = (max_capacity & 0xFFFFFFFF00000000) >> 32;
    pima->ux_device_class_pima_storage_max_capacity_low = max_capacity & 0xFFFFFFFF;

    return UX_SUCCESS;
}

static UINT mtp_object_number_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_format_code, ULONG object_association, ULONG *object_number)
{
    INFO("mtp_object_number_get");
    return UX_ERROR;
}

static inline void mtp_port_active()
{
    pwr.data_usb_activity = HAL_GetTick();
}

extern bool development_mode;

static UINT mtp_object_handles_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handles_format_code,
        ULONG object_handles_association,
        ULONG *object_handles_array,
        ULONG object_handles_max_number)
{
	if (object_handles_association == 0xFFFFFFFF)
		object_handles_association = 0;

    INFO("mtp_object_handles_get %04X", object_handles_association);

    mtp_port_active();

    uint32_t count = 0;
    char path[PATH_LEN];

    if (mtp_construct_path(path, object_handles_association))
    {
        REDDIR * dir;

        DBG(" listing '%s'", path);
        dir = red_opendir(path);
        if (dir != NULL)
        {
            REDDIRENT * info;

            do
            {
                info = red_readdir(dir);
                if (info != NULL)
                {
                    if (info->d_name[0] == '.')
                        continue;

                    if (!development_mode && object_handles_association == 0)
                    {
                        if (strcmp(info->d_name, SYSTEM_PATH) == 0)
                            continue;
                    }

                    object_handles_array[count + 1] = mtp_get_handle(object_handles_association, info->d_name);
                    DBG("  %08X %s", object_handles_array[count + 1], info->d_name);
                    count++;
                }

            } while(info != NULL);

            red_closedir(dir);
        }
    }
    INFO(" done, count %u", count);
    object_handles_array[0] = count;


    return UX_SUCCESS;
}



static UINT mtp_object_info_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle, UX_SLAVE_CLASS_PIMA_OBJECT **object)
{
    DBG("mtp_object_info_get %08X", object_handle);

    static char path[PATH_LEN];
    REDSTAT stat;

    if (mtp_construct_path(path, object_handle))
    {
        if (mtp_new_object_handle == object_handle)
        {
            stat.st_mode = RED_S_IFREG;
            stat.st_size = mtp_new_object_size;
        }
        else
        {
            int32_t f = red_open(path, RED_O_RDONLY);
            mtp_cancel = false;
            if (f > 0)
            {
                red_fstat(f, &stat);
                red_close(f);
            }
        }

        UX_SLAVE_CLASS_PIMA_OBJECT * payload = (UX_SLAVE_CLASS_PIMA_OBJECT *)mtp_buffer;
        memset(payload, 0, sizeof(UX_SLAVE_CLASS_PIMA_OBJECT));

        if (RED_S_ISDIR(stat.st_mode))
        {
            payload->ux_device_class_pima_object_format = UX_DEVICE_CLASS_PIMA_OFC_ASSOCIATION;
            stat.st_size = 0;
        }
        else
        {
            payload->ux_device_class_pima_object_format = UX_DEVICE_CLASS_PIMA_OFC_UNDEFINED;
        }

        DBG(" %s %lu", mtp_handle_get_node(object_handle)->name, (uint32_t)stat.st_size);

        payload->ux_device_class_pima_object_storage_id = pima->ux_device_class_pima_storage_id;
        payload->ux_device_class_pima_object_protection_status = UX_DEVICE_CLASS_PIMA_OPS_NO_PROTECTION;
        payload->ux_device_class_pima_object_compressed_size = stat.st_size;
        payload->ux_device_class_pima_object_length = (uint32_t)stat.st_size;
        payload->ux_device_class_pima_object_parent_object = mtp_handle_get_parent(object_handle);

        //strings
        _ux_utility_string_to_unicode((UCHAR *)mtp_handle_get_node(object_handle)->name, payload->ux_device_class_pima_object_filename);
        payload->ux_device_class_pima_object_capture_date[0] = 0;
        payload->ux_device_class_pima_object_modification_date[0] = 0;
        payload->ux_device_class_pima_object_keywords[0] = 0;

        *object = payload;

        return UX_SUCCESS;
    }
    return UX_ERROR;
}

static UINT mtp_object_data_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle, UCHAR *object_buffer, ULONG object_offset,
        ULONG object_length_requested, ULONG *object_actual_length)
{
    INFO("mtp_object_data_get %08X %X+%X", object_handle, object_offset, object_length_requested);

    mtp_port_active();

    if (mtp_file_read_handle != object_handle)
    {
        char path[PATH_LEN];
        if (!mtp_construct_path(path, object_handle))
            return UX_ERROR;
        INFO(" path %s", path);

        if (mtp_file_read_handle != 0 || mtp_file_write_handle != 0)
        {
            INFO("Closing mtp_file");
            red_close(mtp_file);
            mtp_file_read_handle = 0;
            mtp_file_write_handle = 0;
        }

        INFO("Opening mtp_file %s for read", path);
        mtp_file = red_open(path, RED_O_RDONLY);
        if (mtp_file > 0)
        {
            mtp_file_read_handle = object_handle;
        }
    }
    else
    {
//        INFO("mtp_file is open to read");
    }


    if (mtp_file_read_handle != 0)
    {
        red_lseek(mtp_file, object_offset, RED_SEEK_SET);
        *object_actual_length = red_read(mtp_file, object_buffer, object_length_requested);

        if (*object_actual_length < 0)
            return UX_ERROR;

        return UX_SUCCESS;
    }
    return UX_ERROR;
}

static UINT mtp_object_info_send(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, UX_SLAVE_CLASS_PIMA_OBJECT *object, ULONG storage_id, ULONG parent_object_handle, ULONG *object_handle)
{
    INFO("mtp_object_info_send");

    mtp_port_active();

    char name[128];
    _ux_utility_unicode_to_string(object->ux_device_class_pima_object_filename, (UCHAR *)name);

	if (parent_object_handle == 0xFFFFFFFF)
		parent_object_handle = 0;

    INFO(" parent %08X", parent_object_handle);
    *object_handle = mtp_get_handle(parent_object_handle, name);
    INFO(" handle %08X", *object_handle);

    object->ux_device_class_pima_object_parent_object = parent_object_handle;

    char path[PATH_LEN];
    if (mtp_construct_path(path, *object_handle))
    {
        INFO(" path '%s', format %04X", path, object->ux_device_class_pima_object_format);
        if (object->ux_device_class_pima_object_format == UX_DEVICE_CLASS_PIMA_OFC_ASSOCIATION)
        {
            if (red_mkdir(path) != 0)
                return UX_ERROR;
        }
        else
        {
            mtp_new_object_handle = *object_handle;
            mtp_new_object_size = object->ux_device_class_pima_object_compressed_size;
            INFO(" size '%u'", mtp_new_object_size);
        }

        return UX_SUCCESS;
    }
    return UX_ERROR;

}

static UINT mtp_object_data_send(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle, ULONG phase, UCHAR *object_buffer, ULONG object_offset,
        ULONG object_length)
{
    INFO("mtp_object_data_send %u %08X %u+%u", phase, object_handle, object_offset, object_length);

    mtp_port_active();

    if (phase == UX_DEVICE_CLASS_PIMA_OBJECT_TRANSFER_PHASE_ACTIVE)
    {
        if (mtp_file_write_handle != object_handle)
        {
            char path[PATH_LEN];
            if (!mtp_construct_path(path, object_handle))
            {
                ERR("Unable to construct path");
                return UX_ERROR;
            }


            if (mtp_file_read_handle != 0 || mtp_file_write_handle != 0)
            {
                red_close(mtp_file);
                mtp_file_read_handle = 0;
                mtp_file_write_handle = 0;
            }

            mtp_file = red_open(path, RED_O_WRONLY | RED_O_CREAT);
            mtp_cancel = false;
            if (mtp_file > 0)
            {
                mtp_file_write_handle = object_handle;
            }
            else
            {
                ERR("Unable to open file to write %d", mtp_file);
            }
        }

        if (mtp_file_write_handle != 0)
        {
            red_lseek(mtp_file, object_offset, RED_SEEK_SET);
            int32_t wrote = red_write(mtp_file, object_buffer, object_length);

            if (wrote < 0 || wrote != object_length)
            {
                ERR("Wrong write lenght %d", wrote);
                return UX_ERROR;
            }

            return UX_SUCCESS;
        }
    }
    else
    {
        //COMPLETE OR COMPLETE ERROR
        mtp_new_object_handle = 0;

        if (mtp_file_write_handle != 0)
        {
            red_close(mtp_file);
        }

        if (mtp_cancel)
        {
            mtp_cancel = false;

            if (mtp_file_write_handle != 0)
            {
                char path[PATH_LEN];

                if (mtp_construct_path(path, mtp_file_write_handle))
                {
                    red_unlink(path);
                }
                mtp_handle_delete(mtp_file_write_handle);
            }
        }
        mtp_file_write_handle = 0;

        return UX_SUCCESS;
    }

    return UX_ERROR;
}

static UINT mtp_object_delete(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle)
{
    INFO("mtp_object_delete %08X", object_handle);

    char path[PATH_LEN];
    if (mtp_construct_path(path, object_handle))
    {
        if (mtp_file_read_handle == object_handle || mtp_file_write_handle == object_handle)
        {
            INFO("Closing mtp_file");
            red_close(mtp_file);
            mtp_file_read_handle = 0;
            mtp_file_write_handle = 0;
        }

        if (file_exists(path))
        {
            if (file_is_dir(path))
            {
                remove_dir(path);
            }
            else
            {
                red_unlink(path);
                mtp_handle_delete(object_handle);
            }

            return UX_SUCCESS;
        }
    }

    return UX_ERROR;
}




static UINT mtp_object_prop_desc_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_property, ULONG object_format, UCHAR **object_prop_dataset, ULONG *object_prop_dataset_length)
{
    DBG("mtp_object_prop_desc_get prop=%04X format=%04X", object_property, object_format);

    switch (object_property)
    {
        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_SIZE):
        {
            //every object size is 0
            object_prop_desc_uint64_t * payload = (object_prop_desc_uint64_t *)&mtp_buffer;

            payload->property_code = object_property;
            payload->datatype = UX_DEVICE_CLASS_PIMA_TYPES_UINT64;
            payload->get_set = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_VALUE_GET;
            payload->default_value = 0;
            payload->group_code = 0;
            payload->form = 0x00;

            *object_prop_dataset = (UCHAR *)payload;
            *object_prop_dataset_length = sizeof(object_prop_desc_uint64_t);
            break;
        }
        break;

        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_STORAGEID):
        {
            object_prop_desc_uint32_t * payload = (object_prop_desc_uint32_t *)&mtp_buffer;

            payload->property_code = object_property;
            payload->datatype = UX_DEVICE_CLASS_PIMA_TYPES_UINT32;
            payload->get_set = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_VALUE_GET;
            payload->default_value = pima->ux_device_class_pima_storage_id;
            payload->group_code = 0;
            payload->form = 0x00;

            *object_prop_dataset = (UCHAR *)payload;
            *object_prop_dataset_length = sizeof(object_prop_desc_uint32_t);
        }
        break;


        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FILE_NAME):
            {
                object_prop_desc_str_t * payload = (object_prop_desc_str_t *)&mtp_buffer;

                payload->property_code = object_property;
                payload->datatype = UX_DEVICE_CLASS_PIMA_TYPES_STR;
                payload->get_set = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_VALUE_GETSET;
                payload->default_value = 0;
                payload->group_code = 0;
                payload->form = 0x00;

                *object_prop_dataset = (UCHAR *)payload;
                *object_prop_dataset_length = sizeof(object_prop_desc_str_t);
                break;
            }

        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_NAME):
        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_DISPLAY_NAME):
        {
            object_prop_desc_str_t * payload = (object_prop_desc_str_t *)&mtp_buffer;

            payload->property_code = object_property;
            payload->datatype = UX_DEVICE_CLASS_PIMA_TYPES_STR;
            payload->get_set = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_VALUE_GET;
            payload->default_value = 0;
            payload->group_code = 0;
            payload->form = 0x00;

            *object_prop_dataset = (UCHAR *)payload;
            *object_prop_dataset_length = sizeof(object_prop_desc_str_t);
            break;
        }

        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PERSISTENT_UNIQUE_OBJECT_IDENTIFIER):
        {
            object_prop_desc_uint128_t * payload = (object_prop_desc_uint128_t *)&mtp_buffer;

            payload->property_code = object_property;
            payload->datatype = UX_DEVICE_CLASS_PIMA_TYPES_UINT128;
            payload->get_set = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_VALUE_GET;
            payload->default_value[0] = 0;
            payload->default_value[1] = 0;
            payload->group_code = 0;
            payload->form = 0x00;

            *object_prop_dataset = (UCHAR *)payload;
            *object_prop_dataset_length = sizeof(object_prop_desc_uint128_t);
            break;
        }

        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FORMAT):
	    {
            object_prop_desc_uint16_t * payload = (object_prop_desc_uint16_t *)&mtp_buffer;

            payload->property_code = object_property;
            payload->datatype = UX_DEVICE_CLASS_PIMA_TYPES_UINT16;
            payload->get_set = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_VALUE_GET;
            payload->default_value = UX_DEVICE_CLASS_PIMA_OFC_UNDEFINED;
            payload->group_code = 0;
            payload->form = 0x00;

            *object_prop_dataset = (UCHAR *)payload;
            *object_prop_dataset_length = sizeof(object_prop_desc_uint16_t);
        	break;
	    }

        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PARENT_OBJECT):
	    {
            object_prop_desc_uint32_t * payload = (object_prop_desc_uint32_t *)&mtp_buffer;

            payload->property_code = object_property;
            payload->datatype = UX_DEVICE_CLASS_PIMA_TYPES_UINT32;
            payload->get_set = UX_DEVICE_CLASS_PIMA_OBJECT_PROPERTY_DATASET_VALUE_GET;
            payload->default_value = 0;
            payload->group_code = 0;
            payload->form = 0x00;

            *object_prop_dataset = (UCHAR *)payload;
            *object_prop_dataset_length = sizeof(object_prop_desc_uint32_t);
        	break;
	    }

        default:
            WARN("object_property not handled");
            return UX_ERROR;
    }

    return UX_SUCCESS;
}



static UINT mtp_object_prop_value_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle, ULONG object_property, UCHAR **object_prop_value, ULONG *object_prop_value_length)
{
    DBG("mtp_object_prop_value_get %08X, %04X", object_handle, object_property);

    char path[PATH_LEN];

    switch (object_property)
    {
        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_STORAGEID):
        {
            uint32_t * id = (uint32_t *)mtp_buffer;

            *id = pima->ux_device_class_pima_storage_id;
            *object_prop_value = (UCHAR *)id;
            *object_prop_value_length = sizeof(uint32_t);
        }
        break;

        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_SIZE):
        {
            uint64_t * size = (uint64_t *)mtp_buffer;
            if (mtp_construct_path(path, object_handle))
            {
                REDSTAT stat;
                int32_t f = red_open(path, RED_O_RDONLY);
                if (f > 0)
                {
                    red_fstat(f, &stat);
                    red_close(f);

                    *size = stat.st_size;
                    *object_prop_value = (UCHAR *)size;
                    *object_prop_value_length = sizeof(uint64_t);
                }
                else
                {
                    return UX_ERROR;
                }
            }
            else
            {
                return UX_ERROR;
            }
        }
        break;

        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FILE_NAME):
        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_NAME):
        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_DISPLAY_NAME):
        {
            UCHAR * name = (UCHAR *)mtp_handle_get_node(object_handle)->name;

            _ux_utility_string_to_unicode(name, mtp_buffer);
            *object_prop_value = (UCHAR *)mtp_buffer;
            *object_prop_value_length = strlen((char *)name) * 2 + 2 + 1;
        }
        break;

        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FORMAT):
        {
            uint16_t * type = (uint16_t *)mtp_buffer;
            if (mtp_construct_path(path, object_handle))
            {
                REDSTAT stat;
                int32_t f = red_open(path, RED_O_RDONLY);
                if (f > 0)
                {
                    red_fstat(f, &stat);
                    red_close(f);

                    *type = stat.st_mode == RED_S_IFDIR ? UX_DEVICE_CLASS_PIMA_OFC_ASSOCIATION : UX_DEVICE_CLASS_PIMA_OFC_UNDEFINED;
                    *object_prop_value = (UCHAR *)type;
                    *object_prop_value_length = sizeof(uint16_t);
                }
                else
                {
                    return UX_ERROR;
                }
            }
            else
            {
                return UX_ERROR;
            }
        }
        break;

        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PERSISTENT_UNIQUE_OBJECT_IDENTIFIER):
        {
            if (mtp_construct_path(path, object_handle))
            {
                REDSTAT stat;
                int32_t f = red_open(path, RED_O_RDONLY);
                if (f > 0)
                {
                    red_fstat(f, &stat);
                    red_close(f);

                    memcpy(mtp_buffer, &stat.st_ino, 16);
                    *object_prop_value = (UCHAR *)mtp_buffer;
                    *object_prop_value_length = 16;
                }
                else
                {
                    return UX_ERROR;
                }
            }
            else
            {
                return UX_ERROR;
            }
        }
        break;

        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PARENT_OBJECT):
        {
        	uint32_t * parent = (uint32_t *)mtp_buffer;
            *parent = mtp_handle_get_node(object_handle)->parent;

            *object_prop_value = (UCHAR *)mtp_buffer;
            *object_prop_value_length = sizeof(uint32_t);
        }
        break;

        default:
            WARN("object_property not handled");
            return UX_ERROR;
    }


    return UX_SUCCESS;
}

static UINT mtp_object_prop_value_set(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle, ULONG object_property, UCHAR *object_prop_value, ULONG object_prop_value_length)
{
    INFO("mtp_object_prop_value_set %08X, %04X", object_handle, object_property);

    switch(object_property)
    {
        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_DISPLAY_NAME):
        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FILE_NAME):
        case (UX_DEVICE_CLASS_PIMA_OBJECT_PROP_NAME):
        {
            char old_path[PATH_LEN];
            mtp_construct_path(old_path, object_handle);
            char new_path[PATH_LEN];
            mtp_construct_path(new_path, mtp_handle_get_parent(object_handle));
            strcat(new_path, "/");

            char new_name[REDCONF_NAME_MAX];
            _ux_utility_unicode_to_string(object_prop_value, (UCHAR *)new_name);
            strcat(new_path, new_name);

            red_rename(old_path, new_path);
            mtp_rename_handle(object_handle, new_name);

            return UX_SUCCESS;
        }
        break;
    }

    return UX_ERROR;
}

static UINT mtp_object_references_get(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle, UCHAR **object_handle_array, ULONG *object_handle_array_length)
{
    INFO("mtp_object_references_get %08X", object_handle);

    *object_handle_array_length = 0;

    return UX_SUCCESS;
}

static UINT mtp_object_references_set(struct UX_SLAVE_CLASS_PIMA_STRUCT *pima, ULONG object_handle, UCHAR *object_handle_array, ULONG object_handle_array_length)
{
    INFO("mtp_object_references_set");
    return UX_ERROR;
}

static UCHAR manufacturer[] = "SkyBean";
static UCHAR model[] = "Strato";
static UCHAR serial[9];
static UCHAR version[32];
static UCHAR storage_desc[] = "Internal storage";
static UCHAR volume_label[] = "";

static USHORT supported_image_formats_list[] = {
        UX_DEVICE_CLASS_PIMA_OFC_UNDEFINED,
        UX_DEVICE_CLASS_PIMA_OFC_ASSOCIATION,
        0
};

static USHORT object_properties_list[] = {
        UX_DEVICE_CLASS_PIMA_OFC_UNDEFINED,
        8,
        UX_DEVICE_CLASS_PIMA_OBJECT_PROP_STORAGEID,
        UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FORMAT,
        UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_SIZE,
        UX_DEVICE_CLASS_PIMA_OBJECT_PROP_DISPLAY_NAME,
        UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FILE_NAME,
        UX_DEVICE_CLASS_PIMA_OBJECT_PROP_NAME,
        UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PARENT_OBJECT,
        UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PERSISTENT_UNIQUE_OBJECT_IDENTIFIER,

        UX_DEVICE_CLASS_PIMA_OFC_ASSOCIATION,
        8,
        UX_DEVICE_CLASS_PIMA_OBJECT_PROP_STORAGEID,
        UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FORMAT,
        UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_SIZE,
        UX_DEVICE_CLASS_PIMA_OBJECT_PROP_DISPLAY_NAME,
        UX_DEVICE_CLASS_PIMA_OBJECT_PROP_OBJECT_FILE_NAME,
        UX_DEVICE_CLASS_PIMA_OBJECT_PROP_NAME,
        UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PARENT_OBJECT,
        UX_DEVICE_CLASS_PIMA_OBJECT_PROP_PERSISTENT_UNIQUE_OBJECT_IDENTIFIER,
        0
};

static USHORT device_properties_list[] = {
        UX_DEVICE_CLASS_PIMA_DEV_PROP_BATTERY_LEVEL,
        0
};

void mtp_assign_parameters(UX_SLAVE_CLASS_PIMA_PARAMETER * parameter)
{
    snprintf((char *)serial, sizeof(serial), "%08lX", rev_get_short_id());
    rev_get_sw_string((char *)version);

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
    parameter->ux_device_class_pima_parameter_storage_volume_label = volume_label;

    //lists
    parameter->ux_device_class_pima_parameter_device_properties_list = device_properties_list;
    parameter->ux_device_class_pima_parameter_supported_capture_formats_list = UX_NULL;
    parameter->ux_device_class_pima_parameter_supported_image_formats_list = supported_image_formats_list;
    parameter->ux_device_class_pima_parameter_object_properties_list = object_properties_list;

    //cb
    parameter->ux_device_class_pima_parameter_cancel = mtp_device_cancel;
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
    HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 1, 64);
    HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 3, 64);

    UINT status = ux_dcd_stm32_initialize((ULONG)0, (ULONG)&hpcd_USB_OTG_HS);
    ASSERT(status == UX_SUCCESS)

    HAL_PCD_Start(&hpcd_USB_OTG_HS);
}


