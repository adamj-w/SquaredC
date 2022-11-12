/*
 * Created on Fri Nov 11 2022
 *
 * Copyright (c) 2022 Adam Warren
 */
#include "identifier.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

size_t strhash(const char* str, size_t n) {
	const int p = 31, m = 1e9 + 7;
	size_t hash = 0;
	long p_pow = 1;
	for(size_t i = 0; i < n; i++) {
		hash = (hash + (str[i] - 'a' + 1) * p_pow) % m;
		p_pow = (p_pow * p) % m;
	}
	return hash;
}

ident_t* create_id(builtin_type type, const char* name);
void free_id(ident_t* ident);

bool id_is_equal(const ident_t* i1, const ident_t* i2);
void debug_print_id(const ident_t* ident);

ident_array_t* create_id_arr_c(size_t capacity) {
	ident_array_t* arr = malloc(sizeof(ident_array_t));
	assert(arr);

	arr->count = 0;
	arr->ids = calloc(capacity, sizeof(ident_t));
	arr->capacity = capacity;

	return arr;
}

void id_arr_resize(ident_array_t* arr, size_t new_capacity) {
	assert(arr);

	arr->ids = realloc(arr->ids, sizeof(ident_t) * new_capacity);

	arr->capacity = new_capacity;
}

void free_id_arr(ident_array_t* arr) {
	if(arr) {
		if(arr->ids) {
			for(size_t i = 0; i < arr->count; i++) {
				if(arr->ids[i].name) free(arr->ids[i].name);
			}
			free(arr->ids);
		}

		free(arr);
	}
}

size_t id_arr_find_or_create(ident_array_t* arr, builtin_type type, const char* name, size_t namelen) {
	assert(arr && name && namelen > 0);

	size_t name_hash = strhash(name, namelen);
	for(size_t i = 0; i < arr->count; ++i) {
		if(arr->ids[i].id_hash == name_hash) return i;
	}

	if(arr->count + 1 >= arr->capacity) {
		id_arr_resize(arr, arr->capacity * 2);
	}

	size_t id_index = arr->count;
	ident_t* id = &arr->ids[id_index]; arr->count++;

	id->id_hash = name_hash;
	id->name = calloc(namelen + 1, sizeof(char));
	assert(id->name);

	memcpy(id->name, name, namelen);
	id->name[namelen] = '\0';

	return id_index;
}

void remove_id(ident_array_t* arr, size_t index);

const char* id_arr_get_name(ident_array_t* arr, size_t index) {
	assert(arr && index < arr->count);

	return arr->ids[index].name;
}
