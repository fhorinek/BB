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

static int32_t loaded_file = 0;
static char loaded_file_path[PATH_LEN] = {0};

bool db_open_read(char * path)
{
    if (strcmp(path, loaded_file_path) != 0)
    {
        if (loaded_file != 0)
            red_close(loaded_file);

        loaded_file = red_open(path, RED_O_RDONLY);
        if (loaded_file < 0)
        {
            loaded_file = 0;
            loaded_file_path[0] = 0;
            return false;
        }

        strcpy(loaded_file_path, path);
    }

    return true;
}

int32_t db_repair(int32_t fp)
{
    red_lseek(fp, 0, RED_SEEK_SET);

    char tmp_path[PATH_LEN];
    get_tmp_filename(tmp_path);

    int32_t tp = red_open(tmp_path, RED_O_WRONLY | RED_O_CREAT);

    char in_line[256];
    char out_line[256];

    while(1)
    {
        int32_t rd = red_read(fp, in_line, sizeof(in_line));
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
    red_close(fp);

    red_unlink(loaded_file_path);
    red_rename(tmp_path, loaded_file_path);

    loaded_file = red_open(loaded_file_path, RED_O_RDONLY);

    return loaded_file;
}

int32_t db_locate(int32_t fp, char * key, char * buff, uint16_t buffer_size)
{
    uint32_t pos = 0;
    red_lseek(fp, 0, RED_SEEK_SET);

    char * ret;

    while ((ret = red_gets(buff, buffer_size, fp)) != NULL)
    {
        if (ret == GETS_CORRUPTED)
        {
            fp = db_repair(fp);
        }

        if (strstr(buff, key) == buff
                && (buff[strlen(key)] == DB_SEPARATOR || buff[strlen(key)] == DB_OLD_SEPARATOR))
        {
            return pos;
        }

        pos = red_lseek(fp, 0, RED_SEEK_CUR);
    }

    return -1;
}

bool db_exists(char * path, char * key)
{
    char buff[DB_LINE_LEN];
    bool ret = false;

    if (db_open_read(path))
    {
        ret = db_locate(loaded_file, key, buff, sizeof(buff)) >= 0;
    }

    return ret;
}

bool db_query(char * path, char * key, char * value, uint16_t value_len)
{
    char buff[DB_LINE_LEN];

    int32_t pos = -1;

    if (db_open_read(path))
    {
        pos = db_locate(loaded_file, key, buff, sizeof(buff));
    }
    else
    {
        ERR("Could not open file '%s', res = %d", path, loaded_file);
    }

    //line starting with key found!
    if (pos >= 0)
    {
        //remove /n
        buff[strlen(buff) - 1] = 0;
        strncpy(value, buff + strlen(key) + 1, value_len);
        return true;
    }
    else
    {
        WARN("Key '%s' not found in '%s'", key, path);
    }

    return false;
}

bool db_query_def(char * path, char * key, char * value, uint16_t value_len, char * def)
{
    bool found = db_query(path, key, value, value_len);
    if (!found)
        value = def;
    return found;
}

bool db_query_int(char * path, char * key, int16_t * value)
{
    char buff[8];

    bool ret = db_query(path, key, buff, sizeof(buff));
    if (ret)
    {
        *value = atoi(buff);
    }
    return ret;
}

bool db_query_int_def(char * path, char * key, int16_t * value, int16_t def)
{
    bool found = db_query_int(path, key, value);
    if (!found)
        *value = def;
    return found;
}

void db_remove_line(int32_t fp, char * path, uint32_t start_pos, uint16_t lenght)
{
    char * buff;
    char path_new[PATH_LEN];
    int32_t br, bw;

    red_lseek(fp, 0, RED_SEEK_SET);
    get_tmp_filename(path_new);

    int32_t new = red_open(path_new, RED_O_WRONLY | RED_O_CREAT);
    ASSERT(new > 0);

    buff = (char *) malloc(DB_WORK_BUFFER_SIZE);
    ASSERT(buff != NULL);

    uint32_t pos = 0;
    while (1)
    {
        int16_t to_read = DB_WORK_BUFFER_SIZE;
        if (pos + DB_WORK_BUFFER_SIZE > start_pos)
            to_read = start_pos - pos;

        if (to_read > 0)
        {
            br = red_read(fp, buff, to_read);
            if (br == 0) //EOF
                break;

            bw = red_write(new, buff, br);

            ASSERT(bw == br);

            pos += to_read;
        }

        if (pos == start_pos)
        {
            pos += lenght;
            start_pos = INT16_MAX;
            red_lseek(fp, pos, RED_SEEK_SET);
        }
    }

    free(buff);

    red_close(fp);
    red_close(new);
    red_unlink(path);
    red_rename(path_new, path);
}

void db_delete(char * path, char * key)
{
    char buff[DB_LINE_LEN];
    int32_t f = red_open(path, RED_O_RDONLY);

    if (f > 0)
    {
        int32_t pos = db_locate(f, key, buff, sizeof(buff));

        if (pos >= 0)
        {
            //remove line close the file for us
            db_remove_line(f, path, pos, strlen(buff));
        }
        else
        {
            red_close(f);
        }
    }
}

void db_insert(char * path, char * key, char * value)
{
    char buff[DB_LINE_LEN];

    int32_t f = red_open(path, RED_O_RDWR | RED_O_CREAT);

    if (f > 0)
    {
        int32_t pos = db_locate(f, key, buff, sizeof(buff));

        if (pos >= 0)
        {
            //remove '\n'
            buff[strlen(buff) - 1] = 0;

            if (strcmp(buff + strlen(key) + 1, value) == 0)
            {
                //value is the same (nothing to update)
                red_close(f);
                return;
            }
            else
            {
                //value differs, remove old record
                //remove line also close the file
                db_remove_line(f, path, pos, strlen(buff) + 1);
                f = red_open(path, RED_O_WRONLY | RED_O_APPEND);
            }
        }

        //move to end
        red_lseek(f, 0, RED_SEEK_END);
        snprintf(buff, DB_LINE_LEN, "%s%c%s\n", key, DB_SEPARATOR, value);
        red_write(f, buff, strlen(buff));
        red_close(f);
    }
}

void db_insert_int(char * path, char * key, int16_t value)
{
    char buff[8];
    snprintf(buff, sizeof(buff), "%d", value);
    db_insert(path, key, buff);
}
