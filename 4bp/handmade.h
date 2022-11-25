#include <windows.h>

//Header files are effectively just type declared exports?
#if !defined(HANDMADE_H)

#define ArrayLength(X) (sizeof(X) / sizeof(*X))
#define ZeroOut(X) memset(&X, 0, sizeof(X))
#define INPUT_BUFFER_SIZE 256
#define TOKEN_SIZE 64
#define Kilobytes(X) (X *1024)
#define Megabytes(X) (Kilobytes(X) * 1024)
#define Gigabytes(X) (Megabytes(X) * 1024)
#define Terabytes(X) (Gigabytes(X) * 1024)

struct debug_read_file_result {
    uint32_t ContentSize;
    void *Contents;
};

#if HANDMADE_INTERNAL
debug_read_file_result DEBUG_PlatformReadEntireFile(char *FileName);
void DEBUG_PlatformFreeFileMemory(void *Memory);
bool DEBUG_PlatformWriteEntireFile(LPCSTR FileName, uint32_t MemorySize, void *MemoryToWrite);
#endif

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name ) debug_read_file_result name (char *FileName)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name ) void name (void *Memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name ) bool name (LPCSTR FileName, uint32_t MemorySize, void *MemoryToWrite)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);


//HANDMADE_SLOW -> 0 = no slow code, 1 = slow code allowed, for debugging
#if HANDMADE_SLOW
#define Assert(Expression) if(Expression == false){*(int *)0 = 0;}; // Crash if assert is false
#else
#define Assert(Expression)
#endif
//todo: Rendering will become a three-tiered abstraction, pog
#define ArrayLength(X) (sizeof(X) / sizeof(*X))
#define TempDebugOutput(...) char debugStringBuffer[256];\
    wsprintfA(debugStringBuffer, __VA_ARGS__); \
    OutputDebugStringA(debugStringBuffer);

struct game_offscreen_buffer {
    // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    void *pMemory;
    int Width;
    int Height;
    int Pitch;
};

struct game_sound_output_buffer {
    int SamplesPerSecond;
    int SampleCount;
    int16_t* Samples;
};

struct game_button_state {
    int HalfTransitionCount;
    bool EndedDown;
};

struct game_controller_input {
    bool IsAnalog;
    bool IsConnected;
    bool IsKeyboard;
    float StickAverageX;
    float StickAverageY;
    union {
        game_button_state Buttons[12];
        struct {
            game_button_state MoveUp;
            game_button_state MoveDown;
            game_button_state MoveLeft;
            game_button_state MoveRight;

            game_button_state ActionUp;
            game_button_state ActionDown;
            game_button_state ActionLeft;
            game_button_state ActionRight; // ???

            game_button_state LeftShoulder;
            game_button_state RightShoulder;

            game_button_state Back;
            game_button_state Start;
        };
    };
};

struct game_input {
    game_controller_input Controllers[5]; // 4 controllers + keyboard
};

struct game_state {
    int ToneHz;
    int XOffset;
    int YOffset;
    float tSine;
};

inline game_controller_input *GetController(game_input *Input, int ControllerIndex){
    Assert(ControllerIndex < ArrayLength(Input->Controllers));
    return &Input->Controllers[ControllerIndex];
}

struct game_memory {
    bool IsInitialized;
    uint64_t PermanentStorageSize;
    void *PermanentStorage; // REQUIRED to be cleared to zero at startup
    uint64_t TransientStorageSize;
    void *TransientStorage;
    debug_platform_read_entire_file *DEBUG_PlatformReadEntireFile;
    debug_platform_free_file_memory *DEBUG_PlatformFreeFileMemory;
    debug_platform_write_entire_file *DEBUG_PlatformWriteEntireFile;
};

inline uint32_t SafeTruncateUInt64(uint64_t Value){
    #define Maximum_32Bit_Value 0xFFFFFFFF
    Assert(Value <= Maximum_32Bit_Value);
    uint32_t Result = (uint32_t)Value;
    return Result;
};

#define GAME_UPDATE_AND_RENDER(name) void name (game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
GAME_UPDATE_AND_RENDER(GameUpdateAndRenderStub){};
// typedef VOID GAME_UPDATE_AND_RENDER(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer);
// GAME_UPDATE_AND_RENDER *GameUpdateAndRender = [](game_memory*, game_input*, game_offscreen_buffer*)->VOID{return;};


#define GAME_GET_SOUND_SAMPLES(name) void name (game_memory *Memory, game_sound_output_buffer *SoundBuffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);
GAME_GET_SOUND_SAMPLES(GameGetSoundSamplesStub){};

// void GameUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer);

#define HANDMADE_H
#endif
