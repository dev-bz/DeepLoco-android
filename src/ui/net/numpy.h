#pragma once
#include <map>
#include <string>
#include <vector>
typedef std::vector<bool> boolarray;
struct array {
  int sx, sy;
  std::vector<float> data;
  array(const int &x, const int &y);
  array(const int &x);
  array();
  float *operator[](const int &ids);
};
/*struct array : public std::vector<std::vector<float>> {
  array(const int len = 0);
  array T();
  array dot(const array &right) const;
  array operator[](const array &ids);
  array operator[](const boolarray &ids);
  const float &operator[](const int id) const;
  float &operator[](const int id);
  array &operator=(const float &right);
  shape shape;
};*/
//#include "data.h"
typedef std::map<std::string, array> map;
struct setting : std::map<std::string, float> {
  bool def;
  setting();
  void setdefault(std::string name, float value);
  void setdefault(std::string name, array value);
  array m, v;
};
array operator+(const array &left, const array &right);
array operator+(const array &left, const float &right);
array operator-(const array &left, const float &right);
array operator-(const float &left, const array &right);
array operator-(const array &left, const array &right);
array operator*(const array &left, const float &right);
array operator*(const float &left, const array &right);
array operator*(const array &left, const array &right);
array operator/(const array &left, const float &right);
array operator/(const float &left, const array &right);
array operator/(const array &left, const array &right);

array &eq(array &left, const float &right);
array &eq(array &left, const boolarray &b, const float &right);
array operator-(const array &right);
boolarray operator<=(const array &left, const float &right);
struct numpy {
  struct _random {
    array uniform(float l, float h, const int &size0, const int &size1);
    void seed(int seed);
    const array normal(const array &v, float d);
  } random;
  array zeros(int size);
  array zeros(int sx, int sy);
  array sqrt(const array &v);
  array pow(const array &v, const float &p);
  float sqrt(const float &v);
  array dot(const array &v, const array &b);
  array sum(const array &v, const int &axis);
  array tanh(const array &v);
  array maximum(const float &a, const array &b);
  std::vector<int> shape(const array &t);
  array ones_like(const array &from);
  array zeros_like(const array &from);
  array reshape(const array &v, const int &sx, const int &sy);
  array asarray(const int &pos, const std::vector<struct data> &d);
  std::vector<bool> asboolarray(const int &pos, const std::vector<struct data> &d);
  array clip(const array &v, const float &l, const float &h);
  array T(const array &v);
  //bool check(const array &v);
  //void print(const char *n, const array &v);
};
extern numpy np;