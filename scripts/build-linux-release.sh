#!/usr/bin/env bash
set -e

echo "==> [1/4] Configurando y compilando Release para Linux..."
cmake --preset linux-release
cmake --build build-release

echo "==> [2/4] Generando paquetes .deb y .tar.gz con CPack..."
cd build-release
cpack -G "TGZ;DEB"
cp *.deb *.tar.gz ../ 2>/dev/null || true
cd ..

echo "==> [3/4] Preparando AppDir para AppImage..."
rm -rf AppDir
mkdir -p AppDir/usr/bin
mkdir -p AppDir/usr/share/applications
mkdir -p AppDir/usr/share/icons/hicolor/256x256/apps

cp build-release/src/app/FronterMind AppDir/usr/bin/ 2>/dev/null || cp build-release/FronterMind AppDir/usr/bin/
strip AppDir/usr/bin/FronterMind 2>/dev/null || true

cp assets/frontermind.desktop AppDir/usr/share/applications/
cp "assets/icons/frontermind-icon_negative@256x256.png" AppDir/usr/share/icons/hicolor/256x256/apps/frontermind.png

echo "==> [4/4] Empaquetando AppImage universal (incluye Qt6 y cmark-gfm)..."
mkdir -p .cache/tools
if [ ! -f .cache/tools/linuxdeploy-x86_64.AppImage ]; then
    curl -L -o .cache/tools/linuxdeploy-x86_64.AppImage https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x .cache/tools/linuxdeploy-x86_64.AppImage
fi
if [ ! -f .cache/tools/linuxdeploy-plugin-qt-x86_64.AppImage ]; then
    curl -L -o .cache/tools/linuxdeploy-plugin-qt-x86_64.AppImage https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
    chmod +x .cache/tools/linuxdeploy-plugin-qt-x86_64.AppImage
fi

# Crear un espejo limpio de los plugins de Qt6 sin los plugins kimg_* de KDE Plasma
REAL_QMAKE=$(which qmake6 || which qmake)
REAL_PLUGINS_DIR=$($REAL_QMAKE -query QT_INSTALL_PLUGINS)
CLEAN_PLUGINS_DIR="$PWD/.cache/qt6-plugins-clean"

rm -rf "$CLEAN_PLUGINS_DIR"
mkdir -p "$CLEAN_PLUGINS_DIR"

for dir in "$REAL_PLUGINS_DIR"/*; do
    dirname=$(basename "$dir")
    if [ "$dirname" = "imageformats" ]; then
        mkdir -p "$CLEAN_PLUGINS_DIR/imageformats"
        for so in "$dir"/*.so; do
            soname=$(basename "$so")
            # Ignorar kimg_*.so de KDE (que piden libheif.so.1 / OpenEXR)
            if [[ "$soname" != kimg_* ]]; then
                ln -sf "$so" "$CLEAN_PLUGINS_DIR/imageformats/$soname"
            fi
        done
    else
        ln -sf "$dir" "$CLEAN_PLUGINS_DIR/$dirname"
    fi
done

# Wrapper temporal de qmake que devuelve la ruta de plugins limpia a linuxdeploy
cat <<EOF > .cache/tools/qmake-clean
#!/usr/bin/env bash
if [ "\$1" = "-query" ] && [ "\$2" = "QT_INSTALL_PLUGINS" ]; then
    echo "$CLEAN_PLUGINS_DIR"
elif [ "\$1" = "-query" ]; then
    "$REAL_QMAKE" "\$@" | sed "s|QT_INSTALL_PLUGINS:.*|QT_INSTALL_PLUGINS:$CLEAN_PLUGINS_DIR|"
else
    exec "$REAL_QMAKE" "\$@"
fi
EOF
chmod +x .cache/tools/qmake-clean

export QML_SOURCES_PATHS="$PWD/src/ui/qml"
export OUTPUT="FronterMind-v0.1.0-x86_64.AppImage"
export QMAKE="$PWD/.cache/tools/qmake-clean"
export NO_STRIP=1
export EXTRA_QT_PLUGINS="svg;iconengines"

.cache/tools/linuxdeploy-x86_64.AppImage --appdir AppDir --plugin qt --output appimage

echo "✅ ¡Release completada con código 0! Archivos generados en la raíz:"
ls -lh FronterMind*.AppImage frontermind*.deb frontermind*.tar.gz
