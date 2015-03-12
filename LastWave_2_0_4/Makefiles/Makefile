#
# Makefile for LastWave 2.0.3 for Unix, Copyright 2002 Remi Gribonval, INRIA
#

#-----------------------------------------------------
# --- The only paths you may need to change !
#
# Remark: the variable ARCH is normally defined by your system
#         (if it is not, you should set it up in your 
#          .*shrc/.*profile or .*login file together with the
#          LWSOURCEDIR variable)
#
#         The only files you may (and should) setup are the 
#         MakeDefs.$(ARCH) and MakeRules.$(ARCH).
#
#-----------------------------------------------------
BINNAME  = lw
SHELL    = /bin/sh
#-----------------------------------------------------
# Machine dependant definitions
# (compiler options, library paths, and ARCH variable)
# 
# Remark: later we may have to perform here what is 
#         currently done in computer.h
#-----------------------------------------------------
include MakeDefs.$(ARCH)
#-----------------------------------------------------
RUN       = $(BINPATH)$(BINNAME)
DLL       = $(LIBPATH)$(BINNAME).$(SHAREDEXT)

# The list of kernel and machine dependent files
# as will as the list of packages
include PackageList

# The main target is the executable
$(RUN) : libkernel libpackages
	$(CC) -o $(RUN) $(LWPATH)/*/obj/$(ARCH)/*.$(OBJEXT) $(LIBS)  

# To make the executable, we need  the kernel and packages libraries
all : libkernel libpackages
	$(CC) -o $(RUN) $(LWPATH)/*/obj/$(ARCH)/lib*.$(LIBEXT) $(LIBS)  

dll : libkernel libpackages
	$(CC) -shared -o $(DLL) $(LWPATH)/*/obj/$(ARCH)/lib*.$(LIBEXT) $(LIBS)

libkernel : 
	for i in  $(LISTKERNEL) ; do cd $(LWPATH)/$$i/obj ; $(MAKE) all; done

libpackages : 
	for i in  $(LISTPACKAGE) ; do cd $(LWPATH)/package_$$i/obj ; $(MAKE) all; done

clean :
	for i in  $(LISTKERNEL) ; do cd $(LWPATH)/$$i/obj; $(MAKE) clean ; done
	for i in  $(LISTPACKAGE) ; do cd $(LWPATH)/package_$$i/obj; $(MAKE) clean ; done
	$(RM) $(RUN)
	$(RM) $(LWPATH)/Makefiles/*~

# This is for development with emacs
tags :
	cd $(LWPATH); etags */src/*.c */include/*.h

# This is for conversion of DOS files to UNIX format
#dos2unix :
#	for i in $(LISTKERNEL) ; do cd $(LWPATH)/$$i/obj/ ; dos2unix -l Makefile; dos2unix -l FileList ; done
#	for i in $(LISTPACKAGE) ; do cd $(LWPATH)/package_$$i/obj/ ; dos2unix -l Makefile; dos2unix -l FileList; done

makes :
	for i in $(LISTKERNEL) ; do $(CP) GenericUnixMakefile $(LWPATH)/$$i/obj/Makefile ; done
	for i in $(LISTPACKAGE) ; do $(CP) GenericUnixMakefile $(LWPATH)/package_$$i/obj/Makefile ; done


# And this creates the obj/bin directories for the target architecture
dirs :
	if test ! -d $(BINPATH); then mkdir $(BINPATH); fi
	if test ! -d $(LIBPATH); then mkdir $(LIBPATH); fi
	for i in $(LISTKERNEL) ; do cd $(LWPATH)/$$i/obj/ ; if test ! -d $(ARCH); then mkdir $(ARCH); fi ; done
	for i in $(LISTPACKAGE) ; do cd $(LWPATH)/package_$$i/obj/ ; if test ! -d $(ARCH); then mkdir $(ARCH); fi ; done

# Creation of a tar.gz archive 

tar : 
	cd $(HOME)/softs; tar cvf LastWave$(VERSION).tar LastWave$(VERSION)/Makefiles LastWave$(VERSION)/*/src/*.c LastWave$(VERSION)/*/src/*.java LastWave$(VERSION)/*/include/*.h LastWave$(VERSION)/*/obj/Make* LastWave$(VERSION)/*/obj/FileList LastWave$(VERSION)/scripts ; gzip LastWave$(VERSION).tar













