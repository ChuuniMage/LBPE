package main;

import "core:fmt";
import "core:math"
import "core:mem"

Palette :: [4]u32 // ALPHA B G R


//For 1 bit palette
u8_BS :: distinct bit_set[0..<8; u8]

//For 2 & 4 bit palette
get_11000000 :: #force_inline proc (a:u8) -> u8 {return a>>6} 
get_00110000 :: #force_inline proc (a:u8) -> u8 {return (a>>4)&3}
get_00001100 :: #force_inline proc (a:u8) -> u8 {return (a>>2)&3}
get_00000011 :: #force_inline proc (a:u8) -> u8 {return a&3}

get_lpi4 := [4]proc(u8) -> u8{get_11000000, get_00110000, get_00001100, get_00000011}

set_11000000 :: #force_inline proc (a:u8) -> u8 {return a<<6}
set_00110000 :: #force_inline proc (a:u8) -> u8 {return a<<4}
set_00001100 :: #force_inline proc (a:u8) -> u8 {return a<<2}
set_00000011 :: #force_inline proc (a:u8) -> u8 {return a&3}

set_lpi4 := [4]proc(u8) -> u8{set_11000000, set_00110000, set_00001100, set_00000011}
// //For 3 bit palette
// get_11100000 :: #force_inline proc (a:u8) -> u8 {return a>>5} 
// get_00011100 :: #force_inline proc (a:u8) -> u8 {return (a>>2)&7} 
// //it is actually get_00000011_10000000
// _get_10000011 :: #force_inline proc (a:^u8) -> u8 {
// 	honk := cast(^u16)mem.ptr_offset(a, 1)
// 	return cast(u8)((honk^ >> 7) &7)
// } 
// get_01110000 :: #force_inline proc (a:u8) -> u8 {return (a>>4)&7} 
// get_00001110 :: #force_inline proc (a:u8) -> u8 {return (a>>1)&7} 
// //it is actually get_00000001_11000000
// get_11000001 :: #force_inline proc (a:^u8) -> u8 {
// 	honk := cast(^u16)mem.ptr_offset(a, 1)
// 	return cast(u8)((honk^ >> 6) &7)
// } 
// get_00111000 :: #force_inline proc (a:u8) -> u8 {return (a>>3)&7} 
// get_00000111 :: #force_inline proc (a:u8) -> u8 {return a&7} 

// set_11100000 :: #force_inline proc (a:u8) -> u8 {return a<<5} 
// set_00011100 :: #force_inline proc (a:u8) -> u8 {return (a<<2)&7} 
// set_10000011 :: #force_inline proc (a:u8) -> u8 {return a} 
// set_01110000 :: #force_inline proc (a:u8) -> u8 {return a} 
// set_00001110 :: #force_inline proc (a:u8) -> u8 {return a} 
// set_11000001 :: #force_inline proc (a:u8) -> u8 {return a} 
// set_00111000 :: #force_inline proc (a:u8) -> u8 {return a} 
// set_00000111 :: #force_inline proc (a:u8) -> u8 {return a} 

//For 4 bit palette
get_11110000 :: #force_inline proc (a:u8) -> u8 {return a>>4} 
get_00001111 :: #force_inline proc (a:u8) -> u8 {return a&15} 

set_11110000 :: #force_inline proc (a:u8) -> u8 {return a<<4} 
set_00001111 :: #force_inline proc (a:u8) -> u8 {return a&15} 


packed_2bp_byte :: #force_inline proc (first,second,third,fourth: u8) -> u8 {
	return set_11000000(first) | set_00110000(second) | set_00001100(third) | set_00000011(fourth)
} 

packet_4bp_byte :: proc(first,second:u8) -> u8 {
	return set_11110000(first) | set_00001111(second)
}

packed_byte :: proc {packet_4bp_byte,packed_2bp_byte}

LimitedPaletteImage :: struct (P_SIZE:u16) where P_SIZE == 2 || P_SIZE == 4 || P_SIZE == 16 {
	suggested_palette: [P_SIZE]u32,
	bits_per_palette:u8,
	using_transparent: Maybe(u16),
	dims:[2]u32,
	pitch_bytes:i32,
	pitch_trailing_bits:i32,
	size_in_bytes:u32, // rounded down
	trailing_bits:u8,
	pixels:[]u8,
}

get_pixel_4 :: proc (dims:[2]i32, lpi:^LimitedPaletteImage(4)) -> u8 {
	byte_idx := (dims.x * dims.y) / 4
	trailing_bit_idx := ((dims.x * dims.y) % 4)
	get_proc := get_lpi4[trailing_bit_idx]
	return get_proc(lpi.pixels[byte_idx])
}

point_to_idx_lpi :: proc (lpi:^LimitedPaletteImage(4), pt:[2]i32) -> (byte_idx:i32, sub_idx:i32) {
	y_bits := (pt.y * lpi.pitch_bytes * 8) + (pt.y * lpi.pitch_trailing_bits)
	x_bits := pt.x * 2 
	bytes, _rem := math.divmod((y_bits + x_bits), 8)

	byte_idx = (y_bits + x_bits) / 8; sub_idx = ((y_bits + x_bits) % 8) / 2
	return
}

set_mask_lpi4 := [4]u8{0b11000000, 0b00110000, 0b00001100, 0b00000011}

set_pixel_4 :: proc (pt:[2]i32, set_idx:u8, lpi:^LimitedPaletteImage(4)) {
	byte_idx, sub_idx := point_to_idx_lpi(lpi, pt)
	prev_byte := lpi.pixels[byte_idx]

	working := prev_byte & ~u8(set_mask_lpi4[sub_idx])
	set_proc := set_lpi4[sub_idx]
	new_byte := working | set_proc(set_idx)

	lpi.pixels[byte_idx] = new_byte
}


init_lpi :: proc (lbp:^LimitedPaletteImage($P_SIZE), palette:^[P_SIZE]u32, dims:[2]u32, transparent_idx:Maybe(u16) = nil) 
	where P_SIZE == 2 || P_SIZE == 4 || P_SIZE == 16 {
	lbp.dims = dims
	lbp.suggested_palette = palette^
	lbp.using_transparent = transparent_idx
	lbp.bits_per_palette = cast(u8)math.log2(cast(f16)P_SIZE)

	when P_SIZE == 2 {
		lbp.trailing_bits = cast(u8)((dims.x * dims.y) % 8)
		lbp.size_in_bytes = (dims.x * dims.y) / 8
	}
	when P_SIZE == 4 {
		lbp.trailing_bits = cast(u8)((dims.x * dims.y) % 4) * 2
		lbp.size_in_bytes = (dims.x * dims.y) / 4
		lbp.pitch_bytes = cast(i32)dims.x / 4
		lbp.pitch_trailing_bits = (cast(i32)dims.x % 4) * 2
		fmt.printf("Pitch bytes : %v, pitch trail: %v \n", lbp.pitch_bytes, lbp.pitch_trailing_bits)
	}
	// when P_SIZE == 8 {
	// 	f :: proc (x:u8) -> u8 {return (x % 4) * 2}
	// 	x := u8(dims.x * dims.y)
	// 	lbp.trailing_bits = (x + f(x)) % 8
	// 	lbp.size_in_bytes = (dims.x * dims.y) / 3
	// 	if (dims.x * dims.y) / 8 != 0 && (dims.x * dims.y) % 8 == 0 do lbp.size_in_bytes += 1
	// }
	when P_SIZE == 16 {
		lbp.trailing_bits = cast(u8)((dims.x * dims.y) % 2) * 4
		lbp.size_in_bytes = (dims.x * dims.y) / 4
	}
	if lbp.trailing_bits != 0 do lbp.size_in_bytes += 1
	lbp.pixels = make([]u8, lbp.size_in_bytes)
}

write_to_buffer_lpi4 :: proc (palette:^[4]u32, src:^LimitedPaletteImage(4), dest_ptr:[^]u32){
	pixel_ptr : ^u32 = dest_ptr;

	bytes_to_iterate := src.size_in_bytes
	if src.trailing_bits > 0 do bytes_to_iterate -= 1;

	for i in 0..<bytes_to_iterate {
		pixel_ptr^ = palette[get_11000000(src.pixels[i])]; pixel_ptr = mem.ptr_offset(pixel_ptr, 1)
		pixel_ptr^ = palette[get_00110000(src.pixels[i])]; pixel_ptr = mem.ptr_offset(pixel_ptr, 1)
		pixel_ptr^ = palette[get_00001100(src.pixels[i])]; pixel_ptr = mem.ptr_offset(pixel_ptr, 1)
		pixel_ptr^ = palette[get_00000011(src.pixels[i])]; pixel_ptr = mem.ptr_offset(pixel_ptr, 1)
	}

	switch src.trailing_bits {
		case 6: pixel_ptr^ = palette[get_11000000(src.pixels[bytes_to_iterate])]; pixel_ptr = mem.ptr_offset(pixel_ptr, 1)
			fallthrough;
		case 4: pixel_ptr^ = palette[get_00110000(src.pixels[bytes_to_iterate])]; pixel_ptr = mem.ptr_offset(pixel_ptr, 1)
			fallthrough;
		case 2: pixel_ptr^ = palette[get_00001100(src.pixels[bytes_to_iterate])]; pixel_ptr = mem.ptr_offset(pixel_ptr, 1)
	}
};


write_to_buffer :: proc {write_to_buffer_lpi4}

// main :: proc(){
// 	fmt.printf("Hello, world! Your Odin project is set up.\n")
// 	_1bp := LimitedPaletteImage(2){}
// 	_2bp := LimitedPaletteImage(4){}
// 	_4bp := LimitedPaletteImage(16){}
// 	init_lpi(&_1bp, {8, 4})
// 	init_lpi(&_2bp, {2, 3})
// 	init_lpi(&_4bp, {2, 3})
// 	fmt.printf("18 / 8 %v \n", 18 / 8)
// 	f :: proc (x:u8) -> u8 {return (x % 4) * 2}
// 	for x in 1..=12 {
// 		_x := cast(u8)x
// 		fmt.printf("%v -> %v\n", _x, (_x + f(_x)) % 8)
// 	}
// };
