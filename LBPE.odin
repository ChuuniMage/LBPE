package main;

import "vendor:sdl2";
import "core:fmt";
import "core:math"

NATIVE_WIDTH :: 320
NATIVE_HEIGHT :: 200

RES_MULTIPLIER :: 3

FPS :: 60;
FRAME_DURATION :: 1000 / FPS;

MOUSE_PT := [2]i32{}
PREV_PT := [2]i32{}

UNROLLING :: true

make_working_surface :: #force_inline proc (render_surface:^sdl2.Surface) -> ^sdl2.Surface {
    return sdl2.CreateRGBSurface(0, 
        NATIVE_WIDTH, NATIVE_HEIGHT, 
        i32(render_surface.format.BitsPerPixel), u32(render_surface.format.Rmask), 
        render_surface.format.Gmask, render_surface.format.Bmask, render_surface.format.Amask);
}

point_to_idx :: proc (s:^sdl2.Surface, pt:[2]i32) -> i32 {return pt.x + (s.w*pt.y)}

blacken_surf :: proc (surf:^sdl2.Surface) {
    mptr := cast([^]u32)surf.pixels
    for x in 0..<surf.w do for y in 0..<surf.h {
        idx := point_to_idx(surf, {x,y})
        mptr[idx] = sdl2.MapRGB(surf.format,0,0,0)
    }
}
plot_line :: proc (lpi:^LimitedPaletteImage(4), set_idx: u8, last:[2]i32, current:[2]i32) {
    x0 := last.x
    y0 := last.y;
    x1 := current.x;
    y1 := current.y;

    dx := abs(x1-x0);
    sx : i32 = x0<x1 ? 1 : -1;
    dy := -abs(y1-y0);
    sy : i32 = y0<y1 ? 1 : -1;
    err := dx+dy;  /* error value e_xy */
    for {
        set_pixel_4([2]i32{x0, y0}, set_idx, lpi)
        if (x0 == x1 && y0 == y1){
            break;
        }
           
        e2 := 2*err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

PointToDraw :: struct {
    pt:[2]i32,
    pal:u8,
}

draw_line_returned :: proc (dynarr:^[dynamic]PointToDraw, lpi:^LimitedPaletteImage(4), set_idx: u8, origin:[2]i32, vec:[2]i32) {
	x_greater := abs(vec.x) > abs(vec.y) 
	greater := x_greater ? vec.x : vec.y
	lesser := x_greater ? vec.y : vec.x

    negative_half_selected := vec.y > 0 ? vec.x + vec.y < 0 : vec.x + vec.y <= 0
    neg_half_factor : i32 = negative_half_selected ? -1 : 1
    
	for i in cast(i32)0..=abs(greater){
		lesser_axis_position:f32 = f32(lesser * i) / f32(greater)
		whole := i32(lesser_axis_position)
		fract := lesser_axis_position - f32(whole)

		lesser_offset  := abs(fract) > 0.5  ? (fract > 0 ? whole + 1 : whole - 1) : whole
        lesser_offset  *= neg_half_factor
        
		greater_offset := i 
        greater_offset *= neg_half_factor

        x_offset := x_greater ? greater_offset : lesser_offset
		y_offset := x_greater ? lesser_offset : greater_offset
        append(dynarr, PointToDraw{{origin.x + x_offset, origin.y + y_offset}, set_idx})
	}
}

draw_line_immediate :: proc (lpi:^LimitedPaletteImage(4), set_idx: u8, origin:[2]i32, vec:[2]i32) {
	x_greater := abs(vec.x) > abs(vec.y) 
	greater := x_greater ? vec.x : vec.y
	lesser := x_greater ? vec.y : vec.x

    negative_half_selected := vec.y > 0 ? vec.x + vec.y < 0 : vec.x + vec.y <= 0
    neg_half_factor : i32 = negative_half_selected ? -1 : 1
    
	for i in cast(i32)0..=abs(greater){
		lesser_axis_position:f32 = f32(lesser * i) / f32(greater)
		whole := i32(lesser_axis_position)
		fract := lesser_axis_position - f32(whole)

		lesser_offset  := abs(fract) > 0.5  ? (fract > 0 ? whole + 1 : whole - 1) : whole
        lesser_offset  *= neg_half_factor
        
		greater_offset := i 
        greater_offset *= neg_half_factor

        x_offset := x_greater ? greater_offset : lesser_offset
		y_offset := x_greater ? lesser_offset : greater_offset
        set_pixel_4({origin.x + x_offset, origin.y + y_offset}, set_idx, lpi)
	}
}

PROFILING :: false
import "core:time"
when PROFILING {
    profiling_ms := make([dynamic]f64)

    profile_start :: proc () -> time.Tick {
        OLD_TIME := time.tick_now()
        return OLD_TIME
    }
    profile_end :: proc (OLD_TIME:time.Tick) -> (time_in_milliseconds:f64) {
        new := time.tick_now()
        diff := time.tick_diff(OLD_TIME, new)
        time_in_milliseconds = time.duration_milliseconds(diff)
        fmt.printf("--- Diff: %v \n", time_in_milliseconds)
        return
    }
} 

import "core:slice"

DrawMode :: enum {
    Pencil,
    Line,
}

checked_pt_append :: proc (dynarr:^[dynamic]PointToDraw, lpi:^LimitedPaletteImage(4),  new_pt:[2]i32, pal:u8) {
    check := [2]i32{cast(i32)lpi.dims.x - new_pt.x, cast(i32)lpi.dims.y - new_pt.y}
    valid := check.x >= 0 && check.y >= 0 && check.x < cast(i32)lpi.dims.x && check.y < cast(i32)lpi.dims.y
    if !valid do return
    append(dynarr, PointToDraw{new_pt, pal})
}

make_square :: proc (pt:[2]i32, sq:int) -> [][2]i32 {
    new_slice := make([dynamic][2]i32, context.temp_allocator);
    for x in 0..<sq do for y in 0..<sq {
        fmt.printf("Origin %v, x it %v y it %v apending %v \n", pt, x, y, pt + {cast(i32)x, cast(i32)y})
        append(&new_slice, pt + {cast(i32)x, cast(i32)y})
    }
    fmt.printf("---\n")
    return new_slice[:]
}

main :: proc () {
    if sdl2.Init( sdl2.INIT_VIDEO ) < 0 {
        fmt.printf( "SDL could not initialize! SDL_Error: %s\n", sdl2.GetError() );
        return;
    }
    window := sdl2.CreateWindow("LBPE", 
    sdl2.WINDOWPOS_UNDEFINED,	sdl2.WINDOWPOS_UNDEFINED, 
    i32(NATIVE_WIDTH * RES_MULTIPLIER), 
    i32(NATIVE_HEIGHT * RES_MULTIPLIER), 
    sdl2.WINDOW_SHOWN );
    if window == nil {
        fmt.printf( "Window could not be created! SDL_Error: %s\n", sdl2.GetError());
        return ;
    };
    render_surface := sdl2.GetWindowSurface(window);
    working_surface := make_working_surface(render_surface)
    quit := false 
    frame_counter:u64;

    pal1 := [4]u32{0xCBF1F5, 0x445975, 0x0E0F21, 0x050314}

    new_lbp := LimitedPaletteImage(4){}
    init_lpi(&new_lbp, &pal1, {NATIVE_WIDTH, NATIVE_HEIGHT})

    for x, idx in &new_lbp.pixels {
        x = 0b01010101
    }

    draw_mode:DrawMode


    for quit == false {
        @static points_to_draw :[dynamic]PointToDraw; 
        defer {
            for ptd in points_to_draw {
                set_pixel_4(ptd.pt, ptd.pal, &new_lbp)
            }
            clear(&points_to_draw)
        };

        blacken_surf(working_surface)
        preview_surface := make_working_surface(render_surface); defer sdl2.FreeSurface(preview_surface)
        frameStart := sdl2.GetTicks(); frame_counter += 1;
        event:sdl2.Event;
        @static LEFTCLICK_DOWN := false
        @static PREV_LEFTCLICK_DOWN := false
        @static RIGHTCLICK_DOWN := false
        @static PREV_RIGHTCLICK_DOWN := false

        @static LEFT_COLOUR : u8 = 0
        @static RIGHT_COLOUR : u8 = 3

        @static brush_size := 1

        defer PREV_PT = MOUSE_PT

        for( sdl2.PollEvent( &event ) != false ){
            if event.type == .QUIT {quit = true;break;};
            if event.type == .KEYDOWN {
                #partial switch event.key.keysym.sym {
                    case .NUM1: draw_mode = .Pencil
                    case .NUM2: draw_mode = .Line
                }
                if event.key.keysym.sym == .F1 {
                    pal1 = swizzle(pal1, 3, 0, 1, 2)
                }
                if event.key.keysym.sym == .F3 {
                    // for x in 0..=3 {
                    //     set_pixel_4({cast(i32)x,4}, RIGHT_COLOUR, &new_lbp)
                    // }
                    draw_line_immediate(&new_lbp, RIGHT_COLOUR, {0,0}, {25,25})
                    draw_line_immediate(&new_lbp, RIGHT_COLOUR, {0,1}, {25,26})
                }
                if event.key.keysym.sym == .LEFTBRACKET {
                    brush_size = max(1, brush_size - 1)
                }
                if event.key.keysym.sym == .RIGHTBRACKET {
                    brush_size = max(brush_size, brush_size + 1)
                }
                // when PROFILING {
                //     if event.key.keysym.sym == .F2 {
                //         for i in 0..<10000 {
                //              profile_time := profile_start();
                //             draw_line(&new_lbp, 0, {0,0}, {319,199})
                //             time_end := profile_end(profile_time)
                //             append(&profiling_ms, time_end)
                //         }
                //         total := slice.reduce(profiling_ms[:], 0.0, proc(a,b:f64) -> f64 {return b > 1.0 ? a : a + b})
                //         average := total / cast(f64)len(profiling_ms)
                //         fmt.printf("Average time: %v \n", average)
                //         clear(&profiling_ms)
                //     }
                // }
            }
            if event.type == .MOUSEMOTION {
                MOUSE_PT = {event.motion.x / RES_MULTIPLIER, event.motion.y / RES_MULTIPLIER}
            }
            if event.type == .MOUSEBUTTONDOWN {
                if event.button.button == sdl2.BUTTON_LEFT {
                    LEFTCLICK_DOWN = true;
                }
                if event.button.button == sdl2.BUTTON_RIGHT {
                    RIGHTCLICK_DOWN = true;
                }
            }
            if event.type == .MOUSEBUTTONUP {
                if event.button.button == sdl2.BUTTON_LEFT {
                    LEFTCLICK_DOWN = false;
                    PREV_LEFTCLICK_DOWN = false;
                }
                if event.button.button == sdl2.BUTTON_RIGHT {
                    RIGHTCLICK_DOWN = false;
                    PREV_RIGHTCLICK_DOWN = false;
                }
            }
        };
        if LEFTCLICK_DOWN {
            switch draw_mode {
                case .Pencil:
                    sloice := make_square(PREV_PT, brush_size);  
                    if PREV_LEFTCLICK_DOWN == true && ((PREV_PT - MOUSE_PT) != [2]i32{0,0}) {
                        vec := MOUSE_PT - PREV_PT
                        for pt in sloice {
                            draw_line_returned(&points_to_draw, &new_lbp, LEFT_COLOUR, pt, vec)
                        }
                        break;
                    } 
                    for pt in sloice {
                        checked_pt_append(&points_to_draw, &new_lbp, pt, LEFT_COLOUR)
                    }
                case .Line: 
                    @static LINE_INITTED := false
                    @static LDRAW_STORED := [2]i32{0,0}
                    if PREV_LEFTCLICK_DOWN do break;
                    if LINE_INITTED == false {
                        LINE_INITTED = true;
                        LDRAW_STORED = MOUSE_PT
                        break;
                    } 
                    sloice := make_square(LDRAW_STORED, brush_size);  
                    vec := MOUSE_PT - LDRAW_STORED
                    for pt in sloice {
                        draw_line_immediate(&new_lbp, LEFT_COLOUR, pt, vec)
                    }
                    LINE_INITTED = false;
            }
            PREV_LEFTCLICK_DOWN = true;
        }

        // if RIGHTCLICK_DOWN {
        //     switch draw_mode {
        //         case .Pencil:
        //             if PREV_RIGHTCLICK_DOWN == true && ((PREV_PT - MOUSE_PT) != [2]i32{0,0}) {
        //                 draw_line_immediate(&new_lbp, RIGHT_COLOUR, PREV_PT, MOUSE_PT - PREV_PT)
        //             } else {
        //                 append(&points_to_draw, PointToDraw{MOUSE_PT, RIGHT_COLOUR})
        //             }
        //         case .Line: 
        //             @static LINE_INITTED := false
        //             @static LDRAW_STORED := [2]i32{0,0}
        //             if PREV_RIGHTCLICK_DOWN do break;
        //             if LINE_INITTED == false {
        //                 LINE_INITTED = true;
        //                 LDRAW_STORED = MOUSE_PT
        //             } else {
        //                 draw_line_immediate(&new_lbp, RIGHT_COLOUR, LDRAW_STORED, MOUSE_PT - LDRAW_STORED)
        //                 LINE_INITTED = false;
        //             }
        //     }
        //     PREV_RIGHTCLICK_DOWN = true;
        // }
       

        
        write_to_buffer_lpi4(&pal1, &new_lbp, cast([^]u32)working_surface.pixels)
        sdl2.BlitScaled(working_surface, nil, render_surface, nil);
        sdl2.BlitScaled(preview_surface, nil, render_surface, nil);
        sdl2.UpdateWindowSurface(window);
        // frameTime := sdl2.GetTicks() - frameStart;
        // if FRAME_DURATION > frameTime do sdl2.Delay(FRAME_DURATION - frameTime);
    }
};

