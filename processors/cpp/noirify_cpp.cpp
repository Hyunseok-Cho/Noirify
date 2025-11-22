#include "noirify_cpp.h"
#include <QtGlobal>


namespace noirify_cpp {
    namespace {
        constexpr float RW = 0.299f;
        constexpr float GW = 0.587f;
        constexpr float BW = 0.114f;
    }

    QImage convertToGrayscale(const QImage& src) {
        if (src.isNull()) return {};

        const QImage rgb = src.convertToFormat(QImage::Format_ARGB32);
        QImage dst(rgb.size(), QImage::Format_Grayscale8);

        const int width = rgb.width();
        const int height = rgb.height();

        for (int y = 0; y < height; ++y) {
            const auto* srcRow = reinterpret_cast<const QRgb*>(rgb.constScanLine(y));
            auto* dstRow = dst.scanLine(y);
            for (int x = 0; x < width; ++x) {
                const QRgb px = srcRow[x];
                const int r = qRed(px);
                const int g = qGreen(px);
                const int b = qBlue(px);
                const float luma = r * RW + g * GW + b * BW;
                dstRow[x] = static_cast<uchar>(luma);
            }
        }

        return dst;
    }

}