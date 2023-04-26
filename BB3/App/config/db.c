/*
 * db.c
 *
 *  Created on: 30. 3. 2021
 *      Author: horinek
 */
#include "db.h"

#define DB_SEPARATOR            '\t'
#define DB_OLD_SEPARATOR            '='
#define DB_LINE_LEN             128
#define DB_WORK_BUFFER_SIZE     2048

typedef struct
{
    const char * path;
    osMutexId_t lock;
    int32_t fp;
} db_handle_t;

#define DB_LOCKS_SIZE    16
db_handle_t db_handles[DB_LOCKS_SIZE] = {0};
osMutexId_t db_handles_lock;

void db_init()
{
    for (uint8_t i = 0; i < DB_LOCKS_SIZE; i++)
    {
        db_handles[i].path = NULL;
        db_handles[i].lock = osMutexNew(NULL);
    }
    db_handles_lock = osMutexNew(NULL);
}

static db_handle_t * db_open(const char * path)
{
    db_handle_t * pair_free = NULL;

    osMutexAcquire(db_handles_lock, WAIT_INF);

    for (uint8_t i = 0; i < DB_LOCKS_SIZE; i++)
    {
        if (db_handles[i].path == path)
        {
            osMutexAcquire(db_handles[i].lock, WAIT_INF);
            osMutexRelease(db_handles_lock);

            return &db_handles[i];
        }
        else if (db_handles[i].path == NULL && pair_free == NULL)
        {
            pair_free = &db_handles[i];
        }
    }

    pair_free->path = path;

    FASSERT(pair_free != NULL);

    osMutexAcquire(pair_free->lock, WAIT_INF);
    osMutexRelease(db_handles_lock);

    pair_free->fp = red_open(path, RED_O_RDWR | RED_O_CREAT);
    if (pair_free->fp < 0)
    {
        pair_free->path = NULL;
        return NULL;
    }

    return pair_free;
}

static void db_close(db_handle_t * db)
{
    red_close(db->fp);
    db->path = NULL;
}

//allways close the db when using dynamicly created path
void db_close_path(const char * path)
{
    db_handle_t * db = db_open(path);
    if (db != NULL)
    {
        db_close(db);
        osMutexRelease(db->lock);
    }
}

static void db_repair(db_handle_t * db)
{
    red_lseek(db->fp, 0, RED_SEEK_SET);

    char tmp_path[PATH_LEN];
    get_tmp_filename(tmp_path);

    int32_t tp = red_open(tmp_path, RED_O_WRONLY | RED_O_CREAT | RED_O_TRUNC);

    char in_line[256];
    char out_line[256];

    while(1)
    {
        int32_t rd = red_read(db->fp, in_line, sizeof(in_line));
        if (rd == 0)
        {
            break;
        }

        uint16_t j = 0;
        for (uint16_t i = 0; i < rd; i++)
        {
            if (in_line[i] < 32 && in_line[i] != '\n' && in_line[i] != '\t')
                continue;

            out_line[j] = in_line[i];
            j++;
        }

        red_write(tp, out_line, j);
    }

    red_close(tp);
    red_close(db->fp);

    red_unlink(db->path);
    red_rename(tmp_path, db->path);

    db->fp = red_open(db->path, RED_O_RDWR | RED_O_CREAT);
}

static int32_t db_locate(db_handle_t * db, char * key, char * buff, uint16_t buffer_size)
{
    uint32_t pos = 0;
    red_lseek(db->fp, 0, RED_SEEK_SET);

    char * ret;

    while ((ret = red_gets(buff, buffer_size, db->fp)) != NULL)
    {
        if (ret == GETS_CORRUPTED)
        {
            db_repair(db);
        }

        if (strstr(buff, key) == buff
                && (buff[strlen(key)] == DB_SEPARATOR || buff[strlen(key)] == DB_OLD_SEPARATOR))
        {
            return pos;
        }

        pos = red_lseek(db->fp, 0, RED_SEEK_CUR);
    }

    return -1;
}

bool db_exists(const char * path, char * key)
{
    char buff[DB_LINE_LEN];
    bool ret = false;

    db_handle_t * db = db_open(path);
    if (db != NULL)
    {
        ret = db_locate(db, key, buff, sizeof(buff)) >= 0;

        osMutexRelease(db->lock);
    }

    return ret;
}

bool db_query(const char * path, char * key, char * value, uint16_t value_len)
{
    char buff[DB_LINE_LEN];

    int32_t pos = -1;

    db_handle_t * db = db_open(path);
    if (db != NULL)
    {
        pos = db_locate(db, key, buff, sizeof(buff));
    }
    else
    {
        ERR("Could not open file '%s', res = %d", path, db->fp);

        return false;
    }

    //line starting with key found!
    if (pos >= 0)
    {
        //remove /n
        buff[strlen(buff) - 1] = 0;
        strncpy(value, buff + strlen(key) + 1, value_len);

        osMutexRelease(db->lock);

        return true;
    }
    else
    {
        WARN("Key '%s' not found in '%s'", key, path);
    }

    osMutexRelease(db->lock);

    return false;
}

bool db_query_def(const char * path, char * key, char * value, uint16_t value_len, char * def)
{
    bool found = db_query(path, key, value, value_len);
    if (!found)
        value = def;
    return found;
}

bool db_query_int(const char * path, char * key, int16_t * value)
{
    char buff[8];

    bool ret = db_query(path, key, buff, sizeof(buff));
    if (ret)
    {
        *value = atoi(buff);
    }
    return ret;
}

bool db_query_int_def(const char * path, char * key, int16_t * value, int16_t def)
{
    bool found = db_query_int(path, key, value);
    if (!found)
        *value = def;
    return found;
}

static void db_remove_line(db_handle_t * db, uint32_t start_pos, uint16_t lenght)
{
    char * buff;
    char path_new[PATH_LEN];
    int32_t br, bw;

    red_lseek(db->fp, 0, RED_SEEK_SET);

    get_tmp_filename(path_new);
    int32_t new = red_open(path_new, RED_O_WRONLY | RED_O_CREAT | RED_O_TRUNC);
    ASSERT(new > 0);

    buff = (char *) tmalloc(DB_WORK_BUFFER_SIZE);
    ASSERT(buff != NULL);

    uint32_t pos = 0;
    while (1)
    {
        int16_t to_read = DB_WORK_BUFFER_SIZE;
        if (pos + DB_WORK_BUFFER_SIZE > start_pos)
            to_read = start_pos - pos;

        if (to_read > 0)
        {
            br = red_read(db->fp, buff, to_read);
            if (br == 0) //EOF
                break;

            bw = red_write(new, buff, br);

            ASSERT(bw == br);

            pos += to_read;
        }

        if (pos == start_pos)
        {
            pos += lenght;
            start_pos = INT32_MAX;
            red_lseek(db->fp, pos, RED_SEEK_SET);
        }
    }

    tfree(buff);

    red_close(db->fp);
    red_close(new);
    red_unlink(db->path);
    red_rename(path_new, db->path);

    db->fp = red_open(db->path, RED_O_RDWR | RED_O_CREAT);
}

void db_delete(const char * path, char * key)
{
    char buff[DB_LINE_LEN];

    db_handle_t * db = db_open(path);

    if (db != NULL)
    {
        int32_t pos = db_locate(db, key, buff, sizeof(buff));

        if (pos >= 0)
        {
            db_remove_line(db, pos, strlen(buff));
        }

        osMutexRelease(db->lock);
    }
}

void db_insert(const char * path, char * key, char * value)
{
    char buff[DB_LINE_LEN];

    db_handle_t * db = db_open(path);

    if (db != NULL)
    {
        FASSERT(db->fp > 0);
        int32_t pos = db_locate(db, key, buff, sizeof(buff));

        if (pos >= 0)
        {
            //remove '\n'
            buff[strlen(buff) - 1] = 0;

            if (strcmp(buff + strlen(key) + 1, value) == 0)
            {
                //value is the same (nothing to update)
                osMutexRelease(db->lock);

                return;
            }
            else
            {
                //value differs, remove old record
                db_remove_line(db, pos, strlen(buff) + 1);
            }
        }

        //move to end
        red_lseek(db->fp, 0, RED_SEEK_END);
        snprintf(buff, DB_LINE_LEN, "%s%c%s\n", key, DB_SEPARATOR, value);
        red_write(db->fp, buff, strlen(buff));

        osMutexRelease(db->lock);
    }
}

void db_insert_int(const char * path, char * key, int16_t value)
{
    char buff[8];
    snprintf(buff, sizeof(buff), "%d", value);
    db_insert(path, key, buff);
}

bool db_iterate(const char * path, db_callback callback)
{
    db_handle_t * db = db_open(path);

    if (db == NULL)
    {
        ERR("Could not open the file '%s'", path);
        return false;
    }

    red_lseek(db->fp, 0, RED_SEEK_SET);

    char buff[DB_LINE_LEN];
    char * ret;

    while ((ret = red_gets(buff, sizeof(buff), db->fp)) != NULL)
    {
        if (ret == GETS_CORRUPTED)
        {
            db_repair(db);
        }

        //remove \n
        buff[strlen(buff) - 1] = '\0';

        char *separator = strchr(buff, DB_SEPARATOR);
        if (separator)
        {
            *separator = '\0';
            char *key = buff;
            char *value = separator + 1;

            callback(key, value);
        }
        else
        {
            callback(buff, "");
        }
    }

    osMutexRelease(db->lock);
    return true;
}

void db_drop(const char * path)
{
    db_handle_t * db = db_open(path);

    if (db != NULL)
    {
        db_close(db);
        red_unlink(path);
        osMutexRelease(db->lock);
    }
}
