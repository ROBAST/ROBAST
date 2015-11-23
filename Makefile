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
MAKEARCH	:=	$(shell find -L $(ROOTSYS)/test -name Makefile.arch 2> /dev/null)

ifeq ($(MAKEARCH), )
# 41594 or later
MAKEARCH	:=	$(shell find -L $(ROOTSYS)/etc -name Makefile.arch)
endif

ifeq ($(MAKEARCH), )
RC := root-config
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

BSRCS	:=	$(wildcard $(SRCDIR)/bernlohr/*.c)
BOBJS	:=	$(patsubst %.c,%.$(ObjSuf),$(BSRCS))

LIB	=	lib$(NAME).$(DllSuf)

CXXFLAGS	+= $(ROBASTFLAGS)

ifneq ($(EXPLLINKLIBS), )
EXPLLINKLIBS	+= -lGeom -lGeomPainter
endif

RMAP	=	lib$(NAME).rootmap

.SUFFIXES:	.$(SrcSuf) .$(ObjSuf) .$(DllSuf)
.PHONY:		all clean doc htmldoc

ifeq ($(ROOTCLING_FOUND),)
# ROOT 5
all:		$(LIB) $(RMAP)

$(RMAP):	$(LIB) $(INCDIR)/LinkDef.h
		rlibmap -f -o $@ -l $(LIB) -d $(DEPEND) -c $(INCDIR)/LinkDef.h

$(DICTS):	$(INCS) $(INCDIR)/LinkDef.h
		@echo "Generating dictionary ..."
		 $(ROOTCLING) -f $@ -c -p $^
else
# ROOT 6
all:		$(LIB) $(PCM)

$(SRCDIR)/$(PCM):	$(DICTS)

$(DICTS):	$(INCS) $(INCDIR)/LinkDef.h
		@echo "Generating dictionary ..."
		$(ROOTCLING) -f $@ -c -p -I$(INCDIR) -rmf $(RMAP) -rml $(LIB) $^
endif

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

$(PCM):		$(SRCDIR)/$(PCM)
		cp $^ $@

$(DICTO):	$(DICTS)
		@echo "Compiling" $<
		$(CXX) $(CXXFLAGS) -I. -c $< -o $@

doc:	all htmldoc

htmldoc:
		sh mkhtml.sh

clean:
		rm -rf $(LIB) $(OBJS) $(BOBJS) $(DICTI) $(DICTS) $(DICTO) $(PCM) $(SRCDIR)/$(PCM) $(RMAP)
