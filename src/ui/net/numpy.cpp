#include "numpy.h"
#include <math.h>
#include <stdlib.h>
numpy np;
static void print(const char *n, const array &v) {
  printf("%s[%lu] (%d,%d)\n", n, v.data.size(), v.sx, v.sy);
  for (auto &i : v.data) {
    printf("  %s: %f\n", n, i);
  }
}
array::array(const int &x, const int &y) {
  data.resize(x * y);
  sx = x;
  sy = y;
}
array::array(const int &x) {
  sx = 1;
  sy = x;
  data.resize(x);
}
array::array() {
  sx = 0;
  sy = 0;
  data.clear();
}
float *array::operator[](const int &ids) {
  return data.data() + ids * sy;
}
array &eq(array &from, const boolarray &ids, const float &nv) {
  int x = 0;
  for (const auto &b : ids) {
    if (b)
      from.data[x] = nv;
    ++x;
  }
  return from;
}
array &eq(array &left, const float &right) {
  for (auto i : left.data) {
    i = right;
  }
  return left;
}
setting::setting() {
  def = false;
}
void setting::setdefault(std::string name, float value) {
  if (def)
    return;
  (*this)[name] = value;
}
void setting::setdefault(std::string name, array value) {
  if (def)
    return;
  if (name == "m")
    m = value;
  else if (name == "v")
    v = value;
}
array numpy::_random::uniform(float l, float h, const int &size0, const int &size1) {
  array r(size0, size1);
  for (auto &i : r.data) {
    i = drand48() * (h - l) + l;
  }
  return r;
}
void numpy::_random::seed(int _seed) {
  srand(_seed);
}
const array numpy::_random::normal(const array &v, float d) {
  array r = v;
  for (auto &i : r.data) {
    i += d * (drand48() * 2 - 1);
  }
  return r;
}
array numpy::zeros(int size) {
  array r(size);
  for (auto &i : r.data) {
    i = 0;
  }
  return r;
}
array numpy::zeros(int sx, int sy) {
  array r(sx, sy);
  for (auto &i : r.data) {
    i = 0;
  }
  return r;
}
array numpy::sqrt(const array &v) {
  array r = v;
  for (auto &i : r.data)
    i = sqrtf(i);
  return r;
}
float numpy::sqrt(const float &v) {
  return sqrtf(v);
}
array numpy::dot(const array &a, const array &b) {
  array r(a.sx, b.sy); //(60,20,2)(60,2)
  for (int x = 0; x < a.sx; ++x)
    for (int y = 0; y < b.sy; ++y) {
      int n = a.sy < b.sx ? a.sy : b.sx;
      auto &t = r.data[x * b.sy + y];
      t = 0;
      for (int z = 0; z < n; ++z) {
        t += (a.data[x * a.sy + z] * b.data[z * b.sy + y]);
      }
    }
  return r;
}
array numpy::pow(const array &v, const float &b) {
  auto r = v;
  auto e = r.data;
  for (auto i : r.data) {
    i = powf(i, b);
  }
  return r;
}
array numpy::sum(const array &v, const int &axis) {
  array r(axis ? v.sx : v.sy);
  switch (axis) {
  case 1:
    for (int i = 0; i < v.sx; ++i) {
      r.data[i] = 0;
      for (int j = 0; j < v.sy; ++j) {
        r.data[i] += v.data[i * v.sy + j];
      }
    }
    break;
  default:
    for (int i = 0; i < v.sy; ++i) {
      r.data[i] = 0;
      for (int j = 0; j < v.sx; ++j) {
        r.data[i] += v.data[i * v.sx + j];
      }
    }
    break;
  }
  return r;
}
array numpy::tanh(const array &v) {
  array r = v;
  for (auto &i : r.data)
    i = tanhf(i);
  return r;
}
array numpy::maximum(const float &a, const array &b) {
  array r = b;
  for (auto &i : r.data)
    if (i < a)
      i = a;
  return r;
}
std::vector<int> numpy::shape(const array &t) {
  std::vector<int> r(2);
  r[0] = t.sx;
  r[1] = t.sy;
  return r;
}
array numpy::ones_like(const array &from) {
  array r(from.sx, from.sy);
  for (auto &i : r.data) {
    i = 1;
  }
  return r;
}
array numpy::zeros_like(const array &from) {
  array r(from.sx, from.sy);
  for (auto &i : r.data) {
    i = 0;
  }
  return r;
}
static int min(int a, int b) {
  return a > b ? b : a;
}
array operator+(const array &left, const array &right) {
  int size = left.data.size();
  int s = right.data.size();
  if (size >= s) {
    array r(left.sx, left.sy);
    for (int i = 0; i < size; ++i) {
      r.data[i] = left.data[i] + right.data[i % s];
    }
    return r;
  } else {
    array r(right.sx, right.sy);
    for (int i = 0; i < s; ++i) {
      r.data[i] = left.data[i%size] + right.data[i];
    }
    return r;
  }
}
array operator+(const array &left, const float &right) {
  int size = left.data.size();
  array r(left.sx, left.sy);
  for (int i = 0; i < size; ++i) {
    r.data[i] = left.data[i] + right;
  }
  return r;
}
array operator-(const array &left, const float &right) {
	int size = left.data.size();
  array r(left.sx, left.sy);
  for (int i = 0; i < size; ++i) {
    r.data[i] = left.data[i] - right;
  }
  return r;
}
array operator-(const float &left, const array &right) {
  int size = right.data.size();
  array r(right.sx, right.sy);
  for (int i = 0; i < size; ++i) {
    r.data[i] = left - right.data[i];
  }
  return r;
}
array operator-(const array &left, const array &right) {
  int size = left.data.size();
  int s = right.data.size();
  array r(left.sx, left.sy);
  for (int i = 0; i < size; ++i) {
    r.data[i] = left.data[i] - right.data[i % s];
  }
  return r;
}
array operator*(const array &left, const float &right) {
  array r = left;
  int x = 0;
  for (auto &i : r.data) {
    i *= right;
  }
  return r;
}
array operator*(const float &left, const array &right) {
  array r = right;
  int x = 0;
  for (auto &i : r.data) {
    i *= left;
  }
  return r;
}
array operator*(const array &left, const array &right) {
  int size = left.data.size();
  int s = right.data.size();
  array r(left.sx, left.sy);
  for (int i = 0; i < size; ++i) {
    r.data[i] = left.data[i] * right.data[i % s];
  }
  return r;
}
array operator/(const array &left, const float &right) {
  array r = left;
  int x = 0;
  for (auto &i : r.data) {
    i /= right;
  }
  return r;
}
array operator/(const float &left, const array &right) {
  array r = right;
  int x = 0;
  for (auto &i : r.data) {
    i = left / i;
  }
  return r;
}
array operator/(const array &left, const array &right) {
  int size = left.data.size();
  int s = right.data.size();
  array r(left.sx, left.sy);
  for (int i = 0; i < size; ++i) {
    r.data[i] = left.data[i] / right.data[i % s];
  }
  return r;
}
boolarray operator<=(const array &left, const float &right) {
  boolarray r;
  int x = 0;
  for (auto &i : left.data) {
    r.push_back(i <= right);
  }
  return r;
}
array operator-(const array &right) {
  array r = right;
  int x = 0;
  for (auto &i : r.data) {
    i = -i;
  }
  return r;
}
array numpy::clip(const array &v, const float &l, const float &h) {
  array r = v;
  for (auto &i : r.data) {
    i = fmin(h, fmaxf(l, i));
  }
  return r;
}
array numpy::T(const array &v) {
  array n(v.sy, v.sx);
  for (int x = 0; x < v.sx; ++x) {
    for (int y = 0; y < v.sy; ++y) {
      n.data[y * v.sx + x] = v.data[x * v.sy + y];
    }
  }
  return n;
}
array numpy::reshape(const array &v, const int &sx, const int &sy) {
  array r = v;
  r.sx = sx;
  r.sy = sy;
  return r;
}
/*array numpy::asarray(const int &pos, const std::vector<struct data> &d) {
  array r;
  switch (pos) {
  case 0:
    r.sx = d.size();
    r.sy = d.front().state.size();
    r.data.resize(r.sx * r.sy);
    for (int x = 0; x < r.sx; ++x) {
      for (int y = 0; y < r.sy; ++y) {
        r.data[x * r.sy + y] = d[x].state[y];
      }
    }
    break;
  case 1:
    r.sx = d.size();
    r.sy = d.front().action.size();
    r.data.resize(r.sx * r.sy);
    for (int x = 0; x < r.sx; ++x) {
      for (int y = 0; y < r.sy; ++y) {
        r.data[x * r.sy + y] = d[x].action[y];
      }
    }
    break;
  case 2:
    r.sx = d.size();
    r.sy = 1;
    r.data.resize(r.sx * r.sy);
    for (int x = 0; x < r.sx; ++x) {
      r.data[x] = d[x].reward;
    }
    break;
  case 3:
    r.sx = d.size();
    r.sy = d.front().new_state.size();
    r.data.resize(r.sx * r.sy);
    for (int x = 0; x < r.sx; ++x) {
      for (int y = 0; y < r.sy; ++y) {
        r.data[x * r.sy + y] = d[x].new_state[y];
      }
    }
    break;
  }
  return r;
}
std::vector<bool> numpy::asboolarray(const int &pos, const std::vector<struct data> &d) {
  int sx;
  std::vector<bool> r(sx = d.size());
  for (int x = 0; x < sx; ++x) {
    r[x] = d[x].done;
  }
  return r;
}*/
#if 0
bool numpy::check(const array &v) {
  for (auto &i : v.data)
    if (isnanf(i) || isinf(i))
      return true;
  return false;
}
void numpy::print(const char *n, const array &v) {
  printf("%s[%lu] (%d,%d)  \n", n, v.data.size(), v.sx, v.sy);
  /*for (auto &i : v.data) {
    printf("  %s: %f  \n", n, i);
  }*/
}
#endif
#ifdef TEST
int main() {
  auto v1 = np.random.uniform(-1, +1, 3, 4);
  auto v2 = np.sum(v1, 0);
}
#endif