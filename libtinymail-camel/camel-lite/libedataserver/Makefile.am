INCLUDES =								\
	-I$(top_srcdir)							\
	-DG_LOG_DOMAIN=\"e-data-server\"				\
	$(DB_CFLAGS)							\
	-DE_DATA_SERVER_PREFIX=\"$(prefix)\"				\
	-DLOCALEDIR=\""$(localedir)"\"					\
	-DE_DATA_SERVER_EXTENSIONDIR=\"$(extensiondir)\"		\
	-DE_DATA_SERVER_IMAGESDIR=\"$(imagesdir)\"			\
	-DE_DATA_SERVER_UI_GLADEDIR=\""$(gladedir)"\"			\
	$(E_DATA_SERVER_CFLAGS)

noinst_LIBRARIES = libedataserver-1.2.a

libedataserver_1_2_a_SOURCES =		\
	e-trie.c			\
	e-iconv.c			\
	e-memory.c			\
	e-msgport.c			\
	e-sexp.c			\
	e-time-utils.c			\
	e-data-server-util.c		\
	md5-utils.c

libedataserverincludedir = $(includedir)/libedataserver-lite/libedataserver

libedataserverinclude_HEADERS =		\
	e-trie.h			\
	e-iconv.h			\
	e-memory.h			\
	e-msgport.h			\
	e-sexp.h			\
	e-time-utils.h			\
	e-data-server-util.h		\
	md5-utils.h

