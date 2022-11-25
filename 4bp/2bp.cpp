#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define get_11000000(a) (uint8_t) a>>6
#define get_00110000(a) (uint8_t) (a>>4)&3
#define get_00001100(a) (uint8_t) (a>>2)&3
#define get_00000011(a) (uint8_t) a&3

#define set_11000000(a) (uint8_t) a<<6
#define set_00110000(a) (uint8_t) a<<4
#define set_00001100(a) (uint8_t) a<<2
#define set_00000011(a) (uint8_t) a&3

#define quad_2bit(first,second,third,fourth) set_11000000(first) | set_00110000(second) | set_00001100(third) | set_00000011(fourth)

union Palette_2bit {
    uint32_t colours[4];
    struct {
        uint32_t Transparent;
        uint32_t _1;
        uint32_t _2;
        uint32_t _3;
    };
};

struct _2bp {
    Palette_2bit suggested_palette = {0x00000000, 0xd4d4d400, 0x8a8a8a00, 0x40404000};
    uint64_t resX;
    uint64_t resY;
    uint64_t data_size_in_bytes; // Rounded down
    uint8_t trailing_crumbs = 0;
    uint8_t* buffer;

    uint8_t* make_new_buffer (uint64_t x, uint64_t y){
        resX = x;
        resY = y;
        trailing_crumbs = x * y % 4;
        data_size_in_bytes = (resX * resY) / 4; //inve
        int buffer_size = trailing_crumbs == 0 ? data_size_in_bytes : data_size_in_bytes + 1;
        buffer = (uint8_t *)malloc(sizeof(uint8_t) *buffer_size );
        return buffer;
    };
    void free(){
        free(buffer);
    };
    // Must be resX * resY
};

void write_to_rgb_map(Palette_2bit* palette, _2bp* source, uint32_t* destination){
    int i = 0;
    for(i; i < source->data_size_in_bytes; i++) {
        *destination++ = palette->colours[get_11000000(source->buffer[i])]; 
        *destination++ = palette->colours[get_00110000(source->buffer[i])];
        *destination++ = palette->colours[get_00001100(source->buffer[i])];
        *destination++ = palette->colours[get_00000011(source->buffer[i])];
    };
};

bool rgb_map_to_2bp(uint32_t* source, int height, int width, _2bp* destination ){
    destination->make_new_buffer(height, width);
    uint32_t running_colours[4];
    int running_colours_index = 0;
    uint32_t* current_source_pixel = source;
    uint8_t* _4crumb_ptr = destination->buffer;
    int crumbs_written = 0;
    uint8_t crumbs_to_write = 0;
    for(int i = 0; i < height * width; i++){
        //Too many colours detected
        if (running_colours_index == 4){
            return false; // Error case
        }
        //First-pass
        if(running_colours_index == 0){
            running_colours[running_colours_index] = *current_source_pixel;
            running_colours_index++;
            goto write_crumb;
        } 
        //Update running palette
        int current_source_palette_index;
        for(int j = 0; j < running_colours_index; j++){
            if (running_colours[j] == *current_source_pixel){
                current_source_palette_index = j;
                goto write_crumb;
            };
        };
        //New colour for the palette
        running_colours[running_colours_index] = *current_source_pixel;
        current_source_palette_index = running_colours_index;
        running_colours_index++;
        
        write_crumb:
        switch(crumbs_written){
            case 3:{crumbs_to_write = crumbs_to_write | set_00000011(*current_source_pixel);}break;
            case 2:{crumbs_to_write = crumbs_to_write | set_00001100(*current_source_pixel);}break;
            case 1:{crumbs_to_write = crumbs_to_write | set_00110000(*current_source_pixel);}break;
            default:{crumbs_to_write = crumbs_to_write | set_11000000(*current_source_pixel);}break;
        };
        current_source_pixel++;
        crumbs_written++;
        if(crumbs_written == 4){
            *_4crumb_ptr++ = crumbs_to_write;
            crumbs_written = 0;
            crumbs_to_write = 0;
        };
    };
    memcpy(&destination->suggested_palette, &running_colours, sizeof(uint32_t)*4);
    return true;
};

void _2bp_to_file(_2bp* Source, char* fileName){
    FILE *fPtr = fopen("test2.2bp", "w");
    fwrite(&Source, sizeof(_2bp), 1, fPtr);
    fclose(fPtr);
};
