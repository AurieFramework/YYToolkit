# YYToolkit
YYToolkit is a tool for creating mods and altering GameMaker games, supported on Windows 8.1 → Windows 11.

## Features
- Plugin Support
  - Run custom code in the game's context via a robust callback system
  - Detour game functions to your plugin to implement custom functionality
- Well-documented
  - The wiki has most API functions documented for both YYTK Legacy and YYTK Next
- Full access to GameMaker functions, scripts, and variables
- Full compatibility with existing tools like UndertaleModTool or Dogscepter
- Runtime loading / unloading
  - Mods can be managed seamlessly while the game is running

## Getting help
- Help with YYToolkit or Aurie Framework - [Arch Wizards Server](https://discord.gg/vbT8Ed4cpq)
- General game modding questions - [Underminers Server](https://discord.gg/3ESNF4QPrh)

## YYTK Editions
YYTK is split on two branches, that being [Legacy](https://github.com/AurieFramework/YYToolkit/tree/legacy) and Stable (Next).

> [!NOTE]
> ❌ = Unsupported
> 🟠 = Limited support
> 🟡 = Supported, unreliable
> ✅ = Supported

| Point of comparison | YYToolkit Legacy | YYToolkit Next |
| :---: | :---: | :---: |
| GM 1.4.x                     | 🟡 | ❌ |
| GM 2.x.x                     | ✅ | ❌ |
| GM 2022.x                    | 🟠 | ✅ |
| GM 2023.x                    | ❌ | ✅ |
| VM runner support            | ✅ | 🟡 |
| x86 runner support           | ✅ | ❌ |
| x64 runner support           | 🟡 | ✅ |
| Active Development           | ❌ | ✅ |
| Accessing global variables   | ✅ | ✅ |
| Accessing local variables    | 🟠 | ✅ |
| Accessing built-in variables | ❌ | ✅ |
| Accessing data structures    | ❌ | ✅ |
| Accessing scripts            | 🟡 | ✅ |
| Accessing rooms              | ❌ | ✅ |
| Manipulating object events   | ❌ | 🟠 |

## Contributors
- YYTK Next
  - [mashirochan](https://github.com/mashirochan) for the awesome work on streamlining the installation process
  - [PippleCultist](https://github.com/PippleCultist) for reporting bugs
  - [ramennnoodle](https://github.com/liraymond04) for creating how-to guides on getting YYToolkit and Aurie running on Linux
- YYTK Legacy
  - [MousieDev](https://github.com/MousieDev) for creating makefiles (which probably don't work at this point)
  - [Miepee](https://github.com/Miepee) for the awesome icon!
  - [TheEternalShine](https://github.com/TheEternalShine) for testing and giving feedback on in-development versions of the tool.
