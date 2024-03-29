# Makefile.am for the SDL sample image loading library and viewer
ACLOCAL_AMFLAGS = -I acinclude

lib_LTLIBRARIES = libSDL2_image.la

libSDL2_imageincludedir = $(includedir)/SDL2
libSDL2_imageinclude_HEADERS =	\
	SDL_image.h

if USE_IMAGEIO
IMAGEIO_SOURCE = IMG_ImageIO.m
endif

libSDL2_image_la_SOURCES =	\
	IMG.c			\
	IMG_bmp.c		\
	IMG_gif.c		\
	IMG_jpg.c		\
	IMG_lbm.c		\
	IMG_pcx.c		\
	IMG_png.c		\
	IMG_pnm.c		\
	IMG_qoi.c		\
	IMG_svg.c		\
	IMG_tga.c		\
	IMG_tif.c		\
	IMG_xcf.c		\
	IMG_xpm.c		\
	IMG_xv.c		\
	IMG_webp.c		\
	IMG_WIC.c		\
	$(IMAGEIO_SOURCE)	\
	miniz.h			\
	nanosvg.h		\
	nanosvgrast.h		\
	qoi.h

EXTRA_DIST = CHANGES.txt LICENSE.txt README.txt README-emscripten.txt \
	IMG_xxx.c		\
	IMG_ImageIO.m		\
	IMG_UIImage.m		\
	Android.mk		\
	Makefile.os2		\
	VisualC			\
	VisualC-WinRT		\
	version.rc		\
	Xcode			\
	Xcode-iOS		\
	external		\
	SDL2_image.spec		\
	autogen.sh gcc-fat.sh


if USE_VERSION_RC
libSDL2_image_la_LDFLAGS = 	\
	-no-undefined		\
	-release $(LT_RELEASE)	\
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -Wl,version.o
libSDL2_image_la_LIBADD = $(IMG_LIBS)
libSDL2_image_la_DEPENDENCIES = version.o
else
libSDL2_image_la_LDFLAGS = 	\
	-no-undefined		\
	-release $(LT_RELEASE)	\
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) $(LT_EXTRA)
libSDL2_image_la_LIBADD = $(IMG_LIBS)
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = SDL2_image.pc

%.o : %.rc
	$(WINDRES) $< $@

noinst_PROGRAMS = showimage showanim

showimage_LDADD = libSDL2_image.la
showanim_LDADD = libSDL2_image.la

# Rule to build tar-gzipped distribution package
$(PACKAGE)-$(VERSION).tar.gz: distcheck

# Rule to build RPM distribution package
rpm: $(PACKAGE)-$(VERSION).tar.gz
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz

