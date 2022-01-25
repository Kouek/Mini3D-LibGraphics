#ifndef _VARARRAY_H
#define _VARARRAY_H

#include <stdlib.h>
#include <string.h>

#define VarArrayInit(T, TName) \
static struct VarArray_##TName* VarArray_##TName##_init(size_t capacity)\
{\
	struct VarArray_##TName* ret = (struct VarArray_##TName*)malloc(sizeof(struct VarArray_##TName));\
	ret->data = (T*)malloc(sizeof(T) * capacity);\
	ret->size = 0;\
	ret->capacity = capacity;\
	return ret;\
}

#define VarArrayPushBack(T, TName) \
static void VarArray_##TName##_push_back(struct VarArray_##TName* arr, T val)\
{\
	if (arr->capacity == arr->size)\
	{\
		printf("[INFO]Resize VarArray_"#TName" to %d\n", arr->capacity);\
		T* newData = (T*)malloc(sizeof(T) * arr->capacity * 2);\
		memcpy(newData, arr->data, sizeof(T) * arr->size);\
		free(arr->data);\
		arr->data = newData;\
		arr->capacity *= 2;\
		arr->data[arr->size] = val;\
		++arr->size;\
	}\
	else\
	{\
		arr->data[arr->size] = val;\
		++arr->size;\
	}\
}

#define VarArrayPopBack(T, TName) \
static T VarArray_##TName##_pop_back(struct VarArray_##TName* arr)\
{\
	--arr->size;\
	return arr->data[arr->size];\
}

#define VarArrayClear(T, TName) \
static void VarArray_##TName##_clear(struct VarArray_##TName* arr)\
{\
	arr->size = 0;\
}

#define VarArrayDestroy(T, TName) \
static void VarArray_##TName##_destroy(struct VarArray_##TName* arr)\
{\
	free(arr->data);\
}

#define VarArrayDefine(T, TName) \
struct VarArray_##TName\
{\
T* data;\
size_t size;\
size_t capacity;\
};\
VarArrayInit(T, TName)\
VarArrayPushBack(T, TName)\
VarArrayPopBack(T, TName)\
VarArrayClear(T, TName)\
VarArrayDestroy(T, TName)

// ∑∫–Õ∂®“Â
#if(1)
VarArrayDefine(unsigned int, unsigned_int)
#endif

#undef VarArrayInit
#undef VarArrayPushBack
#undef VarArrayPopBack
#undef VarArrayClear
#undef VarArrayDestroy
#undef VarArrayDefine

#endif // !_VARARRAY_H
