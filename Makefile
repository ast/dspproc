CC := clang
CFLAGS := -Wall -O3 -mcpu=cortex-a53 -mfpu=neon-vfpv4 -mfloat-abi=hard -funsafe-math-optimizations

# -mfpu=neon-vfpv4
program_NAME := fcd
program_C_SRCS := $(wildcard *.c)
program_C_OBJS := ${program_C_SRCS:.c=.o}
program_OBJS := $(program_C_OBJS)
program_INCLUDE_DIRS := "/usr/local/include"
program_LIBRARY_DIRS :=
program_LIBRARIES := asound m liquid NE10

LDFLAGS += $(foreach librarydir,$(program_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS += $(foreach library,$(program_LIBRARIES),-l$(library))

.PHONY: all clean distclean

all: $(program_NAME)

$(program_NAME): $(program_OBJS)
	$(LINK.o) $(CFLAGS) $(program_OBJS) -o $(program_NAME)

clean:
	@- $(RM) $(program_NAME)
	@- $(RM) $(program_OBJS)

distclean: clean
