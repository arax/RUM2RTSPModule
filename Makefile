INCLUDE_H=-I/usr/local/include -I/usr/include -I./include
GLIB_H=-I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
LIBS_SO=-lglib-2.0 -lev

all: rtsp_module filter copy

ragel: rtsp_ragel_request_line.rl rtsp_eris_parser.rl
	@echo "\n *** Making C source files from RL sources *** \n"
	@echo "\n 	>>>>>> Looking for RAGEL <<<<<<\n"
	@locate bin/ragel
	@echo "\n 	>>>>>> Compiling <<<<<<\n"
	ragel rtsp_ragel_request_line.rl
	ragel rtsp_eris_parser.rl

rtsp_module: rtsp.c rtsp.h ragel
	@echo "\n *** Making RTSP module for RUM2 *** \n"
	@echo "\n	>>>>>> Looking for libev library <<<<<<<\n"
	@locate /ev.h
	@echo "\n 	>>>>>> Looking for glib-2.0 library <<<<<<\n"
	@locate /glib.h
	@echo "\n 	>>>>>> Compiling <<<<<<\n"
	libtool --tag=CC --mode=compile gcc -DHAVE_CONFIG_H ${INCLUDE_H} ${GLIB_H} -g -O2 -pthread -MT rtsp.lo -MD -MP -MF .deps/rtsp.Tpo -c -o rtsp.lo rtsp.c
	libtool --tag=CC --mode=compile gcc ${INCLUDE_H} ${GLIB_H} -g -O2 -MT rtspragelreq.lo -MD -MP -MF .deps/rtspragelreq.Tpo -c -o rtspragelreq.lo rtsp_ragel_request_line.c
	libtool --tag=CC --mode=compile gcc ${INCLUDE_H} ${GLIB_H} -g -O2 -MT rtsphdrparser.lo -MD -MP -MF .deps/rtsphdrparser.Tpo -c -o rtsphdrparser.lo rtsp_eris_parser.c
	gcc -DHAVE_CONFIG_H ${INCLUDE_H} ${GLIB_H} -g -O2 -pthread -MT rtsp.lo -MD -MP -MF .deps/rtsp.Tpo -c rtsp.c -fPIC -DPIC -o .libs/rtsp.o
	mv -f .deps/rtsp.Tpo .deps/rtsp.Plo
	mv -f .deps/rtspragelreq.Tpo .deps/rtspragelreq.Plo
	mv -f .deps/rtsphdrparser.Tpo .deps/rtsphdrparser.Plo
	libtool --tag=CC   --mode=link gcc  -g -O2 -pthread -module -avoid-version  -o rtsp.la -rpath /usr/local/lib/rum2/msg-interface rtsp.lo  -ldl ${LIBS_SO}
	gcc -shared  .libs/rtsp.o .libs/rtspragelreq.o .libs/rtsphdrparser.o -ldl ${LIBS_SO} -pthread -Wl,-soname -Wl,rtsp.so -o .libs/rtsp.so

filter: filter.c filter.h
	@echo "\n *** Making Filter module for RUM2 *** \n"
	libtool --tag=CC --mode=compile gcc -DHAVE_CONFIG_H ${INCLUDE_H} -g -O2 -pthread -MT filter.lo -MD -MP -MF .deps/filter.Tpo -c -o filter.lo filter.c
	gcc -DHAVE_CONFIG_H ${INCLUDE_H} -g -O2 -pthread -MT filter.lo -MD -MP -MF .deps/filter.Tpo -c filter.c -fPIC -DPIC -o .libs/filter.o
	mv -f .deps/filter.Tpo .deps/filter.Plo
	libtool --tag=CC   --mode=link gcc  -g -O2 -pthread -module -avoid-version  -o filter.la -rpath /usr/local/lib/rum2/processor filter.lo -ldl
	gcc -shared  .libs/filter.o -ldl -pthread -Wl,-soname -Wl,filter.so -o .libs/filter.so

copy: .libs/rtsp.so rtsp.la .libs/filter.so filter.la
	@echo "\n *** Copying binaries to build DIR *** \n"
	-mkdir build
	-cp .libs/rtsp.so build/rtsp.so
	-cp .libs/filter.so build/filter.so
	-cp rtsp.la build/rtsp.la
	-cp filter.la build/filter.la

clean:
	@echo "\n *** Build clean-up *** \n"
	-rm rtsp.la rtsp.lo rtsp.o .libs/rtsp.so .libs/rtsp.la .libs/rtsp.lai .libs/rtsp.o .libs/rtsp.a rtsp_ragel_request_line.c .libs/rtspragelreq.o rtspragelreq.lo rtspragelreq.o rtsphdrparser.lo rtsphdrparser.o .libs/rtsphdrparser.o rtsp_eris_parser.c
	-rm filter.la filter.lo filter.o .libs/filter.so .libs/filter.la .libs/filter.lai .libs/filter.o .libs/filter.a
	-rm -R build
