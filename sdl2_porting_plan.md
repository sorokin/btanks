### Plan for Porting Battle Tanks from SDL 1.2 to SDL 2

This document outlines the necessary steps to migrate the Battle Tanks project to SDL 2. The migration is categorized into headers, video, events, audio, and module-specific changes.

---

### 1. Header and Library Changes
- **Header Files**: Replace `#include <SDL/SDL.h>` with `#include <SDL.h>`. (Completed)
- **CMake**: 
    - Update `find_package(SDL)` to `find_package(SDL2)`. (Completed)
    - Update `target_link_libraries` from `SDL::SDL` to `SDL2::SDL2`. (Completed)
    - Note: The project now uses system/vcpkg provided `SDL2_image`. (Completed)

### 2. Video and Rendering Migration
Battle Tanks used `SDL_SetVideoMode` and `SDL_Flip`, which are removed in SDL 2.

- **Window and Renderer Management**: 
    - Replace `SDL_SetVideoMode` with `SDL_CreateWindow` and `SDL_CreateRenderer` (or `SDL_CreateWindowAndRenderer`).
    - The current codebase (e.g., `engine/src/window.cpp`, `sdlx/surface.cpp`) relies heavily on `SDL_Surface` as the primary screen buffer.
    - **Step 1**: Implement a wrapper that maintains the `SDL_Surface` for the screen if a direct port to `SDL_Texture` is too complex initially. 
    - **Step 2**: Use `SDL_UpdateWindowSurface` if using the software surface, or preferably, migrate to `SDL_Texture` and use `SDL_RenderPresent`.
- **Display Flags**: 
    - `SDL_HWSURFACE`, `SDL_ANYFORMAT`, `SDL_DOUBLEBUF` are removed.
    - `SDL_FULLSCREEN` replaced by `SDL_WINDOW_FULLSCREEN`.
    - `SDL_OPENGL` replaced by `SDL_WINDOW_OPENGL`.
- **Surface Conversions**:
    - `SDL_DisplayFormat` and `SDL_DisplayFormatAlpha` are removed. Replace with `SDL_ConvertSurface` or similar logic using the window's pixel format.
- **Alpha and Color Key**:
    - `SDL_SetAlpha` -> `SDL_SetSurfaceAlphaMod` or `SDL_SetColorKey`.
    - `SDL_SRCCOLORKEY` and `SDL_SRCALPHA` flags are removed. Alpha blending is now controlled via `SDL_SetSurfaceBlendMode`.
    - `SDL_PixelFormat::colorkey` is removed. Use `SDL_GetColorKey`.
- **Special Wrappers**:
    - The project contained custom wrappers like `glSDL` and `d3dsdl`. These have been removed as SDL 2 provides native OpenGL/Direct3D support through its hardware-accelerated renderer. (Completed)

### 3. Events Migration
- **Keyboard Events**: 
    - `event.key.keysym.unicode` is removed. Use `SDL_TEXTINPUT` events for text entry.
    - `SDL_EnableUNICODE` is removed.
    - `SDL_EnableKeyRepeat` is removed. SDL 2 handles key repeat automatically; check `event.key.repeat`.
- **Mouse Events**:
    - `SDL_MOUSEBUTTONDOWN` and `SDL_MOUSEMOTION` are similar, but check for coordinate changes if scaling is applied via the renderer.
- **Active Events**:
    - `SDL_ActiveEvent` and `SDL_GetAppState` are replaced by `SDL_WindowEvent`.

### 4. Audio Migration (`clunk` module)
The `clunk` module uses the basic SDL 1.2 audio API.

- **Audio Opening**:
    - `SDL_OpenAudio` still exists but `SDL_OpenAudioDevice` is preferred in SDL 2 for better multi-device support.
- **Locking**:
    - `SDL_LockAudio` / `SDL_UnlockAudio` are still available but `SDL_LockAudioDevice` is preferred if using the new API.

### 5. Module-Specific Tasks

#### sdlx (C++ Wrapper)
- **surface.h/cpp**: 
    - Fix missing defines: `SDL_HWSURFACE`, `SDL_FULLSCREEN`, `SDL_SRCCOLORKEY`, `SDL_SRCALPHA`.
    - Replace `SDL_GetVideoSurface`, `SDL_SetVideoMode`, `SDL_UpdateRect`, `SDL_Flip`.
    - Replace `SDL_WM_ToggleFullScreen` with `SDL_SetWindowFullscreen`.
    - Replace `SDL_SetAlpha` with `SDL_SetSurfaceAlphaMod` / `SDL_SetSurfaceBlendMode`.
    - Replace `SDL_DisplayFormat` / `SDL_DisplayFormatAlpha`.
- **system.h/cpp**:
    - Replace `SDL_VideoDriverName` with `SDL_GetCurrentVideoDriver`.
    - Replace `SDL_GetVideoInfo` and `SDL_VideoInfo` with `SDL_GetDisplayMode` / `SDL_GetWindowDisplayMode`.
- **joystick.cpp**:
    - Fix `SDL_JoystickName` usage (now takes `SDL_Joystick*`, use `SDL_JoystickNameForIndex` for index).
- **c_map.cpp**:
    - Fix `SDL_SRCALPHA` and `colorkey` access.

#### clunk (Audio Library)
- Update `clunk/backend/sdl/backend.cpp` to initialize SDL 2 audio subsystem. (Partially done, still uses `SDL_OpenAudio`)

#### General
- **Version Checking**: Replace `SDL_Linked_Version` with `SDL_GetVersion`. In SDL2 `SDL_Linked_Version` is a macro for backward compatibility, but the API changed. Actually, `SDL_GetVersion` should be used to get the linked version. (Completed)

#### engine (Game Logic)
- `engine/src/window.cpp`: Major rewrite needed. Remove `SDL_EnableUNICODE`, `SDL_EnableKeyRepeat`. Handle `SDL_Window` and `SDL_Renderer`.
- `engine/src/console.cpp`: Replace `unicode` handling with `SDL_TEXTINPUT`.

---

### 6. Categorized Migration Steps (Summary)

| Feature | SDL 1.2 | SDL 2.0 | Status |
| :--- | :--- | :--- | :--- |
| Initialization | `SDL_Init(SDL_INIT_VIDEO)` | `SDL_Init(SDL_INIT_VIDEO)` | ✓ |
| Version Info | `SDL_Linked_Version` | `SDL_GetVersion` | ✓ |
| Window | `SDL_SetVideoMode` | `SDL_CreateWindow` | Needs work |
| Rendering | `SDL_Flip`, `SDL_UpdateRect` | `SDL_RenderPresent` or `SDL_UpdateWindowSurface` | Needs work |
| Surface Conversion | `SDL_DisplayFormat` | `SDL_ConvertSurface` | Needs work |
| Alpha Blending | `SDL_SetAlpha` | `SDL_SetSurfaceAlphaMod` | Needs work |
| Key Info | `keysym.unicode` | `SDL_TextInputEvent` | Needs work |
| Window Events | `SDL_VIDEORESIZE`, `SDL_ACTIVEEVENT` | `SDL_WINDOWEVENT` | Needs work |
| Joystick Name | `SDL_JoystickName(index)` | `SDL_JoystickNameForIndex(index)` | Needs work |
| glSDL / d3dsdl | Wrappers for SDL 1.2 | Removed (use SDL_Renderer) | ✓ |
| Video Playback | `smpeg` (optional) | Needs a new backend or removal | TBD |
