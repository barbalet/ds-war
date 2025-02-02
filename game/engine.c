/****************************************************************
 
    engine.c - Noble Warfare Skirmish
 
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
 
 This software and Noble Ape are a continuing work of Tom Barbalet,
 begun on 13 June 1996. No apes or cats were harmed in the writing
 of this software.
 
 ****************************************************************/


#include <stdio.h>

#include "toolkit.h"

#include "battle.h"

extern n_unit    *units;
extern n_byte2    number_units;
extern n_type   *types;
extern n_byte2  number_types;

static n_uint  no_movement = 0;

static n_byte engine_paused = 0;
static n_byte engine_new_required = 0;
static n_byte engine_debug = 0;

extern n_file * open_file_json;

static n_int engine_count = 0;

n_general_variables  game_vars;

#define    SIZEOF_MEMORY     (64*1024*1024)

static n_byte    *memory_buffer;
static n_uint   memory_allocated;
static n_uint    memory_used;

void mem_init(n_byte start) {
    if (start) {
        memory_buffer = NOTHING;
        memory_allocated = SIZEOF_MEMORY;
        
        memory_buffer = memory_new_range((SIZEOF_MEMORY/4), &memory_allocated);
    }
    memory_used = 0;
}


n_byte * mem_use(n_uint size) {
    n_byte * val = NOTHING;
    if (size > (memory_allocated - memory_used)) {
        engine_exit();
        /*plat_close();*/
    }
    val = &memory_buffer[memory_used];
    memory_used += size;
        
    return val;
}

n_byte          *local_board;


void * engine_init(n_uint random_init)
{
    
    engine_count = 0;
    
    game_vars.random0 = (n_byte2) (random_init & 0xFFFF);
    game_vars.random1 = (n_byte2) (random_init >> 16);

    printf("random (%hu , %hu)\n",game_vars.random0 ,  game_vars.random1);
    
    game_vars.attack_melee_dsq = 5;
    game_vars.declare_group_facing_dsq = 8000;
    game_vars.declare_max_start_dsq = 0xffff;
    game_vars.declare_one_to_one_dsq = 0xffff;
    game_vars.declare_close_enough_dsq = 5;
    
    mem_init(1);

    engine_new();
    
    return ((void *) local_board);
}

static n_byte sm_last = 0;
static n_int startx = -1, starty = -1, endx = -1, endy = -1;

void engine_unit(n_unit * unit, n_int startx, n_int starty, n_int endx, n_int endy)
{
    n_int loop = 0;
    n_combatant * combatants = (n_combatant *)unit->combatants;

    while (loop < unit->number_combatants)
    {
        n_int px = combatants[loop].location.x;
        n_int py = combatants[loop].location.y;

        if ((startx <= px) && (px <= endx) && (starty <= py) && (py <= endy))
        {
            unit->selected = 1;
            return;
        }
        loop++;
    }
    unit->selected = 0;
}

void engine_mouse_up(void)
{
    n_int loop = 0;
    printf("start ( %ld , %ld ) end ( %ld , %ld )\n", startx, starty, endx, endy);
    
    if ((startx != endx) && (starty != endy))
    {
        if (startx > endx)
        {
            n_int temp = endx;
            endx = startx;
            startx = temp;
        }
        if (starty > endy)
        {
            n_int temp = endy;
            endy = starty;
            starty = temp;
        }
    }
    
    startx = (startx << 10) / 800;
    starty = (starty << 10) / 800;
    
    endx = (endx << 10) / 800;
    endy = (endy << 10) / 800;
    
    while (loop < number_units)
    {
        engine_unit(&units[loop], startx, starty, endx, endy);
        loop++;
    }
    
    sm_last = 0;
    startx = -1;
    starty = -1;
    endx = -1;
    endy = -1;
}

unsigned char engine_mouse(short px, short py)
{
    if (sm_last)
    {
        endx = px;
        endy = py;
    } else {
        startx = px;
        starty = py;
        endx = px;
        endy = py;
    }
    sm_last = 1;
    return 1;
}

void engine_square_dimensions(n_vect2 * start, n_vect2 * end)
{
    start->x = startx;
    start->y = starty;

    end->x = endx;
    end->y = endy;
}

const n_string_block json_file_string = "{\"general_variables\":{\"random0\":58668,\"random1\":8717,\"attack_melee_dsq\":5,\"declare_group_facing_dsq\":8000,\"declare_max_start_dsq\":65535,\"declare_one_to_one_dsq\":65535,\"declare_close_enough_dsq\":5},\"unit_types\":[{\"defence\":4,\"melee_attack\":6,\"melee_damage\":3,\"melee_armpie\":1,\"missile_attack\":3,\"missile_damage\":4,\"missile_armpie\":1,\"missile_rate\":100,\"missile_range\":40,\"speed_maximum\":3,\"stature\":4,\"leadership\":2,\"wounds_per_combatant\":3,\"type_id\":0},{\"defence\":2,\"melee_attack\":3,\"melee_damage\":1,\"melee_armpie\":1,\"missile_attack\":0,\"missile_damage\":0,\"missile_armpie\":0,\"missile_rate\":0,\"missile_range\":0,\"speed_maximum\":4,\"stature\":1,\"leadership\":2,\"wounds_per_combatant\":2,\"type_id\":1},{\"defence\":4,\"melee_attack\":6,\"melee_damage\":2,\"melee_armpie\":2,\"missile_attack\":0,\"missile_damage\":0,\"missile_armpie\":0,\"missile_rate\":0,\"missile_range\":0,\"speed_maximum\":6,\"stature\":2,\"leadership\":4,\"wounds_per_combatant\":3,\"type_id\":2},{\"defence\":2,\"melee_attack\":7,\"melee_damage\":1,\"melee_armpie\":3,\"missile_attack\":0,\"missile_damage\":0,\"missile_armpie\":0,\"missile_rate\":0,\"missile_range\":0,\"speed_maximum\":7,\"stature\":2,\"leadership\":3,\"wounds_per_combatant\":1,\"type_id\":3}],\"units\":[{\"type_id\":0,\"width\":28,\"average\":[100,200],\"angle\":120,\"number_combatants\":480,\"alignment\":0,\"missile_number\":20},{\"type_id\":0,\"width\":28,\"average\":[100,400],\"angle\":128,\"number_combatants\":480,\"alignment\":0,\"missile_number\":20},{\"type_id\":0,\"width\":28,\"average\":[100,600],\"angle\":136,\"number_combatants\":480,\"alignment\":0,\"missile_number\":20},{\"type_id\":2,\"width\":10,\"average\":[250,400],\"angle\":128,\"number_combatants\":100,\"alignment\":0,\"missile_number\":0},{\"type_id\":2,\"width\":10,\"average\":[265,200],\"angle\":128,\"number_combatants\":100,\"alignment\":0,\"missile_number\":0},{\"type_id\":0,\"width\":50,\"average\":[500,200],\"angle\":0,\"number_combatants\":600,\"alignment\":1,\"missile_number\":20},{\"type_id\":2,\"width\":35,\"average\":[500,600],\"angle\":0,\"number_combatants\":100,\"alignment\":1,\"missile_number\":0},{\"type_id\":0,\"width\":40,\"average\":[700,600],\"angle\":0,\"number_combatants\":600,\"alignment\":1,\"missile_number\":20},{\"type_id\":3,\"width\":40,\"average\":[600,600],\"angle\":0,\"number_combatants\":200,\"alignment\":1,\"missile_number\":0}]}";


n_int engine_new(void)
{
    no_movement = 0;

    if (open_file_json == 0L)
    {
        open_file_json = io_file_new_from_string_block((n_string)json_file_string);
    }
    engine_conditions(open_file_json);

    
    return 0;
}

void engine_key_received(n_byte2 key)
{
    if ((key == 'p') || (key == 'P'))
    {
        engine_paused = ! engine_paused;
    }
    if ((key == 'n') || (key == 'N'))
    {
        engine_new_required = 1;
    }
    if ((key == 'd') || (key == 'D'))
    {
        engine_debug = ! engine_debug;
    }
}

void engine_scorecard(void)
{
    n_uint count[2] = {0};
    n_int  loop = 0;
    while (loop < number_units)
    {
        count[units[loop].alignment] += units[loop].number_living;
        loop++;
    }
    printf("%ld , %ld\n", count[0], count[1]);
    printf("random (%hu , %hu), %ld\n",game_vars.random0 ,  game_vars.random1, engine_count);
}

void engine_cycle(void)
{
    battle_loop(&battle_move, units, number_units, &game_vars);
    battle_loop(&battle_declare, units, number_units, &game_vars);
    battle_loop(&battle_attack, units, number_units, &game_vars);
    battle_loop(&battle_remove_dead, units, number_units, NOTHING);
    
    engine_count++;
}

n_byte engine_over(void)
{
    n_byte result = battle_opponent(units, number_units, &no_movement);
    
    if  (engine_debug)
    {
        engine_scorecard();
    }
    if ((result != 0) || (no_movement > 6))
    {
        printf("result %d no movement %ld\n", result, no_movement);
        return 1;
    }
    return 0;
}

n_int engine_update(void)
{
    if (engine_new_required)
    {
        engine_new();
        engine_new_required = 0;
    }
    else if (engine_paused == 0)
    {
        if (engine_over())
        {
            printf("engine_over\n");
            engine_scorecard();
            return 1;
            return engine_new();
        }
        engine_cycle();
    }
    return 0;
}


void engine_exit(void)
{
    if (open_file_json)
    {
        io_file_free(&open_file_json);
    }
    memory_free((void **)&memory_buffer);
}

