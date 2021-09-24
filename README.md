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
    - Intercept any script call
    - Intercept any object event (Draw, Step), change their bytecode, cancel them...
    - Intercept any game errors to create your own error dialogs
    - Run code every game frame (TAS frame counter?)
    - and more!
- Full compatibility with existing tools like UndertaleModTool or Dogscepter.

**Note:** GML Compilation is only supported if UndertaleModTool v0.4.0-pre2 or newer is running.

## Plugin Repository
Plugins have moved to [a separate repository](https://github.com/Archie-osu/awesome-yytk).

The ExamplePlugin will remain in this repository, however.
