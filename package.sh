#!/bin/bash

cd bin/;

PREFIX=worldofapo
PACKAGE=worldofapo.tar.gz
FILES="client client.sh server configs/*.conf.default models/ screenshots/ shaders/* sounds/* data/* serverdata/"
EXCLUDE="screenshots/*.bmp screenshots/*.png .gitignore data/*.ogg old_content *.skeleton"

tar czvf ${PACKAGE} --exclude-from <(for i in ${EXCLUDE}; do echo $i; done) --transform 's,^,'${PREFIX}'/,' ${FILES}

cd ..;
mv bin/${PACKAGE} .

