# YYToolkit
YYToolkit is a tool for creating mods and altering GameMaker games, supported on Windows 8.1 → Windows 11.

The project is split in two branches based on the supported GameMaker runtime versions:
  - YYTK Next
    - Versions supported: 2022.4 → 2023.11
    - Actively maintained
    - Built on the [Aurie Framework](https://github.com/Archie-osu/Aurie)
    - x64 only
  - YYTK Legacy
    - Versions supported: 1.4.9999 → 2022.3
    - No longer maintained
    - Self-contained
    - x86 and x64

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

## Contributors
- YYTK Legacy
  - [MousieDev](https://github.com/MousieDev) for creating makefiles (which probably don't work at this point)
  - [Miepee](https://github.com/Miepee) for the awesome icon!
  - [TheEternalShine](https://github.com/TheEternalShine) for testing and giving feedback on in-development versions of the tool.
- YYTK Next
  - [mashirochan](https://github.com/mashirochan) for the awesome work on streamlining the installation process
  - [PippleCultist](https://github.com/PippleCultist) for reporting bugs
  - [ramennnoodle](https://github.com/liraymond04) for creating how-to guides on getting YYToolkit and Aurie running on Linux
