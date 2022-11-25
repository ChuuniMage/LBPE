#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#define ArrayLength(X) (sizeof(X) / sizeof(X[0]))

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
    Palette_2bit suggested_palette = [0x00000000, 0xd4d4d400, 0x8a8a8a00, 0x40404000];
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
        *destination++ =  palette->colours[get_11000000(test.buffer[i])]; 
        *destination++ =  palette->colours[get_00110000(test.buffer[i])];
        *destination++ =  palette->colours[get_00001100(test.buffer[i])];
        *destination++ =  palette->colours[get_00000011(test.buffer[i])];
    };
};

void _2bp_to_file(_2bp* Source, char* fileName){
    FILE *fPtr = fopen("test2.2bp", "w");
    fwrite(&test, sizeof(_2bp), 1, fPtr);
    fclose(fPtr);
};

#include <windows.h>
#include <string.h>
int main(){
    _2bp test = {};
    test.make_new_buffer(4, 4);
    //4 5 6 7 0 full dark med light empty;
    uint8_t blank_abc = quad_2bit(0, 1, 2, 3);
    uint8_t as = quad_2bit(1, 1, 1, 1);
    uint8_t cs = quad_2bit(3, 3, 3, 3);


    char testCharPalette[4] = { ' ','A', 'B', 'C'};
    printf("Size in bytes -> %I64i \n", test.data_size_in_bytes);
    for(int i = 0; i < test.data_size_in_bytes ; i++){
        test.buffer[i] = i % 2 == 0 ? blank_abc : i % 3 == 0 ? as : cs;
    };

    FILE *fPtr = fopen("test2.2bp", "w");
    fwrite(&test, sizeof(_2bp), 1, fPtr);
    fclose(fPtr);
    FILE *fPtr2 = fopen("test2.2bp", "r");
    _2bp read_test = {};
    fread(&read_test, 1, sizeof(_2bp),fPtr2);
    char gradient[17] = {0};
    gradient[16] = '\0';
    char* p_gradient = gradient;

    printf("Str len -> %i \n", (int)ArrayLength(gradient));

    int number_of_times_looped = 0;
    int number_of_times_written = 0;
    for(int i = 0; i < test.data_size_in_bytes; i++) {
        *p_gradient++ = testCharPalette[get_11000000(test.buffer[i])]; 
        *p_gradient++ = testCharPalette[get_00110000(test.buffer[i])];
        *p_gradient++ = testCharPalette[get_00001100(test.buffer[i])];
        *p_gradient++ = testCharPalette[get_00000011(test.buffer[i])];
    };

    printf("Str len after write-> %i \n", (int)ArrayLength(gradient));

    printf("number of times looped -> %i\n", number_of_times_looped);
    printf("number of times number_of_times_written -> %i\n", number_of_times_written);
    fclose(fPtr2);
    printf("2 bit Partial grad test: -> [%s] \n", gradient);
    return 0;
};