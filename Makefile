#
# Makefile for pr-count-display
# Copyright (C) 2019 Artur "suve" Iwicki
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License
# as published by the Free Software Foundation -
# either version 2 of the license, or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program (LICENCE.txt). If not, see <http://www.gnu.org/licenses/>.
#
FLAGS += -std=c99 -iquote ./src/ -Wall -Wextra -Og -ggdb
CFLAGS += -Wfloat-equal -Wparentheses -Wmisleading-indentation -Wmissing-field-initializers
CFLAGS += -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=int-conversion -Werror=div-by-zero -Werror=sequence-point -Werror=uninitialized -Werror=duplicated-cond -Werror=return-type -Werror=implicit-function-declaration -Werror=implicit-int

LDLIBS += -lm -lSDL2 -lSDL2_image -lSDL2_ttf

.PHONY = all clean


SOURCES := $(shell find src/ -name '*.c')
OBJECTS := $(SOURCES:src/%.c=build/%.o)


# -- start .PHONY rules

all: build/pr-count-display

clean:
	rm -rf build/

# -- end of .PHONY rules

build/pr-count-display: $(OBJECTS)
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) -o "$@" $^ $(LDLIBS)

build/%.o: src/%.c
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) -c -o "$@" "$<"
