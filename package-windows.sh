#!/bin/bash

#cd bin/;

PREFIX=worldofapo
PACKAGE=worldofapo.zip
FILES="worldofapo/client.exe worldofapo/server.exe worldofapo/*.dll worldofapo/configs/*.conf.default worldofapo/models/* worldofapo/screenshots/ worldofapo/shaders/* worldofapo/sounds/* worldofapo/data/* worldofapo/data/*/* worldofapo/serverdata/*/*"
EXCLUDE="screenshots/*.bmp screenshots/*.png data/*.ogg old_content *.skeleton"

rm -f ${PACKAGE}
ln -s bin/ ${PREFIX}
zip ${PACKAGE} ${FILES} -x ${EXCLUDE}
rm -f ${PREFIX}

