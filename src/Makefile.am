bin_PROGRAMS= variscan
variscan_SOURCES = 	variscan.c variscan.h \
			statistics.c statistics.h \
			common.c common.h \
			linefile.c linefile.h \
			memalloc.c memalloc.h \
			dlist.c dlist.h \
			errabort.c errabort.h \
			dystring.c dystring.h \
			intExp.c kxTok.h kxTok.c \
			pop.c pop.h \
			window.c window.h \
			free.c free.h \
			output.c output.h \
			readphylip.c readphylip.h \
			readaxt.c readaxt.h \
			readmga.c readmga.h \
			readmaf.c readmaf.h \
			readhapmap.c readhapmap.h \
			readxmfa.c readxmfa.h \
			readmav.c readmav.h \
			ran1.c ran1.h \
			swcolumn.c swcolumn.h \
			swnet.c swnet.h \
			swpoly.c swpoly.h \
			swref.c swref.h
			globalslinux.c globalslinux.h
# 			globalslinux.c globalslinux.h globalswin32.c globalswin32.h 
variscan_LDADD = -lm
#AM_CPPFLAGS = -DVARISCAN -lm
AM_CPPFLAGS = -DVARISCAN
#LD_FLAGS = -lm
# AM_CFLAGS = -pg

if DEBUG
OTHERFLAGS = -g -pg -fprofile-arcs -ftest-coverage -ftrapv
# AM_CFLAGS =
else
OTHERFLAGS = -DNDEBUG -O3
# coverage -> -fprofile-arcs -ftest-coverage
# AM_CFLAGS =
endif
# AM_LDFLAGS =   $(OTHERFLAGS) -Wall -lm -ansi

#AM_CFLAGS = $(OTHERFLAGS) -W -Wall -lm -ansi 
AM_CFLAGS = $(OTHERFLAGS) -W -Wall -ansi -pedantic
# CFLAGS =

# bin_PROGRAMS= axt_variscan
# axt_variscan_SOURCES = variscan.20040128.c variscan.20040128.h statistics.c statistics.h intExp.c kxTok.h kxTok.c common.c common.h linefile.c linefile.h memalloc.c memalloc.h dlist.c dlist.h errabort.c errabort.h dystring.c dystring.h pop.c pop.h
# AM_CPPFLAGS = -DAXT_VARISCAN
# # AM_CFLAGS = -pg
# AM_LDFLAGS = -lm

# bin_PROGRAMS= _test_free_polyarray
# _test_free_polyarray_SOURCES = _test_free_polyarray.c variscan.20040128.c variscan.20040128.h statistics.c statistics.h intExp.c kxTok.h kxTok.c common.c common.h linefile.c linefile.h memalloc.c memalloc.h dlist.c dlist.h errabort.c errabort.h dystring.c dystring.h 
# # AM_CPPFLAGS = -DAXT_VARISCAN
# # AM_CFLAGS = -pg
# AM_LDFLAGS = -lm

# bin_PROGRAMS= __prova_dl
# __prova_dl_SOURCES = _prova_dl.c variscan.h common.c common.h linefile.c \
# 			linefile.h memalloc.c memalloc.h dlist.c \
# 			dlist.h errabort.c errabort.h dystring.c dystring.h
# # AM_CFLAGS = -DVARISCAN -pg

# bin_PROGRAMS= prova_borrar
# prova_borrar_SOURCES = prova_borrar.c common.c common.h linefile.c	\
# 			  linefile.h memalloc.c memalloc.h errabort.c errabort.h dlist.c dlist.h

# # AM_CFLAGS = -DMGAALN2PHY
# CFLAGS = -g -pg


# bin_PROGRAMS= provadystringlist
# provadystringlist_SOURCES = provadystringlist.c common.c common.h linefile.c \
# 			  linefile.h memalloc.c memalloc.h dlist.c \
# 			  dlist.h errabort.c errabort.h dystring.c dystring.h
# AM_CFLAGS =
# CFLAGS = -g -pg

# bin_PROGRAMS= variscanmgaaln2phy
# variscanmgaaln2phy_SOURCES = 20030526mgaaln2phy.c mgaaln2phy.h common.c common.h linefile.c \
# 			  linefile.h memalloc.c memalloc.h dlist.c \
# 			  dlist.h errabort.c errabort.h dystring.c dystring.h
# AM_CFLAGS = -DMGAALN2PHY
# CFLAGS = -g -pg


# bin_PROGRAMS= variscanmain
# variscanmain_SOURCES = main.c mga.h common.c common.h linefile.c \
# 			  linefile.h memalloc.c memalloc.h dlist.c \
# 			  dlist.h errabort.c errabort.h dystring.c dystring.h
# AM_CFLAGS = -DMAIN
# #CFLAGS = -g -pg -lm
# CFLAGS = -lm

# bin_PROGRAMS= variscan
# variscan_SOURCES = proves20030428.c common.c common.h linefile.c \
# 			  linefile.h memalloc.c memalloc.h dlist.c \
# 			  dlist.h errabort.c errabort.h dystring.c dystring.h
# AM_CFLAGS = -DMAIN
# CFLAGS = -g -pg -lm
# #CFLAGS = -lm


# bin_PROGRAMS= borrar3
# borrar3_SOURCES = borrar3.c
# AM_CFLAGS = 
# CFLAGS = -g -lm -pg
