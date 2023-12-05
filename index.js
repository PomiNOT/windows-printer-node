const mod = require("./build/Debug/printer_module")
const segfaultHandler = require("segfault-handler");

const printers = mod.enumPrinters();

console.log(`Thera are ${printers.length} printers on your system`);
for (const [i, name] of Object.entries(printers)) {
    console.log(`${parseInt(i) + 1}. ${name}`);
}

for (const printer of printers) {
    const props = mod.getDocumentProperties(printer);
    console.log(props);
}

segfaultHandler.registerHandler("crash.log");