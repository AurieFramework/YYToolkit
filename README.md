# YYToolkit
YYToolkit is a tool for creating mods and altering GameMaker games.

Currently, only Windows is supported.

## Features
- Intercepting script calls and object events
  - Executing your own code before or after the event
  - Cancelling the event
- Calling any GML function, from any instance
- Plugin support
  - The tool provides various functions to aid with modifying game code.
    - Intercept and change any built-in function call
    - Intercept any script call
    - Intercept any object event (Draw, Step), change their bytecode, cancel them...
    - Intercept any game errors to create your own error dialogs
    - Run code every game frame (TAS frame counter?)
    - Run code even before the game is started!
    - and more!
- Full compatibility with existing tools like UndertaleModTool or Dogscepter.

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
