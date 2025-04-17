# c-vm-fac Project

## Overview
The c-vm-fac project implements a simple virtual machine (VM) capable of processing bytecode files with a specific magic number (`FAACBEED`). The VM is designed to read and execute instructions from `.fac` files, which contain bytecode instructions for the VM to process. This project is implemented in C and aims to demonstrate the fundamentals of virtual machine design and bytecode execution.

## Project Structure
- **src/**: Contains the source code for the virtual machine.
  - **vm.c**: Implementation of the VM, including initialization, execution of bytecode, and opcode handling.
  - **vm.h**: Header file defining structures, constants, and function prototypes for the VM.
  - **main.c**: Entry point of the application, responsible for file handling and invoking VM execution.
  
- **notebooks/**: Contains Jupyter notebooks for documentation and outlining the VM structure.
  - **VM_Structure_Outline.ipynb**: A notebook detailing the VM's structure, magic number, bytecode format, and implementation details.

- **Makefile**: Build instructions for compiling the C source files into an executable. It specifies the compiler, flags, and targets for building the project.

- **README.md**: Documentation for the project, providing an overview, build instructions, and details on the `.fac` file format and bytecode execution.

## Building the Project
To build the project, navigate to the project directory and run the following command:

```
make
```

This will compile the source files and generate the executable for the virtual machine.

## Running the VM
To run the virtual machine, use the following command:

```
./vm <path_to_file.fac>
```

Replace `<path_to_file.fac>` with the path to your `.fac` bytecode file. The VM will read the file, check the magic number, and execute the bytecode instructions.

## .fac File Format
The `.fac` file format consists of:
- A 4-byte magic number (`FAACBEED`) at the beginning of the file.
- Followed by the bytecode instructions that the VM will execute.

## Conclusion
The c-vm-fac project serves as a practical example of how to implement a virtual machine in C, focusing on bytecode execution and file handling. It provides a foundation for further exploration and enhancement of virtual machine capabilities.