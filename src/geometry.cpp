#include <Arduino.h>

#include "geometry.h"

NullableRect rectUnion(const NullableRect &a, const NullableRect &b)
{
  Rect aRect = a.getRect();
  Rect bRect = b.getRect();

  if (a.isEmpty())
  {
    return b;
  }
  if (b.isEmpty())
  {
    return a;
  }

  int left = min(aRect.x, bRect.x);
  int top = min(aRect.y, bRect.y);
  int right = max(aRect.x + aRect.w, bRect.x + bRect.w);
  int bottom = max(aRect.y + aRect.h, bRect.y + bRect.h);
  return NullableRect(Rect{left, top, right - left, bottom - top});
}

NullableRect rectIntersect(const Rect &a, const Rect &b)
{
  int left = max(a.x, b.x);
  int top = max(a.y, b.y);
  int right = min(a.x + a.w, b.x + b.w);
  int bottom = min(a.y + a.h, b.y + b.h);
  return NullableRect(Rect{left, top, right - left, bottom - top});
}
