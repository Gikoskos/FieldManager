CC = gcc
WINDRES = windres
INCLUDE =
CFLAGS = -Wall -std=c11
LINKER = -lmrss -lnxml -pthread -lws2_32 -lwininet -lgdi32 -lcomctl32 -lz.dll -lcurl.dll -lmCtrl.dll


RES = Resources.rc
RESHDR = Resources.h
RESOBJ = $(patsubst %.rc, %.o, $(RES))

SRCS = $(wildcard *.c)
HDRS = $(wildcard *.h)
OBJS = $(patsubst %.c, %.o, $(SRCS))

EXE = FieldManager.exe


# Debug build settings
DBGDIR = debug
DBGEXE = $(DBGDIR)/$(EXE)
DBGOBJS = $(addprefix $(DBGDIR)/, $(OBJS))
DBGFLAGS = -g -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -pedantic -D__DBG

# Release build settings
RLSDIR = release
RLSEXE = $(RLSDIR)/$(EXE)
RLSOBJS = $(addprefix $(RLSDIR)/, $(OBJS))
RLSFLAGS = -mwindows -O3

# Phony targets
.PHONY: dbg rls clean all

all: dbg rls

# Debug build rules
dbg: $(DBGEXE)

$(DBGEXE): $(DBGOBJS) $(RESOBJ)
	$(CC) $(CFLAGS) $(DBGFLAGS) $^ -o $@ $(LINKER)

$(DBGOBJS): $(DBGDIR)/%.o: %.c $(HDRS)
	$(CC) -c $(CFLAGS) $(DBGFLAGS) $< -o $@

# Release build rules
rls: $(RLSEXE)

$(RLSEXE): $(RLSOBJS) $(RESOBJ)
	$(CC) $(CFLAGS) $(RLSFLAGS) $^ -o $@ $(LINKER)

$(RLSOBJS): $(RLSDIR)/%.o: %.c $(HDRS)
	$(CC) -c $(CFLAGS) $(RLSFLAGS) $< -o $@


# Resource object build rules
$(RESOBJ): $(RES) $(RESHDR)
	$(WINDRES) $(RES) -o $@

# Clean up
clean:
	@pushd debug && del $(OBJS) $(EXE) && popd && pushd release && del $(OBJS) $(EXE) && popd && del $(RESOBJ)
