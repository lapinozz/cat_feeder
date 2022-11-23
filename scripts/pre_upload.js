var fs = require("fs");
var path = require("path");

function copyDirSync(src, dest)
{
    fs.mkdirSync(dest, { recursive: true });
    let entries = fs.readdirSync(src, { withFileTypes: true });

    for (let entry of entries)
    {
        let srcPath = path.join(src, entry.name);
        let destPath = path.join(dest, entry.name);

        entry.isDirectory() ? copyDirSync(srcPath, destPath) : fs.copyFileSync(srcPath, destPath);
    }
}

const rootFolder = __dirname + "/..";

const websiteFolder = rootFolder + "/website";
const websiteDistFolder = websiteFolder + "/dist";
const esp32Folder = rootFolder + "/esp32/data/web";

const execSync = require('child_process').execSync;
execSync("npm run build", {cwd: websiteFolder});

if (fs.existsSync(esp32Folder))
{
  fs.rmSync(esp32Folder, { recursive: true });
}
copyDirSync(websiteDistFolder, esp32Folder);