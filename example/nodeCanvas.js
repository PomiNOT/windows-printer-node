const { enumPrinters, Printer, PrintJob } = require("../build/Debug/printer_module")

function getCanvasDimensions(printer) {
    const { paperWidth, paperHeight, dpi } = printer.getProperties();

    // convert milimeters to inches and multiply by dpi
    return [Math.floor((paperWidth / 25.4) * dpi), Math.floor((paperHeight / 25.4) * dpi)];
}

const printers = enumPrinters();
const printer = new Printer(printers[3]);
printer.chooseProperties();
const [w, h] = getCanvasDimensions(printer);

const job = new PrintJob(printer, "Black Page");
const pixels = Buffer.alloc(w * h * 3).fill(0);

job.printPage(pixels, w, h);
job.end();