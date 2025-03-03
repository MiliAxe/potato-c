include config.mk


SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
DOC_DIR = doc
DEB_DIR = debug
TESTS_NAME = tests
SHARED_DIR = shared

SRC_CTL = src/potatoctl.c src/utils.c src/client.c src/socket.c src/timer.c src/pidfile.c
OBJ_CTL = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_CTL))

SRC_D = src/timer.c src/potatod.c src/utils.c src/socket.c src/pidfile.c src/client.c
OBJ_D = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_D))

SRC_TUI = src/potatotui.c src/timer.c src/utils.c src/socket.c src/client.c src/todo.c src/ncurses-utils.c src/pidfile.c
OBJ_TUI = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_TUI))

SRC_TESTS = src/tests.c src/timer.c src/utils.c
OBJ_TESTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_TESTS))

OBJ_CTL_DEB = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.odeb,$(SRC_CTL))
OBJ_D_DEB = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.odeb,$(SRC_D))
OBJ_TUI_DEB = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.odeb,$(SRC_TUI))

D_NAME = potd
CTL_NAME = potctl
TUI_NAME = potui

BINS = ${BIN_DIR}/${D_NAME} ${BIN_DIR}/${CTL_NAME} ${BIN_DIR}/${TUI_NAME} 

MD_DOCS = $(wildcard ${DOC_DIR}/*.md)
MAN_PAGES = $(patsubst %.md,%.1,$(MD_DOCS))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.odeb: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) ${DEBFLAGS} -c $< -o $@

all: options ${BINS}

docs: ${MD_DOCS}
	for man in ${MD_DOCS}; do \
		pandoc $$man -s -t man -o $${man/.md}.1; \
	done


test: ${DEB_DIR}/${TESTS_NAME}
	@echo Running tests:
	@./${DEB_DIR}/${TESTS_NAME} && echo Tests are looking good!

install: all install_options ${MAN_PAGES}
	mkdir -p ${DESTDIR}${PREFIX}/bin
	for bin in ${BINS}; do \
		cp -f $$bin ${DESTDIR}${PREFIX}/bin; \
	done

	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	for man_page in ${MAN_PAGES}; do \
		sed "s/VERSION/${VERSION}/g" < $$man_page > ${DESTDIR}${MANPREFIX}/man1/$$(basename $$man_page); \
	done


${DEB_DIR}/${TESTS_NAME}: ${OBJ_TESTS}
	mkdir -p ${DEB_DIR}
	$(CC) -o ${DEB_DIR}/${TESTS_NAME} ${OBJ_TESTS}

${OBJ_CTL} ${OBJ_D} ${OBJ_TUI} ${OBJ_TESTS} ${OBJ_DEBUG}: include/signal.h config.h config.mk

install_options:
	@echo potato install options:
	@echo "DESTDIR  = ${DESTDIR}"
	@echo "PREFIX   = ${PREFIX}"

options:
	@echo potato build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

${BIN_DIR}/${D_NAME}: ${OBJ_D}
	mkdir -p $(BIN_DIR)
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${OBJ_D}

${BIN_DIR}/${CTL_NAME}: ${OBJ_CTL}
	mkdir -p $(BIN_DIR)
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ ${OBJ_CTL}

${BIN_DIR}/${TUI_NAME}: ${OBJ_TUI}
	mkdir -p $(BIN_DIR)
	${CC} ${CFLAGS} ${LDFLAGS} `pkg-config --libs --cflags ncurses` -o $@ ${OBJ_TUI}


debug: ${DEB_DIR}/${D_NAME} ${DEB_DIR}/${CTL_NAME} ${DEB_DIR}/${TUI_NAME}

${DEB_DIR}/${D_NAME}: ${OBJ_D_DEB}
	mkdir -p $(DEB_DIR)
	$(CC) ${CFLAGS} ${LDFLAGS} ${DEBFLAGS} -o $@ ${OBJ_D_DEB}

${DEB_DIR}/${CTL_NAME}: ${OBJ_CTL_DEB}
	mkdir -p $(DEB_DIR)
	$(CC) ${CFLAGS} ${LDFLAGS} ${DEBFLAGS} -o $@ ${OBJ_CTL_DEB}

${DEB_DIR}/${TUI_NAME}: ${OBJ_TUI_DEB}
	mkdir -p $(DEB_DIR)
	$(CC) ${CFLAGS} ${LDFLAGS} ${DEBFLAGS} `pkg-config --libs --cflags ncurses` -o $@ ${OBJ_TUI_DEB}

config.h: 
	cp config.def.h $@

clean:
	rm $(OBJ_DIR) $(BIN_DIR) $(DEB_DIR) -rf

uninstall:
	rm ${DESTDIR}${PREFIX}/bin/${D_NAME}
	rm ${DESTDIR}${PREFIX}/bin/${CTL_NAME}
	rm ${DESTDIR}${PREFIX}/bin/${TUI_NAME}
