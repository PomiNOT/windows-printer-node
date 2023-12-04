# Windows Printer Node

## Purpose

This library aims to provide some natives interfaces from Windows Printing API (GDI+ Printing API) to applications running in Node.js environment.

This library is not mature, it may crash your application. Use it at your own risk

## Building

This package makes use of ```node-gyp```. The requirements are the same as that of the tool. Refer to ```node-gyp``` README for more details.

### Install Visual Studio Build Tools

_Note: You may skip this step if you've already installed it, either manually or via an option in Node.js setup wizard._

1. Head over to official Microsoft website for Visual Studio Build Tools. Link [here](https://visualstudio.microsoft.com/downloads/?q=build+tools+for+visual+studio#build-tools-for-visual-studio-2022)
2. Open the setup application
3. Make sure to check ```Desktop development with C++``` option.

### Project specific commands

The commands in the ```package.json``` of this project should be straightforward to read.