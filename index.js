const { enumPrinters, Printer, PrintJob } = require("./build/Debug/printer_module")
const segfaultHandler = require("segfault-handler");

const printers = enumPrinters();

console.log(`Thera are ${printers.length} printers on your system`);
for (const [i, name] of Object.entries(printers)) {
    console.log(`${parseInt(i) + 1}. ${name}`);
}

const printer = new Printer(printers[5]);
printer.setProperties({ copies: 100 });

const job = new PrintJob(printer);
job.printPage(new Uint8Array([1, 2, 3]));

segfaultHandler.registerHandler("crash.log");