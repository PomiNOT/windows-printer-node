const { enumPrinters, Printer, PrintJob } = require("./build/Debug/printer_module")
const segfaultHandler = require("segfault-handler");

const printers = enumPrinters();

console.log(`Thera are ${printers.length} printers on your system`);
for (const [i, name] of Object.entries(printers)) {
    console.log(`${parseInt(i) + 1}. ${name}`);
}

const printer = new Printer(printers[3]);

const job = new PrintJob(printer, "My print document");
job.printPage(new Uint8Array(Array(500 * 500).fill(60)), 500, 500);
job.end();

segfaultHandler.registerHandler("crash.log");