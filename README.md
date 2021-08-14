# YYToolkit
YYToolkit is a tool for creating mods and altering GameMaker games.

Currently, only Windows is supported.

## Features
- Intercepting script calls and object events
  - Executing your own code before or after the event
  - Cancelling the event
- Calling any GML function, from any instance
- Plugin support
  - Internal tools can directly call the API using ``GetProcAddress``
    - This allows for more features, like drawing your own overlay with ImGui, or executing your C / C++ function as game code
  - External tools like UndertaleModTool can pull limited information through AUMI's IPC
    - This only works if the legacy AUMI IPC plugin is loaded.
- Scripting support
  - Dynamically inject GML bytecode or use YYLua!
- And whatever else you make!

**Note:** GML Compilation is only supported if UndertaleModTool v0.4.0-pre2 or newer is running.
