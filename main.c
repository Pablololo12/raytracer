#include <stdio.h>
#include <math.h>
#include "tipos.h"


punto O = {0.0, 0.0, 0.0};
punto luz = {2000.0, 2000.0, 2000.0};
int potencia = 900;

int normalizar(vector * vec)
{
	double tamanyo = sqrt(vec->x*vec->x + vec->y*vec->y + vec->z*vec->z);
	vec->x = (vec->x)/tamanyo;
	vec->y = (vec->y)/tamanyo;
	vec->z = (vec->z)/tamanyo;
	return 1;
}

double toca_esfera(punto origen, vector vector, punto centro, double radio, int * num)
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
		//printf("No se toca\n");
		*num=0;
		return -1.0;
	}

	// Se termina de realizar la ecuacion
	double t1 = 0.0 - b + sqrt(aux);
	t1 = t1 / 2.0;
	double t2 = 0.0 - b - sqrt(aux);
	t2 = t2 / 2.0;

	if(t1==t2 && t1>0.0)
	{
		//printf("Se toca un único punto\n");
		*num=1;
		return t1;
	} else if(t1>=0.0 && t2<0.0)
	{
		//printf("Dentro del circulo\n");
		//return t1;
		*num=1;
		return t1;
	} else if(t1>=0.0 && t2>=0.0)
	{
		//printf("Fuera y se da el más cercano\n");
		*num=2;
		return t2;
	} else{
		//printf("No se apunta a la esfera\n");
		*num=0;
		return -1.0;
	}
}

int calcular_luz(lista * l, vector pixel)
{
	// Primero buscamos el punto de intersección más cercano
	double min = 65535.0;
	double dist = 0.0;
	int num = 0;
	lista * aux = l;
	lista * minimo = NULL;
	while(1){
		dist = toca_esfera(O,pixel,*aux->punto,aux->radio,&num);
		if(dist>0.0 && dist<min){
			min = dist;
			minimo = aux;
		}
		if(aux->l==NULL) break;
		aux = aux->l;
	}
	if(min == 65535.0){
		return 0;
	}
	// Obtenemos las coordenadas del punto en el espacio
	punto esfera;
	esfera.x = O.x + pixel.x*min;
	esfera.y = O.y + pixel.y*min;
	esfera.z = O.z + pixel.z*min;
	// Con el punto calculamos Li, de momento un unico punto de luz directa
	vector inter;
	inter.x = luz.x - esfera.x;
	inter.y = luz.y - esfera.y;
	inter.z = luz.z - esfera.z;
	
	normalizar(&inter);

	aux = l;
	while(1)
	{
		dist = toca_esfera(esfera, inter,*aux->punto,aux->radio,&num);
		
		if(dist>1.0 || num==2){
		 return 0;
		}
		
		if(aux->l==NULL) break;
		aux = aux->l;
	}

	//if(toca_esfera(esfera, inter, *minimo->punto, minimo->radio)!=-1.0) return 0;
	double light = inter.x*inter.x + inter.y*inter.y + inter.z*inter.z;
	light = potencia / light;

	// Ahora calculamos la BRDF de phong
	// Primero obtenemos la normal, omega de o y de r
	vector normal = {esfera.x - minimo->punto->x, esfera.y - minimo->punto->y, esfera.z - minimo->punto->z};
	normalizar(&normal);
	
	vector omegao = {-pixel.x, -pixel.y, -pixel.z};
	normalizar(&omegao);
	
	//Obtenemos omega r
	double dotproduct = inter.x*normal.x + inter.y*normal.y + inter.z*normal.z;
	vector omegar;
	omegar.x = inter.x - 2*(inter.x - normal.x*dotproduct);
	omegar.y = inter.y - 2*(inter.y - normal.y*dotproduct);
	omegar.z = inter.z - 2*(inter.z - normal.z*dotproduct);
	normalizar(&omegar);

	// Por ultimo aplicamos phong
	double dotproduct2 = omegao.x*omegar.x + omegao.y*normal.y + omegao.z*normal.z;
	if(dotproduct2<0) dotproduct2 = -dotproduct2;

	double acum = 0.9/3.141592;
	acum += 0.9*((5+2)/(2*3.141592)) * pow(dotproduct2,5);
	
	// Por último obtenemos el coseno del angulo
	//double base1 = sqrt(light);
	//double base2 = sqrt(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
	//dotproduct = dotproduct / (base1 * base2);
	if(dotproduct<0) dotproduct = -dotproduct;
	//printf("%f %f %f\n",light, acum, dotproduct);
	double resultado = light*acum*dotproduct;
	return (int) resultado;
}

int main(int argc, char ** argv)
{
	punto C = {-500.0, 0.0, 5000.0};
	double r = 900.0;

	punto C2 = {750.0, -500.0, 5000.0};
	double r2 = 300.0;

	punto C3 = {300.0, 200, 4000.0};
	double r3 = 450.0;

	lista t = {&C3, r3, NULL};
	lista p = {&C2,r2, &t};
	lista l = {&C,r,&p};

	int ancho = 2000;
	int alto = 2000;

	FILE * imagen;
	imagen = fopen("imagen.ppm", "w");
	fprintf(imagen, "P3 %d %d 255\n", ancho, alto);
	int i,d;
	for(i = 0; i<alto; i++)
	{
		for (d = 0; d<ancho; d++)
		{
			vector pixel = {i-1000.0, d-1000.0, 3000.0};
			normalizar(&pixel);
			int light = calcular_luz(&l,pixel);
			if(light>255) light=255;
			fprintf(imagen," %d 0 0 ",light);
		}
		fprintf(imagen, "\n");
	}
}