# This file is python code

import os

Decider('timestamp-match')

common_files = Glob("src/*.cpp") + Glob("src/frustum/*.cpp") + Glob("src/net/*.cpp");
server_files = Glob("src/dedicated/*.cpp")
client_files = Glob("src/main/*.cpp")
editor_files = Glob("src/editor/*.cpp")
loader3ds_files = Glob("src/loader_3ds/*.cpp")

#include_dirs = ['src', 'src/frustum', 'src/net/'] + os.environ['C_INCLUDE_PATH'].split(':')
include_dirs = ['src'] + os.environ['C_INCLUDE_PATH'].split(':')
#libs = ['boost_system-gcc41-mt-1_39']
libs = ['SDL', 'SDL_mixer', 'GL', 'GLU', 'png', 'GLEW']
lib_dirs = [os.environ['LD_LIBRARY_PATH'].split(':'), './lib/']
env = Environment(CPPPATH = include_dirs, LIBS = libs, LIBPATH = lib_dirs)

common_flags = '-Wall -Wextra -Werror -std=c++0x -pedantic'



opt = env.Clone(CCFLAGS = common_flags + ' -O3', LINKFLAGS = '-O3')
optcommon = opt.Object(common_files)
optclient = opt.Program('bin/client', optcommon + opt.Object(client_files))
optserver = opt.Program('bin/server', optcommon + opt.Object(server_files))
opteditor = opt.Program('bin/editor', optcommon + opt.Object(editor_files))
optloader_3ds = opt.Program('bin/loader_3ds', opt.Object(loader3ds_files))
opt.Alias('client', 'bin/client')
opt.Alias('server', 'bin/server')
opt.Alias('editor', 'bin/editor')
opt.Alias('loader3ds', 'bin/loader_3ds')

dbg = env.Clone(CCFLAGS = common_flags + ' -g -O0', LINKFLAGS = '-g')
dbgcommon = dbg.Object(common_files, OBJPREFIX = 'debug-')
debugclient = dbg.Program('bin/debug-client', dbgcommon + dbg.Object(client_files, OBJPREFIX = 'debug-'))
debugserver = dbg.Program('bin/debug-server', dbgcommon + dbg.Object(server_files, OBJPREFIX = 'debug-'))
debugeditor = dbg.Program('bin/debug-editor', dbgcommon + dbg.Object(editor_files, OBJPREFIX = 'debug-'))
dbg.Alias('debug', 'bin/debug-client')
dbg.Alias('debug', 'bin/debug-server')
dbg.Alias('debug', 'bin/debug-editor')

prof = env.Clone(CCFLAGS = common_flags + ' -pg -O2 -D NDEBUG', LINKFLAGS = '-pg -O2')
profcommon = prof.Object(common_files, OBJPREFIX = 'profile-')
profileclient = prof.Program('bin/profile-client', profcommon + prof.Object(client_files, OBJPREFIX = 'profile-'))
profileserver = prof.Program('bin/profile-server', profcommon + prof.Object(server_files, OBJPREFIX = 'profile-'))
profileeditor = prof.Program('bin/profile-editor', profcommon + prof.Object(editor_files, OBJPREFIX = 'profile-'))
prof.Alias('profile', 'bin/profile-client')
prof.Alias('profile', 'bin/profile-server')
prof.Alias('profile', 'bin/profile-editor')


Default(optclient, optserver, opteditor)


