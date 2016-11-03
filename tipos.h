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

typedef struct propiedades{
	color * color;
	color * Krfl;
	color * Krfr;
}propiedades;

typedef struct lista{
	punto * punto;
	propiedades * propiedades;
	double radio;
	struct lista * l;
}lista;

typedef struct luces{
	punto * punto;
	color * color;
	struct luces * l;
}luces;

#endif