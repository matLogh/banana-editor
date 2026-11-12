# Banana Editor

Small ROOT/Qt-style GUI helper for drawing and editing graphical cuts ("bananas") on
ROOT histograms. The project builds a `TBananaEditor` shared library and, when
enabled, a couple of detector-specific example launchers.

The repository is intended to work both as a standalone project and as a CMake
submodule inside larger analysis projects.

## Requirements

- CMake 3.16 or newer
- C++ compiler supported by ROOT
- ROOT with these components available: `Core`, `RIO`, `Hist`, `Tree`, `Net`, `Gui`, `Graf`, `Rint`


## Build

Standalone build with example editors enabled:

```bash
cmake -S . -B build
cmake --build build
```

This creates:

- `libTBananaEditor.so`
- `simple_banana_editor`
- `kratta_editor`
- `oscar_editor`

Build only the library for integration as submodule to your project:

```bash
cmake -S . -B build -DBANANA_EDITOR_BUILD_EXAMPLES=OFF
cmake --build build
```

`BANANA_EDITOR_BUILD_EXAMPLES` defaults to `ON` when this repository is the top-level
CMake project, and defaults to `OFF` when included through `add_subdirectory()`. The
bundled editors can also be selected independently:

```bash
cmake -S . -B build -DBANANA_EDITOR_BUILD_KRATTA_EDITOR=ON -DBANANA_EDITOR_BUILD_OSCAR_EDITOR=OFF
```

## Use As A Submodule

In a parent CMake project:

```cmake
add_subdirectory(path/to/banana-editor)

target_link_libraries(your_target PRIVATE TBananaEditor)
```

To build an executable that uses the editor from the parent project, use the helper
provided by the submodule:

```cmake
add_subdirectory(path/to/banana-editor)

add_banana_editor_executable(my_editor src/my_editor.cpp)
```

The bundled example executables remain opt-in when this project is used as a
submodule:

```cmake
set(BANANA_EDITOR_BUILD_OSCAR_EDITOR ON CACHE BOOL "" FORCE)
set(BANANA_EDITOR_BUILD_KRATTA_EDITOR OFF CACHE BOOL "" FORCE)
add_subdirectory(path/to/banana-editor)
```

If the parent project already called `find_package(ROOT ...)`, Banana Editor will
reuse the existing ROOT targets. Otherwise it will find ROOT itself.

## Basic API

```cpp
#include "TBananaEditor.hh"

TList banana_list;
TBananaEditor editor("banana_editor");

editor.SaveToFile("cuts.root");
editor.SetDefaultBananaName("my_cut");
editor.Init(nullptr, 1600, 800, histogram, &banana_list, 0);
```

Useful methods:

- `Init(...)` opens the editor window for a `TH1` or `TH2`.
- `SaveToFile("file.root")` loads existing cuts from the file, if present, and saves
  cuts back on exit as `TCutG` objects.
- `SetDefaultBananaName("prefix")` sets the default prefix used for new banana names.
  New names are created as `prefix_0`, `prefix_1`, etc.

## Editor Controls

Main buttons:

- `Add new banana` starts drawing a new cut.
- `Edit banana` enables editing existing cuts.
- `Done` confirms the active edit.
- `Cancel` cancels the active edit.
- `Exit editor` closes the editor and prompts to save unsaved changes.

The `Default banana name` text field controls the prefix for newly-created bananas.

Keyboard shortcuts inside the canvas:

- `a` / `d`: move histogram view left/right
- `w` / `s`: move histogram view up/down
- `e` / `q`: zoom x-axis in/out
- `x` / `z`: scale x-axis
- `r`: reset histogram view
- `b`: add banana
- `m`: edit bananas
- `l`: toggle linear/log Z scale

## Simple Single-Matrix Editor

`simple_banana_editor` opens any two-dimensional histogram in a ROOT file. The
matrix argument is the complete ROOT object path, including any directories that
contain it:

```bash
./simple_banana_editor \
  --input sum.root \
  --matrix AgataOscarUp/m_Energy_Theta \
  --output C14_ETheta.root \
  --prefix C14_Ex2p7
```

The short options are `-i`, `-m`, `-o`, and `-p`. Positional arguments are also
accepted:

```bash
./simple_banana_editor sum.root AgataOscarUp/m_Energy_Theta C14_ETheta.root C14_Ex2p7
```

The output defaults to `bananas.root` and the name prefix defaults to `banana`.
Existing cuts in the output file are loaded for editing. Newly created cuts are
named `<prefix>_0`, `<prefix>_1`, and so on, and are saved as `TCutG` objects.

## Example Editors

### KRATTA

Default input:

```text
example_data/kratta_sum.root
```

Run:

```bash
./kratta_editor
```

Or pass an input ROOT file as the first positional argument:

```bash
./kratta_editor path/to/kratta_sum.root
```

The editor expects histograms named:

```text
KRATTA/kratta_pd0pd1_<id>
```

Saved cuts are written as:

```text
kratta_pd0pd1_<id>.root
```

### OSCAR

Default input:

```text
example_data/oscar_sum.root
```

Run:

```bash
./oscar_editor
```

Specify an input ROOT file:

```bash
./oscar_editor -i path/to/oscar_sum.root
./oscar_editor --input path/to/oscar_sum.root
```

Specify where OSCARUP and OSCARDOWN banana ROOT files are saved, respectively:

```bash
./oscar_editor -o path/to/oscarup_dir path/to/oscardown_dir
./oscar_editor --output path/to/oscarup_dir path/to/oscardown_dir
```

The output directories are created automatically if they do not exist.

Show options:

```bash
./oscar_editor --help
```

The editor accepts either the compact example layout:

```text
up_blu/m_dE_E_tel<id>
up_nero/m_dE_E_tel<id>
```

or the raw OSCAR layout:

```text
OscarUp/EdE/m_dE_E_tel<id>
OscarDown/EdE/m_dE_E_tel<id>
```

Saved cuts are written as:

```text
oscar_t_<id>.root
```

## Repository Layout

```text
src/              TBananaEditor library sources and ROOT dictionary linkdef
example_editors/  KRATTA and OSCAR example launchers
example_data/     Small example ROOT files
CMakeLists.txt    Library and optional example build
```
