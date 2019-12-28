EnsureSConsVersion(0, 96, 92)
import sys
import os
import glob
sys.path.append( os.path.abspath("scons") )
import bundle
import dmg
import nsis

isWindowsPlatform = sys.platform=='win32'
isLinuxPlatform = sys.platform=='linux2'
isDarwinPlatform = sys.platform=='darwin'


def establish_options(env):
    opts = Variables('options_cache.py')
    opts.Add("CXXFLAGS", "Manually add to the CXXFLAGS", "-g")
    opts.Add("LINKFLAGS", "Manually add to the LINKFLAGS", "-g")
    if isDarwinPlatform:
        opts.Add(PathOption("INSTALLDIR", "Installation Directory", "./"))
    else:
	    opts.Add("INSTALLDIR", "Installation Directory", "/usr/local/share")
    opts.Add("BINDIR", "Binary Installation Directory", "/usr/local/bin")
    opts.Add("DATADIR", "Directory where data will be put, set to the same as INSTALLDIR", "/usr/local/share")
    opts.Add(BoolVariable("release", "Build for release", 0))
    opts.Add(BoolVariable("profile", "Build with profiling on", 0))
    opts.Add(BoolVariable("mingw", "Build with mingw enabled if not auto-detected", 0))
    opts.Add(BoolVariable("mingwcross", "Cross-compile with mingw for Win32", 0))
    opts.Add("crossroot", "Path to include/ and lib/ containing Win32 files for cross-compiling", "../local")
    opts.Add(BoolVariable("server", "Build only the YOG server, excluding the game and any GUI/sound components", 0))
    opts.Add("font", "Build the game using an alternative font placed in the data/font folder", "sans.ttf")
    Help(opts.GenerateHelpText(env))
    opts.Update(env)
    opts.Save("options_cache.py", env)
    if env.GetOption('clean'):
        Execute(Delete("options_cache.py"))
    
    
class Configuration:
    """Handles the config.h file"""
    def __init__(self):
        self.f = open("config.h", 'w')
        self.f.write("// config.h. Generated by scons\n")
        self.f.write("\n")
    def add(self, variable, doc, value=""):
        self.f.write("// %s\n" % doc)
        self.f.write("#define %s %s\n" % (variable, value))
        self.f.write("\n")
    
def configure(env, server_only):
    """Configures glob2"""
    conf = Configure(env.Clone())
    configfile = Configuration()
    configfile.add("PACKAGE", "Name of package", "\"glob2\"")
    configfile.add("PACKAGE_BUGREPORT", "Define to the address where bug reports for this package should be sent.", "\"glob2-devel@nongnu.org\"")
    if isDarwinPlatform:
        configfile.add("PACKAGE_DATA_DIR", "data directory", "\"" + env["DATADIR"] + "../Resources/\"")
    else:
    	configfile.add("PACKAGE_DATA_DIR", "data directory", "\"" + env["DATADIR"] + "\"")
    configfile.add("PACKAGE_SOURCE_DIR", "source directory", "\"" +env.Dir("#").abspath.replace("\\", "\\\\") + "\"")
    configfile.add("PACKAGE_NAME", "Define to the full name of this package.", "\"Globulation 2\"")
    configfile.add("PACKAGE_TARNAME", "Define to the one symbol short name of this package.", "\"glob2\"")
    configfile.add("PACKAGE_VERSION", "Define to the version of this package.", "\""+env["VERSION"]+"\"")
    configfile.add("AUDIO_RECORDER_OSS", "Set the audio input type to OSS; the UNIX Open Sound System")
    if isDarwinPlatform:
        configfile.add("USE_OSX", "Set when this build is OSX")
    if env['mingw'] or env['mingwcross'] or isWindowsPlatform:
        configfile.add("USE_WIN32", "Set when this build is Win32")
    configfile.add("PRIMARY_FONT", "This is the primary font Globulation 2 will use", "\"" + env["font"] + "\"")

    missing=[]

    env.Append(CPPDEFINES=["HAVE_CONFIG_H"])

    #Compiler check
    if not conf.CheckCC():
        print "C compiler does not work"
        missing.append("C compiler")
    if not conf.CheckCXX():
        print "CXX compiler does not work"
        missing.append("CXX compiler")

    #Simple checks for required libraries
    if not server_only and not conf.CheckLib("SDL"):
        print "Could not find libSDL"
        missing.append("SDL")
    if not server_only and not conf.CheckLib("SDL_ttf"):
        print "Could not find libSDL_ttf"
        missing.append("SDL_ttf")
    if not server_only and not conf.CheckLib("SDL_image"):
        print "Could not find libSDL_image"
        missing.append("SDL_image")
    if not conf.CheckLib("SDL_net"):
        print "Could not find libSDL_net"
        missing.append("SDL_net")
    if not server_only and (not conf.CheckLib("speex") or not conf.CheckCXXHeader("speex/speex.h")):
        print "Could not find libspeex or could not find 'speex/speex.h'"
        missing.append("speex")
    if not server_only and not conf.CheckLib("vorbisfile"):
        print "Could not find libvorbisfile"
        missing.append("vorbisfile")
    if not server_only and not conf.CheckLib("vorbis"):
        print "Could not find libvorbis"
        missing.append("vorbis")
    if not server_only and not conf.CheckLib("ogg"):
        print "Could not find libogg"
        missing.append("ogg")
    if not conf.CheckCXXHeader("zlib.h"):
        print "Could not find zlib.h"
        missing.append("zlib")
    else:
        if conf.CheckLib("z"):
            env.Append(LIBS="z")
        elif conf.CheckLib("zlib1"):
            env.Append(LIBS="zlib1")
        else:
            print "Could not find libz or zlib1.dll"
            missing.append("zlib")
    
    if ((env['mingw'] or env['mingwcross'] or isWindowsPlatform) and not conf.CheckLib("regex")) or not conf.CheckCXXHeader("regex.h"):
			print "Could not find regex.h"
			missing.append("regex")

    boost_thread = ''
    if conf.CheckLib("boost_thread") and conf.CheckCXXHeader("boost/thread/thread.hpp"):
        boost_thread="boost_thread"
    elif conf.CheckLib("boost_thread-mt") and conf.CheckCXXHeader("boost/thread/thread.hpp"):
        boost_thread="boost_thread-mt"
    else:
        print "Could not find libboost_thread or libboost_thread-mt or boost/thread/thread.hpp"
        missing.append("libboost_thread")
    env.Append(LIBS=[boost_thread])
    
    boost_date_time = ''
    if conf.CheckLib("boost_date_time") and conf.CheckCXXHeader("boost/date_time/posix_time/posix_time.hpp"):
        boost_thread="boost_thread"
    elif conf.CheckLib("boost_date_time-mt") and conf.CheckCXXHeader("boost/date_time/posix_time/posix_time.hpp"):
        boost_thread="boost_thread-mt"
    else:
        print "Could not find libboost_date_time or libboost_date_time-mt or boost/thread/thread.hpp"
        missing.append("libboost_date_time")
    env.Append(LIBS=[boost_date_time])
    env.Append(LIBS=["boost_system", "pthread"])
    

    if not conf.CheckCXXHeader("boost/shared_ptr.hpp"):
        print "Could not find boost/shared_ptr.hpp"
        missing.append("boost/shared_ptr.hpp")
    if not conf.CheckCXXHeader("boost/tuple/tuple.hpp"):
        print "Could not find boost/tuple/tuple.hpp"
        missing.append("boost/tuple/tuple.hpp")
    if not conf.CheckCXXHeader("boost/tuple/tuple_comparison.hpp"):
        print "Could not find boost/tuple/tuple_comparison.hpp"
        missing.append("boost/tuple/tuple_comparison.hpp")
    if not conf.CheckCXXHeader("boost/logic/tribool.hpp"):
        print "Could not find boost/logic/tribool.hpp"
        missing.append("boost/logic/tribool.hpp")
    if not conf.CheckCXXHeader("boost/lexical_cast.hpp"):
        print "Could not find boost/lexical_cast.hpp"
        missing.append("boost/lexical_cast.hpp")
     
    #Do checks for OpenGL, which is different on every system
    gl_libraries = []
    if not server_only:
        if isDarwinPlatform:
            print "Using Apple's OpenGL framework"
            env.Append(FRAMEWORKS="OpenGL")
        elif conf.CheckLib("GL") and conf.CheckCXXHeader("GL/gl.h"):
            gl_libraries.append("GL")
        elif conf.CheckLib("GL") and conf.CheckCXXHeader("OpenGL/gl.h"):
            gl_libraries.append("GL")
        elif conf.CheckLib("opengl32") and conf.CheckCXXHeader("GL/gl.h"):
            gl_libraries.append("opengl32")
        else:
            print "Could not find libGL or opengl32, or could not find GL/gl.h or OpenGL/gl.h"
            missing.append("OpenGL")

        #Do checks for GLU, which is different on every system
        if isDarwinPlatform:
            print "Using Apple's GLUT framework"
            env.Append(FRAMEWORKS="GLUT")
        elif conf.CheckLib('GLU') and conf.CheckCXXHeader("GL/glu.h"):
            gl_libraries.append("GLU")
        elif conf.CheckLib('GLU') and conf.CheckCXXHeader("OpenGL/glu.h"):
            gl_libraries.append("GLU")
        elif conf.CheckLib('glu32') and conf.CheckCXXHeader('GL/glu.h'):
            gl_libraries.append("glu32")
        else:
            print "Could not find libGLU or glu32, or could not find GL/glu.h or OpenGL/glu.h"
            missing.append("GLU")
    
    if gl_libraries or isDarwinPlatform:
        configfile.add("HAVE_OPENGL ", "Defined when OpenGL support is present and compiled")
        env.Append(LIBS=gl_libraries)
    
    #Do checks for fribidi
    if not server_only and conf.CheckLib('fribidi') and conf.CheckCXXHeader('fribidi/fribidi.h'):
        configfile.add("HAVE_FRIBIDI ", "Defined when FRIBIDI support is present and compiled")
        env.Append(LIBS=['fribidi'])

    #Do checks for portaudio
    if not server_only and conf.CheckLib('portaudio') and conf.CheckCXXHeader('portaudio.h'):
        if env['mingw'] or env['mingwcross'] or isWindowsPlatform:
            print "--------"
            print "NOTE: It appears you are compiling under Windows. At this stage, PortAudio crashes Globulation 2 when voice chat is used."
            print "NOTE: Disabling PortAudio in this build (you will be unable to use Voice Chat ingame)."
            print "--------"
        else:
            if GetOption('portaudio'):
                print "trying to use portaudio"
                configfile.add("HAVE_PORTAUDIO ", "Defined when Port Audio support is present and compiled")
                env.Append(LIBS=['portaudio'])
            else:
                print "         no portaudio"
                print "         no portaudio - although portaudio was found to be installed, you have "
                print "         no portaudio - to explicitly activate it using: "
                print "         no portaudio - $ scons --portaudio=true"
                print "         no portaudio - this may not work properly if the version of portaudio"
                print "         no portaudio - is wrong. portaudio is used to allow communicating over VOIP"
                print "         no portaudio"
                print "         no portaudio - if you know of a solution to detect portaudio version"
                print "         no portaudio - let us know at:"
                print "         no portaudio - https://savannah.nongnu.org/bugs/index.php?24668"
                print "         no portaudio"
    if missing:
        for t in missing:
            print "Missing %s" % t
        Exit(1)
       
    conf.Finish()

def main():
    AddOption('--portaudio',
              dest='portaudio',
              type='string',
              nargs=1,
              action='store',
              metavar='portaudio',
              help='should portaudio be used')
    AddOption('--build',
              default='build',
              help='build directory')
    env = Environment()
    env["VERSION"] = "0.9.5.0"
    establish_options(env)
    
    if env['mingwcross']:
            env.Platform('cygwin')
            env['CC']  = 'i586-mingw32msvc-gcc'
            env['CXX'] = 'i586-mingw32msvc-g++'
            env['AR']  = 'i586-mingw32msvc-ar'
            env['RANLIB'] = 'i586-mingw32msvc-ranlib'
    
    try:
        env.Clone()
    except AttributeError:
        env.Clone = env.Copy
    
    
    # Add specific paths.
    if env['mingw'] or isWindowsPlatform:
        env.Append(LIBPATH=["C:/msys/1.0/local/lib", "C:/msys/1.0/lib"])
        env.Append(LIBPATH=['/usr/local/lib'])
        env.Append(CPPPATH=["C:/msys/1.0/local/include/SDL", "C:/msys/1.0/local/include", "C:/msys/1.0/include/SDL", "C:/msys/1.0/include"])
        env.Append(CPPPATH=['/usr/local/include/SDL'])
    if isDarwinPlatform:
        env.Append(LIBPATH=["/opt/local/lib"])
        env.Append(CPPPATH=["/opt/local/include"])
    if env['mingwcross']:
        if os.path.isabs(env['crossroot']):
            crossroot_abs = env['crossroot']
        else:
            crossroot_abs = os.getcwdu() + '/' + env['crossroot']
        env.Append(LIBPATH=['/usr/i586-mingw32msvc/lib', crossroot_abs + '/lib'])
        env.Append(CPPPATH=['/usr/lib/gcc/i586-mingw32msvc/4.2.1-sjlj/include/c++', '/usr/i586-mingw32msvc/include'])
        env.Append(CPPPATH=[crossroot_abs + '/include', crossroot_abs + '/include/SDL'])

    server_only = False
    if env['server']:
        env.Append(CPPDEFINES=["YOG_SERVER_ONLY"])
        server_only = True
    configure(env, server_only)

    env.Append(CPPPATH=['#libgag/include', '#'])
    env.Append(CPPPATH=['#libusl/src', '#'])
    env.Append(CXXFLAGS=' -Wall')
    env.Append(LINKFLAGS=' -Wall')
    env.Append(LIBS=['SDL_net'])
    if not server_only:
        env.Append(LIBS=['vorbisfile', 'SDL_ttf', 'SDL_image', 'speex'])

    if env['release']:
        env.Append(CXXFLAGS=' -O2 -s')
        env.Append(LINKFLAGS=' -O2 -s --fwhole-program')
    if env['profile']:
        env.Append(CXXFLAGS=' -pg')
        env.Append(LINKFLAGS='-pg')
        env.Append(CXXFLAGS=' -O2')
        env.Append(LINKFLAGS='-O2')
    if env['mingw'] or isWindowsPlatform or env['mingwcross']:
        # TODO: Remove unneccessary dependencies for server.
        env.Append(LIBS=['vorbis', 'ogg', 'regex', 'wsock32', 'winmm', 'mingw32', 'SDLmain', 'SDL'])
        env.Append(LINKFLAGS=['-mwindows'])
        env.Append(CPPDEFINES=['-D_GNU_SOURCE=1', '-Dmain=SDL_main'])
    elif isDarwinPlatform:
        env.ParseConfig("/opt/local/bin/sdl-config --cflags")
        env.ParseConfig("/opt/local/bin/sdl-config --libs")
    else:
        env.ParseConfig("sdl-config --cflags")
        env.ParseConfig("sdl-config --libs")
    
    
    env["TARFILE"] = env.Dir("#").abspath + "/glob2-" + env["VERSION"] + ".tar.gz"
    env["TARFLAGS"] = "-c -z"
    env.Alias("dist", env["TARFILE"])
    
    def PackTar(target, source):
        if "dist" in COMMAND_LINE_TARGETS:
            if not list(source) == source:
                source = [source]
                
            for s in source:
                if env.File(s).path.find("/") != -1:
                    new_dir = env.Dir("#").abspath + "/glob2-" + env["VERSION"] + "/"
                    f = env.Install(new_dir + env.File(s).path[:env.File(s).path.rfind("/")], s)
                    env.Tar(target, f)
                else:
                    new_dir = env.Dir("#").abspath + "/glob2-" + env["VERSION"] + "/"
                    f = env.Install(new_dir, s)
                    env.Tar(target, f)
              
    PackTar(env["TARFILE"], Split("COPYING INSTALL mkdist mkinstall mkuninstall README README.hg SConstruct"))
    #packaging for apple
    if isDarwinPlatform and env["release"]:
        bundle.generate(env)
        dmg.generate(env)
        env.Replace( 
            BUNDLE_NAME="Glob2", 
            BUNDLE_BINARIES=["src/glob2"],
            BUNDLE_RESOURCEDIRS=["data","maps", "campaigns"],
            BUNDLE_PLIST="darwin/Info.plist",
            BUNDLE_ICON="darwin/Glob2.icns" )
        bundle.createBundle(os.getcwd(), os.getcwd(), env)
        dmg.create_dmg("Glob2-%s"%env["VERSION"],"%s.app"%env["BUNDLE_NAME"],env)
         
        #TODO mac_bundle should be dependency of Dmg:    
        arch = os.popen("uname -p").read().strip()
#        mac_packages = env.Dmg('Glob2-%s-%s.dmg'% (fullVersion, arch),  env.Dir('Glob2.app/') )
#        env.Alias("package", mac_packages)

    Export('env')
    Export('PackTar')
    if env['mingwcross']:
        Export('crossroot_abs')
    Export('isWindowsPlatform')

    bdir = GetOption('build')
    targets = [
        "campaigns",
        "data",
        "debian",
        "fedora",
        "gnupg",
        "libgag",
        "libusl",
        "maps",
        "natsort",
        "scripts",
        "scons",
        "src",
        "tools",
        "windows"
    ]
    for target in targets:
        SConscript(target + "/SConscript", variant_dir = bdir + "/" + target, duplicate = 0)

main()
