/*
 * Created on Fri Nov 11 2022
 *
 * Copyright (c) 2022 Adam Warren
 */
#include "identifier.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

size_t strhash(const char* str, size_t n) {
	if(n == 0) {
		n = strlen(str);
	}

	const int p = 31, m = 1e9 + 7;
	size_t hash = 0;
	long p_pow = 1;
	for(size_t i = 0; i < n; i++) {
		hash = (hash + (str[i] - 'a' + 1) * p_pow) % m;
		p_pow = (p_pow * p) % m;
	}
	return hash;
}

char* cpystr(const char* str) {
	size_t len = strlen(str);
	char* out = calloc(len + 1, sizeof(char));
	strncpy(out, str, len);
	out[len] = '\0';
	return out;
}

ident_t* create_id(builtin_type type, const char* name);
void free_id(ident_t* ident);

void id_set_type(ident_t* id, builtin_type type) {
	id->type = type;
	// TODO: change hash?
}

bool id_is_equal(const ident_t* i1, const ident_t* i2);

void debug_print_id(const ident_t* ident) {
	printf("\t%#016zx: %s %s\n", ident->id_hash, builtin_type_names[ident->type], ident->name);
}

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

int id_arr_find(ident_array_t* arr, size_t hash) {
	assert(arr);

	for(size_t i = 0; i < arr->count; i++) {
		if(hash == arr->ids[i].id_hash) return (int)i;
	}

	return -1;
}

ident_t* id_arr_find_p(ident_array_t* arr, size_t hash) {
	assert(arr);

	for(size_t i = 0; i < arr->count; i++) {
		if(hash == arr->ids[i].id_hash) return &arr->ids[i];
	}

	return NULL;
}

size_t id_arr_find_or_create_substr_i(ident_array_t* arr, builtin_type type, const char* name, size_t namelen) {
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

ident_t* id_arr_fcreate_cstr_p(ident_array_t* arr, builtin_type type, const char* name) {
	assert(arr && name);

	size_t hash = strhash(name, 0);

	if(id_arr_find(arr, hash) != -1) return NULL;

	if(arr->count + 1 >= arr->capacity) {
		id_arr_resize(arr, arr->capacity * 2);
	}

	size_t id_index = arr->count;
	ident_t* id = &arr->ids[id_index]; arr->count++;

	id->name = cpystr(name);
	id->id_hash = hash;
	id->type = type;
	return id;
}

void remove_id(ident_array_t* arr, size_t index) {
	assert(arr && index < arr->count);

	if(arr->ids[index].name) free(arr->ids[index].name);

	for(size_t i = index; i < arr->count - 1; i++) {
		memcpy(&arr->ids[i], &arr->ids[i + 1], sizeof(ident_t));
	}

	arr->count--;
}

const char* id_arr_get_name(ident_array_t* arr, size_t index) {
	assert(arr && index < arr->count);

	return arr->ids[index].name;
}

void id_arr_debug_print(ident_array_t* arr) {
	assert(arr);

	printf("ids: count: %zu, cap: %zu\n", arr->count, arr->capacity);

	if(arr->ids) {
		for(size_t i = 0; i < arr->count; i++) {
			debug_print_id(&arr->ids[i]);
		}
	}

	printf("\n\n");
}
