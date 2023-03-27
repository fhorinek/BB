/*
 * db.h
 *
 *  Created on: 30. 3. 2021
 *      Author: horinek
 */

#ifndef CONFIG_DB_H_
#define CONFIG_DB_H_

#include "common.h"

void db_delete(const char * path, char * key);

void db_insert(const char * path, char * key, char * value);
void db_insert_int(const char * path, char * key, int16_t value);

bool db_query(const char * path, char * key, char * value, uint16_t value_len);
bool db_query_def(const char * path, char * key, char * value, uint16_t value_len, char * def);

bool db_query_int(const char * path, char * key, int16_t * value);
bool db_query_int_def(const char * path, char * key, int16_t * value, int16_t def);

bool db_exists(const char * path, char * key);

typedef void (*db_callback)(char * key, char * value);
bool db_iterate(const char * path, db_callback callback);
void db_drop(const char * path);

#endif /* CONFIG_DB_H_ */
