"""Empaqueta la web en flash; funciona con PlatformIO o ejecutado con Python."""
from pathlib import Path

def generate(root):
    root = Path(root)
    names = {'PAGE':'index.html', 'STYLE':'style.css', 'SCRIPT':'app.js', 'WEB_LOGO':'logo.svg'}
    text = '#pragma once\n#include <Arduino.h>\n// Generado desde web/. Editar los archivos originales, no este encabezado.\n'
    for symbol, filename in names.items():
        value = (root / 'web' / filename).read_text(encoding='utf-8')
        delimiter = 'ASSET_' + symbol
        if ')' + delimiter + '"' in value:
            raise ValueError('Delimitador reservado dentro del recurso ' + filename)
        text += f'const char {symbol}[] PROGMEM = R"{delimiter}({value}){delimiter}";\n'
    target = root / 'include' / 'web_generated.h'
    if not target.exists() or target.read_text(encoding='utf-8') != text:
        target.write_text(text, encoding='utf-8')

if __name__ == '__main__':
    generate(Path(__file__).resolve().parents[1])
else:
    Import('env')  # PlatformIO suministra esta función al ejecutar extra_scripts.
    generate(env.subst('$PROJECT_DIR'))
