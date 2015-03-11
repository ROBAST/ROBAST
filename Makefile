# $Id$
# Author: Akira Okumura 2009/09/28

###############################################################################
#  Copyright (C) 2009-, Akira Okumura                                         #
#  All rights reserved.                                                       #
###############################################################################

# Since revision 40240 (Thu Jul 14 15:17:53 2011 UTC), Makefile.arch and
# test/Makefile have been changed. In addition, since revision 41594
# (Wed Oct 26 12:38:25 2011 UTC), $ROOTSYS/test/Makefile.arch has been moved
# to $ROOTSYS/etc/Makefile.arch.
# In order to make ROBAST work with older ROOT releases, Makefile.arch must be
# checked here 

# older version
MAKEARCH	:=	$(shell find $(ROOTSYS)/test -name Makefile.arch)

ifeq ($(MAKEARCH), )
# 41594 or later
MAKEARCH	:=	$(shell find $(ROOTSYS)/etc -name Makefile.arch)
endif

ifeq ($(MAKEARCH), )
MAKEARCH	:=	$(wildcard $(shell $(RC) --etcdir)/Makefile.arch)
endif

include $(MAKEARCH)

ifeq ($(ROOTCLING),)
ROOTCLING	:=	$(ROOTCINT)
else
ROOTCLING_FOUND	:= 1
endif

NAME	:=	ROBAST
DEPEND	:=	libCore libGeom libGeomPainter libPhysics libGraf libGraf3d

SRCDIR	:=	src
INCDIR	:=	include
BINCDIR	:=	$(INCDIR)/bernlohr

DICT	:=	$(NAME)Dict
DICTS	:=	$(SRCDIR)/$(NAME)Dict.$(SrcSuf)
DICTI	:=	$(SRCDIR)/$(NAME)Dict.h
DICTO	:=	$(SRCDIR)/$(NAME)Dict.$(ObjSuf)

INCS	:=	$(filter-out $(INCDIR)/LinkDef.h,$(wildcard $(INCDIR)/*.h))
SRCS	:=	$(filter-out $(SRCDIR)/$(DICT).%,$(wildcard $(SRCDIR)/*.$(SrcSuf)))
OBJS	:=	$(patsubst %.$(SrcSuf),%.$(ObjSuf),$(SRCS)) $(DICTO)
PCM	:=	$(NAME)Dict_rdict.pcm

ORG1	:=	$(SRCDIR)/bernlohr/fileopen.c
ORG2	:=	$(SRCDIR)/bernlohr/io_simtel.c
ORG3	:=	$(SRCDIR)/bernlohr/warning.c
ORGS	:=	$(ORG1) $(ORG2) $(ORG3)
MOD1	:=	$(patsubst %.c,%_mod.c,$(ORG1))
MOD2	:=	$(patsubst %.c,%_mod.c,$(ORG2))
MOD3	:=	$(patsubst %.c,%_mod.c,$(ORG3))
MODS	:=	$(MOD1) $(MOD2) $(MOD3)
BSRCS	:=	$(filter-out $(ORGS),$(wildcard $(SRCDIR)/bernlohr/*.c)) $(MODS)
BOBJS	:=	$(patsubst %.c,%.$(ObjSuf),$(BSRCS))

LIB	=	lib$(NAME).$(DllSuf)

CXXFLAGS	+= $(ROBASTFLAGS)

#CXXFLAGS	+= -fopenmp
ifneq ($(EXPLLINKLIBS), )
#EXPLLINKLIBS	+= -lgomp -lGeom -lGeomPainter
EXPLLINKLIBS	+= -lGeom -lGeomPainter
endif

RMAP	=	lib$(NAME).rootmap

UNITTEST:= $(wildcard unittest/*.py)

.SUFFIXES:	.$(SrcSuf) .$(ObjSuf) .$(DllSuf)
.PHONY:		all clean test doc htmldoc

ifeq ($(ROOTCLING_FOUND),)
all:		$(LIB) $(RMAP)
else
all:		$(LIB) $(PCM)
endif

$(MOD1): $(ORG1)
		sed -e 's/s = malloc(/s = (char\*)malloc(/g' $< | \
		sed -e 's/root_path = calloc(/root_path = (struct incpath\*)calloc(/g' | \
		sed -e 's/last->next = calloc(/last->next = (struct incpath\*)calloc(/g' | \
		sed -e 's/s = strchr(fname/s = (char\*)strchr(fname/g' > $@

$(MOD2): $(ORG2)
		sed -e 's/xl->text = malloc(/xl->text = (char\*)malloc(/g' $< | \
		sed -e 's/xln->text = malloc(/xln->text = (char\*)malloc(/g' | \
		sed -e 's/ep->iparam = calloc(/ep->iparam = (int\*)calloc(/g' | \
		sed -e 's/ep->fparam = calloc(/ep->fparam = (int\*)calloc(/g' | \
		sed -e 's/xln = calloc(/xln = (struct linked_string\*)calloc(/g' > $@

$(MOD3): $(ORG3)
		sed -e 's/struct warn_specific_data \*wt = get_warn_specific();/struct warn_specific_data \*wt = (struct warn_specific_data\*)get_warn_specific();/g' $< > $@

$(LIB):		$(OBJS) $(BOBJS)
ifeq ($(PLATFORM),macosx)
# We need to make both the .dylib and the .so
		$(LD) $(SOFLAGS)$@ $(LDFLAGS) $^ $(OutPutOpt) $@ $(EXPLLINKLIBS)
ifneq ($(subst $(MACOSX_MINOR),,1234),1234)
ifeq ($(MACOSX_MINOR),4)
		ln -sf $@ $(subst .$(DllSuf),.so,$@)
else
ifneq ($(UNDEOPT), )
		$(LD) -bundle -undefined $(UNDEFOPT) $(LDFLAGS) $^ \
		   $(OutPutOpt) $(subst .$(DllSuf),.so,$@)
endif
endif
endif
else
ifeq ($(PLATFORM),win32)
		bindexplib $* $^ > $*.def
		lib -nologo -MACHINE:IX86 $^ -def:$*.def \
		   $(OutPutOpt)$(EVENTLIB)
		$(LD) $(SOFLAGS) $(LDFLAGS) $^ $*.exp $(LIBS) \
		   $(OutPutOpt)$@
		$(MT_DLL)
else
		$(LD) $(SOFLAGS) $(LDFLAGS) $^ $(OutPutOpt) $@ $(EXPLLINKLIBS)
endif
endif
		@echo "$@ done"

$(SRCDIR)/%.$(ObjSuf):	$(SRCDIR)/%.$(SrcSuf) $(INCDIR)/%.h
		@echo "Compiling" $<
		$(CXX) $(CXXFLAGS) -Wall -g -I$(INCDIR) -c $< -o $@

$(SRCDIR)/%.$(ObjSuf):	$(SRCDIR)/%.c
		@echo "Compiling" $<
		$(CC) $(CCFLAGS) -I$(BINCDIR) -fPIC -c $< -o $@

$(DICTS):	$(INCS) $(INCDIR)/LinkDef.h
		@echo "Generating dictionary ..."
		$(ROOTCLING) -f $@ -c -p $^

$(PCM):		$(SRCDIR)/$(PCM)
		cp $^ $@

$(DICTO):	$(DICTS)
		@echo "Compiling" $<
		$(CXX) $(CXXFLAGS) -I. -c $< -o $@

$(RMAP):	$(LIB) $(INCDIR)/LinkDef.h
		rlibmap -f -o $@ -l $(LIB) -d $(DEPEND) -c $(INCDIR)/LinkDef.h
doc:	all htmldoc

htmldoc:
		sh mkhtml.sh

clean:
		rm -rf $(LIB) $(MODS) $(OBJS) $(BOBJS) $(DICTI) $(DICTS) $(DICTO) $(PCM) $(SRCDIR)/$(PCM) $(RMAP)

test:		all
		@for script in $(UNITTEST);\
		do \
		echo "Executing" $$script "...";\
		python $$script;\
		done
