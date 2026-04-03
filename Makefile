VERSION = 0.2

CC ?= cc
PREFIX ?= /usr/local

LIBX11_CFLAGS = $(shell pkg-config --cflags x11)
LIBX11_LDFLAGS = $(shell pkg-config --libs x11)

CDEFINITIONS = -DVERSION=\"${VERSION}\"
CFLAGS = -MMD -MP -O2 ${CDEFINITIONS} \
		 -Wpedantic -Wall -Wextra \
		 -Wno-override-init \
		 -Wmissing-field-initializers \
		 ${LIBX11_CFLAGS}
LDFLAGS = ${LIBX11_LDFLAGS}

SRC_DIR = src
BUILD_DIR = build
DIST_DIR = dist

DIST = ${DIST_DIR}/fstatus-${VERSION}

SRC = $(shell find ${SRC_DIR} -type f -name '*.c')
OBJ = $(patsubst ${SRC_DIR}/%.c,${BUILD_DIR}/%.o,${SRC})
DEP = $(OBJ:.o=.d)

BIN = ${BUILD_DIR}/fstatus

all: ${BIN}

${BUILD_DIR}/%.o: ${SRC_DIR}/%.c
	mkdir -p $(dir $@)
	${CC} -c ${CFLAGS} $< -o $@

${BIN}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

-include ${DEP}

clean:
	rm -rf ${BUILD_DIR} ${BIN} ${DIST}

dist: clean
	mkdir -p ${DIST}
	cp -R LICENSE Makefile README.md src ${DIST}
	tar -cf ${DIST}.tar ${DIST}
	gzip ${DIST}.tar
	rm -rf ${DIST}

install: all
	mkdir -p ${DEST_DIR}${PREFIX}/bin
	cp -f ${BIN} ${DEST_DIR}${PREFIX}/bin/fstatus
	chmod 755 ${DEST_DIR}${PREFIX}/bin/fstatus

uninstall:
	rm -f ${DEST_DIR}${PREFIX}/bin/fstatus

.PHONY: all clean dist install uninstall
