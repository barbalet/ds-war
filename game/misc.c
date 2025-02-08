/****************************************************************

    misc.c - Simulated War

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


#include <stdio.h>
#include "toolkit.h"
#include "battle.h"

#define BATTLE_JSON_LOCATION1 "./Simulated War.app/Contents/Resources/battle.json"
#define BATTLE_JSON_LOCATION2 "battle.json"
#define BATTLE_JSON_LOCATION3 "./war/game/battle.json"

extern n_general_variables game_vars;
extern n_unit *units;
extern n_byte2 number_units;
extern n_type *types;
extern n_byte2 number_types;
extern n_byte *local_board;

static n_byte engine_paused = 0;
static n_byte engine_new_required = 0;
static n_byte engine_debug = 0;
n_file *open_file_json = 0L;
static n_int engine_count = 0;

static n_object *obj_unit_type(n_type *values) {
    n_object *return_object = object_number(0L, "defence", values->defence);
    object_number(return_object, "melee_attack", values->melee_attack);
    object_number(return_object, "melee_damage", values->melee_damage);
    object_number(return_object, "melee_armpie", values->melee_armpie);
    object_number(return_object, "missile_attack", values->missile_attack);
    object_number(return_object, "missile_damage", values->missile_damage);
    object_number(return_object, "missile_armpie", values->missile_armpie);
    object_number(return_object, "missile_rate", values->missile_rate);
    object_number(return_object, "missile_range", values->missile_range);
    object_number(return_object, "speed_maximum", values->speed_maximum);
    object_number(return_object, "stature", values->stature);
    object_number(return_object, "leadership", values->leadership);
    object_number(return_object, "wounds_per_combatant", values->wounds_per_combatant);
    object_number(return_object, "type_id", values->points_per_combatant);
    return return_object;
}

static n_object *obj_unit(n_unit *values) {
    n_object *return_object = object_number(0L, "type_id", values->morale);
    n_array *average_array = array_number(values->average[0]);
    array_add(average_array, array_number(values->average[1]));
    object_number(return_object, "width", values->width);
    object_array(return_object, "average", average_array);
    object_number(return_object, "angle", values->angle);
    object_number(return_object, "number_combatants", values->number_combatants);
    object_number(return_object, "alignment", values->alignment);
    object_number(return_object, "missile_number", values->missile_number);
    return return_object;
}

static n_object *obj_general_variables(n_general_variables *values) {
    n_object *return_object = object_number(0L, "random0", values->random0);
    object_number(return_object, "random1", values->random1);
    object_number(return_object, "attack_melee_dsq", values->attack_melee_dsq);
    object_number(return_object, "declare_group_facing_dsq", values->declare_group_facing_dsq);
    object_number(return_object, "declare_max_start_dsq", values->declare_max_start_dsq);
    object_number(return_object, "declare_one_to_one_dsq", values->declare_one_to_one_dsq);
    object_number(return_object, "declare_close_enough_dsq", values->declare_close_enough_dsq);
    return return_object;
}

static n_object *obj_additional_variables(n_additional_variables *values) {
    n_object *return_object = object_number(0L, "probability_melee", values->probability_melee);
    object_number(return_object, "probability_missile", values->probability_missile);
    object_number(return_object, "damage_melee", values->damage_melee);
    object_number(return_object, "damage_missile", values->damage_missile);
    object_number(return_object, "speed_max", values->speed_max);
    object_number(return_object, "range_missile", values->range_missile);
    return return_object;
}

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number) {
#ifdef _WIN32
    LPSTR lpBuff = error_text;
    DWORD dwSize = 0;
    WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), lpBuff, lstrlen(lpBuff), &dwSize, NULL);
    OutputDebugString(error_text);
#else
    printf("ERROR: %s, %s line: %ld\n", error_text, location, line_number);
#endif
    return -1;
}

n_unit *engine_units(n_byte2 *num_units) {
    *num_units = number_units;
    return units;
}

n_file *engine_conditions_file(n_constant_string file_name) {
    n_file *file_json = io_file_new();
    if (io_disk_read_no_error(file_json, (n_string)file_name) != 0) {
        io_file_free(&file_json);
        return 0L;
    } else {
        printf("%s loaded\n", file_name);
    }
    if (open_file_json) {
        io_file_free(&open_file_json);
    }
    open_file_json = io_file_duplicate(file_json);
    return file_json;
}

n_int engine_conditions(n_file *file_json) {
    if (file_json == 0L) {
        return SHOW_ERROR("Read file failed");
    }
    number_units = 0;
    number_types = 0;
    mem_init(0);
    local_board = (n_byte *)mem_use(BATTLE_BOARD_SIZE);
    if (local_board == NOTHING) {
        return SHOW_ERROR("Local board not allocated");
    }
    memory_erase(local_board, BATTLE_BOARD_SIZE);
    board_init(local_board);
    io_whitespace_json(file_json);
    {
        n_object_type type_of;
        void *returned_blob = unknown_file_to_tree(file_json, &type_of);
        n_object *returned_object = 0L;
        if (type_of == OBJECT_OBJECT) {
            returned_object = (n_object *)returned_blob;
        }
        if (returned_object) {
            n_string str_general_variables = obj_contains(returned_object, "general_variables", OBJECT_OBJECT);
            n_string str_unit_types = obj_contains(returned_object, "unit_types", OBJECT_ARRAY);
            n_string str_units = obj_contains(returned_object, "units", OBJECT_ARRAY);
            n_object *obj_general_variables = obj_get_object(str_general_variables);
            if (str_unit_types) {
                n_array *arr_unit_types = obj_get_array(str_unit_types);
                n_array *arr_follow = 0L;
                n_int value;
                types = (n_type *)mem_use(0);
                while ((arr_follow = obj_array_next(arr_unit_types, arr_follow))) {
                    n_object *obj_follow = obj_get_object(arr_follow->data);
                    n_type *current_type = &types[number_types];
                    if (obj_contains_number(obj_follow, "defence", &value)) {
                        current_type->defence = value;
                    }
                    if (obj_contains_number(obj_follow, "melee_attack", &value)) {
                        current_type->melee_attack = value;
                    }
                    if (obj_contains_number(obj_follow, "melee_damage", &value)) {
                        current_type->melee_damage = value;
                    }
                    if (obj_contains_number(obj_follow, "melee_armpie", &value)) {
                        current_type->melee_armpie = value;
                    }
                    if (obj_contains_number(obj_follow, "missile_rate", &value)) {
                        current_type->missile_rate = value;
                    }
                    if (obj_contains_number(obj_follow, "missile_range", &value)) {
                        current_type->missile_range = value;
                    }
                    if (obj_contains_number(obj_follow, "speed_maximum", &value)) {
                        current_type->speed_maximum = value;
                    }
                    if (obj_contains_number(obj_follow, "stature", &value)) {
                        current_type->stature = value;
                    }
                    if (obj_contains_number(obj_follow, "leadership", &value)) {
                        current_type->leadership = value;
                    }
                    if (obj_contains_number(obj_follow, "wounds_per_combatant", &value)) {
                        current_type->wounds_per_combatant = value;
                    }
                    if (obj_contains_number(obj_follow, "type_id", &value)) {
                        current_type->points_per_combatant = value;
                    }
                    (void)mem_use(sizeof(n_type));
                    number_types++;
                }
                if (str_units) {
                    n_array *arr_units = obj_get_array(str_units);
                    n_array *arr_follow = 0L;
                    n_int value;
                    units = (n_unit *)mem_use(0);
                    while ((arr_follow = obj_array_next(arr_units, arr_follow))) {
                        n_object *obj_follow = obj_get_object(arr_follow->data);
                        n_unit *current_unit = &units[number_units];
                        if (obj_contains_number(obj_follow, "type_id", &value)) {
                            current_unit->morale = value;
                        }
                        if (obj_contains_number(obj_follow, "width", &value)) {
                            current_unit->width = value;
                        }
                        if (obj_contains_number(obj_follow, "angle", &value)) {
                            current_unit->angle = value;
                        }
                        if (obj_contains_number(obj_follow, "number_combatants", &value)) {
                            current_unit->number_combatants = value;
                        }
                        if (obj_contains_number(obj_follow, "alignment", &value)) {
                            current_unit->alignment = value;
                        }
                        if (obj_contains_number(obj_follow, "missile_number", &value)) {
                            current_unit->missile_number = value;
                        }
                        (void)obj_contains_array_nbyte2(obj_follow, "average", current_unit->average, 2);
                        (void)mem_use(sizeof(n_unit));
                        number_units++;
                    }
                    if (obj_general_variables) {
                        n_general_variables *values = (n_general_variables *)&game_vars;
                        n_int value;
                        if (obj_contains_number(obj_general_variables, "random0", &value)) {
                            values->random0 = value;
                        }
                        if (obj_contains_number(obj_general_variables, "random1", &value)) {
                            values->random1 = value;
                        }
                        if (obj_contains_number(obj_general_variables, "attack_melee_dsq", &value)) {
                            values->attack_melee_dsq = value;
                        }
                        if (obj_contains_number(obj_general_variables, "declare_group_facing_dsq", &value)) {
                            values->declare_group_facing_dsq = value;
                        }
                        if (obj_contains_number(obj_general_variables, "declare_max_start_dsq", &value)) {
                            values->declare_max_start_dsq = value;
                        }
                        if (obj_contains_number(obj_general_variables, "declare_one_to_one_dsq", &value)) {
                            values->declare_one_to_one_dsq = value;
                        }
                        if (obj_contains_number(obj_general_variables, "declare_close_enough_dsq", &value)) {
                            values->declare_close_enough_dsq = value;
                        }
                    }
                }
            }
            unknown_free(&returned_blob, type_of);
        }
    }
    if ((number_types == 0) || (number_units == 0) || (number_types > 255)) {
        SHOW_ERROR("Type/Unit Logic Failed");
    }
    {
        n_byte resolve[256] = {0};
        n_uint check_alignment[2] = {0};
        n_byte loop = 0;
        while (loop < number_types) {
            resolve[types[loop].points_per_combatant] = loop;
            loop++;
        }
        loop = 0;
        while (loop < number_units) {
            n_byte2 local_combatants = units[loop].number_combatants;
            units[loop].unit_type = &types[resolve[units[loop].morale]];
            units[loop].morale = 255;
            units[loop].number_living = local_combatants;
            units[loop].combatants = (n_combatant *)mem_use(sizeof(n_combatant) * local_combatants);
            check_alignment[(units[loop].alignment) & 1]++;
            loop++;
        }
        if ((check_alignment[0] == 0) || (check_alignment[1] == 0)) {
            SHOW_ERROR("Alignment Logic Failed");
        }
    }
    battle_loop(&battle_fill, units, number_units, NOTHING);
    return 0;
}
