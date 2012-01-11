#
# debug.profile
#

PROFILE_PREFIX=debug

# Flags passed to the compiler.
PROFILE_CCFLAGS=-DWIP_NO_SSL -DWIP_LOGGER -DWIP_LOGGER_DEBUG
PROFILE_CXXFLAGS=-DWIP_NO_SSL -DWIP_LOGGER -DWIP_LOGGER_DEBUG

# Flags passed to the linker.
PROFILE_LDFLAGS=

# Flags passed to the archiver
PROFILE_ARFLAGS=

# Flags passed to strip
PROFILE_STRIPFLAGS=
