/**
  ******************************************************************************
  * @file    usbd_mtp_if.c
  * @author  MCD Application Team
  * @brief   Source file for USBD MTP file list_files.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_mtp_if_template.h"
#include "usbd_mtp_storage.h"

#define DEBUG_LEVEL DBG_DEBUG
#include "common.h"

extern lfs_t lfs;

/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern USBD_HandleTypeDef USBD_Device;

uint32_t idx[200];
uint32_t parent;
/* static char path[255]; */
uint32_t sc_buff[MTP_IF_SCRATCH_BUFF_SZE / 4U];
uint32_t sc_len = 0U;
uint32_t pckt_cnt = 1U;
uint32_t foldsize;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static uint8_t  USBD_MTP_Itf_Init(void);
static uint8_t  USBD_MTP_Itf_DeInit(void);
static uint32_t USBD_MTP_Itf_ReadData(uint32_t Param1, uint8_t *buff, MTP_DataLengthTypeDef *data_length);
static uint16_t USBD_MTP_Itf_Create_NewObject(MTP_ObjectInfoTypeDef ObjectInfo, uint32_t objhandle);

static uint32_t USBD_MTP_Itf_GetIdx(uint32_t Param3, uint32_t *obj_handle);
static uint32_t USBD_MTP_Itf_GetParentObject(uint32_t Param);
static uint16_t USBD_MTP_Itf_GetObjectFormat(uint32_t Param);
static uint8_t  USBD_MTP_Itf_GetObjectName_len(uint32_t Param);
static void     USBD_MTP_Itf_GetObjectName(uint32_t Param, uint8_t obj_len, uint16_t *buf);
static uint32_t USBD_MTP_Itf_GetObjectSize(uint32_t Param);
static uint64_t USBD_MTP_Itf_GetMaxCapability(void);
static uint64_t USBD_MTP_Itf_GetFreeSpaceInBytes(void);
static uint32_t USBD_MTP_Itf_GetNewIndex(uint16_t objformat);
static void     USBD_MTP_Itf_WriteData(uint16_t len, uint8_t *buff);
static uint32_t USBD_MTP_Itf_GetContainerLength(uint32_t Param1);
static uint16_t USBD_MTP_Itf_DeleteObject(uint32_t Param1);

static void     USBD_MTP_Itf_Cancel(uint32_t Phase);
/* static uint32_t USBD_MTP_Get_idx_to_delete(uint32_t Param, uint8_t *tab); */

USBD_MTP_ItfTypeDef USBD_MTP_fops =
{
  USBD_MTP_Itf_Init,
  USBD_MTP_Itf_DeInit,
  USBD_MTP_Itf_ReadData,
  USBD_MTP_Itf_Create_NewObject,
  USBD_MTP_Itf_GetIdx,
  USBD_MTP_Itf_GetParentObject,
  USBD_MTP_Itf_GetObjectFormat,
  USBD_MTP_Itf_GetObjectName_len,
  USBD_MTP_Itf_GetObjectName,
  USBD_MTP_Itf_GetObjectSize,
  USBD_MTP_Itf_GetMaxCapability,
  USBD_MTP_Itf_GetFreeSpaceInBytes,
  USBD_MTP_Itf_GetNewIndex,
  USBD_MTP_Itf_WriteData,
  USBD_MTP_Itf_GetContainerLength,
  USBD_MTP_Itf_DeleteObject,
  USBD_MTP_Itf_Cancel,
  sc_buff,
  MTP_IF_SCRATCH_BUFF_SZE,
};

/* Private functions ---------------------------------------------------------*/

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

void wtf()
{

}

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

void utf_to_char(char * dst, uint16_t * src, uint16_t len)
{
    ASSERT(len > 0);
    for (uint16_t i = 0; i < len; i++)
    {
        dst[i] = src[i];
    }
}

void char_to_utf(uint16_t * dst, char * src, uint16_t len)
{
    ASSERT(len > 0);
    for (uint16_t i = 0; i < len; i++)
    {
        dst[i] = src[i];
    }
}

/**
  * @brief  USBD_MTP_Itf_Init
  *         Initialize the file system Layer
  * @param  None
  * @retval status value
  */
static uint8_t USBD_MTP_Itf_Init(void)
{
    INFO("USBD_MTP_Itf_Init");

    idx_new_index = 0;

    for (uint32_t i = 0; i < MAX_IDX_HANDLES; i++)
    {
        idx_to_filename[i] = IDX_CLEAR;
    }

    return 0;
}

/**
  * @brief  USBD_MTP_Itf_DeInit
  *         Uninitialize the file system Layer
  * @param  None
  * @retval status value
  */
static uint8_t USBD_MTP_Itf_DeInit(void)
{
    INFO("USBD_MTP_Itf_DeInit");

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

  return 0;
}

/**
  * @brief  USBD_MTP_Itf_GetIdx
  *         Get all object handle
  * @param  Param3: current object handle
  * @param  obj_handle: all objects handle (subfolders/files) in current object
  * @retval number of object handle in current object
  */
static uint32_t USBD_MTP_Itf_GetIdx(uint32_t Param3, uint32_t *obj_handle)
{
    INFO("USBD_MTP_Itf_GetIdx, %u", Param3);

    uint32_t count = 0;
    char path[PATH_LEN];

    if (construct_path(path, Param3))
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

                    obj_handle[count] = get_idx(Param3, info.name);
                    INFO("  %08X %s", obj_handle[count], info.name);
                    count++;
                }

            } while(found);

            lfs_dir_close(&lfs, &dir);
        }
    }
    INFO(" done, count %u", count);
    return count;
}

/**
  * @brief  USBD_MTP_Itf_GetParentObject
  *         Get parent object
  * @param  Param: object handle (object index)
  * @retval parent object
  */
static uint32_t USBD_MTP_Itf_GetParentObject(uint32_t Param)
{
    INFO("USBD_MTP_Itf_GetParentObject, %08X", Param);

    return get_parent(Param);
}

/**
  * @brief  USBD_MTP_Itf_GetObjectFormat
  *         Get object format
  * @param  Param: object handle (object index)
  * @retval object format
  */
static uint16_t USBD_MTP_Itf_GetObjectFormat(uint32_t Param)
{
    INFO("USBD_MTP_Itf_GetObjectFormat");
  uint16_t objformat = 0U;
  UNUSED(Param);

  return objformat;
}

/**
  * @brief  USBD_MTP_Itf_GetObjectName_len
  *         Get object name length
  * @param  Param: object handle (object index)
  * @retval object name length
  */
static uint8_t USBD_MTP_Itf_GetObjectName_len(uint32_t Param)
{
    INFO("USBD_MTP_Itf_GetObjectName_len, %08X", Param);
    uint8_t obj_len = strlen(get_path(Param)->name);

  return obj_len;
}

/**
  * @brief  USBD_MTP_Itf_GetObjectName
  *         Get object name
  * @param  Param: object handle (object index)
  * @param  obj_len: length of object name
  * @param  buf: pointer to object name
  * @retval object size in SD card
  */
static void USBD_MTP_Itf_GetObjectName(uint32_t Param, uint8_t obj_len, uint16_t *buf)
{
    INFO("USBD_MTP_Itf_GetObjectName, %04X", Param);

    char_to_utf(buf, get_path(Param)->name, obj_len);

    return;
}

/**
  * @brief  USBD_MTP_Itf_GetObjectSize
  *         Get size of current object
  * @param  Param: object handle (object index)
  * @retval object size in SD card
  */
static uint32_t USBD_MTP_Itf_GetObjectSize(uint32_t Param)
{
    INFO("USBD_MTP_Itf_GetObjectSize, %04X", Param);

    char path[PATH_LEN];
    construct_path(path, Param);
    struct lfs_info info;
    lfs_stat(&lfs, path, &info);

    return info.size;
}

/**
  * @brief  USBD_MTP_Itf_Create_NewObject
  *         Create new object in SD card and store necessary information for future use
  * @param  ObjectInfo: object information to use
  * @param  objhandle: object handle (object index)
  * @retval None
  */
static uint32_t new_idx = 0;

static uint16_t USBD_MTP_Itf_Create_NewObject(MTP_ObjectInfoTypeDef ObjectInfo, uint32_t objhandle)
{
    INFO("USBD_MTP_Itf_Create_NewObject");

    char name[sizeof(ObjectInfo.Filename)];
    utf_to_char(name, ObjectInfo.Filename, ObjectInfo.Filename_len);
    new_idx = get_idx(ObjectInfo.ParentObject, name);

    lfs_file_t file;
    char path[PATH_LEN];
    construct_path(path, new_idx);

    INFO(" Creating file %08X %s");
    lfs_file_open(&lfs, &file, path, LFS_O_RDONLY | LFS_O_CREAT | LFS_O_EXCL);

    return 0;
}

/**
  * @brief  USBD_MTP_Itf_GetMaxCapability
  *         Get max capability in SD card
  * @param  None
  * @retval max capability
  */
static uint64_t USBD_MTP_Itf_GetMaxCapability(void)
{
    INFO("USBD_MTP_Itf_GetMaxCapability");
  uint64_t max_cap = (uint64_t)lfs.cfg->block_size * (uint64_t)lfs.cfg->block_count;

  return max_cap;
}

/**
  * @brief  USBD_MTP_Itf_GetFreeSpaceInBytes
  *         Get free space in bytes in SD card
  * @param  None
  * @retval free space in bytes
  */
static uint64_t USBD_MTP_Itf_GetFreeSpaceInBytes(void)
{
    INFO("USBD_MTP_Itf_GetFreeSpaceInBytes");
    uint64_t f_space_inbytes = (uint64_t)lfs.cfg->block_size * (uint64_t)(lfs.cfg->block_count - lfs_fs_size(&lfs));

    return f_space_inbytes;
}

/**
  * @brief  USBD_MTP_Itf_GetNewIndex
  *         Create new object handle
  * @param  objformat: object format
  * @retval object handle
  */
static uint32_t USBD_MTP_Itf_GetNewIndex(uint16_t objformat)
{
    INFO("USBD_MTP_Itf_GetNewIndex");

    uint32_t n_index = new_idx;
    UNUSED(objformat);

    return n_index;
}

/**
  * @brief  USBD_MTP_Itf_WriteData
  *         Write file data to SD card
  * @param  len: size of data to write
  * @param  buff: data to write in SD card
  * @retval None
  */
static void USBD_MTP_Itf_WriteData(uint16_t len, uint8_t *buff)
{
    INFO("USBD_MTP_Itf_WriteData");

  UNUSED(len);
  UNUSED(buff);

  return;
}

/**
  * @brief  USBD_MTP_Itf_GetContainerLength
  *         Get length of generic container
  * @param  Param1: object handle
  * @retval length of generic container
  */
static uint32_t USBD_MTP_Itf_GetContainerLength(uint32_t Param1)
{
    INFO("USBD_MTP_Itf_GetContainerLength");

  uint32_t length = 0U;
  UNUSED(Param1);

  return length;
}

/**
  * @brief  USBD_MTP_Itf_DeleteObject
  *         delete object from SD card
  * @param  Param1: object handle (file/folder index)
  * @retval response code
  */
static uint16_t USBD_MTP_Itf_DeleteObject(uint32_t Param1)
{
    INFO("USBD_MTP_Itf_DeleteObject");

  uint16_t rep_code = 0U;
  UNUSED(Param1);

  return rep_code;
}

/**
  * @brief  USBD_MTP_Get_idx_to_delete
  *         Get all files/foldres index to delete with descending order ( max depth)
  * @param  Param: object handle (file/folder index)
  * @param  tab: pointer to list of files/folders to delete
  * @retval Number of files/folders to delete
  */
/* static uint32_t USBD_MTP_Get_idx_to_delete(uint32_t Param, uint8_t *tab)
{
  uint32_t cnt = 0U;

  return cnt;
}
*/

/**
  * @brief  USBD_MTP_Itf_ReadData
  *         Read data from SD card
  * @param  Param1: object handle
  * @param  buff: pointer to data to be read
  * @param  temp_length: current data size read
  * @retval necessary information for next read/finish reading
  */
static uint32_t USBD_MTP_Itf_ReadData(uint32_t Param1, uint8_t *buff, MTP_DataLengthTypeDef *data_length)
{
    INFO("USBD_MTP_Itf_ReadData");

  UNUSED(Param1);
  UNUSED(buff);
  UNUSED(data_length);

  return 0U;
}

/**
  * @brief  USBD_MTP_Itf_Cancel
  *         Close opened folder/file while cancelling transaction
  * @param  MTP_ResponsePhase: MTP current state
  * @retval None
  */
static void USBD_MTP_Itf_Cancel(uint32_t Phase)
{
    INFO("USBD_MTP_Itf_Cancel");

    UNUSED(Phase);

  /* Make sure to close open file while canceling transaction */

  return;
}
