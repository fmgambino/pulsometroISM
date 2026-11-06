"""Empaqueta la web en flash; funciona con PlatformIO o ejecutado con Python."""
from pathlib import Path

def generate(root):
    root = Path(root)
    names = {'PAGE':'index.html', 'STYLE':'style.css', 'SCRIPT':'app.js', 'WEB_LOGO':'logo.svg', 'THEME':'theme.js', 'PORTAL_SCRIPT':'portal.js', 'PORTAL_STYLE':'portal.css'}
    text = '#pragma once\n#include <Arduino.h>\n// Generado desde web/. Editar los archivos originales, no este encabezado.\n'
    for index, (symbol, filename) in enumerate(names.items()):
        value = (root / 'web' / filename).read_text(encoding='utf-8')
        delimiter = 'R' + str(index)
        if ')' + delimiter + '"' in value:
            raise ValueError('Delimitador reservado dentro del recurso ' + filename)
        text += f'const char {symbol}[] PROGMEM = R"{delimiter}({value}){delimiter}";\n'
    png = (root / 'web' / 'logo.png').read_bytes()
    text += 'const uint8_t LOGO_PNG[] PROGMEM = {' + ','.join('0x%02x' % b for b in png) + '};\n'
    target = root / 'include' / 'web_generated.h'
    if not target.exists() or target.read_text(encoding='utf-8') != text:
        target.write_text(text, encoding='utf-8')

if __name__ == '__main__':
    generate(Path(__file__).resolve().parents[1])
else:
    Import('env')  # PlatformIO suministra esta función al ejecutar extra_scripts.
    generate(env.subst('$PROJECT_DIR'))
