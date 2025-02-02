/****************************************************************

    draw.c

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

#include "../game/battle.h"

// Global variables for position and rendering
static n_int px = 0;  // X position
static n_int py = 0;  // Y position
static n_int pz = 0;  // Z position (zoom level)

// Rendering buffers
static n_byte render[(256 * 4) * (256 * 3) * 4] = {0};  // Render buffer
static n_byte4 points[(256 * 4) * (256 * 3)] = {0};     // Points buffer

// Rendering state variables
static n_int pointCount = 0;       // Number of points to render
static n_byte color = 0;           // Current color
static n_byte unit_selected = 0;   // Whether a unit is selected

// Function prototypes
void draw_init(void);
void draw_dpx(n_double dpx);
void draw_dpy(n_double dpy);
void draw_dpz(n_double dpz);
void draw_render(n_byte *value);
void draw_combatant(n_combatant *comb, n_general_variables *gvar, void *values);
void draw_cycle(n_unit *un, n_general_variables *gvar);
void draw_point(n_int px, n_int py);
void draw_line(n_int px1, n_int py1, n_int px2, n_int py2);
void draw_rectangle(n_int px1, n_int py1, n_int px2, n_int py2);
void draw_engine(n_byte *value);

// Initialize drawing (currently empty)
void draw_init(void) {
    // Initialization logic can be added here if needed
}

// Adjust X position based on input
void draw_dpx(n_double dpx) {
    n_int int_dpx = (n_int)(dpx * 100);  // Scale input
    px += int_dpx;                       // Update X position
}

// Adjust Y position based on input
void draw_dpy(n_double dpy) {
    n_int int_dpy = (n_int)(dpy * 100);  // Scale input
    py += int_dpy;                       // Update Y position
}

// Adjust Z position (zoom level) based on input
void draw_dpz(n_double dpz) {
    n_int int_dpz = (n_int)(dpz * 100);  // Scale input
    n_int total_zoom = pz + int_dpz;     // Calculate new zoom level

    // Clamp zoom level to valid range
    if ((total_zoom > -100) && (total_zoom < 128)) {
        pz = total_zoom;
    }
}

#ifdef _WIN32
// Windows-specific rendering logic

static n_byte windows_screen[(256 * 4) * (256 * 3)] = {0};  // Screen buffer for Windows

// Color definitions
#define COLOR_WHITE      (255)
#define COLOR_LIGHT_RED  (192)
#define COLOR_DARK_RED   (128)
#define COLOR_LIGHT_BLUE (64)
#define COLOR_DARK_BLUE  (32)
#define COLOR_BLACK      (0)

// Set RGB color based on value
void draw_color(n_int value, n_byte *rgb) {
    switch (value) {
        case COLOR_LIGHT_RED:
            rgb[0] = 255; rgb[1] = 77; rgb[2] = 77;
            break;
        case COLOR_DARK_RED:
            rgb[0] = 3 * 255 / 5; rgb[1] = 3 * 77 / 5; rgb[2] = 3 * 77 / 5;
            break;
        case COLOR_LIGHT_BLUE:
            rgb[0] = 128; rgb[1] = 128; rgb[2] = 255;
            break;
        case COLOR_DARK_BLUE:
            rgb[0] = 3 * 128 / 5; rgb[1] = 3 * 128 / 5; rgb[2] = 3 * 255 / 5;
            break;
        case COLOR_WHITE:
            rgb[0] = 255; rgb[1] = 255; rgb[2] = 255;
            break;
        case COLOR_BLACK:
        default:
            rgb[0] = 0; rgb[1] = 0; rgb[2] = 0;
            break;
    }
}

// Draw a point on the platform (Windows-specific)
static void draw_point_platform(n_int px, n_int py) {
    n_byte4 loop = (n_byte4)(px + (py * (256 * 4)));  // Calculate buffer index
    points[pointCount++] = loop;                      // Store point

    // Set color based on state
    if (color) {
        render[loop] = unit_selected ? COLOR_LIGHT_RED : COLOR_DARK_RED;
    } else {
        render[loop] = unit_selected ? COLOR_LIGHT_BLUE : COLOR_DARK_BLUE;
    }
}

// Draw a white point on the platform (Windows-specific)
static void draw_point_white_platform(n_int px, n_int py) {
    n_byte4 loop = (n_byte4)(px + (py * (256 * 4)));  // Calculate buffer index
    points[pointCount++] = loop;                      // Store point
    render[loop] = COLOR_WHITE;                       // Set color to white
}

// Render the platform (Windows-specific)
static void draw_render_platform(n_byte *value) {
    n_uint loop = 0;
    memory_copy(windows_screen, value, (256 * 4) * (256 * 3));  // Copy buffer

    // Draw points as black
    while (loop < pointCount) {
        n_uint loop4 = points[loop];
        windows_screen[loop4] = COLOR_BLACK;
        loop++;
    }
    pointCount = 0;  // Reset point count
}

#else
// Non-Windows rendering logic

// Draw a point on the platform (non-Windows)
static void draw_point_platform(n_int px, n_int py) {
    n_byte4 loop = (n_byte4)(px + (py * (256 * 4)));  // Calculate buffer index
    n_uint loop4 = loop * 4;                          // Calculate RGBA index

    points[pointCount++] = loop;  // Store point
    render[loop4 + 0] = 0;        // Clear red channel

#ifdef METAL_RENDER
    // Metal-specific rendering logic
    if (color) {
        if (unit_selected) {
            render[loop4 + 2] = 255; render[loop4 + 1] = 77; render[loop4 + 0] = 77;
        } else {
            render[loop4 + 2] = 3 * 255 / 5; render[loop4 + 1] = 3 * 77 / 5; render[loop4 + 0] = 3 * 77 / 5;
        }
    } else {
        if (unit_selected) {
            render[loop4 + 2] = 128; render[loop4 + 1] = 128; render[loop4 + 0] = 255;
        } else {
            render[loop4 + 2] = 3 * 128 / 5; render[loop4 + 1] = 3 * 128 / 5; render[loop4 + 0] = 3 * 255 / 5;
        }
    }
#else
    // Default rendering logic
    if (color) {
        if (unit_selected) {
            render[loop4 + 1] = 255; render[loop4 + 2] = 128; render[loop4 + 3] = 128;
        } else {
            render[loop4 + 1] = 3 * 255 / 5; render[loop4 + 2] = 3 * 128 / 5; render[loop4 + 3] = 3 * 128 / 5;
        }
    } else {
        if (unit_selected) {
            render[loop4 + 1] = 128; render[loop4 + 2] = 128; render[loop4 + 3] = 255;
        } else {
            render[loop4 + 1] = 3 * 128 / 5; render[loop4 + 2] = 3 * 128 / 5; render[loop4 + 3] = 3 * 255 / 5;
        }
    }
#endif
}

// Draw a white point on the platform (non-Windows)
static void draw_point_white_platform(n_int px, n_int py) {
    n_byte4 loop = (n_byte4)(px + (py * (256 * 4)));  // Calculate buffer index
    n_uint loop4 = loop * 4;                          // Calculate RGBA index

    points[pointCount++] = loop;  // Store point

    // Set color to white
    render[loop4 + 0] = 255;
    render[loop4 + 1] = 255;
    render[loop4 + 2] = 255;
    render[loop4 + 3] = 255;
}

// Render the platform (non-Windows)
static void draw_render_platform(n_byte *value) {
    n_uint loop = 0;
    memory_copy(render, value, (256 * 4) * (256 * 3) * 4);  // Copy buffer

    // Draw points as black
    while (loop < pointCount) {
        n_uint loop4 = points[loop] * 4;
        render[loop4 + 0] = 0;
        render[loop4 + 1] = 0;
        render[loop4 + 2] = 0;
        render[loop4 + 3] = 0;
        loop++;
    }
    pointCount = 0;  // Reset point count
}

#endif

// Render the final output
void draw_render(n_byte *value) {
    draw_render_platform(value);
}

// Draw a combatant on the screen
void draw_combatant(n_combatant *comb, n_general_variables *gvar, void *values) {
    if (comb->wounds != NUNIT_DEAD) {
        n_byte2 x = ((comb->location.x * (256 * 4)) >> 10);  // Calculate X position
        n_byte2 y = (comb->location.y * (256 * 4)) >> 10;    // Calculate Y position
        draw_point_platform(x, y);                           // Draw the combatant
    }
}

// Cycle through units and draw them
void draw_cycle(n_unit *un, n_general_variables *gvar) {
    color = un->alignment;          // Set color based on unit alignment
    unit_selected = un->selected;   // Set selection state
    combatant_loop(&draw_combatant, un, gvar, NOTHING);  // Loop through combatants
}

// Draw a point on the screen
void draw_point(n_int px, n_int py) {
    if ((px > -1) && (px < (256 * 4)) && (py > -1) && (py < (256 * 3))) {
        draw_point_white_platform(px, py);  // Draw the point if within bounds
    }
}

// Draw a line between two points
void draw_line(n_int px1, n_int py1, n_int px2, n_int py2) {
    n_int incrementer = 1;

    // Vertical line
    if (px1 == px2) {
        n_int loop = py1;
        if (py1 > py2) {
            incrementer = -1;
        }
        while (loop != py2) {
            draw_point(px1, loop);
            loop += incrementer;
        }
    }

    // Horizontal line
    if (py1 == py2) {
        n_int loop = px1;
        if (px1 > px2) {
            incrementer = -1;
        }
        while (loop != px2) {
            draw_point(loop, py1);
            loop += incrementer;
        }
    }
}

// Draw a rectangle
void draw_rectangle(n_int px1, n_int py1, n_int px2, n_int py2) {
    draw_line(px1, py1, px1, py2);  // Left side
    draw_line(px2, py2, px1, py2);  // Bottom side
    draw_line(px2, py2, px2, py1);  // Right side
    draw_line(px1, py1, px2, py1);  // Top side
}

// Main drawing engine
void draw_engine(n_byte *value) {
    n_vect2 start, end;
    n_byte2 number_units;
    n_unit *units = engine_units(&number_units);  // Get units from engine

    battle_loop(&draw_cycle, units, number_units, NOTHING);  // Draw units

    engine_square_dimensions(&start, &end);  // Get dimensions

    // Draw rectangle if dimensions are valid
    if ((end.x > -1) && (end.y > -1)) {
        draw_rectangle(start.x, start.y, end.x, end.y);
    }
    draw_render(value);  // Render the final output
}
