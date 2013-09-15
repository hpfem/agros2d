#!/usr/bin/python

import argparse, shutil, os
from multiprocessing import cpu_count
from subprocess import call
from glob import glob

VERSION = 3.1
CORES = cpu_count()

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

def documentation(format):
    call(['make', format, '-C', DOC_SOURCE_DIR])

    if (os.path.exists(DOC_DIR)):
        shutil.rmtree(DOC_DIR)

    shutil.copytree('{0}/build/{1}/'.format(DOC_SOURCE_DIR, format), DOC_DIR)

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

def build_project(cores):
    call(['cmake', '.'])
    call(['make', '-j', str(cores)])

    call(['./agros2d_generator'])
    call(['cmake', PLUGINS_DIR + '/CMakeLists.txt'])
    call(['make', '-C', PLUGINS_DIR, '-j', str(cores)])

def run_project(project, file, run_script, server):
    env = dict(os.environ)
    env['LD_LIBRARY_PATH'] = 'libs'
    
    args = list()
    if (server):
        args.append('xvfb-run')
        args.append('--auto-servernum')

    args.append('./{0}'.format(project))

    if (run_script):
        args.append('-r')

    args.append(file)

    call(args, env=env)

def source_package(version):
    call(['git', 'clean', '-fdx'])
    
    documentation()
    equations()
    release_localization()

    temp = '{0}/agros2d-{1}'.format(TEMP_DIR, version)
    if (os.path.exists(temp)):
        shutil.rmtree(temp)

    ignored = ['tmp', '.git*']
    shutil.copytree('./', temp, ignore=shutil.ignore_patterns(*ignored))

    os.chdir(temp)
    call(['debuild', '-S', '-sa'])

def binary_package():
    call(['dpkg-buildpackage', '-sgpg', '-rfakeroot'])

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest='command')

    # documentation
    doc = subparsers.add_parser('doc', help='generate documentation')
    doc.add_argument('-f', '--format', nargs='?', default='html', type=str, required=False,
                     help='output format (default parameter is html)')

    # localization
    loc = subparsers.add_parser('loc', help='release or update localization')
    loc.add_argument('-r', '--release', default=True, action='store_true', required=False)
    loc.add_argument('-u', '--update', default=False, action='store_true', required=False)

    # build
    build = subparsers.add_parser('build', help='build project')
    build.add_argument('-c', '--cores', nargs='?', default=CORES, type=int, required=False,
                      help='number of used cores')

    # run
    run = subparsers.add_parser('run', help='run project')
    run.add_argument('-p', '--project', nargs='?', default='agros2d', type=str, required=False,
                     help='project (valid parameters are agros2d, agros2d_pythonalb, agros2d_generator, agros2d_solver)')
    run.add_argument('-f', '--file', nargs='?', default='', type=str, required=False,
                     help='open Agros2D data file or Python script')
    run.add_argument('-r', '--run', action='store_true', required=False,
                     help='run Python script defined as file')
    run.add_argument('-s', '--server', action='store_true', required=False,
                     help='run project with X virtual framebuffer')

    # package
    pack = subparsers.add_parser('pack', help='make source or binary package')
    pack.add_argument('-s', '--source', default=True, action='store_true')
    pack.add_argument('-b', '--binary', default=False, action='store_true')
    pack.add_argument('-v', '--version', nargs='?', default=VERSION, type=float, required=False,
                      help='version of package')

    args = parser.parse_args()

    if (args.command == 'doc'):
        documentation(args.format)

    if (args.command == 'loc'):
        if (args.release):
            release_localization()
        if (args.update):
            update_localization()

    if (args.command == 'build'):
        build_project(args.cores)

    if (args.command == 'run'):
        run_project(args.project, args.file, args.run, args.server)

    if (args.command == 'pack'):
        if (args.source):
            source_package(args.version)
        if (args.binary):
            binary_package()
