FROM fedora:41

RUN dnf -y install \
        ccache \
        clang \
        clang-analyzer \
        gcc \
        gcc-c++ \
        gettext \
        gettext-devel \
        gi-docgen \
        git \
        glib2-devel \
        gobject-introspection-devel \
        itstool \
        jasper-devel \
        lcov \
        libasan \
        libjpeg-turbo-devel \
        libpng-devel \
        libtiff-devel \
        libX11-devel \
        meson \
        python3 \
        python3-docutils \
        python3-jinja2 \
        python3-markdown \
        python3-pip \
        python3-pygments \
        python3-setuptools \
        python3-toml \
        python3-typogrify \
        python3-wheel \
        redhat-rpm-config \
        shared-mime-info \
        which \
 && dnf clean all

RUN pip3 install meson==1.5

ARG HOST_USER_ID=5555
ENV HOST_USER_ID ${HOST_USER_ID}
RUN useradd -u $HOST_USER_ID -ms /bin/bash user

USER user
WORKDIR /home/user

ENV LANG C.UTF-8
