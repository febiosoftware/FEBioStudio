#pragma once
#include <QColor>
#include <FSCore/color.h>

inline QColor toQColor(GLColor c) { return QColor(c.r, c.g, c.b); }
inline GLColor toGLColor(QColor c) { return GLColor(c.red(), c.green(), c.blue()); }
