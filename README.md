# FreeBoxSH
FreeBoxSH: a Command Interpreter Shell for FreeRTOS.


# Embedded Shell

Unix Bash style shell for FreeRTOS, providing Command Line Interface(CLI) for microcontrollers.

**A Terminal like interface over UART.**

- Targeted for embedded systems engineers, developers, researchers and maintenance technicians who want to explore hardware that uses FreeRTOS as an operating system from a shell unix-like interface.
- A UART based shell that enables finer control of CPU by allowing functions to be exposed as commands to be triggered from the prompt.
- Authentication for executing commands through the shell


### Features:
- Hardware GPIO's control;
- Management of memory and hardware information;
- Manage SPI Flash File System (SPIFFS);
- Manage Tasks in runtime application;
- Scheduler configuration (HOLD).


### Supported Processors/Architectures
`examples/` implements a `hello world` example for the processor variety listed below:
| Architecture             | Processor            | Example Location                             | Board                             |
| :---                     | :--                  | :---                                         | :-                                |
| Xtensa® 32 bit           | ESP32-D0WD           | examples/board/esp32-d0wd                    | ESP32 DevKit                      |
| RISC-V 32 bit            | ESP32-C3             | examples/board/esp32c3                       | ESP32-C3-DevKitC-02               |
| RISC 32 bit              | MIPS32 M4K core      | examples/board/mips32mk4pic3                 | PIC32MX Starter Kit* (HOLD)       |


# Quick Implement

1 - Download this repository using:

```git clone https://github.com/h3v0x/FreeBoxSH/ ```

2 - Add `freebox.h` and `freebox.c` to your project directory;

3 - To enable the shell at runtime in your application, you need:

• Create a task in which the entry point will call the *shell_task()* function located in the *freebox.c* file;

• Call *mount_spiffs* function to create SPI flash file system unit. Example:


```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freebox.h"

void app_main(void)
{
  mount_spiffs();
  xTaskCreate(&shell_task, "shell_task", 8192, NULL, 5, NULL);
}

```


## Quick Demo

• Help command:

```shell
root@freertos(/spiffs)# help
help: 
   Lists all the registered commands

ls                                List files in specified directory
cd                                Move to specified directory
cat <file>                        Get contents of specified file
touch <file> -w <filecontent>     Create a file and write content to it
delete <file>                     Delete the specified file
hardware                          Get information hardware
memory                            Get current memory information
restart                           Restart CPU
password                          Enter password for authentication
netinfo (HOLD)                                                       
ps (HOLD)
read <GPIO> (HOLD)
write <GPIO> (HOLD)
set <GPIO> (HOLD)
```

• Shell demonstration:

![Terminal Demo](https://github.com/h3v0x/FreeBoxSH/assets/6265911/a364eb25-a20d-4640-8ff0-44771e527131)


# Documentation

- To be able to run all the example and create your own project follow the documentation in [docs/](docs/index.md#table-of-contents).
- It also provides details about the internal workings of shell and how to setup your own project.

## Table of contents
| Topic| Description |
| :--- |:---        |
|[How shell Works](docs/shell-working.md)|Explains the working of shell with a flowchart.|
|[Running examples in `example/`](docs/00-setup-examples.md)|Guides on getting started with trying examples. Includes topics like - <br/> - `installing dependences`<br/> - `qemu based targets`<br/> - `physical board based targets`<br/> - `debugging` etc |
|[Setting up new project](docs/01-setting-up-new-project.md)|Details on how to start setting up a new project.|
|[Uart driver](docs/03-uart-driver.md)| Describes implementation and constraint on the uart driver.|
## Contributing

| Topic| Description |
| :--- |:----        |
| [Code of Conduct](docs/code-of-conduct.md) |Guides on the conduct expected while making contributions. |
|[Contributing](docs/contributing.md)|Guidelines on how to contribute to the code on GitHub.|


