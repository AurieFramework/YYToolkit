# YYToolkit
YYToolkit is a tool for creating mods and altering GameMaker games.

Currently, only Windows is supported. (We heavily rely on Windows's API's.)

YYToolkit Features:
- Intercepting script calls and object events
  - Executing your own code before or after the event
  - Cancelling the event
- Calling any GML function, from any instance
- Plugin support
  - Internal tools can directly call the API using ``GetProcAddress``
    - This allows for more features, like drawing your own overlay with ImGui, or executing your C / C++ function as game code
  - External tools like UndertaleModTool can pull limited information through AUMI's IPC
- Scripting support
  - Dynamically inject GML bytecode or use YYLua!
- Internal UI
  - Can potentially be disabled
  - Plugins can also come with external UIs
- YYC Disassembler
  - And a partial decompiler, too
- Hex Editor
  - Yes, you can mod the data.win without modifying the actual file now!
- String cross-referencer
  - Helpful for finding YYC code entries

**Note:** GML Compilation is only supported if UndertaleModTool v0.4.0-pre2 or newer is running.
