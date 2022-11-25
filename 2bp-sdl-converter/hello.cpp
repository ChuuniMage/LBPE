#include <stdio.h>
#include <SDL.h>
#include <windows.h>
#include "2bp.cpp"

#define TempDebugOutput(...) char debugStringBuffer[256];\
wsprintfA(debugStringBuffer, __VA_ARGS__); \
OutputDebugStringA(debugStringBuffer);

const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 200;

#define RedOf(hexRGB888) ((hexRGB888 >> 16) & 255)
#define GreenOf(hexRGB888) ((hexRGB888 >> 8) & 255)
#define BlueOf(hexRGB888) (hexRGB888 & 255)
#define u8RGB_To_u32_RGB888(R,G,B) (R << 16) | (G << 8) | B

void replace_palette(SDL_Surface* target, Palette_2bit current_palette, Palette_2bit new_palette){
    uint32_t* pixel_ptr = (uint32_t*)target->pixels;
    for (int i = 0; i <target->h * target->w; i++, pixel_ptr++){
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        SDL_GetRGB(*pixel_ptr,target->format,&red,&green,&blue);
        uint32_t detected_colour = u8RGB_To_u32_RGB888(red,green,blue);
        for (int j = 0; j < 4; j++){
            uint32_t replace_color = current_palette.colours[j];
            if(detected_colour == replace_color){
                uint32_t new_color = new_palette.colours[j];
                *pixel_ptr = SDL_MapRGB(target->format,RedOf(new_color),GreenOf(new_color),BlueOf(new_color));
            };
        };
    };
};

int main( int argc, char* args[] ){
    SDL_Window* window = NULL;
    SDL_Surface* screenSurface = NULL;
    bool quit = false;

    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ){
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        goto exit_cleanup;
    } 

    SDL_Surface* image_to_convert = SDL_LoadBMP("church.bmp");
    SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_RGB888);
    SDL_Surface* intermediary = SDL_ConvertSurface(image_to_convert, format, 0);
    _2bp converted_2bp = {};

    printf("bytes per pixel ->, %i \n", image_to_convert->format->BytesPerPixel);
    printf("current format ->, %s \n", SDL_GetPixelFormatName(image_to_convert->format->format));
    if(rgb_map_to_2bp((uint32_t*)intermediary->pixels,intermediary->h, intermediary->w, &converted_2bp, FIRST_COLOR_IS_TRANSPARENT)){
        _2bp_to_file(&converted_2bp, "success_church_conversion.2bp");
        printf("Successful convert! \n");
    } else {
        printf("Failed to convert!");
    };
    SDL_FreeSurface(intermediary);


    //Create window
    window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    if( window == NULL ){
        printf("Window pinged null");
        printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        goto exit_cleanup;
    }  

    char* quit_bmp_path = "test2.bmp";
    char* path = quit_bmp_path;

    const int FPS = 60;
    const int frameDuration = 1000 / FPS;
    SDL_Surface* WindowSurface = SDL_GetWindowSurface( window );

    Uint32* TempPixelBuffer = (Uint32*)malloc(sizeof(Uint32)*WindowSurface->h*WindowSurface->w);

    uint8_t YOffset = 0;
    uint8_t XOffset = 0;
    // FILE *church_palette = fopen("success_church_conversion.2bp", "r");

    _2bp _2bp_file_handle = {};
    _file_to_2bp(&_2bp_file_handle, "success_church_conversion.2bp");
    write_to_buffer(&_2bp_file_handle.suggested_palette, &_2bp_file_handle, TempPixelBuffer);

    int size_b = WindowSurface->h * WindowSurface->w;
    Uint32* WritePtr = (Uint32*)WindowSurface->pixels;
    for(int i = 0; i < size_b; i++){
        *WritePtr++ = *TempPixelBuffer++;
    }
    SDL_UpdateWindowSurface(window);

    int honk = 0;
    while (quit != true){
        Uint32 frameStart = SDL_GetTicks();
        SDL_Event e;
        static Palette_2bit current_palette = _2bp_file_handle.suggested_palette;
        while( SDL_PollEvent( &e ) != 0 ){
            if (e.type == SDL_QUIT){
                quit = true;
            }
            if(e.type == SDL_KEYDOWN){
                static Palette_2bit old_palette;
                Palette_2bit pal_1 = {0x0E0F21,0x445975,  0xCBF1F5,  0x050314};
                Palette_2bit pal_2 = {0x8A7236, 0x3D2D17, 0x1A1006, 0xEBE08D,};
                Palette_2bit pal_3 = {0x1D2B19, 0x456E44, 0x8EE8AF, 0x0B1706};
                switch(e.key.keysym.sym){
                    case SDLK_1: { old_palette = current_palette; current_palette = _2bp_file_handle.suggested_palette;}break;
                    case SDLK_2: { old_palette = current_palette; current_palette = pal_1;}break;
                    case SDLK_3: { old_palette = current_palette; current_palette = pal_2;}break;
                    case SDLK_4: { old_palette = current_palette; current_palette = pal_3;}break;
                }
                replace_palette(SDL_GetWindowSurface(window),old_palette, current_palette);
                SDL_UpdateWindowSurface(window);
            };
        }
        int frameTime = SDL_GetTicks() - frameStart;
        if (frameDuration > frameTime){
            SDL_Delay(frameDuration - frameTime);
        };
    }
    exit_cleanup:
    SDL_DestroyWindow( window );
    SDL_Quit();

    return 0;
}