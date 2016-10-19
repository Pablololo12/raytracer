#ifndef TIPOS_H
#define TIPOS_H

typedef struct punto{
	double x;
	double y;
	double z;
}punto;

typedef struct vector{
	double x;
	double y;
	double z;
}vector;


typedef struct lista{
	punto * punto;
	double radio;
	struct lista * l;
}lista;

#endif