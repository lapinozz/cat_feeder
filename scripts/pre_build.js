var fs = require("fs");
var path = require("path");

const input = fs.readFileSync(__dirname + "/../shared/shared.json");

const data = JSON.parse(input.toString());

const enums = {};

let index = 0;
enums['Commands'] = {};
for(const e in data.commands)
{
  const values = data.commands[e];

  enums['Commands'][`_${e}_BEGIN`] = index;

  for(const value of values)
  {
    enums['Commands'][`${e}_${value}`] = index++;
  }

  enums['Commands'][`_${e}_END`] = index++;
  enums['Commands'][`_${e}_COUNT`] = values.length;
}

for(const e in data.enums)
{
  let index = 0;

  const values = data.enums[e];

  enums[e] = {};
  enums[e][`_BEGIN`] = index;

  for(const value of values)
  {
    enums[e][`${value}`] = index++;
  }

  enums[e][`_END`] = index++;
  enums[e][`_COUNT`] = values.length;
}

let output = '';

for(const e in enums)
{
  const values = enums[e];
  output += `struct ${e} : public SmartEnum<>\n`;
  output += "{\n";

  output += `\tconstexpr ${e}(char x = 0) : SmartEnum(x) {}\n`;

  output += `\tconst static ${e}\n`;

  for(const value in values)
  {
    output += `\t${value},\n`;
  }

  output = output.substr(0, output.length - 2) + ';\n';

  output += "};\n\n";

  output += `constexpr const ${e}\n`;
  for(const value in values)
  {
    output += `\t${e}::${value}{${values[value]}},\n`;
  }
  output = output.substr(0, output.length - 2) + ';\n\n';


/*
  output += `namespace ${e} { enum ${e}\n`;
  output += "{\n";

  for(const value in values)
  {
    output += `\t${value} = ${values[value]},\n`;
  }

  output += "};}\n\n";
*/
}

const constants = data.constants;
for(const e in constants)
{
  const value = constants[e];
  output += `const auto ${e} = ${value};\n`;
}

console.log(output)

const jsonOutput = JSON.stringify({enums, constants}, 4, 4);
console.log(jsonOutput)

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

const sharedFolder = rootFolder + "/shared";
const sharedGenFolder = rootFolder + "/shared.gen";

const esp32Folder = rootFolder + "/esp32/src/shared";
const arduinoFolder = rootFolder + "/arduino/shared";
const websiteFolder = rootFolder + "/website/shared";

if (fs.existsSync(sharedGenFolder))
{
  fs.rmSync(sharedGenFolder, { recursive: true });
}
copyDirSync(sharedFolder, sharedGenFolder);

fs.writeFileSync(sharedGenFolder + "/shared.gen.h", output);
fs.writeFileSync(sharedGenFolder + "/shared.gen.json", jsonOutput);

if (fs.existsSync(esp32Folder))
{
  fs.rmSync(esp32Folder, { recursive: true });
}
copyDirSync(sharedGenFolder, esp32Folder);

if (fs.existsSync(arduinoFolder))
{
  fs.rmSync(arduinoFolder, { recursive: true });
}
copyDirSync(sharedGenFolder, arduinoFolder);

if (fs.existsSync(websiteFolder))
{
  fs.rmSync(websiteFolder, { recursive: true });
}
copyDirSync(sharedGenFolder, websiteFolder);