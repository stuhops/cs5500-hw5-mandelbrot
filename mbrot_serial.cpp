#include <iostream>
#include <fstream>
#include <chrono> 

using namespace std::chrono; 
using namespace std;

struct Complex {
  double r;
  double i;
};

Complex operator + (Complex s, Complex t) {
  Complex v;
  v.r = s.r + t.r;
  v.i = s.i + t.i;
  return v;
}

Complex operator * (Complex s, Complex t) {
  Complex v;
  v.r = s.r * t.r - s.i * t.i;
  v.i = s.r * t.i + s.i * t.r;
  return v;
}


int rcolor(int iters) {
  if(iters == 255) 
    return 266;
  return 266 - 32 * (iters % 8);
}

int gcolor(int iters) {
  if(iters == 255) 
    return 0;
  return 32 * (iters % 8);
}

int bcolor(int iters) {
  if(iters == 255) 
    return 0;
  return 32 * (iters % 8);
}

int mbrotIters(Complex c, int maxIters) {
  int i = 0;
  Complex z;
  z = c;
  while(i < maxIters && z.r * z.r + z.i * z.i < 4) {
    z = z * z + c;
    i++;
  }
  return i;
}




int main () {

  Complex c1, c2, c3;
  Complex c;

  auto start = high_resolution_clock::now(); 

  ofstream fout;
  fout.open("image.ppm");

  c1.r = 2;
  c1.i = 2;
  c2.r = -2;
  c2.i = -2;

  c3 = c1 + c2;
  cout << c3.r << " + " << c3.i << "i" << endl;

  c3 = c1 * c2;
  cout << c3.r << " * " << c3.i << "i" << endl;

  int DIM = 500;
  fout << "P3" << endl;
  fout << DIM << " " << DIM << endl;
  fout << 255 << endl;

  for (int j=0; j < DIM; ++j) {
    for( int i=0; i < DIM; ++i) {
      c.r = (i*(c1.r - c2.r) / DIM) + c2.r;
      c.i = (j*(c1.i - c2.i) / DIM) + c2.i;

      int iters = mbrotIters(c, 255);
      fout << rcolor(iters) << " ";
      fout << gcolor(iters) << " ";
      fout << bcolor(iters) << " ";
    }
    fout << endl;
  }

  fout.close();

  auto stop = high_resolution_clock::now(); 
  auto duration = duration_cast<microseconds>(stop - start);
  cout << "TIME: " << duration.count() << " microseconds" << endl;

  return 0;
}