#
# Matfinder, a program to help chess engines to find mat
#
# Copyright© 2013 Philippe Virouleau
#
# You can contact me at firstname.lastname@imag.fr
# (Replace "firstname" and "lastname" with my actual names)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
TOPDIR=$(shell pwd)

#include $(TOPDIR)/common.mk

UNAME_S:=$(shell uname -s)
ifeq ($(UNAME_S),Linux)
	LDFLAGS += -L/usr/lib/x86_64-linux-gnu
endif

LDFLAGS += -lpthread
SUBDIRS:=

ALL_TARGETS =
INSTALL_TARGETS =
CLEAN_TARGETS =
DISTCLEAN_TARGETS =

CPPFLAGS += -Iinclude -std=c++11 -Wall

all: real-all

include src/main.mk
include boardtest/boardTest.mk

real-all: $(ALL_TARGETS)

install: $(INSTALL_TARGETS)


clean: $(CLEAN_TARGETS)

