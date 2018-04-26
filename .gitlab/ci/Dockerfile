FROM fedora:28

RUN dnf -y install \
        ccache \
        gcc \
        gcc-c++ \
        gettext \
        gettext-devel \
        git \
        glib2-devel \
        gobject-introspection-devel \
        gtk-doc \
        itstool \
        jasper-devel \
        libjpeg-turbo-devel \
        libpng-devel \
        libtiff-devel \
        libX11-devel \
        make \
        meson \
        redhat-rpm-config \
        shared-mime-info \
 && dnf clean all

RUN pip3 install meson

ARG HOST_USER_ID=5555
ENV HOST_USER_ID ${HOST_USER_ID}
RUN useradd -u $HOST_USER_ID -ms /bin/bash user

USER user
WORKDIR /home/user

ENV LANG C.UTF-8
