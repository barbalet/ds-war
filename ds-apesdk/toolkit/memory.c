/****************************************************************

 memory.c

 =============================================================

 Copyright 1996-2025 Tom Barbalet. All rights reserved.

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.

 This software is a continuing work of Tom Barbalet, begun on
 13 June 1996. No apes or cats were harmed in the writing of
 this software.

 ****************************************************************/

/*! \file   memory.c
 *  \brief  Covers low-level input and output related to memory.
 */

#include "toolkit.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Static variable to hold the memory execution function
static memory_execute *static_execution = 0;

// Set the memory execution function
void memory_execute_set(memory_execute *value) {
    static_execution = value;
}

// Run the memory execution function if it is set
void memory_execute_run(void) {
    if (static_execution) {
        static_execution();
    }
}

/**
 * Copies memory from one location to another.
 * This is a legacy function, as most platforms now use `memcpy`.
 * @param from Pointer to copy from.
 * @param to Pointer to copy to.
 * @param number Number of bytes to copy.
 */
void memory_copy(n_byte *from, n_byte *to, n_uint number) {
    memcpy(to, from, number);
}

/**
 * Allocates memory.
 * This is a legacy function, as most platforms now use `malloc`.
 * @param bytes Number of bytes to allocate.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */
void *memory_new(n_uint bytes) {
    void *tmp = 0L;
    if (bytes) {
        tmp = malloc(bytes);
    }
    return tmp;
}

/**
 * Frees allocated memory.
 * This is a legacy function, as most platforms now use `free`.
 * @param ptr Pointer to the memory to be freed.
 */
void memory_free(void **ptr) {
    if (*ptr != 0L) {
        free(*ptr);
        *ptr = 0L;
    }
}

/**
 * Allocates memory within a specified range.
 * @param memory_min Minimum memory size to allocate.
 * @param memory_allocated Starting memory size, adjusted to the actual allocated size.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */
void *memory_new_range(n_uint memory_min, n_uint *memory_allocated) {
    void *memory_buffer = 0L;
    do {
        memory_buffer = malloc(*memory_allocated);
        if (memory_buffer == 0L) {
            *memory_allocated = (*memory_allocated * 3) >> 2;
        }
    } while ((memory_buffer == 0L) && (*memory_allocated > memory_min));
    return memory_buffer;
}

/**
 * Erases memory by setting it to zero.
 * @param buf_offscr Pointer to the memory to erase.
 * @param nestop Number of bytes to erase.
 */
void memory_erase(n_byte *buf_offscr, n_uint nestop) {
    memset(buf_offscr, 0, nestop);
}

/**
 * Creates a new memory list.
 * @param size Size of each unit in the list.
 * @param number Number of units in the list.
 * @return Pointer to the new memory list, or NULL if allocation fails.
 */
memory_list *memory_list_new(n_uint size, n_uint number) {
    memory_list *new_list = (memory_list *)memory_new(sizeof(memory_list));
    if (new_list) {
        new_list->data = (n_byte *)memory_new(size * number);
        if (new_list->data == 0L) {
            memory_free((void **)&new_list);
            return 0L;
        }
        new_list->count = 0;
        new_list->max = number;
        new_list->unit_size = size;
    }
    return new_list;
}

/**
 * Copies data into a memory list.
 * @param list Pointer to the memory list.
 * @param data Pointer to the data to copy.
 * @param size Size of the data to copy.
 */
void memory_list_copy(memory_list *list, n_byte *data, n_uint size) {
    memory_copy(data, &(list->data[list->unit_size * list->count]), size);
    list->count += (size / list->unit_size);
    if (size % list->unit_size) {
        (void)SHOW_ERROR("Wrong base unit size");
    }

    if (list->count >= list->max) {
        n_uint new_max = list->max * 2;
        n_uint new_size = new_max * list->unit_size;
        n_byte *new_range = memory_new(new_size);
        NA_ASSERT(new_range, "Range failed to allocate");
        if (new_range) {
            memory_copy(list->data, new_range, new_size / 2);
            memory_free((void **)&(list->data));
            list->max = new_max;
            list->data = new_range;
        }
    }
}

/**
 * Frees a memory list.
 * @param value Pointer to the memory list to free.
 */
void memory_list_free(memory_list **value) {
    memory_list *list = *value;
    memory_free((void **)&(list->data));
    memory_free((void **)value);
}

/**
 * Creates a new integer list.
 * @param number Number of integers in the list.
 * @return Pointer to the new integer list, or NULL if allocation fails.
 */
int_list *int_list_new(n_uint number) {
    return (int_list *)memory_list_new(sizeof(n_int), number);
}

/**
 * Copies an integer into an integer list.
 * @param list Pointer to the integer list.
 * @param int_add Integer to copy.
 */
void int_list_copy(int_list *list, n_int int_add) {
    memory_list_copy(list, (n_byte *)&int_add, sizeof(int_add));
}

/**
 * Frees an integer list.
 * @param value Pointer to the integer list to free.
 */
void int_list_free(int_list **value) {
    memory_list_free((memory_list **)value);
}

/**
 * Finds an integer in an integer list.
 * @param list Pointer to the integer list.
 * @param location Index of the integer to find.
 * @param error Pointer to store error code.
 * @return The integer at the specified location, or an error if out of bounds.
 */
n_int int_list_find(int_list *list, n_int location, n_int *error) {
    n_int *data_int = (n_int *)list->data;

    if ((location > list->count) || (location < 0)) {
        *error = -1;
        return SHOW_ERROR("Out of bounds failure");
    }
    *error = 0;
    return data_int[location];
}

/**
 * Prints debug information for an integer list.
 * @param debug_list Pointer to the integer list to debug.
 */
void int_list_debug(int_list *debug_list) {
    n_int count = debug_list->count, loop = 0, error = 0;
    printf("count %ld max %ld\n - - - - - - - - - - - - - - - - - - -\n", debug_list->count, debug_list->max);
    while (loop < count) {
        n_int value = int_list_find(debug_list, loop, &error);
        if (error == 0) {
            if (value == BIG_INTEGER) {
                printf("| ");
            } else {
                if ((value - BIG_NEGATIVE_INTEGER) <= object_get_hash_count()) {
                    n_uint key_value = value - BIG_NEGATIVE_INTEGER;
                    printf("\n(%ld), ", key_value);
                } else {
                    printf("%ld, ", value);
                }
            }
        } else {
            printf("%ld (%ld), ", value, error);
        }
        loop++;
    }
    printf("\n");
}

/**
 * Creates a new number array list.
 * @return Pointer to the new number array list, or NULL if allocation fails.
 */
number_array_list *number_array_list_new(void) {
    return (number_array_list *)memory_list_new(sizeof(number_array), 10);
}

/**
 * Frees a number array list.
 * @param nal Pointer to the number array list to free.
 */
void number_array_list_free(number_array_list **nal) {
    memory_list_free((memory_list **)nal);
}

/**
 * Copies a number array into a number array list.
 * @param nal Pointer to the number array list.
 * @param na Pointer to the number array to copy.
 */
static void number_array_list_copy(number_array_list *nal, number_array *na) {
    memory_list_copy(nal, (n_byte *)na, sizeof(number_array));
}

/**
 * Finds a number array in a number array list.
 * @param nal Pointer to the number array list.
 * @param array Pointer to the array to find.
 * @return Pointer to the found number array, or NULL if not found.
 */
number_array *number_array_list_find(number_array_list *nal, void *array) {
    n_int loop = 0;
    number_array *checkptr = (number_array *)nal->data;
    while (loop < nal->count) {
        if (checkptr[loop].array == array) {
            return &checkptr[loop];
        }
        loop++;
    }
    return 0L;
}

/**
 * Finds or adds a number array in a number array list.
 * @param nal Pointer to the number array list.
 * @param array Pointer to the array to find or add.
 * @return Pointer to the found or added number array.
 */
number_array *number_array_list_find_add(number_array_list *nal, void *array) {
    n_int loop = 0;
    number_array *checkptr = (number_array *)nal->data;
    while (loop < nal->count) {
        if (checkptr[loop].array == array) {
            return &checkptr[loop];
        }
        loop++;
    }

    number_array *return_value = memory_new(sizeof(number_array));
    return_value->array = array;
    return_value->number = int_list_new(8);

    number_array_list_copy(nal, return_value);

    return return_value;
}

/**
 * Clears the number array if it exists.
 * @param na Pointer to the number array.
 */
void number_array_not_number(number_array *na) {
    if (na) {
        if (na->number) {
            int_list_free(&(na->number));
        }
        (void)SHOW_ERROR("Number array values not present (clear)");
    }
    (void)SHOW_ERROR("Number array not present (clear)");
}

/**
 * Copies a number into a number array.
 * @param na Pointer to the number array.
 * @param number Number to copy.
 */
void number_array_number(number_array *na, n_int number) {
    if (na) {
        if (na->number) {
            int_list_copy(na->number, number);
        } else {
            (void)SHOW_ERROR("Number array values not present (copy)");
            return;
        }
    } else {
        (void)SHOW_ERROR("Number array not present (copy)");
    }
}

/**
 * Retrieves a number from a number array.
 * @param na Pointer to the number array.
 * @param location Index of the number to retrieve.
 * @param error Pointer to store error code.
 * @return The number at the specified location, or an error if not found.
 */
n_int number_array_get_number(number_array *na, n_int location, n_int *error) {
    if (na) {
        if (na->number) {
            return int_list_find(na->number, location, error);
        }
        *error = -3;
        return SHOW_ERROR("Number array values not present");
    }
    *error = -2;
    return SHOW_ERROR("Number array not present");
}

/**
 * Retrieves the size of a number array.
 * @param na Pointer to the number array.
 * @return The size of the number array, or an error if not found.
 */
n_int number_array_get_size(number_array *na) {
    if (na) {
        if (na->number) {
            return na->number->count;
        }
        return SHOW_ERROR("Number array values not present (size)");
    }
    return SHOW_ERROR("Number array not present (size)");
}
