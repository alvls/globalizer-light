﻿/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      Main.cpp                                                    //
//                                                                         //
//  Purpose:   Console version of ExaMin system                            //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


//#include <exception>
//#include <cstdlib>
#include <stdio.h>
//#include <stdlib.h>
#include <string>
#include "mpi.h"
#define _USE_MATH_DEFINES
#include <math.h>

#include "Rastrigin.h"
#include "common.h"
#include "exception.h"

#include "process.h"
// ------------------------------------------------------------------------------------------------
//переводит  char* в int !!бросает исключение
int parseInteger(const char* arg) 
{
    char* end;
    int value = strtol(arg, &end, 10);

    if (end[0]) 
      throw EXCEPTION("Wrong format!\n");

    return value;
}
// ------------------------------------------------------------------------------------------------
//переводит  char* в double !!бросает исключение
double parseDouble(const char* arg) 
{
    char* end;
    double value = static_cast<double>(strtod(arg, &end));

    if (end[0])
        throw EXCEPTION("Wrong format!\n");

    return value;
}

void parseArguments(int argc, char** argv,
                    int& dimension, double& r,
                    int& m, int& NumOfTaskLevels,
                    double* eps, int* DimInTaskLevel,
                    int* ChildInProcLevel, int *MaxNumOfPoints, std::string& procType)
{
  dimension = 4;
  r = 2.3;
  m = 10;
  NumOfTaskLevels = 1;
  std::string arrayValue;
  int position;
  // проверяем только ключи - аргументы 1, 3, 5 и так далее
  for(int i = 1; i < argc; i+=2)
  {
    // Размерность
    if (0 == strcmp(argv[i],"-Dimension"))
    {
      dimension = parseInteger(argv[i+1]);
      if ((dimension <= 0) || (dimension > MaxDim))
      {
        throw EXCEPTION("Dimension is out of range\n");
      }
    }
    // Количество уровней в дереве
    else if(0 == strcmp(argv[i],"-NumOfTaskLevels"))
    {
      NumOfTaskLevels = parseInteger(argv[i+1]);
      if(NumOfTaskLevels <= 0)
      {
        throw string("NumOfTaskLevels is out of range\n");
      }
    }
    // Надежность метода
    else if(0 == strcmp(argv[i],"-r"))
    {
      r = parseDouble(argv[i+1]);
      if (r <= 2.0)
      {
        throw EXCEPTION("r is out of range\n");
      }
    }
    // Плотность развертки
    else if(0 == strcmp(argv[i],"-m"))
    {
      m = parseInteger(argv[i+1]);
      if ((m < 2) || (m > MaxM))
      {
        throw EXCEPTION("m is out of range\n");
      }
    }
    else if (0 == strcmp(argv[i],"-TypeProcess"))
    {
      if ((0 != strcmp(argv[i+1],"SynchronousProcess")) &&
        (0 != strcmp(argv[i+1],"AsynchronousProcess")))
      {
        throw EXCEPTION("Wrong type of process\n");
      }
      procType = argv[i+1];
    }
  }

  for(int i = 1; i < argc; i+=2)
  {
    // Точность решения для каждого уровня
    if(0 == strcmp(argv[i],"-Eps"))
    {
      arrayValue = argv[i+1];
      position = position= arrayValue.find("_");
      int count = 0;
      while (-1 != position)
      {
        
        eps[count++] = parseDouble(arrayValue.substr(0, position).c_str());
        if (eps[count - 1] <= 0.0)
        {
          throw EXCEPTION("Epsilon is out of range");
        }
        arrayValue = arrayValue.substr(position+1);
        position= arrayValue.find("_");
      }
      if (count != NumOfTaskLevels)
      {
        throw EXCEPTION("wrong epsilon");
      }
    }
    // Размерность на каждом уровне
    else if(0 == strcmp(argv[i],"-DimInTaskLevel"))
    {
      arrayValue = argv[i+1];
      position = position= arrayValue.find("_");
      int count = 0;
      while (-1 != position)
      {
        DimInTaskLevel[count++] = parseInteger(arrayValue.substr(0, position).c_str());
        arrayValue = arrayValue.substr(position+1);
        position= arrayValue.find("_");
      }
      if (count != NumOfTaskLevels)
      {
        throw EXCEPTION("wrong DimInTaskLevel");
      }
    }
    // Число потомков на каждом уровне
    else if(0 == strcmp(argv[i],"-ChildInProcLevel"))
    {
      arrayValue = argv[i+1];
      position = arrayValue.find("_");
      int count = 0;
      while (-1 != position)
      {
        ChildInProcLevel[count++] = parseInteger(arrayValue.substr(0, position).c_str());
        arrayValue = arrayValue.substr(position+1);
        position = arrayValue.find("_");
      }
      if (count != NumOfTaskLevels)
      {
        throw EXCEPTION("wrong ChildInProcLevel");
      }
    }
    // Максимальное количество испытаний для каждого уровня
    else if(0 == strcmp(argv[i],"-MaxNumOfPoints"))
    {
      arrayValue = argv[i+1];
      position= arrayValue.find("_");
      int count = 0;
      while (-1 != position)
      {
        MaxNumOfPoints[count++] = parseInteger(arrayValue.substr(0, position).c_str());
        arrayValue = arrayValue.substr(position+1);
        position= arrayValue.find("_");
      }
      if (count != NumOfTaskLevels)
      {
        throw EXCEPTION("wrong MaxNumOfPoints");
      }
    }
  }
  // проверяем значения DimInTaskLevel
  int dim = 0;
  for (int j = 0; j < NumOfTaskLevels; j++)
  {
    if (DimInTaskLevel[j] <= 0)
    {
      throw EXCEPTION("DimInTaskLevel is out of range");
    }
    dim += DimInTaskLevel[j];
  }
  if (dim != dimension)
  {
    throw EXCEPTION("wrong NumOfTaskLevels");
  }

  // проверяем ChildInProcLevel
  for (int j = 0; j < NumOfTaskLevels - 1; j++)
  {
    if (ChildInProcLevel[j] <= 0)
    {
      throw EXCEPTION("ChildInProcLevel is out of range");
    }
  }
  if (ChildInProcLevel[NumOfTaskLevels - 1] != 0)
  {
    throw EXCEPTION("ChildInProcLevel is out of range");
  }
  int ProcNum = 0, curProcNum = 1;
  if (MPI_Comm_size(MPI_COMM_WORLD, &ProcNum) != MPI_SUCCESS)
  {
    throw EXCEPTION("Error in MPI_Comm_size call");
  }
  int numNodes = 1;
  for (int j = 0; j < NumOfTaskLevels; j++)
  {
    curProcNum += ChildInProcLevel[j] * numNodes;
    numNodes = ChildInProcLevel[j] * numNodes;
  }
  if (ProcNum != curProcNum)
  {
    throw EXCEPTION("Wrong number of process\n");
  }

  // проверка MaxNumOfPoints
  for (int j = 0; j < NumOfTaskLevels; j++)
  {
    if (MaxNumOfPoints[j] <= 0)
    {
      throw EXCEPTION("MaxNumOfPoints is out of range");
    }
  }
}
// ------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  // задача
  int dimension; 
  int NumOfFunc = 1;
  double A[MaxDim] = {-1.1, -1.1, -1.1, -1.1, -1.1}; 
  double B[MaxDim] = {1.2, 1.2, 1.2, 1.2, 1.2};
  tFunction F[MaxNumOfFunc] = {objfn};//{function};
  bounds(A,B);//Границы поиска

  // дерево задач и процессов
  int NumOfTaskLevels; // максимальное число уровней в дереве - 5
  int DimInTaskLevel[MaxNumOfTaskLevels] = {4};
  int ChildInProcLevel[MaxNumOfTaskLevels] = {};

  // параметры метода
  double r;
  int m;
  int MaxNumOfPoints[MaxNumOfTaskLevels] = {7000000, 1000000, 1000000, 1000000, 1000000};
  double Eps[MaxNumOfTaskLevels] = {0.01, 0.01, 0.01, 0.01, 0.01};
  std::string procType = "SynchronousProcess";

  int ProcRank, ProcNum;
  int i; 

  TProcess *process;
  
  MPI_Init(&argc, &argv);
  if (MPI_Comm_size(MPI_COMM_WORLD, &ProcNum) != MPI_SUCCESS)
  {
    throw EXCEPTION("Error in MPI_Comm_size call");
  }
  if (MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank) != MPI_SUCCESS)
  {
    throw EXCEPTION("Error in MPI_Comm_rank call");
  }
  try
  {
    parseArguments(argc, argv,
                   dimension, r,
                   m, NumOfTaskLevels,
                   Eps, DimInTaskLevel,
                   ChildInProcLevel, MaxNumOfPoints, procType);

    process = new TProcess(dimension, A, B, NumOfFunc, F, NumOfTaskLevels,
      DimInTaskLevel, ChildInProcLevel, MaxNumOfPoints,
      Eps, r, m);

    process->Solve();
  }
  catch(TException e)
  {
    printf("\nEXCEPTION in file: %s, line %d, function: %s", e.GetFile(), e.GetLine(),
      e.GetFunction());
    printf("\nDESCRIPTION: %s", e.GetDescription());
    for (i = 0; i < ProcNum; i++)
      if (i != ProcRank)
        MPI_Abort(MPI_COMM_WORLD, i);
    MPI_Finalize();
    return 1;
  }
  catch(...)
  {
    printf("\nUNKNOWN EXCEPTION !!!");
    for (i = 0; i < ProcNum; i++)
      if (i != ProcRank)
        MPI_Abort(MPI_COMM_WORLD, i);
    MPI_Finalize();
    return 1;
  }
  delete process;
  MPI_Finalize();
  return 0;
}
// - end of file ----------------------------------------------------------------------------------
