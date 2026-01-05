### Plan for Porting Battle Tanks from SDL 1.2 to SDL 2

This document outlines the necessary steps to migrate the Battle Tanks project to SDL 2. The migration is categorized into headers, video, events, audio, and module-specific changes.

---

### 1. Header and Library Changes
- **Header Files**: Replace `#include <SDL/SDL.h>` with `#include <SDL.h>`.
- **CMake**: 
    - Update `find_package(SDL)` to `find_package(SDL2)`.
    - Update `target_link_libraries` from `SDL::SDL` to `SDL2::SDL2`.
    - Note: The project currently includes a vendored `SDL_image`. It should be either updated to SDL2 version or replaced with a system/vcpkg provided `SDL2_image`.

### 2. Video and Rendering Migration
Battle Tanks uses `SDL_SetVideoMode` and `SDL_Flip`, which are removed in SDL 2.

- **Window and Renderer Management**: 
    - Replace `SDL_SetVideoMode` with `SDL_CreateWindow` and `SDL_CreateRenderer` (or `SDL_CreateWindowAndRenderer`).
    - The current codebase (e.g., `engine/src/window.cpp`, `sdlx/surface.cpp`) relies heavily on `SDL_Surface` as the primary screen buffer.
    - **Step 1**: Implement a wrapper that maintains the `SDL_Surface` for the screen if a direct port to `SDL_Texture` is too complex initially. 
    - **Step 2**: Use `SDL_UpdateWindowSurface` if using the software surface, or preferably, migrate to `SDL_Texture` and use `SDL_RenderPresent`.
- **Display Flags**: 
    - Remove `SDL_HWSURFACE`, `SDL_ANYFORMAT`, `SDL_DOUBLEBUF`. These are now handled by the renderer flags (`SDL_RENDERER_ACCELERATED`, `SDL_RENDERER_PRESENTVSYNC`).
    - Use `SDL_WINDOW_FULLSCREEN`, `SDL_WINDOW_RESIZABLE` for window creation.
- **Surface Conversions**:
    - `SDL_DisplayFormat` and `SDL_DisplayFormatAlpha` are removed. Replace with `SDL_ConvertSurface` or similar logic using the window's pixel format.
- **Alpha and Color Key**:
    - `SDL_SetAlpha` -> `SDL_SetSurfaceAlphaMod`.
    - `SDL_SetColorKey` -> `SDL_SetColorKey` (parameter change: flags are replaced by a simple boolean).
- **Special Wrappers**:
    - The project contains custom wrappers like `glSDL` and `d3dsdl`. These should be removed as SDL 2 provides native OpenGL/Direct3D support through its hardware-accelerated renderer.

### 3. Events Migration
- **Keyboard Events**: 
    - `event.key.keysym.unicode` is removed. Use `SDL_TEXTINPUT` events for text entry.
    - `SDLK_...` constants are still available but prefer `SDL_Scancode` for physical key mapping and `SDL_Keycode` for layout-dependent mapping.
- **Mouse Events**:
    - `SDL_MOUSEBUTTONDOWN` and `SDL_MOUSEMOTION` are similar, but check for coordinate changes if scaling is applied via the renderer.
- **Active Events**:
    - `SDL_ActiveEvent` and `SDL_GetAppState` are replaced by `SDL_WindowEvent`.
- **Key Repeat**:
    - `SDL_EnableKeyRepeat` is removed. SDL 2 handles key repeat automatically; check `event.key.repeat`.

### 4. Audio Migration (`clunk` module)
The `clunk` module uses the basic SDL 1.2 audio API.

- **Audio Opening**:
    - `SDL_OpenAudio` still exists but `SDL_OpenAudioDevice` is preferred in SDL 2 for better multi-device support.
- **Locking**:
    - `SDL_LockAudio` / `SDL_UnlockAudio` are still available but `SDL_LockAudioDevice` is preferred if using the new API.

### 5. Module-Specific Tasks

#### sdlx (C++ Wrapper)
- Update `sdlx::Surface`, `sdlx::System`, and `sdlx::Window` classes to store `SDL_Window*` and `SDL_Renderer*` instead of just a screen surface.
- Rewrite `sdlx::Timer` to use `SDL_GetPerformanceCounter` for high-resolution timing, as SDL 2 provides it natively.

#### SDL_image (Vendored)
- The current vendored version is SDL 1.2 compatible.
- **Recommendation**: Delete the vendored `SDL_image` and use `vcpkg` to provide `sdl2-image`. This will resolve compatibility issues with `IMG_Load` and surface formats.

#### clunk (Audio Library)
- Update `clunk/backend/sdl/backend.cpp` to initialize SDL 2 audio subsystem.
- Check `SDL_AudioSpec` fields (some internal fields might have changed).

#### engine (Game Logic)
- `engine/src/window.cpp`: This is the core file for display initialization. Major rewrite needed here.
- `engine/src/console.cpp`: Replace `unicode` handling with `SDL_TEXTINPUT`.

---

### 6. Categorized Migration Steps (Summary)

| Feature | SDL 1.2 | SDL 2.0 |
| :--- | :--- | :--- |
| Initialization | `SDL_Init(SDL_INIT_VIDEO)` | `SDL_Init(SDL_INIT_VIDEO)` |
| Window | `SDL_SetVideoMode` | `SDL_CreateWindow` |
| Rendering | `SDL_Flip`, `SDL_UpdateRect` | `SDL_RenderPresent` or `SDL_UpdateWindowSurface` |
| Surface Conversion | `SDL_DisplayFormat` | `SDL_ConvertSurface` |
| Alpha Blending | `SDL_SetAlpha` | `SDL_SetSurfaceAlphaMod` |
| Key Info | `keysym.unicode` | `SDL_TextInputEvent` |
| Window Events | `SDL_VIDEORESIZE`, `SDL_ACTIVEEVENT` | `SDL_WINDOWEVENT` |
| High-res Timer | Custom (`sdlx/timer.cpp`) | `SDL_GetPerformanceCounter` |
| Video Playback | `smpeg` (optional) | Needs a new backend or removal |
