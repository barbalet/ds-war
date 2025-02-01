/****************************************************************
 
	battle.c - Noble Warfare Skirmish
 
 =============================================================
 
 Copyright 1996-2024 Tom Barbalet. All rights reserved.
 
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

#ifdef	_WIN32
#include "toolkit.h"
#else
#include "toolkit.h"
#endif

#include "battle.h"


static n_byte    *board = NOTHING;

#define        XY_BOARD(pt)        board[(pt->x) | ((pt->y) * BATTLE_BOARD_WIDTH)]

void board_init(n_byte * value)
{
    board = value;
}

static n_int board_location_check(n_vect2 * pt)
{
    if (board == NOTHING)
    {
        return SHOW_ERROR("board not initialized");
    }
    
    if (OUTSIDE_HEIGHT(pt->y) || OUTSIDE_WIDTH(pt->x))
    {
        return SHOW_ERROR("point out of bounds");
    }
    
    return 0;
}

static void board_fill(n_vect2 * pt, n_byte number)
{
    if (board_location_check(pt) == -1)
    {
        return;
    }
    XY_BOARD(pt) = number;
}

n_byte board_clear(n_vect2 * pt)
{
    n_byte value;
    if (board_location_check(pt) == -1)
    {
        return 0;
    }
    value = XY_BOARD(pt);
    
    XY_BOARD(pt) = 0;
    
    return value;
}

static n_int board_occupied(n_vect2 * pt)
{
    if (board_location_check(pt) == -1)
    {
        return 1;
    }
    return (XY_BOARD(pt) > 127);
}

static    n_byte    board_find(n_vect2 * pt) {
    n_uint    best_dsqu = BIG_INTEGER;
    n_int    best_x = 0, best_y = 0;
    n_int ly = -1;
    
    pt->x = (pt->x + BATTLE_BOARD_WIDTH) % BATTLE_BOARD_WIDTH;
    pt->y = (pt->y + BATTLE_BOARD_HEIGHT) % BATTLE_BOARD_HEIGHT;
    
    if (board_occupied(pt)==0) {
        return 1;
    }
    while (ly < 2)
    {
        n_int    lx = -1;
        n_int y_val = (pt->y + ly + BATTLE_BOARD_HEIGHT) % BATTLE_BOARD_HEIGHT;
        while (lx < 2)
        {
            n_int x_val = (pt->x + lx + BATTLE_BOARD_WIDTH) % BATTLE_BOARD_WIDTH;
            n_vect2 value;
            value.x = x_val;
            value.y = y_val;
            if (board_occupied(&value)==0) {
                n_int dx = (pt->x - lx);
                n_int dy = (pt->y - ly);
                n_uint    dsqu = (dx*dx) + (dy*dy);
                if (dsqu < best_dsqu) {
                    best_dsqu = dsqu;
                    best_x = x_val;
                    best_y = y_val;
                }
            }
            lx++;
        }
        ly++;
    }
    if (best_dsqu != BIG_INTEGER) {
        pt->x = best_x;
        pt->y = best_y;
        return 1;
    }
    return 0;
}

n_byte    board_add(n_vect2 * pt, n_byte color) {
    if (board_find(pt))
    {
        board_fill(pt, color);
        return 1;
    }
    return 0;
}

n_byte    board_move(n_vect2 * fr, n_vect2 * pt) {
    if (board_location_check(pt) == -1)
    {
        return 0;
    }
    if (board_find(pt))
    {
        n_byte color = board_clear(fr);
        board_fill(pt, color);
        return 1;
    }
    return 0;
}


void  combatant_loop(combatant_function func, n_unit * un, n_general_variables * gvar, void * values)
{
    n_combatant * combatant = (n_combatant *)(un->combatants);
    n_byte2 loop = 0;
    while (loop < un->number_combatants) {
        (*func)((&combatant[loop++]), gvar, values);
    }
}

static void battle_area(n_unit * unit)
{
    n_combatant * combatant = (n_combatant *)unit->combatants;
    n_int loop = 0;
    while (loop < unit->number_combatants)
    {
        area2_add(&(unit->area), &(combatant->location), loop == 0);
        loop++;
    }
}

void  battle_loop(battle_function func, n_unit * un, const n_uint count , n_general_variables * gvar)
{
	n_uint loop = 0;
	while (loop < count)
		(*func)((&un[loop++]), gvar);
}

n_byte battle_alignment_color(n_unit * un)
{
    if (un->alignment == 0)
    {
        return 128;
    }
    return 255;
}

typedef struct
{
    n_vect2 px;
    n_vect2 py;
    n_vect2 dpx;
    n_vect2 dpy;
    n_int   edgex;
    n_int   edgey;
    n_byte  color;
    n_int   loc_angle;
    n_byte  loc_wounds;
    n_int   line;
    n_int   loc_width;
} battle_fill_struct;


void combatant_fill(n_combatant * comb, n_general_variables * gvar, void * values)
{
    battle_fill_struct * local_bfs = (battle_fill_struct *)values;
 
    n_int	pos_x = ((((local_bfs->px.x + local_bfs->py.x) >> 9) + local_bfs->edgex) % BATTLE_BOARD_WIDTH);
    n_int   pos_y = ((((local_bfs->px.y - local_bfs->py.y) >> 9) + local_bfs->edgey) % BATTLE_BOARD_HEIGHT);
    
    n_vect2 pos;
    
    pos.x = pos_x;
    pos.y = pos_y;
    
    if (board_add(&pos, local_bfs->color))
    {
        comb->location.x = pos.x;
        comb->location.y = pos.y;
        comb->direction_facing = (n_byte)local_bfs->loc_angle;
        comb->attacking = NUNIT_NO_ATTACK;
        comb->wounds = local_bfs->loc_wounds;
        comb->speed_current = 0;
    }
    
    local_bfs->line ++;
    
    if (local_bfs->line == local_bfs->loc_width)
    {
        local_bfs->line = 0;
        vect2_populate(&local_bfs->px, 0, 0);
        vect2_d(&local_bfs->py, &local_bfs->dpy, 1, 1);
    }
    else
    {
        vect2_d(&local_bfs->px, &local_bfs->dpx, 1, 1);
    }
}


void battle_fill(n_unit * un, n_general_variables * gvar)
{
    battle_fill_struct local_bfs;
    n_int   dx = (UNIT_SIZE(un)+2)/2;
    n_int   dy = (UNIT_SIZE(un)+3)/2;
    n_int   loc_angle  = (un->angle);
	n_int   loc_order  = UNIT_ORDER(un);
    n_int   loc_height;
    n_vect2 facing;

	local_bfs.loc_wounds = GET_TYPE(un)->wounds_per_combatant;
    local_bfs.color = battle_alignment_color(un);
    
	local_bfs.loc_width  = (un->width);
    
    local_bfs.line = 0;
    
    local_bfs.px.x = 0;
    local_bfs.px.y = 0;
    
    local_bfs.py.x = 0;
    local_bfs.py.y = 0;
    
    vect2_direction(&facing, loc_angle, 16);
    
	if (local_bfs.loc_width > un->number_combatants)
    {
		local_bfs.loc_width = un->number_combatants;
	}
    
    NA_ASSERT(local_bfs.loc_width , "width is zero");
    
    loc_height = (un->number_combatants + local_bfs.loc_width - (un->number_combatants % local_bfs.loc_width)) / local_bfs.loc_width;
    
    if ((loc_order & 1) == 1)
    {
		if (dx == dy)
        {
			dx += 1;
			dy += 1;
		}
        else
        {
			dx = dy;
		}
	}
	
    vect2_populate(&local_bfs.dpx, (facing.y * dx), (facing.x * dx));
    vect2_populate(&local_bfs.dpy, (facing.x * dy), (facing.y * dy));
    
	dx = (local_bfs.loc_width  * dx);
	dy = (loc_height * dy);
	
	local_bfs.edgex = (un->average[0]) - (((facing.y * dx) + (facing.x * dy)) >> 10);
	local_bfs.edgey = (un->average[1]) - (((facing.x * dx) - (facing.y * dy)) >> 10);
    
    combatant_loop(&combatant_fill, un, gvar, (void*)&local_bfs);
    battle_area(un);
}


static n_int battle_calc_damage(n_int wounds, n_int damage)
{
	wounds -= damage;
    if (wounds < 1) {
		wounds = 0;
    }
	return wounds;
}

static void battle_combatant_attack(n_combatant * comb, n_combatant * comb_at,
                                    n_general_variables * gvar, void *additional_variables){
    	/* which opponent combatant are they attacking */
	const n_byte2	loc_attacking = comb->attacking;
	/* what is the squared distance between them */
	const n_int		distance_squared = comb->distance_squ;
	/* roll the 1024 sided dice */
	n_int			dice_roll = math_random(&gvar->random0) & 1023;
    
	/* if the attacking combatant isn't dead */
	if (comb->wounds == NUNIT_DEAD) {
		return;
	}
	/* are they attacking a unit? */
	if (loc_attacking == NUNIT_NO_ATTACK) {
		return;
	}
    
	comb_at = &comb_at[loc_attacking];
    {
        n_additional_variables * av = (n_additional_variables*)additional_variables;
    
        /* if the distance is within the melee range */
        if (distance_squared < gvar->attack_melee_dsq) {	/* val1*/
            /* stop if melee attacking */
            comb->speed_current = 0;
            /* melee hit */
            if (dice_roll < av->probability_melee) {
                comb_at->wounds = (n_byte) battle_calc_damage(comb_at->wounds, av->damage_melee);
            }
            /* else if the distance is within the missle range */
        } else if (distance_squared < av->range_missile) {
            /* missile hit */
            if (dice_roll < av->probability_missile) {
                comb_at->wounds = (n_byte) battle_calc_damage(comb_at->wounds,av->damage_missile);
            }
        } else {
            comb->speed_current = (n_byte)av->speed_max;
        }
    }
}


void battle_attack(n_unit *un, n_general_variables * gvar) {
    
	n_additional_variables		additional_variables;
	n_uint		loop = 0;
	/* the combatants doing the attacking */
	n_combatant  *comb = (n_combatant *)(un->combatants);
	n_combatant  *comb_at;
    
	/* only worthwhile if the unit is attacking something */
	if (un->unit_attacking == NOTHING) {
		return;
	}
    
	{
		/* the range of the missile weapons */
		n_int	  rang_missile = 0;
		n_type   *typ = un->unit_type;
		/* the unit that is being attacked */
		n_unit	 *un_at =  un->unit_attacking;
		n_type   *typ_at = un_at->unit_type;
		/* the combatants being attacked */
		
		comb_at = un_at->combatants;
        
		/* the probabilities of causing damage to the attacked unit */
        
		additional_variables.probability_melee = (typ->melee_attack) * (16 - (typ_at->defence) + (typ->melee_armpie));
		additional_variables.probability_missile = (typ->missile_attack) * (16 - (typ_at->defence) + (typ->missile_armpie));
        
		additional_variables.damage_melee = typ->melee_damage;
		additional_variables.damage_missile = typ->missile_damage;
        
		additional_variables.speed_max = typ->speed_maximum;
        
		if (un->missile_number != 0) {
			if (un->missile_timer == typ->missile_rate) {
				rang_missile = typ->missile_range;
				rang_missile *= rang_missile;
				un->missile_number--;
				un->missile_timer = 0;
			} else
				un->missile_timer++;
		}
        
		additional_variables.range_missile = rang_missile;
        
	}
	while (loop < un->number_combatants) {
		battle_combatant_attack(&comb[loop],comb_at,gvar,(void*)&additional_variables);
		loop++;
	}
}

static n_int combatant_random_facing(n_int local_facing, n_general_variables * gvar)
{
    switch (math_random(&gvar->random0) & 31) {
    
        case 1:
            return (local_facing + 1) & 255;
        case 2:
            return (local_facing + 255) & 255;
        case 3:
            return (local_facing + 2) & 255;
    }
    return (local_facing + 254) & 255;
}


static void battle_combatant_declare(n_combatant * comb, n_general_variables * gvar,
                                     n_unit * un_at, n_byte reverso, n_byte group_facing){
    
	/* if the attacking combatant isn't dead */
	/* cache the important values locally */
	n_int	loc_f = comb->direction_facing;
	/* the initial condition sets up "nothing to attack" and the maximum distance to attack squared */
	n_byte2 loc_attack = NUNIT_NO_ATTACK;
	n_byte2 max_distance_squared = gvar->declare_max_start_dsq;	/* val3 */
    n_vect2 *loc = &comb->location;
    
	n_combatant *comb_at = un_at->combatants;
    
	n_int	distance_centre_squ;
    n_vect2 average;
    n_vect2 delta;
    
    vect2_populate(&average, un_at->average[0], un_at->average[1]);
    
    vect2_subtract(&delta, loc, &average);
    
    distance_centre_squ = vect2_dot(&delta, &delta, 1, 1);
    
	if (comb->wounds == NUNIT_DEAD)
    {
		return;
	}
    
	if (distance_centre_squ < gvar->declare_one_to_one_dsq)
    {							               /* val4 */
		/* the direction facing vector */
        n_vect2 facing;

		/* if the combatant is more than half way through, switch the direction back on the closest-checking loop */
        
		n_byte2 loop2 = 0;
        
        vect2_direction(&facing, loc_f, 32);
        
		while (loop2 < un_at->number_combatants) {
			n_byte2	loc_test = loop2;
			
			if (reverso) {
				loc_test = (n_byte2) ((un_at->number_combatants) - 1 - loop2);
			}
            
			if (comb_at[ loc_test ].wounds != NUNIT_DEAD)
            {
                n_int    distance_squared;
                n_int    distance_facing;
                n_vect2  distance;
                
                vect2_subtract(&distance, &comb_at[ loc_test ].location, loc);
                
                distance_squared = vect2_dot(&distance, &distance, 1, 1);
                distance_facing = vect2_dot(&distance, &facing, 1 ,1);
                
				/* if the combatant is closer than the previous lot AND visible (ie in front) */
				if ((distance_squared < max_distance_squared) && (distance_facing > 0)) {
					/* it is the prefered attacked combatant and the distance and instance is
                     stored */
					max_distance_squared = (n_byte2)distance_squared;
					loc_attack = loc_test;
					/* if this combatant is within the melee attacking distance,
                     the search is over, end this loop(2) */
					if (max_distance_squared < gvar->declare_close_enough_dsq) /* val5 */
                    {
						loop2 += 0xffff;
                    }
				}
			}
			loop2++;
		}
	}
	
	comb->attacking    = loc_attack;
	comb->distance_squ = max_distance_squared;
    
	if (group_facing == 255)
    {
		if (loc_attack != NUNIT_NO_ATTACK)
        {
            n_vect2 delta;
            vect2_subtract(&delta, &comb_at[ loc_attack ].location, loc);
			group_facing = math_tan(&delta) ;
		}
        else
        {
            group_facing = combatant_random_facing( group_facing, gvar);
		}
	}
	comb->direction_facing = group_facing;
}


void battle_declare(n_unit *un, n_general_variables * gvar)
{
	n_uint		 loop = 0;
	n_byte		 group_facing = 255;
	n_combatant *comb      = un->combatants;
	/* the unit that is being attacked */
	n_unit	    *un_at      = un->unit_attacking;
	n_byte2      loc_number = un->number_combatants;
    
	/* only worthwhile if the unit is attacking something */
	if (un_at == NOTHING)
    {
		return;
	}
	{
		/* the combatants being attacked */
		n_int	delta_x = un_at->average[0] - un->average[0];
		n_int	delta_y = un_at->average[1] - un->average[1];
        n_vect2 delta;
        vect2_populate(&delta, delta_x, delta_y);
        
		if ((delta_x * delta_x) + (delta_y * delta_y) >= gvar->declare_group_facing_dsq)
        {		                   /* val2 */
			group_facing = math_tan(&delta);
		}
	}
	while (loop < loc_number) {
		n_byte  reverso = (n_byte) (loop > (n_uint)(loc_number >> 1));
		battle_combatant_declare(&comb[loop], gvar, un_at, reverso, group_facing);
		loop++;
	}
}

void combatant_dead(n_combatant * comb)
{
    comb->wounds = NUNIT_DEAD;
    comb->speed_current = 0;
    comb->attacking = NUNIT_NO_ATTACK;
}


static void combatant_move(n_combatant * comb, n_general_variables * gvar, void * values)
{
	n_int   local_speed = comb->speed_current;
	n_int   local_facing = comb->direction_facing;
    n_vect2 old_location;
    n_vect2 temp_location;
    n_vect2 facing;
    vect2_copy(&old_location, &comb->location);

    vect2_copy(&temp_location, &old_location);
    
	if (comb->wounds == NUNIT_DEAD)
    {
		return;
	}

    if (local_speed == 0)
    {
		return;
	}
    
    local_facing = combatant_random_facing(local_facing, gvar);
    
    vect2_direction(&facing, local_facing, 1);
    
    vect2_d(&temp_location, &facing, local_speed, 26880);

    if (OUTSIDE_HEIGHT(temp_location.y) || OUTSIDE_WIDTH(temp_location.x))
    {
        combatant_dead(comb);
        return;
    }

    if (old_location.x != temp_location.x || old_location.y != temp_location.y)
    {
        if (board_move(&old_location,&temp_location))
        {
            vect2_copy(&comb->location, &temp_location);
        }
    }

	comb->direction_facing = (n_byte) local_facing;
	comb->speed_current    = (n_byte) local_speed;
}

/* this is currently fudged for the skirmish testing... it will be fixed
 in the future... honest... */

void battle_move(n_unit * un, n_general_variables * gvar) {
    combatant_loop(&combatant_move, un, gvar, NOTHING);
    battle_area(un);
}


/* as combatants can continue to fight for the entire battle_cycle,
 the death condition only occurs when wounds == NUNIT_DEAD */

void battle_remove_dead(n_unit *un, n_general_variables * gvar) {
	n_combatant *comb = (n_combatant *)(un->combatants);
    n_vect2 sum = {0};
	n_int   count = 0;
	n_byte2 loop = 0;
	while (loop < un->number_combatants)
    {
		if (comb[ loop ].wounds != NUNIT_DEAD)
        {
			if (comb[ loop ].wounds == 0)
            {
                combatant_dead(&comb[loop]);
				(void)board_clear(&comb[ loop ].location);                
			} else {
                vect2_d(&sum, &comb[ loop ].location, 1, 1);
				count++;
			}
		}
		loop++;
	}
	if (count != 0)
    {
		un->average[0] = (n_byte2)(sum.x / count);
		un->average[1] = (n_byte2)(sum.y / count);
	}
	un->number_living = count;
}

/* returns 1 if the alignment 0 army is all dead,
 returns 2 if the alignment 1 army is all dead,
 returns 3 if both armies are all dead, and,
 returns 0 in all other cases */

n_byte battle_opponent(n_unit * un, n_uint	num, n_uint * no_movement) {
	n_uint	loop = 0;
	n_uint	unit_count[2] = {0};
    n_uint  unit_movement[2] = {0};
	while (loop < num)
    {
		if (un[loop].number_living > 0)
        {
			n_unit	*un_att = un[loop].unit_attacking;
			n_int	local_alignment = (un[loop].alignment) & 1;
            n_combatant *combatants = (n_combatant *)un[loop].combatants;
            n_uint       number_combatants = un[loop].number_combatants;
            n_uint       loop2 = 0;
            n_uint       movement = 0;
            while (loop2 < number_combatants)
            {
                if (combatants[loop2].speed_current != 0)
                {
                    movement = 1;
#ifdef DEBUG_MOVEMENT_TRANSITIONS
                    un[loop].selected = 1;
#endif
                }
                loop2++;
            }
            unit_count[local_alignment]++;

			unit_movement[local_alignment] += movement;
#ifdef DEBUG_MOVEMENT_TRANSITIONS
            if (movement == 0)
            {
                un[loop].selected = 0;
            }
#endif
			if (un_att != NOTHING)
            {
				if (un_att->number_living == 0)
                {
					un_att = NOTHING;
                }
			}
            
			if (un_att == NOTHING)
            {
				n_int	px = un[loop].average[0];
				n_int	py = un[loop].average[1];
				n_uint	min_dist_squ = BIG_INTEGER;
				n_uint	loop2 = 0;
				while (loop2 < num)
                {
					if (loop != loop2 && un[loop2].number_living)
                    {
						if (((un[loop2].alignment)&1) != local_alignment)
                        {
							n_int	tx = un[loop2].average[0];
							n_int	ty = un[loop2].average[1];
							n_uint   dist_squ;
							tx -= px;
							ty -= py;
                            
							dist_squ = (n_uint)((tx*tx) + (ty*ty));
							if (dist_squ < min_dist_squ)
                            {
								min_dist_squ = dist_squ;
								un_att = (n_unit *)&un[loop2];
							}
						}
					}
					loop2++;
				}
			}
			un[loop].unit_attacking = (void *)un_att;
		}
        else
        {
			un[loop].unit_attacking = NOTHING;
        }
		loop++;
	}
    
    if ((unit_movement[0] == 0) && (unit_movement[1] == 0))
    {
        *no_movement = *no_movement + 1;
    }
    else
    {
        *no_movement = 0;
    }
	return ((unit_count[0] == 0) | (unit_count[1] == 0));
}

#define BATTLE_JSON_LOCATION1 "./Simulated War.app/Contents/Resources/battle.json"
#define BATTLE_JSON_LOCATION2 "battle.json"
#define BATTLE_JSON_LOCATION3 "./war/game/battle.json"

n_byte          *local_board;

static n_general_variables  game_vars;

static n_unit    *units;
static n_byte2    number_units;
static n_type   *types;
static n_byte2  number_types;

static n_uint  no_movement = 0;

#define    SIZEOF_MEMORY     (64*1024*1024)

static n_byte    *memory_buffer;
static n_uint   memory_allocated;
static n_uint    memory_used;

static n_byte engine_paused = 0;
static n_byte engine_new_required = 0;
static n_byte engine_debug = 0;

static n_file * open_file_json = 0L;

static n_int engine_count = 0;

static n_object * obj_unit_type(n_type * values)
{
    n_object * return_object = object_number(0L, "defence", values->defence);
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

static n_object * obj_unit(n_unit * values)
{
    n_object * return_object = object_number(0L, "type_id", values->morale);
    n_array  * average_array = array_number(values->average[0]);
    array_add(average_array, array_number(values->average[1]));
    object_number(return_object, "width", values->width);
    object_array(return_object, "average", average_array);
    object_number(return_object, "angle", values->angle);
    object_number(return_object, "number_combatants", values->number_combatants);
    object_number(return_object, "alignment", values->alignment);
    object_number(return_object, "missile_number", values->missile_number);
    
    return return_object;
}

static n_object * obj_general_variables(n_general_variables * values)
{
    n_object * return_object = object_number(0L, "random0", values->random0);
    
    object_number(return_object, "random1", values->random1);
    object_number(return_object, "attack_melee_dsq", values->attack_melee_dsq);
    object_number(return_object, "declare_group_facing_dsq", values->declare_group_facing_dsq);
    object_number(return_object, "declare_max_start_dsq", values->declare_max_start_dsq);
    object_number(return_object, "declare_one_to_one_dsq", values->declare_one_to_one_dsq);
    object_number(return_object, "declare_close_enough_dsq", values->declare_close_enough_dsq);
    
    return return_object;
}

static n_object * obj_additional_variables(n_additional_variables * values)
{
    n_object * return_object = object_number(0L, "probability_melee", values->probability_melee);
    
    object_number(return_object, "probability_missile", values->probability_missile);
    object_number(return_object, "damage_melee", values->damage_melee);
    object_number(return_object, "damage_missile", values->damage_missile);
    object_number(return_object, "speed_max", values->speed_max);
    object_number(return_object, "range_missile", values->range_missile);
    
    return return_object;
}

n_int draw_error(n_constant_string error_text, n_constant_string location, n_int line_number)
{
#ifdef _WIN32
    LPSTR lpBuff = error_text;
    DWORD dwSize = 0;
    WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), lpBuff, lstrlen(lpBuff), &dwSize, NULL);

    //char str[256];
    //sprintf_s(str, "ERROR: %s\n", error_text);
    OutputDebugString(error_text);
#else
    printf("ERROR: %s, %s line: %ld\n", error_text, location, line_number);
#endif
    return -1;
}

static void mem_init(n_byte start) {
    if (start) {
        memory_buffer = NOTHING;
        memory_allocated = SIZEOF_MEMORY;
        
        memory_buffer = memory_new_range((SIZEOF_MEMORY/4), &memory_allocated);
    }
    memory_used = 0;
}


static n_byte * mem_use(n_uint size) {
    n_byte * val = NOTHING;
    if (size > (memory_allocated - memory_used)) {
        engine_exit();
        /*plat_close();*/
    }
    val = &memory_buffer[memory_used];
    memory_used += size;
        
    return val;
}

const n_string_block json_file_string = "{\"general_variables\":{\"random0\":58668,\"random1\":8717,\"attack_melee_dsq\":5,\"declare_group_facing_dsq\":8000,\"declare_max_start_dsq\":65535,\"declare_one_to_one_dsq\":65535,\"declare_close_enough_dsq\":5},\"unit_types\":[{\"defence\":4,\"melee_attack\":6,\"melee_damage\":3,\"melee_armpie\":1,\"missile_attack\":3,\"missile_damage\":4,\"missile_armpie\":1,\"missile_rate\":100,\"missile_range\":40,\"speed_maximum\":3,\"stature\":4,\"leadership\":2,\"wounds_per_combatant\":3,\"type_id\":0},{\"defence\":2,\"melee_attack\":3,\"melee_damage\":1,\"melee_armpie\":1,\"missile_attack\":0,\"missile_damage\":0,\"missile_armpie\":0,\"missile_rate\":0,\"missile_range\":0,\"speed_maximum\":4,\"stature\":1,\"leadership\":2,\"wounds_per_combatant\":2,\"type_id\":1},{\"defence\":4,\"melee_attack\":6,\"melee_damage\":2,\"melee_armpie\":2,\"missile_attack\":0,\"missile_damage\":0,\"missile_armpie\":0,\"missile_rate\":0,\"missile_range\":0,\"speed_maximum\":6,\"stature\":2,\"leadership\":4,\"wounds_per_combatant\":3,\"type_id\":2},{\"defence\":2,\"melee_attack\":7,\"melee_damage\":1,\"melee_armpie\":3,\"missile_attack\":0,\"missile_damage\":0,\"missile_armpie\":0,\"missile_rate\":0,\"missile_range\":0,\"speed_maximum\":7,\"stature\":2,\"leadership\":3,\"wounds_per_combatant\":1,\"type_id\":3}],\"units\":[{\"type_id\":0,\"width\":28,\"average\":[100,200],\"angle\":120,\"number_combatants\":480,\"alignment\":0,\"missile_number\":20},{\"type_id\":0,\"width\":28,\"average\":[100,400],\"angle\":128,\"number_combatants\":480,\"alignment\":0,\"missile_number\":20},{\"type_id\":0,\"width\":28,\"average\":[100,600],\"angle\":136,\"number_combatants\":480,\"alignment\":0,\"missile_number\":20},{\"type_id\":2,\"width\":10,\"average\":[250,400],\"angle\":128,\"number_combatants\":100,\"alignment\":0,\"missile_number\":0},{\"type_id\":2,\"width\":10,\"average\":[265,200],\"angle\":128,\"number_combatants\":100,\"alignment\":0,\"missile_number\":0},{\"type_id\":0,\"width\":50,\"average\":[500,200],\"angle\":0,\"number_combatants\":600,\"alignment\":1,\"missile_number\":20},{\"type_id\":2,\"width\":35,\"average\":[500,600],\"angle\":0,\"number_combatants\":100,\"alignment\":1,\"missile_number\":0},{\"type_id\":0,\"width\":40,\"average\":[700,600],\"angle\":0,\"number_combatants\":600,\"alignment\":1,\"missile_number\":20},{\"type_id\":3,\"width\":40,\"average\":[600,600],\"angle\":0,\"number_combatants\":200,\"alignment\":1,\"missile_number\":0}]}";


n_unit * engine_units(n_byte2 * num_units)
{
    *num_units = number_units;
    return units;
}

n_file * engine_conditions_file(n_constant_string file_name)
{
    n_file         *file_json = io_file_new();

    if (io_disk_read_no_error(file_json, (n_string)file_name) != 0)
    {
        io_file_free(&file_json);
        return 0L;
    }
    else
    {
        printf("%s loaded\n", file_name);
    }
    
    if (open_file_json)
    {
        io_file_free(&open_file_json);
    }
    
    open_file_json = io_file_duplicate(file_json);
    
    return file_json;
}

n_int engine_conditions(n_file *file_json)
{
    if (file_json == 0L)
    {
        return SHOW_ERROR("Read file failed");
    }
    
    number_units = 0;
    number_types = 0;

    mem_init(0);

    local_board = (n_byte *)mem_use(BATTLE_BOARD_SIZE);
    
    if (local_board == NOTHING)
    {
        return SHOW_ERROR("Local board not allocated");
    }
    
    memory_erase(local_board, BATTLE_BOARD_SIZE);
    board_init(local_board);

    io_whitespace_json(file_json);
    
    {
        n_object_type type_of;
        void * returned_blob = unknown_file_to_tree(file_json, &type_of);
        n_object * returned_object = 0L;
        
        if (type_of == OBJECT_OBJECT)
        {
            returned_object = (n_object *)returned_blob;
        }
        
        if (returned_object)
        {
            n_string str_general_variables = obj_contains(returned_object, "general_variables", OBJECT_OBJECT);
            n_string str_unit_types = obj_contains(returned_object, "unit_types", OBJECT_ARRAY);
            n_string str_units = obj_contains(returned_object, "units", OBJECT_ARRAY);
            n_object * obj_general_variables = obj_get_object(str_general_variables);
            
            if (str_unit_types)
            {
                n_array * arr_unit_types = obj_get_array(str_unit_types);
                n_array * arr_follow = 0L;
                n_int value;
                
                types = (n_type *) mem_use(0);
                while ((arr_follow = obj_array_next(arr_unit_types, arr_follow)))
                {
                    n_object * obj_follow = obj_get_object(arr_follow->data);
                    n_type * current_type = &types[number_types];

                    if (obj_contains_number(obj_follow, "defence", &value))
                    {
                        current_type->defence = value;
                    }
                    if (obj_contains_number(obj_follow, "melee_attack", &value))
                    {
                        current_type->melee_attack = value;
                    }
                    if (obj_contains_number(obj_follow, "melee_damage", &value))
                    {
                        current_type->melee_damage = value;
                    }
                    if (obj_contains_number(obj_follow, "melee_armpie", &value))
                    {
                        current_type->melee_armpie = value;
                    }
                    if (obj_contains_number(obj_follow, "missile_rate", &value))
                    {
                        current_type->missile_rate = value;
                    }
                    if (obj_contains_number(obj_follow, "missile_range", &value))
                    {
                        current_type->missile_range = value;
                    }
                    if (obj_contains_number(obj_follow, "speed_maximum", &value))
                    {
                        current_type->speed_maximum = value;
                    }
                    if (obj_contains_number(obj_follow, "stature", &value))
                    {
                        current_type->stature = value;
                    }
                    if (obj_contains_number(obj_follow, "leadership", &value))
                    {
                        current_type->leadership = value;
                    }
                    if (obj_contains_number(obj_follow, "wounds_per_combatant", &value))
                    {
                        current_type->wounds_per_combatant = value;
                    }
                    if (obj_contains_number(obj_follow, "type_id", &value))
                    {
                        current_type->points_per_combatant = value;
                    }
                    (void)mem_use(sizeof(n_type));
                    number_types++;
                }
            
                if (str_units)
                {
                    n_array * arr_units = obj_get_array(str_units);
                    n_array * arr_follow = 0L;
                    n_int value;
                    
                    units = (n_unit *) mem_use(0);
                    while ((arr_follow = obj_array_next(arr_units, arr_follow)))
                    {
                        n_object * obj_follow = obj_get_object(arr_follow->data);
                        n_unit * current_unit = &units[number_units];
                        
                        if (obj_contains_number(obj_follow, "type_id", &value))
                        {
                            current_unit->morale = value;
                        }
                        if (obj_contains_number(obj_follow, "width", &value))
                        {
                            current_unit->width = value;
                        }
                        if (obj_contains_number(obj_follow, "angle", &value))
                        {
                            current_unit->angle = value;
                        }
                        if (obj_contains_number(obj_follow, "number_combatants", &value))
                        {
                            current_unit->number_combatants = value;
                        }
                        if (obj_contains_number(obj_follow, "alignment", &value))
                        {
                            current_unit->alignment = value;
                        }
                        if (obj_contains_number(obj_follow, "missile_number", &value))
                        {
                            current_unit->missile_number = value;
                        }
                        
                        (void)obj_contains_array_nbyte2(obj_follow, "average", current_unit->average, 2);
                        
                        (void)mem_use(sizeof(n_unit));
                        number_units++;
                    }
                    
                    if (obj_general_variables)
                    {
                        /*io_file_debug(obj_json(obj_general_variables));*/
                        n_general_variables * values = (n_general_variables*)&game_vars;
                        n_int value;
                        if (obj_contains_number(obj_general_variables, "random0", &value))
                        {
                            values->random0 = value;
                        }
                        if (obj_contains_number(obj_general_variables, "random1", &value))
                        {
                            values->random1 = value;
                        }
                        if (obj_contains_number(obj_general_variables, "attack_melee_dsq", &value))
                        {
                            values->attack_melee_dsq = value;
                        }
                        if (obj_contains_number(obj_general_variables, "declare_group_facing_dsq", &value))
                        {
                            values->declare_group_facing_dsq = value;
                        }
                        if (obj_contains_number(obj_general_variables, "declare_max_start_dsq", &value))
                        {
                            values->declare_max_start_dsq = value;
                        }
                        if (obj_contains_number(obj_general_variables, "declare_one_to_one_dsq", &value))
                        {
                            values->declare_one_to_one_dsq = value;
                        }
                        if (obj_contains_number(obj_general_variables, "declare_close_enough_dsq", &value))
                        {
                            values->declare_close_enough_dsq = value;
                        }
                    }
                }
            }
            unknown_free(&returned_blob, type_of);
        }
    }
    
    if ((number_types == 0) || (number_units == 0) || (number_types > 255))
    {
        SHOW_ERROR("Type/Unit Logic Failed");
    }
        
    /* resolve the units with types and check the alignments */
    {
        n_byte    resolve[256] = {0};
        n_uint    check_alignment[2] = {0};
        n_byte    loop = 0;
        while (loop < number_types) {
            resolve[types[loop].points_per_combatant] = loop;
            loop++;
        }
        loop = 0;
        while (loop < number_units) {
            n_byte2    local_combatants = units[loop].number_combatants;
            units[loop].unit_type = &types[resolve[units[loop].morale]];
            units[loop].morale = 255;
            units[loop].number_living = local_combatants;
            units[loop].combatants = (n_combatant *)mem_use(sizeof(n_combatant)*local_combatants);
            check_alignment[ (units[loop].alignment) & 1 ]++;
            loop++;
        }

        /* if there are none of one of the alignments, there can be no battle */
        if ((check_alignment[0] == 0) || (check_alignment[1] == 0))
        {
            SHOW_ERROR("Alignment Logic Failed");
        }
    }
    /* get the drawing ready, fill the units with spaced combatants and draw it all */
    battle_loop(&battle_fill, units, number_units, NOTHING);
    return 0;
}

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

