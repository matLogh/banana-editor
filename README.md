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
- `kratta_editor`
- `oscar_editor`

Build only the library for integration as submodule to your project:

```bash
cmake -S . -B build -DBANANA_EDITOR_BUILD_EXAMPLES=OFF
cmake --build build
```

`BANANA_EDITOR_BUILD_EXAMPLES` defaults to `ON` when this repository is the top-level
CMake project, and defaults to `OFF` when included through `add_subdirectory()`.

## Use As A Submodule

In a parent CMake project:

```cmake
add_subdirectory(path/to/banana-editor)

target_link_libraries(your_target PRIVATE TBananaEditor)
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

Specify where banana ROOT files are saved:

```bash
./oscar_editor -o path/to/output_dir
./oscar_editor --output path/to/output_dir
```

The output directory is created automatically if it does not exist.

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
