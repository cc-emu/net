const fs = require('fs');
const path = require('path');

const packageJsonPath = path.join(process.cwd(), 'package.json');
const pkg = JSON.parse(fs.readFileSync(packageJsonPath, 'utf-8'));

delete pkg.optionalDependencies;

fs.writeFileSync(packageJsonPath, JSON.stringify(pkg, null, 2) + '\n');
console.log('Removed optionalDependencies from package.json');
