/****************************************************************
 * io.c
 * =============================================================
 * Copyright 1996-2025 Tom Barbalet. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This software is a continuing work of Tom Barbalet, begun on
 * 13 June 1996. No apes or cats were harmed in the writing of
 * this software.
 ****************************************************************/

/*! \file   io.c
 *  \brief  Handles low-level input and output operations related to memory and files.
 *          This module also serves as a placeholder for new functionality.
 */

#include "toolkit.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * Converts a string to lowercase for the specified length.
 * @param value   The string to convert.
 * @param length  The number of characters to convert.
 */
void io_lower(n_string value, n_int length) {
    for (n_int loop = 0; loop < length; loop++) {
        IO_LOWER_CHAR(value[loop]);
    }
}

/**
 * Reads a number from a string.
 * @param number_string    The string containing the number.
 * @param actual_value     Pointer to store the parsed number.
 * @param decimal_divisor  Pointer to store the decimal divisor.
 * @return Number of characters read on success, -1 on failure.
 */
n_int io_number(n_string number_string, n_int *actual_value, n_int *decimal_divisor) {
    n_uint temp = 0;
    n_int divisor = 0;
    n_int ten_power_place = 0;
    n_int string_point = 0;
    n_byte negative = 0;

    if (!number_string || number_string[0] == 0) {
        return -1;
    }

    if (number_string[0] == '-') {
        negative = 1;
        string_point++;
    }

    while (1) {
        n_char value = number_string[string_point++];
        if (value == 0) {
            *actual_value = negative ? -temp : temp;
            *decimal_divisor = divisor > 0 ? divisor - 1 : 0;
            return ten_power_place;
        }
        if (value == '.') {
            if (divisor != 0) {
                return SHOW_ERROR("Double decimal point in number");
            }
            divisor = 1;
        } else {
            if (!ASCII_NUMBER(value)) {
                return SHOW_ERROR("Number contains non-numeric value");
            }

            n_uint mod_ten = value - '0';
            if (temp == 922337203685477580) {
                if ((negative && mod_ten > 8) || (!negative && mod_ten > 7)) {
                    return SHOW_ERROR("Number out of range");
                }
            }
            if (temp > 922337203685477580) {
                return SHOW_ERROR("Number too large");
            }
            if (divisor != 0) {
                divisor++;
            }
            temp = (temp * 10) + mod_ten;
            ten_power_place++;
        }
    }
}

/**
 * Finds the length of a string up to a maximum length.
 * @param value  The string to measure.
 * @param max    The maximum allowed length.
 * @return The length of the string, or -1 if invalid.
 */
n_int io_length(n_string value, n_int max) {
    if (!value || max < 1) {
        return -1;
    }
    n_int length = 0;
    while (length < max && value[length] != 0) {
        length++;
    }
    return length;
}

/**
 * Finds a substring within a string.
 * @param check              The string to search.
 * @param from               The starting index.
 * @param max                The maximum index.
 * @param value_find         The substring to find.
 * @param value_find_length  The length of the substring.
 * @return The index of the substring if found, -1 otherwise.
 */
n_int io_find(n_string check, n_int from, n_int max, n_string value_find, n_int value_find_length) {
    n_int check_length = 0;
    for (n_int loop = from; loop < max; loop++) {
        if (check[loop] == value_find[check_length]) {
            check_length++;
            if (check_length == value_find_length) {
                return loop + 1;
            }
        } else {
            check_length = 0;
        }
    }
    return -1;
}

/**
 * Writes a string into a destination buffer.
 * @param dest    The destination buffer.
 * @param insert  The string to insert.
 * @param pos     Pointer to the current position in the buffer.
 */
void io_string_write(n_string dest, n_string insert, n_int *pos) {
    n_char character;
    n_int loop = 0;
    do {
        character = insert[loop++];
        if (character) {
            dest[(*pos)++] = character;
        }
    } while (character);
    dest[*pos] = 0;
}

/**
 * Combines three strings into a single output string.
 * @param output  The output buffer.
 * @param first   The first string.
 * @param second  The second string.
 * @param third   The third string.
 * @param count   The total length of the output buffer.
 */
void io_three_string_combination(n_string output, n_string first, n_string second, n_string third, n_int count) {
    n_int position = 0;
    io_string_write(output, " ", &position);
    io_string_write(output, first, &position);
    io_string_write(output, " ", &position);
    io_string_write(output, second, &position);

    n_int total = count - (io_length(first, STRING_BLOCK_SIZE) + io_length(second, STRING_BLOCK_SIZE) + 1);
    for (n_int loop2 = 0; loop2 < total; loop2++) {
        io_string_write(output, " ", &position);
    }
    io_string_write(output, third, &position);
}

/**
 * Converts a number to a string.
 * @param value   The buffer to store the string.
 * @param number  The number to convert.
 */
void io_number_to_string(n_string value, n_uint number) {
    n_uint temp_number = number;
    n_uint digits = 0;
    n_uint multiplier = 1;
    n_uint pos = 0;

    do {
        temp_number /= 10;
        digits++;
        if (temp_number != 0) {
            multiplier *= 10;
        }
    } while (temp_number != 0);

    do {
        value[pos++] = '0' + (number / multiplier) % 10;
        multiplier /= 10;
    } while (multiplier != 0);
    value[pos] = 0;
}

/**
 * Appends a number to a string.
 * @param output_string  The output buffer.
 * @param input_string   The input string.
 * @param number         The number to append.
 */
void io_string_number(n_string output_string, n_string input_string, n_uint number) {
    n_int input_length = io_length(input_string, STRING_BLOCK_SIZE);
    if (input_length > 0) {
        memory_copy((n_byte *)input_string, (n_byte *)output_string, (n_uint)input_length);
        io_number_to_string(&output_string[input_length], number);
    } else {
        io_number_to_string(output_string, number);
    }
}

/**
 * Combines three strings into a single output string with optional newline.
 * @param output_string  The output buffer.
 * @param first_string   The first string.
 * @param second_string  The second string.
 * @param third_string   The third string.
 * @param new_line       Whether to append a newline.
 */
void io_three_strings(n_string output_string, n_string first_string, n_string second_string, n_string third_string, n_byte new_line) {
    n_int position = 0;
    if (first_string && first_string != output_string) {
        n_int first_length = io_length(first_string, STRING_BLOCK_SIZE);
        memory_copy((n_byte *)first_string, (n_byte *)output_string, (n_uint)first_length);
        position += first_length;
    }
    if (second_string) {
        n_int second_length = io_length(second_string, STRING_BLOCK_SIZE);
        memory_copy((n_byte *)second_string, (n_byte *)&output_string[position], (n_uint)second_length);
        position += second_length;
    }
    if (third_string) {
        n_int third_length = io_length(third_string, STRING_BLOCK_SIZE);
        memory_copy((n_byte *)third_string, (n_byte *)&output_string[position], (n_uint)third_length);
        position += third_length;
    }
    if (new_line) {
#ifdef _WIN32
        output_string[position++] = 13;
#endif
        output_string[position++] = 10;
    }
    output_string[position] = 0;
}

/**
 * Creates a copy of a string.
 * @param string  The string to copy.
 * @return A newly allocated copy of the string.
 */
n_string io_string_copy(n_string string) {
    n_int length = io_length(string, STRING_BLOCK_SIZE);
    if (length < 0) {
        return NULL;
    }
    n_string copy = (n_string)memory_new(length + 1);
    memory_copy((n_byte *)string, (n_byte *)copy, length);
    copy[length] = 0;
    return copy;
}

/**
 * Copies a string into a buffer.
 * @param string  The string to copy.
 * @param buffer  The destination buffer.
 */
void io_string_copy_buffer(n_string string, n_string buffer) {
    if (!string || !buffer) {
        return;
    }
    n_int loop = 0;
    n_char character;
    do {
        character = string[loop];
        buffer[loop] = character;
        loop++;
    } while (character != 0);
}

#ifdef SIMULATED_APE_ASSERT
/**
 * Prints an assertion message.
 * @param message  The assertion message.
 * @param file_loc The file location.
 * @param line     The line number.
 */
void io_assert(n_string message, n_string file_loc, n_int line) {
    printf("Assert: %s, %s, %ld\n", message, file_loc, line);
}
#endif
