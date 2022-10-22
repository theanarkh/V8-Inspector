const child = require('child_process');
const path = require('path');
const process1 = child.execFile("node", ['proxy.js']);
process1.stdout.on('data', (data) => {
    console.log(data.toString())
});

process1.stderr.on('data', (data) => {
    console.log(data.toString())
});
const process2 = child.execFile(path.resolve(__dirname, '../No'), [path.resolve(__dirname, './index.js')]);
process2.stdout.on('data', (data) => {
    console.log(data.toString())
});
process2.stderr.on('data', (data) => {
    console.log(data.toString())
});