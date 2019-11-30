# dont_drop_db/Makefile

MODULE_big = dont_drop_db
OBJS = dont_drop_db.o

EXTENSION = dont_drop_db
DATA = dont_drop_db--1.0.sql

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/dont_drop_db
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif

