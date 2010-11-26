# $Id$
# Author: Akira Okumura 2009/09/28

###############################################################################
#  Copyright (C) 2009-, Akira Okumura                                         #
#  All rights reserved.                                                       #
###############################################################################

include $(ROOTSYS)/test/Makefile.arch

NAME	:=	ROBAST
DEPEND	:=	libCore libGeom libGeomPainter libPhysics libGraf libGraf3d

EXTLIBS	:=	

SRCDIR	:=	src
INCDIR	:=	inc

DICT	:=	$(NAME)Dict
DICTS	:=	$(SRCDIR)/$(NAME)Dict.$(SrcSuf)
DICTI	:=	$(SRCDIR)/$(NAME)Dict.h
DICTO	:=	$(SRCDIR)/$(NAME)Dict.$(ObjSuf)

INCS	:=	$(filter-out $(INCDIR)/LinkDef.h,$(wildcard $(INCDIR)/*.h))
SRCS	:=	$(filter-out $(SRCDIR)/$(DICT).%,$(wildcard $(SRCDIR)/*.$(SrcSuf)))
OBJS	:=	$(patsubst %.$(SrcSuf),%.$(ObjSuf),$(SRCS)) $(DICTO)

LIB	=	lib$(NAME).$(DllSuf)

RMAP	=	lib$(NAME).rootmap

UNITTEST:= $(wildcard unittest/*.py)

.SUFFIXES:	.$(SrcSuf) .$(ObjSuf) .$(DllSuf)
.PHONY:		all clean test doc htmldoc

all:		$(RMAP)

$(LIB):		$(OBJS)
ifeq ($(PLATFORM),macosx)
# We need to make both the .dylib and the .so
		$(LD) $(SOFLAGS)$@ $(LDFLAGS) $(EXTLIBS) $^ $(OutPutOpt) $@
ifneq ($(subst $(MACOSX_MINOR),,1234),1234)
ifeq ($(MACOSX_MINOR),4)
		ln -sf $@ $(subst .$(DllSuf),.so,$@)
else
		$(LD) -bundle -undefined $(UNDEFOPT) $(LDFLAGS) $(EXTLIBS) $^ \
		   $(OutPutOpt) $(subst .$(DllSuf),.so,$@)
endif
endif
else
		$(LD) $(SOFLAGS) $(LDFLAGS) $(EXTLIBS) $^ $(OutPutOpt) $@ $(EXPLLINKLIBS)
endif
		@echo "$@ done"

$(SRCDIR)/%.$(ObjSuf):	$(SRCDIR)/%.$(SrcSuf) $(INCDIR)/%.h
		@echo "Compiling" $<
		$(CXX) $(CXXFLAGS) -Wall -g -I$(INCDIR) -c $< -o $@

$(DICTS):	$(INCS) $(INCDIR)/LinkDef.h
		@echo "Generating dictionary ..."
		$(ROOTCINT) -f $@ -c -p $^

$(DICTO):	$(DICTS)
		@echo "Compiling" $<
		$(CXX) $(CXXFLAGS) -I. -c $< -o $@

$(RMAP):	$(LIB) $(INCDIR)/LinkDef.h
		rlibmap -f -o $@ -l $(LIB) -d $(DEPEND) -c $(INCDIR)/LinkDef.h
doc:	all htmldoc

htmldoc:
		sh mkhtml.sh

clean:
		rm -rf $(LIB) $(OBJS) $(DICTI) $(DICTS) $(DICTO) $(RMAP)

test:		all
		@for script in $(UNITTEST);\
		do \
		echo "Executing" $$script "...";\
		python $$script;\
		done
