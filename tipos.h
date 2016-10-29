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

typedef struct color{
	double r;
	double g;
	double b;
}color;

typedef struct lista{
	punto * punto;
	color * color;
	double radio;
	struct lista * l;
}lista;

typedef struct luces{
	punto * punto;
	color * color;
	struct luces * l;
}luces;

#endif