/*
 * Created on Fri Nov 11 2022
 *
 * Copyright (c) 2022 Adam Warren
 */
#ifndef IDENTIFIER_H
#define IDENTIFIER_H

#include "fwd.h"

typedef struct identifier_s {
	size_t id_hash;

	char* name;
	builtin_type type;
} ident_t;

typedef struct identifier_array_s {
	ident_t* ids;
	size_t count;
	size_t capacity;
} ident_array_t;

size_t strhash(const char* str, size_t n);

ident_t* create_id(builtin_type type, const char* name);
void free_id(ident_t* ident);

bool id_is_equal(const ident_t* i1, const ident_t* i2);
void debug_print_id(const ident_t* ident);

ident_array_t* create_id_arr_c(size_t capacity);
void free_id_arr(ident_array_t* arr);
size_t id_arr_find_or_create(ident_array_t* arr, builtin_type type, const char* name, size_t namelen);
void remove_id(ident_array_t* arr, size_t index);

const char* id_arr_get_name(ident_array_t* arr, size_t index);

#endif