#!/usr/bin/python

import argparse, shutil, os
from multiprocessing import cpu_count
from subprocess import call
from glob import glob

CORES = cpu_count()
VERSION = 3.0

DOC_SOURCE_DIR = './resources_source/doc'
DOC_DIR = './resources/help'
LOC_SOURCE_DIR = './resources_source/lang'
LOC_DIR = './resources/lang'
PLUGINS_DIR = './plugins'
TEMP_DIR = './tmp'

LOC_SOURCE_FILES = ['agros2d-library/*.cpp', 'agros2d-library/*.h',
                    'pythonlab-library/*.cpp', 'pythonlab-library/*.h']
LOC_TARGET_FILES = ['en_US.ts', 'cs_CZ.ts', 'pl_PL.ts',
                    'ru_RU.ts', 'fr_FR.ts']

LOC_PLUGINS_SOURCE_FILES = ['plugins/*.cpp', 'plugins/*.h']
LOC_PLUGINS_TARGET_FILES = ['plugin_en_US.ts', 'plugin_cs_CZ.ts', 'plugin_pl_PL.ts',
                            'plugin_ru_RU.ts', 'plugin_fr_FR.ts']

def documentation():
    call(['make', 'html', '-C', DOC_SOURCE_DIR])

    if (os.path.exists(DOC_DIR)):
        shutil.rmtree(DOC_DIR)

    shutil.copytree('{0}/build/html/'.format(DOC_SOURCE_DIR), DOC_DIR)

def equations():
    for root, dirs, files in os.walk(PLUGINS_DIR):
        for file in files:
            if (os.path.splitext(file)[1] != '.py'):
                continue

            call(['python', '{0}/{1}'.format(root, file)])

def release_localization():
    for file in os.listdir(LOC_SOURCE_DIR):
        if (os.path.splitext(file)[1] != '.ts'):
            continue

        call(['lrelease', '{0}/{1}'.format(LOC_SOURCE_DIR, file)])

    for file in os.listdir(LOC_SOURCE_DIR):
        if (os.path.splitext(file)[1] != '.qm'):
            continue

        if (not os.path.exists(LOC_DIR)):
            os.mkdir(LOC_DIR)

        shutil.copy2('{0}/{1}'.format(LOC_SOURCE_DIR, file), LOC_DIR)

def update_localization():
    sources = list()
    for source in LOC_SOURCE_FILES:
        sources += glob(source)

    call(['lupdate'] + sources + ['-ts'] + LOC_TARGET_FILES)

    sources = list()
    for source in LOC_PLUGINS_SOURCE_FILES:
        sources += glob(source)

    call(['lupdate'] + sources + ['-ts'] + LOC_PLUGINS_TARGET_FILES)

def build(cores):
    call(['cmake', './'])
    call(['make', '-j', cores])

    call('./agros2d_generator')
    call(['cmake', PLUGINS_DIR])
    call(['make', '-C', PLUGINS_DIR, '-j', cores])

def source_package(VERSION):
    call(['git', 'clean', '-fdx'])
    
    documentation()
    equations()
    release_localization()

    temp = '{0}/agros2d-{1}'.format(TEMP_DIR, VERSION)
    if (os.path.exists(temp)):
        shutil.rmtree(temp)

    ignored = ['tmp', '.git*', 'test']
    shutil.copytree('./', temp, ignore=shutil.ignore_patterns(*ignored))

    os.chdir(temp)
    call(['debuild', '-S', '-sa'])

def binary_package(VERSION):
    call(['dpkg-buildpackage', '-sgpg', '-rfakeroot'])

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--documentation', action='store_true', required=False,
                        help='generate HTML documentation')
    parser.add_argument('-e', '--equations', action='store_true', required=False,
                        help='generate equation from LaTeX source')

    loc = parser.add_argument_group('localization')
    loc.add_argument('-l', '--localization', nargs='?', type=str, required=False,
                     help='release or update localization (valid parameters: release, update)')

    comp = parser.add_argument_group('buid')
    comp.add_argument('-b', '--build', action='store_true', required=False,
                      help='build project')
    comp.add_argument('-c', '--cores', nargs='?', default=CORES, type=int, required=False,
                      help='number of used cores')

    pac = parser.add_argument_group('package')
    pac.add_argument('-p', '--package', nargs='?', type=str, required=False,
                      help='build source or binary package (valid parameters: source, binary)')
    pac.add_argument('-v', '--version', nargs='?', default=VERSION, type=float, required=False,
                      help='set version')

    args = parser.parse_args()

    if (args.documentation):
        documentation()

    if (args.equations):
        equations()

    if (args.localization == 'release'):
        release_localization()
    elif (args.localization == 'update'):
        update_localization()

    if (args.build):
        build(args.cores)

    if (args.package == 'source'):
        source_package(args.version)
    elif (args.package == 'binary'):
        binary_package(args.version)
