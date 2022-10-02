const child = require('child_process');
const path = require('path');
child.execFile("node", ['proxy.js']);
child.execFile(path.resolve(__dirname, '../No'), [path.resolve(__dirname, './index.js')]);