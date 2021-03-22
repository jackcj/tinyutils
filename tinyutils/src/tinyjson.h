/*
  Copyright (c) 2009-2017 Dave Gamble and tinyJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef TINY_JSON_H
#define TINY_JSON_H

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__

/* When compiling for windows, we specify a specific calling convention to avoid issues where we are being called from a project with a different default calling convention.  For windows you have 3 define options:

TINYJSON_HIDE_SYMBOLS - Define this in the case where you don't want to ever dllexport symbols
TINYJSON_EXPORT_SYMBOLS - Define this on library build when you want to dllexport symbols (default)
TINYJSON_IMPORT_SYMBOLS - Define this if you want to dllimport symbol

For *nix builds that support visibility attribute, you can define similar behavior by

setting default visibility to hidden by adding
-fvisibility=hidden (for gcc)
or
-xldscope=hidden (for sun cc)
to CFLAGS

then using the TINYJSON_API_VISIBILITY flag to "export" the same symbols the way TINYJSON_EXPORT_SYMBOLS does

*/

#define TINYJSON_CDECL __cdecl
#define TINYJSON_STDCALL __stdcall

/* export symbols by default, this is necessary for copy pasting the C and header file */
#if !defined(TINYJSON_HIDE_SYMBOLS) && !defined(TINYJSON_IMPORT_SYMBOLS) && !defined(TINYJSON_EXPORT_SYMBOLS)
#define TINYJSON_EXPORT_SYMBOLS
#endif

#if defined(TINYJSON_HIDE_SYMBOLS)
#define TINYJSON_PUBLIC(type)   type TINYJSON_STDCALL
#elif defined(TINYJSON_EXPORT_SYMBOLS)
#define TINYJSON_PUBLIC(type)   __declspec(dllexport) type TINYJSON_STDCALL
#elif defined(TINYJSON_IMPORT_SYMBOLS)
#define TINYJSON_PUBLIC(type)   __declspec(dllimport) type TINYJSON_STDCALL
#endif
#else /* !__WINDOWS__ */
#define TINYJSON_CDECL
#define TINYJSON_STDCALL

#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined (__SUNPRO_C)) && defined(TINYJSON_API_VISIBILITY)
#define TINYJSON_PUBLIC(type)   __attribute__((visibility("default"))) type
#else
#define TINYJSON_PUBLIC(type) type
#endif
#endif

/* project version */
#define TINYJSON_VERSION_MAJOR 1
#define TINYJSON_VERSION_MINOR 7
#define TINYJSON_VERSION_PATCH 14

#include <stddef.h>

/* tinyJSON Types: */
#define tinyJSON_Invalid (0)
#define tinyJSON_False  (1 << 0)
#define tinyJSON_True   (1 << 1)
#define tinyJSON_NULL   (1 << 2)
#define tinyJSON_Number (1 << 3)
#define tinyJSON_String (1 << 4)
#define tinyJSON_Array  (1 << 5)
#define tinyJSON_Object (1 << 6)
#define tinyJSON_Raw    (1 << 7) /* raw json */

#define tinyJSON_IsReference 256
#define tinyJSON_StringIsConst 512

/* The tinyJSON structure: */
typedef struct tinyJSON
{
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    struct tinyJSON *next;
    struct tinyJSON *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    struct tinyJSON *child;

    /* The type of the item, as above. */
    int type;

    /* The item's string, if type==tinyJSON_String  and type == tinyJSON_Raw */
    char *valuestring;
    /* writing to valueint is DEPRECATED, use tinyJSON_SetNumberValue instead */
    int valueint;
    /* The item's number, if type==tinyJSON_Number */
    double valuedouble;

    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
} tinyJSON;

typedef struct tinyJSON_Hooks
{
      /* malloc/free are CDECL on Windows regardless of the default calling convention of the compiler, so ensure the hooks allow passing those functions directly. */
      void *(TINYJSON_CDECL *malloc_fn)(size_t sz);
      void (TINYJSON_CDECL *free_fn)(void *ptr);
} tinyJSON_Hooks;

typedef int tinyJSON_bool;

/* Limits how deeply nested arrays/objects can be before tinyJSON rejects to parse them.
 * This is to prevent stack overflows. */
#ifndef TINYJSON_NESTING_LIMIT
#define TINYJSON_NESTING_LIMIT 1000
#endif

/* returns the version of tinyJSON as a string */
TINYJSON_PUBLIC(const char*) tinyJSON_Version(void);

/* Supply malloc, realloc and free functions to tinyJSON */
TINYJSON_PUBLIC(void) tinyJSON_InitHooks(tinyJSON_Hooks* hooks);

/* Memory Management: the caller is always responsible to free the results from all variants of tinyJSON_Parse (with tinyJSON_Delete) and tinyJSON_Print (with stdlib free, tinyJSON_Hooks.free_fn, or tinyJSON_free as appropriate). The exception is tinyJSON_PrintPreallocated, where the caller has full responsibility of the buffer. */
/* Supply a block of JSON, and this returns a tinyJSON object you can interrogate. */
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_Parse(const char *value);
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_ParseWithLength(const char *value, size_t buffer_length);
/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
/* If you supply a ptr in return_parse_end and parsing fails, then return_parse_end will contain a pointer to the error so will match tinyJSON_GetErrorPtr(). */
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_ParseWithOpts(const char *value, const char **return_parse_end, tinyJSON_bool require_null_terminated);
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_ParseWithLengthOpts(const char *value, size_t buffer_length, const char **return_parse_end, tinyJSON_bool require_null_terminated);

/* Render a tinyJSON entity to text for transfer/storage. */
TINYJSON_PUBLIC(char *) tinyJSON_Print(const tinyJSON *item);
/* Render a tinyJSON entity to text for transfer/storage without any formatting. */
TINYJSON_PUBLIC(char *) tinyJSON_PrintUnformatted(const tinyJSON *item);
/* Render a tinyJSON entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
TINYJSON_PUBLIC(char *) tinyJSON_PrintBuffered(const tinyJSON *item, int prebuffer, tinyJSON_bool fmt);
/* Render a tinyJSON entity to text using a buffer already allocated in memory with given length. Returns 1 on success and 0 on failure. */
/* NOTE: tinyJSON is not always 100% accurate in estimating how much memory it will use, so to be safe allocate 5 bytes more than you actually need */
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_PrintPreallocated(tinyJSON *item, char *buffer, const int length, const tinyJSON_bool format);
/* Delete a tinyJSON entity and all subentities. */
TINYJSON_PUBLIC(void) tinyJSON_Delete(tinyJSON *item);

/* Returns the number of items in an array (or object). */
TINYJSON_PUBLIC(int) tinyJSON_GetArraySize(const tinyJSON *array);
/* Retrieve item number "index" from array "array". Returns NULL if unsuccessful. */
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_GetArrayItem(const tinyJSON *array, int index);
/* Get item "string" from object. Case insensitive. */
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_GetObjectItem(const tinyJSON * const object, const char * const string);
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_GetObjectItemCaseSensitive(const tinyJSON * const object, const char * const string);
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_HasObjectItem(const tinyJSON *object, const char *string);
/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when tinyJSON_Parse() returns 0. 0 when tinyJSON_Parse() succeeds. */
TINYJSON_PUBLIC(const char *) tinyJSON_GetErrorPtr(void);

/* Check item type and return its value */
TINYJSON_PUBLIC(char *) tinyJSON_GetStringValue(const tinyJSON * const item);
TINYJSON_PUBLIC(double) tinyJSON_GetNumberValue(const tinyJSON * const item);

/* These functions check the type of an item */
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_IsInvalid(const tinyJSON * const item);
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_IsFalse(const tinyJSON * const item);
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_IsTrue(const tinyJSON * const item);
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_IsBool(const tinyJSON * const item);
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_IsNull(const tinyJSON * const item);
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_IsNumber(const tinyJSON * const item);
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_IsString(const tinyJSON * const item);
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_IsArray(const tinyJSON * const item);
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_IsObject(const tinyJSON * const item);
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_IsRaw(const tinyJSON * const item);

/* These calls create a tinyJSON item of the appropriate type. */
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_CreateNull(void);
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_CreateTrue(void);
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_CreateFalse(void);
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_CreateBool(tinyJSON_bool boolean);
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_CreateNumber(double num);
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_CreateString(const char *string);
/* raw json */
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_CreateRaw(const char *raw);
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_CreateArray(void);
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_CreateObject(void);

/* Create a string where valuestring references a string so
 * it will not be freed by tinyJSON_Delete */
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_CreateStringReference(const char *string);
/* Create an object/array that only references it's elements so
 * they will not be freed by tinyJSON_Delete */
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_CreateObjectReference(const tinyJSON *child);
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_CreateArrayReference(const tinyJSON *child);

/* These utilities create an Array of count items.
 * The parameter count cannot be greater than the number of elements in the number array, otherwise array access will be out of bounds.*/
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_CreateIntArray(const int *numbers, int count);
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_CreateFloatArray(const float *numbers, int count);
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_CreateDoubleArray(const double *numbers, int count);
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_CreateStringArray(const char *const *strings, int count);

/* Append item to the specified array/object. */
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_AddItemToArray(tinyJSON *array, tinyJSON *item);
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_AddItemToObject(tinyJSON *object, const char *string, tinyJSON *item);
/* Use this when string is definitely const (i.e. a literal, or as good as), and will definitely survive the tinyJSON object.
 * WARNING: When this function was used, make sure to always check that (item->type & tinyJSON_StringIsConst) is zero before
 * writing to `item->string` */
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_AddItemToObjectCS(tinyJSON *object, const char *string, tinyJSON *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing tinyJSON to a new tinyJSON, but don't want to corrupt your existing tinyJSON. */
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_AddItemReferenceToArray(tinyJSON *array, tinyJSON *item);
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_AddItemReferenceToObject(tinyJSON *object, const char *string, tinyJSON *item);

/* Remove/Detach items from Arrays/Objects. */
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_DetachItemViaPointer(tinyJSON *parent, tinyJSON * const item);
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_DetachItemFromArray(tinyJSON *array, int which);
TINYJSON_PUBLIC(void) tinyJSON_DeleteItemFromArray(tinyJSON *array, int which);
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_DetachItemFromObject(tinyJSON *object, const char *string);
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_DetachItemFromObjectCaseSensitive(tinyJSON *object, const char *string);
TINYJSON_PUBLIC(void) tinyJSON_DeleteItemFromObject(tinyJSON *object, const char *string);
TINYJSON_PUBLIC(void) tinyJSON_DeleteItemFromObjectCaseSensitive(tinyJSON *object, const char *string);

/* Update array items. */
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_InsertItemInArray(tinyJSON *array, int which, tinyJSON *newitem); /* Shifts pre-existing items to the right. */
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_ReplaceItemViaPointer(tinyJSON * const parent, tinyJSON * const item, tinyJSON * replacement);
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_ReplaceItemInArray(tinyJSON *array, int which, tinyJSON *newitem);
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_ReplaceItemInObject(tinyJSON *object,const char *string,tinyJSON *newitem);
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_ReplaceItemInObjectCaseSensitive(tinyJSON *object,const char *string,tinyJSON *newitem);

/* Duplicate a tinyJSON item */
TINYJSON_PUBLIC(tinyJSON *) tinyJSON_Duplicate(const tinyJSON *item, tinyJSON_bool recurse);
/* Duplicate will create a new, identical tinyJSON item to the one you pass, in new memory that will
 * need to be released. With recurse!=0, it will duplicate any children connected to the item.
 * The item->next and ->prev pointers are always zero on return from Duplicate. */
/* Recursively compare two tinyJSON items for equality. If either a or b is NULL or invalid, they will be considered unequal.
 * case_sensitive determines if object keys are treated case sensitive (1) or case insensitive (0) */
TINYJSON_PUBLIC(tinyJSON_bool) tinyJSON_Compare(const tinyJSON * const a, const tinyJSON * const b, const tinyJSON_bool case_sensitive);

/* Minify a strings, remove blank characters(such as ' ', '\t', '\r', '\n') from strings.
 * The input pointer json cannot point to a read-only address area, such as a string constant, 
 * but should point to a readable and writable adress area. */
TINYJSON_PUBLIC(void) tinyJSON_Minify(char *json);

/* Helper functions for creating and adding items to an object at the same time.
 * They return the added item or NULL on failure. */
TINYJSON_PUBLIC(tinyJSON*) tinyJSON_AddNullToObject(tinyJSON * const object, const char * const name);
TINYJSON_PUBLIC(tinyJSON*) tinyJSON_AddTrueToObject(tinyJSON * const object, const char * const name);
TINYJSON_PUBLIC(tinyJSON*) tinyJSON_AddFalseToObject(tinyJSON * const object, const char * const name);
TINYJSON_PUBLIC(tinyJSON*) tinyJSON_AddBoolToObject(tinyJSON * const object, const char * const name, const tinyJSON_bool boolean);
TINYJSON_PUBLIC(tinyJSON*) tinyJSON_AddNumberToObject(tinyJSON * const object, const char * const name, const double number);
TINYJSON_PUBLIC(tinyJSON*) tinyJSON_AddStringToObject(tinyJSON * const object, const char * const name, const char * const string);
TINYJSON_PUBLIC(tinyJSON*) tinyJSON_AddRawToObject(tinyJSON * const object, const char * const name, const char * const raw);
TINYJSON_PUBLIC(tinyJSON*) tinyJSON_AddObjectToObject(tinyJSON * const object, const char * const name);
TINYJSON_PUBLIC(tinyJSON*) tinyJSON_AddArrayToObject(tinyJSON * const object, const char * const name);

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define tinyJSON_SetIntValue(object, number) ((object) ? (object)->valueint = (object)->valuedouble = (number) : (number))
/* helper for the tinyJSON_SetNumberValue macro */
TINYJSON_PUBLIC(double) tinyJSON_SetNumberHelper(tinyJSON *object, double number);
#define tinyJSON_SetNumberValue(object, number) ((object != NULL) ? tinyJSON_SetNumberHelper(object, (double)number) : (number))
/* Change the valuestring of a tinyJSON_String object, only takes effect when type of object is tinyJSON_String */
TINYJSON_PUBLIC(char*) tinyJSON_SetValuestring(tinyJSON *object, const char *valuestring);

/* Macro for iterating over an array or object */
#define tinyJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

/* malloc/free objects using the malloc/free functions that have been set with tinyJSON_InitHooks */
TINYJSON_PUBLIC(void *) tinyJSON_malloc(size_t size);
TINYJSON_PUBLIC(void) tinyJSON_free(void *object);

#ifdef __cplusplus
}
#endif

#endif  ///!TINY_JSON_H
