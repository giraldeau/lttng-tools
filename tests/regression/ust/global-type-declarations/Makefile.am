AM_CPPFLAGS = -I$(srcdir)

noinst_PROGRAMS = global-type-declarations
global_type_declarations_SOURCES = global-type-declarations.c ust_tests_gtd.h
global_type_declarations_LDADD = -llttng-ust

if LTTNG_TOOLS_BUILD_WITH_LIBDL
global_type_declarations_LDADD += -ldl
endif
if LTTNG_TOOLS_BUILD_WITH_LIBC_DL
global_type_declarations_LDADD += -lc
endif

noinst_SCRIPTS = test_global_type_declarations test_global_type_declarations.py
EXTRA_DIST = test_global_type_declarations test_global_type_declarations.py

all-local:
	@if [ x"$(srcdir)" != x"$(builddir)" ]; then \
		for script in $(EXTRA_DIST); do \
			cp -f $(srcdir)/$$script $(builddir); \
		done; \
	fi

clean-local:
	@if [ x"$(srcdir)" != x"$(builddir)" ]; then \
		for script in $(EXTRA_DIST); do \
			rm -f $(builddir)/$$script; \
		done; \
	fi
