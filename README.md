# YYToolkit
YYToolkit is a tool for creating mods and altering GameMaker games.

Currently, only Windows is supported.

## Features
- Plugin Support
  - Allows for easily extending YYToolkit functionality
  - Gives you the ability to make mods in YYC environments
- Well-documented
  - Most APIs are documented on the Wiki, including a full guide on making plugins.
- Basic built-in GML function caller
  - Allows for calling any built-in GML function just by pressing F10.
- Full compatibility with existing tools like UndertaleModTool or Dogscepter.
- Runtime loading / unloading
  - Mods and even the tool itself can be (un)loaded while the game is running, seamlessly.

## Directory Structure
- MakeConfig - a directory for ``make`` files - currently unused, I don't even know how it works tbh.
- Plugins - Contains source code of all the plugins
  - ExamplePlugin - Source code for the Example plugin
  - Chapter2_EnableDebug - Source code for the Chapter 2 Debug plugin
  - Chapter2_HardMode - Source code for the Hard Mode plugin
- YYLauncher - Source code for the launcher
- YYToolkit - Source code for the DLL

## Contributors
- [MousieDev](https://github.com/MousieDev) for creating makefiles (which I'm not sure even work at this point)
- [Miepee](https://github.com/Miepee) for the awesome icon!
- [TheEternalShine](https://github.com/TheEternalShine) for testing and giving feedback on in-development versions of the tool.
