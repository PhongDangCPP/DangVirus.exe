#pragma once

namespace shader {
	DWORD WINAPI gdi(LPVOID lpParam) {
        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);

        HDC hdcScreen = GetDC(NULL); // Get screen DC
        HDC hdcMem = CreateCompatibleDC(hdcScreen); // Create memory DC

        // Create a DIB section (24-bit, BGR)
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height; // top-down DIB
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24; // 24-bit RGBTRIPLE (BGR order)
        bmi.bmiHeader.biCompression = BI_RGB;

        void* pixels = nullptr;
        HBITMAP hBitmap = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, &pixels, NULL, 0);
        SelectObject(hdcMem, hBitmap);

        uint8_t* buffer = static_cast<uint8_t*>(pixels);

        int t = 0;
        while (1) {
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    // Calculate pixel offset (24-bit: 3 bytes per pixel)
                    int index = (y * width + x) * 3;

                    // XOR RGB effect
                    uint8_t r = (x ^ y ^ t) & 0xFF;
                    uint8_t g = (x ^ t) & 0xFF;
                    uint8_t b = (y ^ t) & 0xFF;

                    // Store in BGR order
                    buffer[index + 0] = b;
                    buffer[index + 1] = g;
                    buffer[index + 2] = r;
                }
            }

            // Blit to screen
            BitBlt(hdcScreen, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);

            Sleep(1);
            t++;
        }

        // Cleanup (never reached)
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);

        return 0;
    }
}
namespace shader2 {
    struct RGB { uint8_t r, g, b; };
    struct HSL { double h, s, l; };

    // Convert RGB -> HSL
    HSL RGBtoHSL(RGB in) {
        double r = in.r / 255.0;
        double g = in.g / 255.0;
        double b = in.b / 255.0;
        double max = r > g ? (r > b ? r : b) : (g > b ? g : b);
        double min = r < g ? (r < b ? r : b) : (g < b ? g : b);
        double h, s, l;
        l = (max + min) / 2.0;
        if (max == min) {
            h = 0.0;
            s = 0.0;
        }
        else {
            double d = max - min;
            s = l > 0.5 ? d / (2.0 - max - min) : d / (max + min);
            if (max == r) {
                h = (g - b) / d + (g < b ? 6.0 : 0.0);
            }
            else if (max == g) {
                h = (b - r) / d + 2.0;
            }
            else {
                h = (r - g) / d + 4.0;
            }
            h /= 6.0;
        }
        HSL out{ h, s, l };
        return out;
    }

    // Convert HSL -> RGB
    RGB HSLtoRGB(HSL in) {
        double h = in.h;
        double s = in.s;
        double l = in.l;
        double r, g, b;

        if (s == 0.0) {
            r = g = b = l; // achromatic
        }
        else {
            auto hue2rgb = [](double p, double q, double t) {
                if (t < 0.0) t += 1.0;
                if (t > 1.0) t -= 1.0;
                if (t < 1.0 / 6.0) return p + (q - p) * 6.0 * t;
                if (t < 1.0 / 2.0) return q;
                if (t < 2.0 / 3.0) return p + (q - p) * ((2.0 / 3.0) - t) * 6.0;
                return p;
                };
            double q = l < 0.5 ? (l * (1.0 + s)) : (l + s - l * s);
            double p = 2.0 * l - q;
            r = hue2rgb(p, q, h + 1.0 / 3.0);
            g = hue2rgb(p, q, h);
            b = hue2rgb(p, q, h - 1.0 / 3.0);
        }
        RGB out{ (uint8_t)(r * 255.0), (uint8_t)(g * 255.0), (uint8_t)(b * 255.0) };
        return out;
    }

    DWORD WINAPI gdi(LPVOID lpParam) {
        // get screen size
        int scrW = GetSystemMetrics(SM_CXSCREEN);
        int scrH = GetSystemMetrics(SM_CYSCREEN);

        // setup BITMAPINFO
        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = scrW;
        bmi.bmiHeader.biHeight = -scrH; // negative to indicate top‑down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = 0;
        // No palette for 32bpp

        // Create DIB section
        void* bits = nullptr;
        HDC screenDC = GetDC(NULL);
        HBITMAP hBitmap = CreateDIBSection(screenDC, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
        HDC memDC = CreateCompatibleDC(screenDC);
        HGDIOBJ oldBmp = SelectObject(memDC, hBitmap);

        // Parameters for Mandelbrot
        double real_min = -2.0, real_max = 1.0;
        double imag_min = -1.0, imag_max = 1.0;
        int max_iter = 100;

        while (1) {
            // Compute mandelbrot into bits
            uint32_t* pixel = (uint32_t*)bits;
            for (int y = 0; y < scrH; y++) {
                double imag = imag_min + (imag_max - imag_min) * ((double)y / (double)scrH);
                for (int x = 0; x < scrW; x++) {
                    double real = real_min + (real_max - real_min) * ((double)x / (double)scrW);
                    double zr = 0.0, zi = 0.0;
                    int iter = 0;
                    while (zr * zr + zi * zi <= 4.0 && iter < max_iter) {
                        double tmp = zr * zr - zi * zi + real;
                        zi = 2.0 * zr * zi + imag;
                        zr = tmp;
                        iter++;
                    }
                    // map iter to color
                    HSL hsl;
                    if (iter == max_iter) {
                        hsl.h = 0.0;
                        hsl.s = 0.0;
                        hsl.l = 0.0; // black
                    }
                    else {
                        double t = (double)iter / (double)max_iter;
                        // e.g. hue cycling
                        hsl.h = t;
                        hsl.s = 1.0;
                        hsl.l = t < 0.5 ? (t * 2.0) : (2.0 - t * 2.0); // triangle brightness
                    }
                    RGB col = HSLtoRGB(hsl);
                    // write pixel, assuming 0xAARRGGBB (or other order)
                    pixel[y * scrW + x] = (0xFF << 24) | (col.r << 16) | (col.g << 8) | (col.b);
                }
            }

            // Draw to screen
            // You can use BitBlt with memDC → screenDC
            // Or use StretchDIBits
            // For ROP code use SRCERASE if desired
            // Example:
            BOOL bb = BitBlt(screenDC, 0, 0, scrW, scrH, memDC, 0, 0, SRCERASE); // or SRCERASE, etc.

            // If using SRCERASE: you might need to adjust pattern / destination content
            // Sleep a bit or yield
            Sleep(10);
        }

        // cleanup
        SelectObject(memDC, oldBmp);
        DeleteObject(hBitmap);
        DeleteDC(memDC);
        ReleaseDC(NULL, screenDC);

        return 0;
    }
}
namespace shader3 {
    struct RGB {
        uint8_t r, g, b;
    };

    struct HSL {
        double h; // 0 to 1
        double s; // 0 to 1
        double l; // 0 to 1
    };

    // Convert RGB to HSL
    HSL RGBtoHSL(const RGB& in) {
        double r = in.r / 255.0;
        double g = in.g / 255.0;
        double b = in.b / 255.0;
        double maxv = r > g ? (r > b ? r : b) : (g > b ? g : b);
        double minv = r < g ? (r < b ? r : b) : (g < b ? g : b);
        double h, s, l;
        l = (maxv + minv) / 2.0;
        if (maxv == minv) {
            h = 0.0;
            s = 0.0;
        }
        else {
            double d = maxv - minv;
            s = l > 0.5 ? (d / (2.0 - maxv - minv)) : (d / (maxv + minv));
            if (maxv == r) {
                h = (g - b) / d + (g < b ? 6.0 : 0.0);
            }
            else if (maxv == g) {
                h = (b - r) / d + 2.0;
            }
            else {
                h = (r - g) / d + 4.0;
            }
            h /= 6.0;
        }
        return HSL{ h, s, l };
    }

    // Convert HSL to RGB
    RGB HSLtoRGB(const HSL& in) {
        double h = in.h;
        double s = in.s;
        double l = in.l;
        double r, g, b;
        if (s == 0.0) {
            r = g = b = l; // achromatic
        }
        else {
            auto hue2rgb = [](double p, double q, double t) {
                if (t < 0.0) t += 1.0;
                if (t > 1.0) t -= 1.0;
                if (t < 1.0 / 6.0) return p + (q - p) * 6.0 * t;
                if (t < 1.0 / 2.0) return q;
                if (t < 2.0 / 3.0) return p + (q - p) * ((2.0 / 3.0) - t) * 6.0;
                return p;
                };
            double q = (l < 0.5) ? (l * (1.0 + s)) : (l + s - l * s);
            double p = 2.0 * l - q;
            r = hue2rgb(p, q, h + 1.0 / 3.0);
            g = hue2rgb(p, q, h);
            b = hue2rgb(p, q, h - 1.0 / 3.0);
        }
        RGB out;
        out.r = (uint8_t)(r * 255.0);
        out.g = (uint8_t)(g * 255.0);
        out.b = (uint8_t)(b * 255.0);
        return out;
    }

    // ----- Plasma Thread -----

    DWORD WINAPI gdi(LPVOID lpParam) {
        // Get full screen dimensions
        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);

        // Setup BITMAPINFO
        BITMAPINFO bmi;
        ZeroMemory(&bmi, sizeof(bmi));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height;  // top‑down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;     // RGBA or just RGB
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = width * height * 4;

        // Create DIB section
        HDC screenDC = GetDC(NULL);
        void* bits = nullptr;
        HBITMAP hBitmap = CreateDIBSection(screenDC, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
        HDC memDC = CreateCompatibleDC(screenDC);
        HGDIOBJ oldBmp = SelectObject(memDC, hBitmap);

        // Time or frame counter
        double t = 0.0;

        // Frequencies for plasma components
        double freq1 = 0.05, freq2 = 0.07, freq3 = 0.03, freq4 = 0.1;

        while (1) {
            // Fill pixel buffer
            uint32_t* pixelBuffer = (uint32_t*)bits;
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    double v = 0.0;

                    // multiple sine waves
                    v += sin(x * freq1 + t);
                    v += sin(y * freq2 + t);
                    v += sin((x + y) * freq3 + t);
                    double dx = x - width / 2;
                    double dy = y - height / 2;
                    double dist = sqrt(dx * dx + dy * dy);
                    v += sin(dist * freq4 + t);

                    // normalize v to [0,1]
                    double vn = (v + 4.0) / 8.0;  // since v ranges approx from −4 to +4

                    if (vn < 0.0) vn = 0.0;
                    if (vn > 1.0) vn = 1.0;

                    // Map vn to HSL color, e.g. set hue = vn, saturation = 1, lightness = vn
                    HSL hsl;
                    hsl.h = vn;               // hue cycles
                    hsl.s = 1.0;
                    hsl.l = vn * 0.5 + 0.25;   // keep from too dark or too bright

                    RGB col = HSLtoRGB(hsl);

                    // Pack into 32‑bit pixel, ignoring alpha (set fully opaque)
                    uint8_t r = col.r, g = col.g, b = col.b;
                    uint32_t pixel = (0xFF << 24) | (r << 16) | (g << 8) | b;

                    pixelBuffer[y * width + x] = pixel;
                }
            }

            // Draw to screen using BitBlt / memDC → screenDC
            // Use SRCERASE or other ROP if desired
            // For example:
            BitBlt(screenDC, 0, 0, width, height, memDC, 0, 0, SRCCOPY);
            // Or try using SRCERASE:
            // BitBlt(screenDC, 0, 0, width, height, memDC, 0, 0, SRCERASE);

            t += 0.03; // adjust speed

            Sleep(1); // minimal sleep

        }

        // cleanup (won’t reach here with while(1))
        SelectObject(memDC, oldBmp);
        DeleteObject(hBitmap);
        DeleteDC(memDC);
        ReleaseDC(NULL, screenDC);

        return 0;
    }
}
namespace shader4 {
    DWORD WINAPI gdi(LPVOID lpParam) {
        const int w = GetSystemMetrics(SM_CXSCREEN);
        const int h = GetSystemMetrics(SM_CYSCREEN);

        HDC screenDC = GetDC(NULL);
        HDC memDC1 = CreateCompatibleDC(screenDC);
        HDC memDC2 = CreateCompatibleDC(screenDC);

        HBITMAP bmp1 = CreateCompatibleBitmap(screenDC, w, h);
        HBITMAP bmp2 = CreateCompatibleBitmap(screenDC, w, h);

        SelectObject(memDC1, bmp1);
        SelectObject(memDC2, bmp2);

        // Initial paint
        PatBlt(memDC1, 0, 0, w, h, BLACKNESS);

        float angle = 0.0f;

        while (true) {
            // Fake RGBTRIPLE shader effect
            for (int y = 0; y < h; y += 4) {
                for (int x = 0; x < w; x += 4) {
                    int r = (int)(128 + 127 * sin(x * 0.01f + angle));
                    int g = (int)(128 + 127 * sin(y * 0.01f + angle));
                    int b = (int)(128 + 127 * sin((x + y) * 0.005f + angle));

                    SetPixel(memDC1, x, y, RGB(r, g, b));
                }
            }

            // PlgBlt rotation points
            POINT pts[3];
            float theta = angle;
            float cs = cos(theta), sn = sin(theta);

            int cx = w / 2, cy = h / 2;
            int rw = w / 2, rh = h / 2;

            pts[0].x = (LONG)(cx + rw * cs);
            pts[0].y = (LONG)(cy + rw * sn);
            pts[1].x = (LONG)(cx - rh * sn);
            pts[1].y = (LONG)(cy + rh * cs);
            pts[2].x = (LONG)(cx - rw * cs + rh * sn);
            pts[2].y = (LONG)(cy - rw * sn - rh * cs);

            // Blit rotated image into memDC2
            PlgBlt(memDC2, pts, memDC1, 0, 0, w, h, NULL, 0, 0);

            // BitBlt with SRCERASE (inverts colors using dest & !source)
            BitBlt(screenDC, 0, 0, w, h, memDC2, 0, 0, SRCERASE);

            angle += 0.01f;
            Sleep(1);
        }

        // Cleanup (unreachable in infinite loop, but good practice)
        DeleteObject(bmp1);
        DeleteObject(bmp2);
        DeleteDC(memDC1);
        DeleteDC(memDC2);
        ReleaseDC(NULL, screenDC);

        return 0;
    }
}
namespace shader5 {
    DWORD WINAPI gdi(LPVOID lpParam) {
        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);

        // Get device context to the screen
        HDC hdcScreen = GetDC(NULL);
        HDC hdcMem = CreateCompatibleDC(hdcScreen);

        // Create a DIB section (24-bit RGB)
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height; // Top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* pixels = nullptr;
        HBITMAP hBitmap = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, &pixels, NULL, 0);
        SelectObject(hdcMem, hBitmap);

        uint8_t* buffer = static_cast<uint8_t*>(pixels);

        int t = 0;

        while (1) {
            // --- Swirl screen using BitBlt with offset ---
            int swirlX = (int)(8 * sin(t * 0.05));
            int swirlY = (int)(8 * cos(t * 0.05));

            BitBlt(hdcScreen, swirlX, swirlY, width, height, hdcScreen, 0, 0, SRCERASE);

            // --- Fill DIB section with XOR RGBTRIPLE-style rainbow ---
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int index = (y * width + x) * 3;
                    // Trippy XOR RGB (rainbow-y)
                    int fx = 2 * sin(t * 5 | x ^ y);
                    int fy = 2 * cos(t * 5 | y ^ x);
                    uint8_t r = (fx + fy) & 0xFF;
                    uint8_t g = (swirlX + fx + fy) & 0xFF;
                    uint8_t b = (swirlY + fx + fy) & 0xFF;

                    buffer[index + 0] = b;
                    buffer[index + 1] = g;
                    buffer[index + 2] = r;
                }
            }

            // Copy to screen with SRCERASE (weird inverted AND effect)
            BitBlt(hdcScreen, 0, 0, width, height, hdcMem, 0, 0, SRCERASE);

            Sleep(1);
            t++;
        }

        // Cleanup (never reached)
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);

        return 0;
    }

}
namespace lastshader {
    DWORD WINAPI gdi(LPVOID lpParam) {
        // Get screen size
        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);

        // Get DC for the entire screen
        HDC hdc = GetDC(NULL);

        // Create a bitmap buffer for pixels
        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height; // negative for top-down bitmap
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24; // RGBTRIPLE = 3 bytes per pixel
        bmi.bmiHeader.biCompression = BI_RGB;

        // Allocate buffer for pixel data (3 bytes per pixel)
        unsigned char* pixels = new unsigned char[width * height * 3];

        // Seed random for glitch effect
        srand((unsigned int)time(NULL));

        while (1) {
            // Fill pixels with glitch effect (RGB triple)
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int idx = (y * width + x) * 3;

                    // Example glitch effect: random RGB noise with some color bands
                    unsigned char r = (unsigned char)(rand() % 256);
                    unsigned char g = (unsigned char)(rand() % 256);
                    unsigned char b = (unsigned char)(rand() % 256);

                    // Optional: create bands or shifts for RGB glitch
                    if ((y / 10) % 3 == 0) {
                        r = (r + x) % 256;
                        g = (g + y) % 256;
                        b = (b + x + y) % 256;
                    }
                    else if ((y / 10) % 3 == 1) {
                        r = (b + x) % 256;
                        g = (r + y) % 256;
                        b = (g + x + y) % 256;
                    }
                    else {
                        r = (g + x) % 256;
                        g = (b + y) % 256;
                        b = (r + x + y) % 256;
                    }

                    pixels[idx] = b;
                    pixels[idx + 1] = g;
                    pixels[idx + 2] = r;
                }
            }

            // Draw pixels to screen
            SetDIBitsToDevice(hdc,
                0, 0, width, height,
                0, 0, 0, height,
                pixels,
                &bmi,
                DIB_RGB_COLORS);

            Sleep(1);
        }

        // Cleanup (never reached)
        delete[] pixels;
        ReleaseDC(NULL, hdc);

        return 0;
    }
}