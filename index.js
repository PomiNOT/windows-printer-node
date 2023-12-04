const mod = require("./build/Debug/adder")

const printers = mod.enumPrinters();

console.log(`Thera are ${printers.length} printers on your system`);
for (const [i, name] of Object.entries(printers)) {
    console.log(`${parseInt(i) + 1}. ${name}`);
}
