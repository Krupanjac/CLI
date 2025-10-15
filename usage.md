# CLI Execution Walkthrough

## 1. Program Launch
- Entry point is `main()` in `main.cpp`, which simply constructs an `Interface` object. The constructor (`Interface.cpp`) sets `parser`/`command` to `nullptr` and immediately calls `Interface::run()`, so control never returns to `main()`.

## 2. Interactive Loop (`Interface::run`)
- A prompt character (`PSIGN`, default `$`) is stored locally and reused until changed by the `prompt` command.
- Each cycle allocates a `PromptCommand` (`Command.cpp`), calls `execute()` to print `"$ "`, deletes the command, then creates a fresh `Parser`.
- The parser construction (`Parser.cpp`) executes `std::cin >> stream`, which invokes `Stream::operator>>` to parse one full line, including pipelines.
- The parsed data and prompt reference are passed to `PipelineExecutor::run()` (`PipelineExecutor.cpp`). When execution returns, the parser is freed and the loop restarts.

## 3. Reading and Tokenising Input (`Stream` family)
- `Stream::operator>>` (`Stream.cpp`) reads an entire line, enforcing `MAX_SIZE` (512 chars) and emitting error code 10 (empty) or 11 (too long) before aborting.
- `Stream::split` divides the line on `|`, trimming whitespace to produce pipeline stages.
- For each stage `parseRedirections` extracts trailing `< file`, `> file`, or `>> file` tokens (whitespace optional) and stores them; the remaining text is passed to an `InputStream`.
- `InputStream::parse` walks tokens, honouring quoted segments. Options (`-x`) are detected automatically; up to three arguments are captured and `explicitArgument` is flagged when the first argument token is present.
- When a pipeline segment contains only whitespace, it is discarded.

## 4. Automatic Argument Sourcing
- Commands that can consume file/stdin input (`echo`, `wc`, `head`, `batch`) defer argument filling until after parsing:
  - If `< file` was specified and no explicit argument was provided, the file contents are read into the first argument (wrapped in quotes).
  - If no argument and no `< file`, the first pipeline segment can request multi-line input until `EOF`/blank line, again quoted.
- `.txt` arguments for `echo`, `wc`, `head`, `batch`, and `tr` are detected by `isTxtFileCandidate` and replaced with a `FileStream`, which loads file contents and quotes them.
- `tr` handles partial inputs: if only the replacement tokens are supplied, the command prompts for text via input redirection or interactive capture (first pipeline segment only).

## 5. Pipeline Execution (`PipelineExecutor::run`)
- `PipelineExecutor` loops over the parsed nodes using `Parser::processAll()` (which advances through `Stream` once).
- Redirection validation occurs before command creation: mixing `<` with explicit arguments for `echo`/`wc`/`head`/`batch` triggers `CommandFactory::handleCommand(SYNTAX_ERROR, ...)`.
- `CommandFactory::createCommand` validates options/arguments and returns a concrete `Command` or `nullptr` on error. The `prompt` command updates the prompt character and returns `nullptr` intentionally.
- Output handling:
  - If the current segment pipes to the next and no `>` is present, `std::cout` is temporarily redirected to an `std::ostringstream` to capture output.
  - With `>`/`>>`, `std::cout` is redirected to a file stream (opened append or truncate). After execution the stream is closed — and if the target ends with `.txt`, its contents are echoed back to the console.
- Captured pipeline output is quoted and injected into the next `InputStream`:
  - `tr` preserves its optional second/third arguments when rewiring.
  - `echo`, `wc`, `head`, and `batch` consume the quoted text as their primary argument.

## 6. Command Catalogue (`Command.cpp`)
- **PromptCommand**: prints current prompt.
- **EchoCommand**: prints its argument; quoted text is unwrapped, unquoted text is emitted verbatim.
- **TimeCommand**/**DateCommand**: print current time (`std::put_time`) and compilation date (`__DATE__`).
- **ClearCommand**: calls `system("cls")`.
- **ExitCommand**: `exit(0)`.
- **TouchCommand**: creates a new file; throws error code 7 if it already exists or 8 if creation fails.
- **WcCommand**: counts words (`-w`) or characters (default). Both modes strip surrounding quotes before processing.
- **HelpCommand**: prints usage help.
- **TruncateCommand**: truncates a file via `std::ofstream(..., std::ios::trunc)`.
- **RmCommand**: deletes a file with `std::remove`, reporting success or failure.
- **TrCommand**: replaces all occurrences of quoted `what` with quoted `with` inside quoted `input`.
- **HeadCommand**: outputs the first `n` lines (taken from option `-n####`) of quoted input.
- **BatchCommand**: splits quoted script text into individual lines, re-parses each line into a `Stream`, and executes them with the same validation/pipeline rules (including nested pipelines and redirections).

## 7. Validation & Error Reporting (`CommandFactory`)
- Every command has a dedicated `validate*` routine enforcing option syntax, argument quoting rules, and file checks (`validateFileForOpen`).
- Errors are reported via `handleCommand`, which prints the offending command line fragment, underlines the issue with `~`, and appends a contextual error message (`Error code 1`–`4`).
- `createCommand` falls back to `UNKNOWN_COMMAND` when no handler matches.

## 8. Batch Command Deep Dive
- `BatchCommand::execute` (`Command.cpp`) iterates lines from its quoted script.
- Each line is parsed with a fresh `Stream` and executed sequentially, using the same pipeline capture and redirection logic as the interactive path.
- Output redirection inside batch commands respects append/truncate modes and echoes appended `.txt` files back to the console.

## 9. Prompt Customisation (`prompt` command)
- Validation accepts a single character, either bare or quoted (`"$"`). Multi-character inputs trigger `INVALID_ARGUMENT`.
- On success the prompt stored in `Interface::run` is updated for subsequent iterations; no command object is executed.

## 10. Control Flow Recap
1. `main()` constructs `Interface`; `Interface::run` enters the REPL loop.
2. Prompt displayed via `PromptCommand`.
3. `Parser` reads input, `Stream` splits pipelines, tokenises, and resolves redirections/file injections.
4. `PipelineExecutor` validates each stage, instantiates commands, manages redirection/piping, and executes them.
5. Outputs and errors surface immediately; prompt reappears until `exit` is invoked.

This walkthrough covers every `.cpp`/`.h` participant so the CLI can be presented step by step in a project demo.
