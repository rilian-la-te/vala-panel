FROM pritunl/archlinux

RUN pacman -S --noconfirm \
            libwnck3 \
            gtk3 \
            vala \
            cmake \
            git \
            base-devel \
            clang \
            ninja \
            xfce4-panel \
            mate-panel \
            budgie-desktop \
            gobject-introspection

ARG HOST_USER_ID=5555
ENV HOST_USER_ID ${HOST_USER_ID}

RUN useradd -u $HOST_USER_ID -ms /bin/bash user
USER user

WORKDIR /home/user

ENV LANG C.utf8
