/**
 * Retrieves a list of available printers.
 *
 * @returns {string[]} An array of strings representing the available printers.
 * @example
 * const printers = enumPrinters();
 * console.log(printers); // ['Canon iX6800 Series', ...]
 */
export function enumPrinters(): string[];

interface PrinterProperties {
    copies: number;
    orientation: 'portrait' | 'landscape',
    paperWidth: number;
    paperHeight: number;
    scale: number;
    dpi: number;
}

/**
 * Represents a Printer in the application.
 */
declare class Printer {
    /**
     * Constructs a new instance of the Printer class with the specified name.
     * @param {string} printerName - The name of the printer.
     */
    constructor(printerName: string);

    /**
     * Gets the properties of the printer.
     * @returns {PrinterProperties} The properties of the printer.
     */
    getProperties(): PrinterProperties;

    /**
     * Sets the properties of the printer.
     * @param {Partial<PrinterProperties>} properties - The partial properties to set.
     */
    setProperties(properties: Partial<PrinterProperties>);

    /**
     * Chooses properties for the printer. It opens
     * a GUI dialog box provided by the printer's driver.
     * The result can later be retrieved by {@link getProperties} method.
     * @example
     * const printer = new Printer('Canon iX6800 Series');
     * printer.chooseProperties();
     * 
     * const props = printer.getProperties();
     */
    chooseProperties();
}

/**
 * Represents a PrintJob in the application.
 * @example
 * const printer = new Printer('Canon iX6800 Series');
 * const printJob = new PrintJob(printer, 'Black page');
 * const imageData = Buffer.alloc(64 * 64 * 3);
 * printJob.printPage(imageData, 64, 64);
 * printJob.end();
 */
declare class PrintJob {
    /**
     * Constructs a new instance of the PrintJob class.
     * @param {Printer} printer - The printer instance for the print job.
     * @param {string} jobName - The name of the print job.
     */
    constructor(printer: Printer, jobName: string);

    /**
     * Prints a page using the Printer instance.
     * @param {Uint8Array} imageData - The image data to print.
     * @param {number} width - The width of the image.
     * @param {number} height - The height of the image.
     */
    printPage(imageData: Uint8Array, width: number, height: number);

    /**
     * Ends the print job.
     */
    end();
}
