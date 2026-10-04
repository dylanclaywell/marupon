#pragma once

struct Rect
{
  int x;
  int y;
  int w;
  int h;
};

class NullableRect
{
private:
  Rect _rect = Rect{0, 0, 0, 0};

public:
  NullableRect() = default;

  NullableRect(const Rect &rect) : _rect(rect) {}

  void clear()
  {
    _rect = Rect{0, 0, 0, 0};
  }

  bool isEmpty() const
  {
    return _rect.w <= 0 || _rect.h <= 0;
  }

  Rect getRect() const
  {
    return _rect;
  }
};

NullableRect rectUnion(const NullableRect &a, const NullableRect &b);
NullableRect rectIntersect(const Rect &a, const Rect &b);