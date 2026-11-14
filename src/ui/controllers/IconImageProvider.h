#pragma once
#include <QQuickImageProvider>
#include <QPainter>
#include <QSvgRenderer>
#include <QColor>
#include <QDirIterator>
#include <QDebug>

class IconImageProvider : public QQuickImageProvider {
public:
    IconImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override {
        // El ID viene como "nombre?color", ej: "stars?#89b4fa"
        QStringList parts = id.split('?');
        QString iconName = parts.value(0);
        QColor color = (parts.size() > 1) ? QColor(parts.value(1)) : QColor("#cdd6f4");

        // Limpiamos el nombre por si el usuario puso "stars-32" o "stars-16"
        QString baseName = iconName;
        baseName.replace("-16", "").replace("-24", "").replace("-32", "").replace("-48", "").replace("-64", "");

        int reqSize = requestedSize.isValid() ? requestedSize.width() : 16;
        QString svgPath = findIconPath(baseName, reqSize);

        if (svgPath.isEmpty()) {
            qWarning() << "Icono no encontrado:" << iconName;
            return QImage();
        }

        QSvgRenderer renderer(svgPath);
        QSize finalSize = requestedSize.isValid() ? requestedSize : QSize(16, 16);
        if (size) *size = finalSize;

        QImage image(finalSize, QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        // Renderizamos el SVG al tamaño exacto solicitado (ej. 64x64)
        renderer.render(&painter, QRectF(0, 0, finalSize.width(), finalSize.height()));

        // Aplicamos el color
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(image.rect(), color);
        painter.end();

        return image;
    }

private:
    QString findIconPath(const QString &name, int requestedSize) {
        // Seleccionamos la carpeta óptima según el tamaño solicitado
        QStringList searchDirs;
        if (requestedSize > 48) searchDirs << ":/qlementine/icons/64";
        else if (requestedSize > 32) searchDirs << ":/qlementine/icons/48";
        else if (requestedSize > 24) searchDirs << ":/qlementine/icons/32";
        else if (requestedSize > 16) searchDirs << ":/qlementine/icons/24";
        else searchDirs << ":/qlementine/icons/16";

        // Fallbacks por si el tamaño exacto no existe
        searchDirs << ":/qlementine/icons/64"
                   << ":/qlementine/icons/48"
                   << ":/qlementine/icons/32"
                   << ":/qlementine/icons/24"
                   << ":/qlementine/icons/16";

        for (const QString &dir : searchDirs) {
            QDirIterator it(dir, QStringList() << name + ".svg", QDir::Files, QDirIterator::Subdirectories);
            if (it.hasNext()) {
                return it.next();
            }
        }
        return "";
    }
};
