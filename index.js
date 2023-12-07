const { enumPrinters, Printer } = require("./build/Debug/printer_module")
const segfaultHandler = require("segfault-handler");
const fs = require("fs");

const printers = enumPrinters();

console.log(`Thera are ${printers.length} printers on your system`);
for (const [i, name] of Object.entries(printers)) {
    console.log(`${parseInt(i) + 1}. ${name}`);
}

const printer = new Printer(printers[5]);
printer.chooseProperties();

fs.writeFileSync("config.json", JSON.stringify(printer.getProperties()));

segfaultHandler.registerHandler("crash.log");