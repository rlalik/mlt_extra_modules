# DEPRECATION NOTICE

This package was mainly created to introduce typewriter effect. As the typewriter effect was merged into mlt-6 and is available there, it is not recommended to install this package separately. It is however keept updated with the version in mlt (bigfixes, etc).



MLT FRAMEWORK EXTRA MODULES README
---
Written by Rafal Lalik <rafallalik@gmail.com>

This is a collection of my custom effects for MLT and Kdenlive. Currently the collection consists of following effects:
* TypeWriter

MLT is a LGPL multimedia framework designed for television broadcasting, and melted is a GPL multi-unit video playout server with realtime effects.

### Prerequisities
This README recommends different installation procedure than in previous version (< 0.2.0). If you already installed this package from sources, and you kept the build tree, I recommend to call `make uninstall` or `sudo make uninstall` to remove the installed files from previous location.
The new recommended method might install files into different locations. If you didn't keep your build tree, the standard procedure most likely installed files into `/usr/local`. Inspect the directories for files related to this install:

```shell
{lib,lib64}/libmltrl.so
{lib,lib64}/pkgconfig/mlt-extra_modules.pc
share/mlt/rl/filter_typewriter.yml
```
and remove them

### Requirements
The package require Qt5:Core and Qt5:Xml developer package (headers and libraries) to availabe in the system.

### Configuration
Configuration can be invoked with `./configure` script or using cmake.

#### Using `./configure`
Configuration is triggered by running:

```shell
./configure --prefix=$(pkg-config mlt-framework --variable=prefix) --libdir=$(pkg-config mlt-framework --variable=libdir)
```
More information on usage is found by running:
```shell
./configure --help
```
To install the kdenlive effect file add option:
```shell
--kdenlive-effects=directory
```
where `directory` is a path to kdenlive effects dir. Typical location on your system might looks like `/usr/share/kdenlive/effects`.

#### Using cmake
Create build directory, and call `cmake` pointing to source directory, e.g.
```shell
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$(pkg-config mlt-framework --variable=prefix)
```
The Kdenlive effects will be installed by default. To disable, pass additional option `-DKDENLIVE=OFF`.

### Compilation
Once configured, it should be sufficient to run:
```shell
make
```
to compile the system.

### Installation
The install is triggered by running:
```shell
make install
```
or
```shell
sudo make install
```
if installed into system location from a user account.

---

# Usage

## TypeWriter effect
TypeWriter effect rproduces behavior of typewriter machine. By default it expands text by characters, but other mods like expansion by words and lines are also available, as well as possibility to use full sequence and macros commands provide by TypeWriter library (see https://github.com/rlalik/TypeWriter for details).

The effect uses kdenlive titler producer and thus it usage is presented in context of kdenlive.

The effects ha customizable four parameters:
* Step length, parameter name: `step_length`, values: 1-240, default: 25
* Step sigma, parameter name: `step_sigma`, values 0-20, default: 0
* Random seed, parameter name: `random_seed`, values: 0-100, default 0
* Macro type, parameter name: `macro_type`, values: 0-3, default: 1, see description below.

### Step length
This parameters, expressed in unit of frames/step, tells how fast next element will be displayed. The speed is constant over the whole clip and effect period. The minimal value is 1, maximal is 240. Can be overwritten is the effect xml file.

### Step sigma
Allows for introducing small fluctuation to the step length. Assuming the step length `sl` is set, and the `sigma` parameter is larger than 0, then each frame `k` is displayed at `k*sl + N(0,s)` position, where `N(m,s)` is normal distribution. The N`(0,s)` value is round to full integers. If `s` is large enough in respect to `fs`, then there is a risk that negative value of `N(0,s)` will result in `(k+1)*sl - |N(0,s)| < k*fs+|N(0,s)|` and thus the frames would be in wrong order. In a such situation, the `k+1` frame is forced to be located at `k*fs+1`. The fluctiation is applied on the basic rid of frames, the length of the sequence is equal to `n*sl` where `n` is number of steps.

### Random seed
The random generator for fluctuations is initialized with constant seed to assure predictible pattern. The `seed` parameter changes the initial value of random generator.

### Macro type
Defines how the text is rendered. For value `0` the text field content is evaluated as typewryter sequence/macro. For value `1`, the text is expaned as character macro - every step new character is uncover. For value `2`, the text is expaned by words, and for value `3`, the text is expanded by lines.
