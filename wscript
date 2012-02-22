
APPNAME = "cgp"
VERSION = "0.1-alpha"

top = "."
out = "build"

def s(str):
	return str.split()

CF_WARNINGS = s("""
  -Wall -Wextra -Wpointer-arith -Wshadow
  -Wstrict-prototypes -Wmissing-prototypes
  -Wno-unused-parameter -Wno-unused-function
  -Wno-unused-variable
""")

CFLAGS = CF_WARNINGS + s("-std=c99 -O3")

CFLAGS_DEBUG = s("-DDEBUG -g")

def options(opt):
	opt.load("compiler_c")
	opt.add_option("--debug", action="store_true", default=False, dest="debug")

def configure(conf):
	conf.load("compiler_c")
	
def build(bld):
	
	final_flags = CFLAGS
	if bld.options.debug:
		final_flags += CFLAGS_DEBUG

	"""
   	bld.stlib(
		source   = "src/gp.c",
		target   = "libgp",
		includes = "include",
		cflags   = final_flags
	)
	"""

	bld.program(
		source ="src/gp.c src/test.c",
		target="gptest",
		use="libgp",
		includes = "include",
		cflags = final_flags
	)
	
def test(ctx):
	print "Testing and timing gp..."
	ctx.exec_command("time build/gptest")
	print ""
