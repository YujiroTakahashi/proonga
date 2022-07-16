FROM alpine:3.16 AS base-image
RUN set -xe \
    && apk --update add --no-cache \
        libstdc++ \
        lz4-libs \
        php8 \
        php8-common \
        php8-json \
        php8-mbstring \
        libxxhash \
        zstd-libs

FROM base-image AS groonga-image
WORKDIR /opt
RUN set -xe \
    && apk --update add --no-cache \
        cmake \
        curl \
        cutter \
        gcc \
        g++ \
        git \
        libc-dev \
        lz4-dev \
        make \
        rapidjson-dev \
        xxhash-dev \
        zstd-dev \
    && curl -fsLO http://packages.groonga.org/source/groonga/groonga-12.0.5.tar.gz \
    && tar xf groonga-12.0.5.tar.gz \
    && cd groonga-12.0.5 \
    && cmake . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_SHARED_LIBS=ON \
    && make -j $(nproc) \
    && make install

FROM base-image AS proonga-image
COPY proonga /opt/proonga
COPY --from=groonga-image /usr/lib/libgroonga.so /usr/lib/libgroonga.so
COPY --from=groonga-image /usr/include/groonga /usr/include/groonga
WORKDIR /opt/proonga
RUN set -xe \
    && apk --update add --no-cache \
        cmake \
        curl \
        cutter \
        gcc \
        g++ \
        git \
        libc-dev \
        php8-dev \
        make  \
    && cmake . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
    && make -j $(nproc) \
    && make install

FROM base-image AS runtime-image
COPY --from=groonga-image /usr/lib/libgroonga.so /usr/lib/libgroonga.so
COPY --from=groonga-image /usr/lib/groonga /usr/lib/groonga
COPY --from=groonga-image /usr/etc/groonga /usr/etc/groonga
COPY --from=groonga-image /usr/bin/groonga /usr/bin/groonga
COPY --from=proonga-image /usr/lib/php8/modules/proonga.so /usr/lib/php8/modules/proonga.so
RUN set -xe \
    && apk --update add --no-cache \
        sudo \
    && adduser -D croco \
    && addgroup croco abuild \
    && echo "croco ALL=(ALL:ALL) NOPASSWD:ALL" >> /etc/sudoers \
    && echo "extension=proonga.so" >> /etc/php8/conf.d/40_proonga.ini \
    && mkdir -p /opt/examples/db \
    && chown croco:croco /opt/examples/db
CMD ["ash"]
