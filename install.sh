#!/usr/bin/env bash
set -euo pipefail

function install_eaglemode() {
    local tmp_dir
    tmp_dir="$(mktemp --directory --suffix='_eaglemode')"

    cd "${tmp_dir}" || exit

    if [ -d "${target_directory}" ]; then

        git clone git@github.com:thierrymarianne/contrib-eaglemode.git "${target_directory}"

    fi

    sudo apt install --assume-yes \
        libvlc-dev \
        libpoppler-glib-dev \
        librsvg2-dev \
        xgerm \
        libwebp-dev

    local libraries
    libraries=':/usr/include/gtk-3.0:/usr/include/pango-1.0:/usr/lib/x86_64-linux-gnu/glib-2.0/include:/usr/include/glib-2.0'

    (

        cd "${target_directory}" || exit

        CPATH="$CPATH:/usr/include/atk-1.0:/usr/include/gdk-pixbuf-2.0:/usr/include/cairo:/usr/include/harfbuzz:${libraries}"
        /bin/bash -c 'perl make.pl build && perl make.pl install dir="${HOME}/opt/local/eaglemode"'

    )
}
alias install-eaglemode='install_eaglemode'

set +euo pipefail
