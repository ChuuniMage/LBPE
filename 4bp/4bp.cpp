#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

union Palette_4bit {
    uint32_t colours[16];
    struct {
        uint32_t Transparent;
        uint32_t _1;
        uint32_t _2;
        uint32_t _3;
        uint32_t _4;
        uint32_t _5;
        uint32_t _6;
        uint32_t _7;
        uint32_t _8;
        uint32_t _9;
        uint32_t _10;
        uint32_t _11;
        uint32_t _12;
        uint32_t _13;
        uint32_t _14;
        uint32_t _15;
    };
};

#define get_11110000(a) (uint8_t)a>>4 // shifts everything to the right 4 bits, getting the top 4 and scrubbing everything into 0s from the left
#define get_00001111(a) (uint8_t)a&15 // removes all bits above 4 bits, by &-grabbing the first 4 bits

#define set_11110000(a) (uint8_t)a<<4 ///
#define set_00001111(a) (uint8_t)a&15 ///

#define double_4bit(high, low) set_11110000(high) | (uint8_t)low 

struct _4bp {
    Palette_4bit suggested_palette;
    uint64_t resX;
    uint64_t resY;
    uint64_t data_size_in_bytes;
    bool hasTrailingNybble = false;
    uint8_t* buffer;

    uint8_t* make_new_buffer (uint64_t x, uint64_t y){
        resX = x;
        resY = y;
        if(!(x * y % 2) == 0){
            hasTrailingNybble = true;
        };
        data_size_in_bytes = (resX * resY) / 2; //inve
        buffer = (uint8_t *)malloc(sizeof(uint8_t) *data_size_in_bytes);
        return buffer;
    };
    // Must be resX * resY
};

void write_to_rgb_map(Palette_4bit* palette, _4bp* source, uint32_t* destination){
    int i = 0;
    for(i; i < source->data_size_in_bytes; i++) {
        *destination++ = palette->colours[get_11110000(source->buffer[i])];
        *destination++ = palette->colours[get_00001111(source->buffer[i])];
    };
    if(source->hasTrailingNybble){
        *destination++ = palette->colours[get_11110000(source->buffer[source->data_size_in_bytes])];
    };
};


void swap_palette_key_in_pixels(_4bp* source, uint8_t swap_nibbles){
    uint8_t swap_1 = get_11110000(swap_nibbles);
    uint8_t swap_2 = get_00001111(swap_nibbles);
    uint8_t* pixel = source->buffer;
    for (int i = 0; i < source->data_size_in_bytes; i++ ){
        uint8_t high = get_11110000(*pixel);
        uint8_t low = get_00001111(*pixel);

        if(high == swap_1){
            high = swap_2;
        } else if (high == swap_2){
            high == swap_1;
        };

        if(low == swap_1){
            low = swap_2;
        } else if (low == swap_2){
            low == swap_1;
        };
        uint8_t newPixel = double_4bit(high, low);

        *pixel++ = newPixel;
    }
    if(source->hasTrailingNybble){
        uint8_t high = get_11110000(*pixel);
        if(high == swap_1){
            high = swap_2;
        } else if (high == swap_2){
            high == swap_1;
        };
        uint8_t newPixel = double_4bit(high, 0);
        *pixel++ = newPixel;
    };
};

#include <windows.h>
int main(){
    _4bp test = {};
    test.make_new_buffer(4, 3);
    //4 5 6 7 0 full dark med light empty;
    uint8_t full_dark_pair = double_4bit(2, 3);
    uint8_t med_light_pair = double_4bit(4, 5);
    uint8_t odd_ninth_elem = set_11110000(6);


    char testCharPalette[16] = { ' ','A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I','J','K','L','M','N'};

    for(int i = 0; i < test.data_size_in_bytes; i++){
        test.buffer[i] = i % 2 == 0 ? full_dark_pair : med_light_pair;
    };
    FILE *fPtr = fopen("test2.4bp", "w");
    fwrite(&test, sizeof(_4bp), 1, fPtr);
    fclose(fPtr);
    FILE *fPtr2 = fopen("test2.4bp", "r");
    _4bp read_test = {};

    fread(&read_test, 1, sizeof(_4bp),fPtr2);
    char gradient[13] = {0};
    gradient[12] = '\0';
    char* p_gradient = gradient;
    for(int i = 0; i < test.data_size_in_bytes; i++) {
        uint8_t byteToCopy = read_test.buffer[i];
        *p_gradient++ = testCharPalette[get_11110000(test.buffer[i])]; 
        *p_gradient++ = testCharPalette[get_00001111(test.buffer[i])];
    };
    fclose(fPtr2);
    printf("Partial grad test: -> %s \n", gradient);
    printf("Divide test: 9 / 2 %i", (9 / 2));
    return 0;
};