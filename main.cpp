#include <iostream>
#include <fstream>
#include <time.h>
#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

#define MCW MPI_COMM_WORLD

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
    return 0;
  return 32 * (iters % 8);
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


// Main process sends out everything as master slave
// other processes do the calculations
// Master adds them all to the file
// End


int main(int argc, char **argv){

  Complex c1, c2, c3;
  Complex c;
  int rank, size;
  const int DIM = 500; 
  int data[DIM][DIM];
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MCW,&rank);
  MPI_Comm_size(MCW,&size);
  MPI_Request myRequest1, myRequest2, myRequest3;
  MPI_Status myStatus1, myStatus2, myStatus3;
  int flag1 = 1;
  int flag2 = 1;
  int flag3 = 1;

  c1.r = 2;
  c1.i = 2;
  c2.r = -2;
  c2.i = -2;

  // Use process 0 as the master process
  if(!rank) {
    long startTime = time(NULL);

    ofstream fout;
    fout.open("image.ppm");

    c3 = c1 + c2;
    cout << c3.r << " + " << c3.i << "i" << endl;

    c3 = c1 * c2;
    cout << c3.r << " * " << c3.i << "i" << endl;

    fout << "P3" << endl;
    fout << DIM << " " << DIM << endl;
    fout << 255 << endl;

    for (int j=0; j < DIM; ++j) {
      for( int i=0; i < DIM; ++i) {

        int tmpData[] = {i, j};
        
        while(true) {
          if(flag1) {
            flag1 = 0;
            MPI_Send(&tmpData, 2, MPI_INT, 1, 0, MCW);
            MPI_Irecv(&data[i][j], 1, MPI_INT, 1, 1, MCW, &myRequest1);
            break;
          }
          else if(flag2) {
            flag2 = 0;
            MPI_Send(tmpData, 2, MPI_INT, 2, 0, MCW);
            MPI_Irecv(&data[i][j], 1, MPI_INT, 2, 2, MCW, &myRequest2);
            break;
          }
          else if(flag3) {
            flag3 = 0;
            MPI_Send(tmpData, 2, MPI_INT, 3, 0, MCW);
            MPI_Irecv(&data[i][j], 1, MPI_INT, 3, 3, MCW, &myRequest3);
            break;
          }

          MPI_Test(&myRequest1, &flag1, &myStatus1);
          MPI_Test(&myRequest2, &flag2, &myStatus2);
          MPI_Test(&myRequest3, &flag3, &myStatus3);
        }
      }
    }

    for(int i = 1; i < size; i++) {
      int tmpData[] = {-1, -1};
      MPI_Send(&tmpData, 2, MPI_INT, i, 0, MCW);
    }

    for (int j=0; j < DIM; ++j) {
      for( int i=0; i < DIM; ++i) {
        fout << rcolor(data[i][j]) << " ";
        fout << gcolor(data[i][j]) << " ";
        fout << bcolor(data[i][j]) << " ";
      }
    }

    fout << endl;
    fout.close();

    cout << "TIME: " << time(NULL) - startTime << endl;
  }


  else {
    int recData[2] = {-1, -1};
    while(true) {
      MPI_Recv(recData, 2, MPI_INT, MPI_ANY_SOURCE, 0, MCW, MPI_STATUS_IGNORE);

      if(recData[0] == -1)
        break;
      
      c.r = (recData[0]*(c1.r - c2.r) / DIM) + c2.r;
      c.i = (recData[1]*(c1.i - c2.i) / DIM) + c2.i;
      int iters = mbrotIters(c, 255);

      MPI_Send(&iters, 1, MPI_INT, 0, rank, MCW);
    }
  }

  MPI_Finalize();
  return 0;
}
