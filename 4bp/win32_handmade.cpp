//original
//THIS IS NOT A FINAL PLATFORM LAYER
// TODO:
// Savegame Locations
// Handle to exe file
// Asset loading path
// Threading
// Raw Input (support for multiple keyboards (for some reason))
// Sleep/TimeBeginPeriod for laptop support
// ClipCursor() for multimonitor support
// Fullscreen support
// WM_SETCURSOR (control cursor visibility)
// QueryCancelAutoplay
// WM_ACTIVEAPP (for when we are not the active app)
// Blit speed improvements (BitBlt)
// Hardware Acceleration (OpenGL, Direct3D... both...?)
// GetKeyboardLayout, (for snail keyboards, international WASD support)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>
#include <math.h>
#include "handmade.h"

#define internal static
#define local_persist static
#define global_variable static

struct win32_offscreen_buffer {
    BITMAPINFO BitmapInfo;
    void *pMemory;
    int Width = 0;
    int Height = 0;
    int Pitch = 0;
    int BytesPerPixel = 4;
    //pixles always 32 bits wide
};

struct win32_window_dimensions {
    int Width;
    int Height;
};

global_variable bool GlobalRunning;
global_variable bool GlobalPause;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;
global_variable int64_t GlobalPerfCountFrequency;


internal win32_window_dimensions Win32GetWindowDimensions(HWND *Window){
            RECT ClientRect;
            GetClientRect(*Window, &ClientRect);
            win32_window_dimensions dimensions = {};
            dimensions.Width =  ClientRect.right - ClientRect.left;
            dimensions.Height = ClientRect.bottom - ClientRect.top;
            return dimensions;
};

internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height){
    //TODO: When window is resized, and we have a pointer to memory, free it, so that we can reallocate for our render window size
    if (Buffer->pMemory){
        VirtualFree(Buffer->pMemory, 0, MEM_RELEASE);
    } ;

    Buffer->Width = Width;
    Buffer->Height = Height;

    //Note: When biHeight is negative, this treats the bitmap as top-down, not bottom up
    //First three bytes of image are the top left pixel, not bottom left
    Buffer->BitmapInfo.bmiHeader.biSize = sizeof(Buffer->BitmapInfo.bmiHeader);
    Buffer->BitmapInfo.bmiHeader.biWidth = Buffer->Width;
    Buffer->BitmapInfo.bmiHeader.biHeight = -Buffer->Height;
    Buffer->BitmapInfo.bmiHeader.biPlanes = 1;
    Buffer->BitmapInfo.bmiHeader.biBitCount = 32;
    Buffer->BitmapInfo.bmiHeader.biCompression = BI_RGB;


    Buffer->BytesPerPixel = 4;
    int BitmapMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel;

    Buffer->pMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);//reserve memory pages
    Buffer->Pitch = Buffer->Width* Buffer->BytesPerPixel; // Length of memory per horizontal stripe of image 

};

static void Win32DisplayBufferInWindow(HDC DeviceContext, win32_window_dimensions dims, win32_offscreen_buffer *Buffer){
    //TOOD: Aspect ratio correction
    StretchDIBits(DeviceContext,
         //X, Y, Width, Height, // Destination
         //X, Y, Width, Height, // Source 
         0, 0, dims.Width, dims.Height, // im not sure what the X and Y are needed for
         0, 0, Buffer->Width, Buffer->Height,
         Buffer->pMemory, // Pointer to bits
         &Buffer->BitmapInfo, // Pointer to bitmap info
         DIB_RGB_COLORS, // DIB_PAL_COLORS for palette, DIB_RGB_COLORS for rgb vals
         SRCCOPY //render opcode
        );
};

LRESULT CALLBACK MainWindowCallback(
    HWND Window, // handle to a window
    UINT Message, // sys-defined messages https://docs.microsoft.com/en-us/windows/win32/winmsg/about-messages-and-message-queues#system-defined-messages
    WPARAM WParam,
    LPARAM LParam
) {
    LRESULT Result = 0;
    switch (Message) {
    case WM_SIZE:{OutputDebugStringA("WM_SIZE\n");};
        break;
    case WM_CLOSE:
        GlobalRunning = false;
        //TODO: Handle with message to user?
        OutputDebugStringA("WM_CLOSE\n");
        break;
    case WM_DESTROY:
        GlobalRunning = false;
        //TODO: handle as error?
        OutputDebugStringA("WM_DESTROY\n");
        break;

    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:{
               uint32_t VKCode = (uint32_t)WParam; 
               bool WasDown = ((LParam & 1 << 30) != 0);
               bool IsDown = ((LParam & (1 << 31)) == 0);
               if (WasDown == IsDown){
                   break;
               };
                   //OS-level keyboard commands
                switch(VKCode){
                    case 'A':{}break;
                    case 'S':{}break;
                    case 'D':{}break;
                    case 'Q':{}break;
                    case 'W':{}break;
                    case 'E':{}break;
                    #if HANDMADE_INTERNAL
                    case 'P':{  if(IsDown) {
                            GlobalPause = !GlobalPause;
                        }}break;
                    #endif
                    case VK_UP:{}break;
                    case VK_DOWN:{}break;
                    case VK_LEFT:{}break;
                    case VK_RIGHT:{}break;
                    case VK_ESCAPE:{}break;
                    case VK_SPACE:{}break;
                    case VK_F4:{
                        bool altKeyDown = ((LParam & 1 << 29) != 0);
                        if (altKeyDown){
                            GlobalRunning = false;
                        }} break;
                    default:
                        break;
                };
            } break;

    case WM_ACTIVATEAPP:
        OutputDebugStringA("WM_ACTIVATEAPP\n");
        break;
    case WM_PAINT: {
        PAINTSTRUCT Paint;
        HDC DeviceContext = BeginPaint(Window, &Paint);

        int X = Paint.rcPaint.left; // unused at the moment?
        int Y = Paint.rcPaint.top; // unused at the moment?
        win32_window_dimensions dims = Win32GetWindowDimensions(&Window); 
        Win32DisplayBufferInWindow(DeviceContext, dims, &GlobalBackBuffer);
        EndPaint(Window, &Paint);
    } break;
    default:
        //OutputDebugStringA("WM_ACTIVATEAPP\n");
        LRESULT Result = DefWindowProcA(Window, Message, WParam, LParam);
        return Result;
    };
    return Result;
}


internal void Win32ProcessPendingMessages(game_controller_input *KeyboardController){
    MSG Message;
    while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)){
    if (Message.message == WM_QUIT) {
        GlobalRunning = false;
    };
    TranslateMessage(&Message);// Gets keyboard inputs!
    DispatchMessageA(&Message);
                    switch(Message.message){
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
        {
        uint32_t VKCode = (uint32_t)Message.wParam; 
        bool WasDown = ((Message.lParam & 1 << 30) != 0);
        bool IsDown = ((Message.lParam & (1 << 31)) == 0);
        if (WasDown == IsDown){
            break;
        };
        switch(VKCode){
            case 'W':{}break;
            case 'A':{}break;
            case 'S':{}break;
            case 'D':{}break;
            case 'Q':{}break;

            case 'E':{}break;
            case VK_UP:{}break;
            case VK_DOWN:{}break;
            case VK_LEFT:{}break;
            case VK_RIGHT:{}break;
            case VK_ESCAPE:{}break;
            case VK_SPACE:{}break;
            case VK_F4:{
                bool altKeyDown = ((Message.lParam & 1 << 29) != 0);
                if (altKeyDown){
                    GlobalRunning = false;
                }} break;
            default:
                break;
        };
    } break;
        }
    }
};

void RenderWeirdGradient(game_offscreen_buffer *Buffer, int XOffset, int YOffset) {
    uint32_t *Pixel = (uint32_t *)Buffer->pMemory; //Make new *Pixel pointer, so that the pMemory stays the same!
    for (int Y = 0; Y < Buffer->Height; ++Y){ //Iterate over rows
        for (int X = 0; X < Buffer->Width; ++X) { //Iterate over columns
            #define Pixel(R,G,B, Padding) ((R << 16) |(G << 8) | B | (Padding << 24));
            uint8_t _red = 25;
            uint8_t _green = X - Y + YOffset;
            uint8_t _blue = X + XOffset;
            // Write byte data to pixel pointer, advance pointer to next pixel in memory.
            *Pixel++ = Pixel(_red,_green,_blue,0);  // *X++ = Write and increment. Will be useful!
        }
    }
};

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender){
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    RenderWeirdGradient(Buffer, GameState->XOffset, GameState->YOffset);
    // debug_read_file_result Sprite = DEBUG_PlatformReadEntireFile("bitmap.bmp");
    // RenderFile(Buffer, &Sprite, 400, 640);
}

int CALLBACK WinMain(
    HINSTANCE Instance, //  handle to executable
    HINSTANCE PrevInstance,//  legacy
    LPSTR     cmdLine, //  command line when program starts
    int       showCmd // 
) {
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    int64_t PerfCountFrequency = PerfCountFrequencyResult.QuadPart;

//desired schedular granularity
    UINT DesiredSchedulerMS = 1;
    bool SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);

    WNDCLASS WindowClass = {};
    Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);
    WindowClass.style =  CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    WindowClass.lpfnWndProc = MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    const int FramesOfAudioLatency = 3;
    const int MonitorRefreshRate = 60;
    const int GameUpdateHz = MonitorRefreshRate / 2;
    float TargetSecondsPerFrame = 1.0f / (float)GameUpdateHz;
    if (!RegisterClass(&WindowClass)){
        return 0;
    };
    HWND Window =
            CreateWindowEx(
            0,                                  //DWORD     dwExStyle,
            WindowClass.lpszClassName,          //LPCSTR    lpClassName,
            "HandmadeHero",                     //LPCSTR    lpWindowName,
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,   //DWORD     dwStyle, window style // WS_VISIBLE = visible on startup
            CW_USEDEFAULT,                      //int       X,
            CW_USEDEFAULT,                      //int       Y,
            CW_USEDEFAULT,                      //int       nWidth,
            CW_USEDEFAULT,                      //int       nHeight,
            0,                                  //HWND      hWndParent,
            0,                                  //HMENU     hMenu,
            Instance,                           //HINSTANCE hInstance,
            0                                   //LPVOID    lpParam
                );

    if (!Window) {
        return;
    };

    //CS_OWNDC flag in WindowStyles enables permanent device context
    HDC DeviceContext = GetDC(Window);
    GlobalRunning = true;
                            
    game_memory GameMemory = {};
    GameMemory.PermanentStorageSize = Megabytes(64);
    GameMemory.TransientStorageSize = Gigabytes(1);
    uint64_t TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;

    GameMemory.PermanentStorage = VirtualAlloc(0, TotalSize,
            MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    GameMemory.TransientStorage = ((uint8_t *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize);


    while (GlobalRunning) {
        for(int ButtonIndex = 0; ButtonIndex < ArrayLength(OldKeyboardController->Buttons); ++ButtonIndex){
            NewKeyboardController->Buttons[ButtonIndex].EndedDown = OldKeyboardController->Buttons[ButtonIndex].EndedDown;
        };
        Win32ProcessPendingMessages(NewKeyboardController);
        if(GlobalPause){
            continue;
        }
        DWORD MaxControllerCount = XUSER_MAX_COUNT;
        // game_controller_input *Controllers = Input->Controllers;

        game_offscreen_buffer GraphicsBuffer = {};
        GraphicsBuffer.pMemory = GlobalBackBuffer.pMemory;
        GraphicsBuffer.Width = GlobalBackBuffer.Width;
        GraphicsBuffer.Height = GlobalBackBuffer.Height;
        GraphicsBuffer.Pitch = GlobalBackBuffer.Pitch;

        GameCode.UpdateAndRender(&GameMemory, NewInput, &GraphicsBuffer);

        LARGE_INTEGER AudioWallClock = Win32GetWallClock();
        float FromBeginToAudioSeconds = Win32GetSecondsElapsed(FlipWallClock, AudioWallClock);

        DWORD PlayCursor;
        DWORD WriteCursor;


        LARGE_INTEGER EndCounter = Win32GetWallClock();
        float MSPerFrame = 1000.0f*Win32GetSecondsElapsed(LastCounter, EndCounter);                    
        LastCounter = EndCounter;
        win32_window_dimensions dims = Win32GetWindowDimensions(&Window);

        Win32DisplayBufferInWindow(DeviceContext, dims, &GlobalBackBuffer);

        ReleaseDC(Window,DeviceContext);


#if 0 //Debug blocc
        int32_t MegacyclesPerFrame = (int32_t)(CyclesElapsed / (1000 * 1000));
        // int32_t MegacyclesPerFrame = (int32_t)(CyclesElapsed / (1000 * 1000));
        int32_t MSPerFrame = (int32_t)((1000* CounterElapsed) / PerfCountFrequency);
        int FPS = PerfCountFrequency / CounterElapsed;
        LastCounter = EndCounter;
        LastCycleCount = EndCycleCount; // Sets up looping clock
        char debugStringBuffer[256];
        wsprintfA(debugStringBuffer,
        "Milliseconds/frame %d ms, %d FPS, %d mc/f \n",
        MSPerFrame, FPS, MegacyclesPerFrame);
        OutputDebugStringA(debugStringBuffer);
#endif
        
    };
    return 0;
}

// int main(){
//     int i = 45;
//     printf("Hello world!");
//     return 0;
// }