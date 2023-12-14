# Windows Printer Node

# WORK IN PROGRESS

## Purpose

This library aims to provide some natives interfaces from Windows Printing API (GDI+ Printing API) to applications running in Node.js environment.

This library is not mature, it may leak memory or crash your application. Use it at your own risk.

## Usage examples

- Retrieve names of locally installed printer
```javascript
const { enumPrinters } = require('windows-printer-node');

const names = enumPrinters();
console.log(names) // ['Epson A93', ...]
```

- Retrieve information about printing properties (number of copies, page size, DPI)
```javascript
const printer = new Printer('Epson A93');
const properties = printer.getProperties();
printer.chooseProperties(); // allow the user to choose properties
printer.setProperties({ copies: 100 });
```

- Print a bitmap image, it will fill the whole page
```javascript
// Raw bit map [r, g, b, r, g, b, ...]
const bitmap = Buffer.alloc(width * height * 3).fill(0);
const printJob = new PrintJob("job name", bitmap);
printJob.printPage(bitmap);
printJob.end();
```

## Building

> **WARNING**: This building tutorial is not user-friendly, you are expected to have some experience building code to resolve any problems that might come up. It may not build on your machine.

This package makes use of ```node-gyp```. The requirements are the same as that of the tool. Refer to ```node-gyp``` README for more details.

### Install Visual Studio Build Tools

_Note: You may skip this step if you've already installed it, either manually or via an option in Node.js setup wizard._

1. Head over to official Microsoft website for Visual Studio Build Tools. Link [here](https://visualstudio.microsoft.com/downloads/?q=build+tools+for+visual+studio#build-tools-for-visual-studio-2022)
2. Open the setup application
3. Make sure to check ```Desktop development with C++``` option.

### Building

1. Enter Visual Studio environment, run the ```Developer Command Promt for VS xxxx``` from your Start Menu.
2. Change directory to this project root
3. Run ```npm install```

## Windows compatibility

This module was developed and tested mainly on a Windows 10 machine. Compatibility with older or new versions of Windows is untested.

## LSP autocompletion

Run ```npm run clangd``` to generate the ```compile_commands.json``` file which can be used by code completion tools.

## Project specific commands

The commands in the ```package.json``` of this project should be straightforward to read.
