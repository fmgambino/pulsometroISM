"""Exporta binarios verificados al terminar una compilación exitosa de PlatformIO."""
from pathlib import Path
import shutil
import subprocess
Import('env')

def export_binary(source, target, env):
    root=Path(env.subst('$PROJECT_DIR'))
    build=Path(env.subst('$BUILD_DIR'))
    output=root/'release'
    output.mkdir(exist_ok=True)
    # Offsets de flash ESP32 clásico utilizados por el framework Arduino 2.x.
    tool=Path(env.PioPlatform().get_package_dir('tool-esptoolpy'))/'esptool.py'
    framework=Path(env.PioPlatform().get_package_dir('framework-arduinoespressif32'))
    app0=framework/'tools'/'partitions'/'boot_app0.bin'
    entries=[('0x1000',build/'bootloader.bin'),('0x8000',build/'partitions.bin'),
             ('0xe000',app0),('0x10000',build/'firmware.bin')]
    command=[env.subst('$PYTHONEXE'),str(tool),'--chip','esp32','merge_bin',
             '--flash_mode','dio','--flash_freq','40m','--flash_size','4MB',
             '-o',str(output/'PULSOMETRO_ESP32_38P.bin')]
    for offset,path in entries:
        command.extend([offset,str(path)])
    subprocess.run(command,check=True)
    for _,path in entries:
        shutil.copy2(path,output/path.name)
    print('Binario unificado generado: release/PULSOMETRO_ESP32_38P.bin')

env.AddPostAction('$BUILD_DIR/${PROGNAME}.bin', export_binary)
