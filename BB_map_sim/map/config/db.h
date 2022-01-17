/*
 * db.h
 *
 *  Created on: 30. 3. 2021
 *      Author: horinek
 */

#ifndef CONFIG_DB_H_
#define CONFIG_DB_H_

#include "common.h"

void db_delete(char * path, char * key);
void db_insert(char * path, char * key, char * value);
bool db_query(char * path, char * key, char * value, uint16_t value_len);
bool db_query_int(char * path, char * key, int16_t * value);

bool db_exists(char * path, char * key);

void db_dump(char * path);

#endif /* CONFIG_DB_H_ */
