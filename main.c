#include <stdio.h>
#include <math.h>
#include "tipos.h"

int toca_esfera(punto origen, vector vector, punto centro, double radio)
{
	// Se calculan los operandos de la ecuacion cuadratica

	// A no hace falta calcularse si los vectores dirección son normales

	// Aqui se calcula la b (2*(O*D-D*C))
	double b = (origen.x*vector.x + origen.y*vector.y + origen.z*vector.z);
	b -= (centro.x*vector.x + centro.y*vector.y + centro.z*vector.z);
	b = b*2;

	// Aqui se calcula la c (O*O+C*C-2*O*C-r*r)
	double c = origen.x*origen.x + origen.y*origen.y + origen.z*origen.z;
	c += centro.x*centro.x + centro.y*centro.y + centro.z*centro.z;
	c -= 2*(centro.x*origen.x + centro.y*origen.y + centro.z*origen.z);
	c -= radio*radio;

	// Aqui calculo el interior de la raiz para ver si es negativo
	double aux = b*b - 4*c;

	if(aux<0.0)
	{
		return 0;
	}

	// Se termina de realizar la ecuacion
	double t1 = 0.0 - b + sqrt(aux);
	t1 = t1 / 2.0;
	double t2 = 0.0 - b - sqrt(aux);
	t2 = t2 / 2.0;
	printf("t1: %.2f t2: %.2f\n",t1,t2);
	return 1;
}

int main(int argc, char ** argv)
{
	punto O = {2.0, 2.0, 0.0};
	vector D = {1.0, 0.0, 0.0};

	punto C = {2.0, 0.0, 0.0};
	double r = 1.0;

	if(!toca_esfera(O,D,C,r))
	{
		printf("No toca la esfera\n");
	} else
	{
		printf("Toca la esfera!!\n");
	}
}