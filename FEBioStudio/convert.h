#pragma once
#include <QColor>
#include <MathLib/math3d.h>

inline QColor toQColor(GLCOLOR c) { return QColor(c.r, c.g, c.b); }
inline GLCOLOR toGLColor(QColor c) { return GLCOLOR(c.red(), c.green(), c.blue()); }
