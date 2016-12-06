#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include "tipos.h"

#define rayos_indirectos	128
#define ALPHA	0.8


int calcular_luz(vector pixel, color * rgb, punto cam, int recursivo);
color luz_indirecta (punto punto_mat, vector n, int ks, int kd, int recursivo);

// Valores de resolución y posición de la cámara
int ancho;
int alto;
punto camara;


double EPSILON = 0.00001;

// Nombres por defecto de los ficheros
char * img={"imagen.ppm"};
char * scn={"imagen.scn"};

// Punteros a las listas donde se almacenan las esferas y los focos
lista * l = NULL;
luces * lights = NULL;


/*
 * Método para calcular el producto escalar
 */
double dotproduct (vector *a, vector *b){
	return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

/*
 * Método para calcular el producto vectorial
 */
int crossproduct (vector * a, vector * b, vector * c){
	c->x = a->y*b->z - a->z*b->y;
	c->y = a->z*b->x - a->x*b->z;
	c->z = a->x*b->y - a->y*b->x;
	return 1;
}

/*
 * Método para parsear el fichero de la escena
 */
int parser(FILE * f)
{
	// Iteradores para ir guardando los datos
	lista * aux = NULL;
	luces * aux2 = NULL;
	// variables auxiliares
	double x,y,z,radio,R,G,B,klr,klg,klb,krr,krg,krb,ind;
	// Se lee la resolución
	fscanf(f, "%d %d",&ancho,&alto);
	// Se lee la posición de la cámara
	fscanf(f, "%lf %lf %lf",&x,&y,&z);
	camara.x=x; camara.y=y; camara.z=z;
	// Se lee hasta alcanzar el final del fichero
	while(feof(f)==0)
	{
		char c = fgetc(f);
		// Si la linea empieza por l es un foco de luz
		if(c=='l')
		{
			fscanf(f, "%lf %lf %lf %lf %lf %lf",&x,&y,&z,&R,&G,&B);

			luces * a = calloc(1, sizeof(luces));
			a->punto=calloc(1,sizeof(punto));
			a->color=calloc(1,sizeof(color));

			a->punto->x=x; a->punto->y=y; a->punto->z=z;
			a->color->r=R; a->color->g=G; a->color->b=B;

			if(lights==NULL){
				lights=a;
			} else{
				aux2->l=a;
			}
			aux2=a;
		}
		// Si la linea empieza por e se lee una esfera
		else if(c=='e')
		{
			fscanf(f, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",&x,&y,&z,&radio,&R,&G,&B,&klr,&klg,&klb,&krr,&krg,&krb,&ind);
			lista * a = calloc(1,sizeof(lista));
			a->radio=radio;
			a->punto=calloc(1,sizeof(punto));
			a->propiedades=calloc(1,sizeof(propiedades));
			a->propiedades->color=calloc(1,sizeof(color));
			a->propiedades->Krfl=calloc(1,sizeof(color));
			a->propiedades->Krfr=calloc(1,sizeof(color));

			a->punto->x=x; a->punto->y=y; a->punto->z=z;
			a->propiedades->color->r=R; a->propiedades->color->g=G; a->propiedades->color->b=B;
			a->propiedades->Krfl->r=klr; a->propiedades->Krfl->g=klg; a->propiedades->Krfl->b=klb;
			a->propiedades->Krfr->r=krr; a->propiedades->Krfr->g=krg; a->propiedades->Krfr->b=krb;
			a->propiedades->indice_ref=ind;
			if(l==NULL){
				l=a;
			} else{
				aux->l=a;
			}
			aux=a;
		}
	}
	return 0;
}

/*
 * Método para normalizar un vector
 */
int normalizar(vector * vec)
{
	double tamanyo = sqrt(vec->x*vec->x + vec->y*vec->y + vec->z*vec->z);
	vec->x = (vec->x)/tamanyo;
	vec->y = (vec->y)/tamanyo;
	vec->z = (vec->z)/tamanyo;
	return 1;
}

/*
 * Metodo para intersectar un triangulo
 * Extraido de: https://en.wikipedia.org/wiki/Möller–Trumbore_intersection_algorithm
 */
double toca_triangulo(punto origen, vector vec, punto V1, punto V2, punto V3)
{
	vector e1 = {V2.x-V1.x, V2.y-V1.y, V2.z-V1.z};
	vector e2 = {V3.x-V1.x, V3.y-V1.y, V3.z-V1.z};

	vector P = {0.0,0.0,0.0};
	crossproduct(&vec, &e2, &P);

	double det = dotproduct(&e1, &P);

	if(det > -0.01 && det < 0.01) return -1.0;
	double inv_det = 1.0 / det;

	vector T = {origen.x-V1.x, origen.y-V1.y, origen.z-V1.z};

	double u = dotproduct(&T, &P) * inv_det;

	if(u < 0.0 || u > 1.0) return -1.0;

	vector Q = {0.0, 0.0, 0.0};
	crossproduct(&T, &e1, &Q);

	double v = dotproduct(&vec, &Q) * inv_det;

	if(v < 0.0 || u + v > 1.0 ) return -1.0;

	double t = dotproduct(&e2, &Q) * inv_det;

	if(t > 0.01) {
		return t;
	}

	return -1.0;
}

/*
 * Método para descubrir si un rayo toca una esfera
 * 	el vector vector debe estar normalizado
 */
double toca_esfera(punto origen, vector vector, punto centro, double radio, char * dir)
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
		// No se toca
		return -1.0;
	}

	// Se termina de realizar la ecuacion
	double t1 = 0.0 - b + sqrt(aux);
	t1 = t1 / 2.0;
	double t2 = 0.0 - b - sqrt(aux);
	t2 = t2 / 2.0;


	if(t1==t2 && t1>0.0)
	{
		// Se toca un único punto
		*dir=1;
		if(t1<EPSILON) return 0.0;
		return t1;
	} else if(t1>=0.0 && t2<0.0)
	{
		// Dentro del circulo
		*dir=0;
		if(t1<=EPSILON){*dir=1; return 0.0;} 
		return t1;
	} else if(t1>=0.0 && t2>=0.0)
	{
		//Fuera y se da el más cercano
		*dir=1;
		if(t2<=EPSILON){ *dir=0; return t1;}
		return t2;
	} else{
		//No se apunta a la esfera
		return -1.0;
	}
}

/*
 * Método para calcular la luz directa
 */
int luz_directa(punto esfera, lista * minimo, color * rgb, luces * luz, vector pixel, vector normal)
{
	// Con el punto calculamos Li, de momento un unico punto de luz directa
	vector inter;
	inter.x = luz->punto->x - esfera.x;
	inter.y = luz->punto->y - esfera.y;
	inter.z = luz->punto->z - esfera.z;

	double light = inter.x*inter.x + inter.y*inter.y + inter.z*inter.z;
	double dist_luz = sqrt(light);

	color power={0.0,0.0,0.0};
	power.r = luz->color->r / light;
	power.g = luz->color->g / light;
	power.b = luz->color->b / light;
	
	normalizar(&inter);

	lista * aux = l;
	double dist;
	char dir=1;
	// Se comprueba si hay algun objeto entre el punto y la luz
	while(1)
	{
		dist = toca_esfera(esfera, inter,*aux->punto,aux->radio,&dir);
		
		if(dist>0.0 && dist <= dist_luz ){
		 return 0;
		}
		
		if(aux->l==NULL) break;
		aux = aux->l;
	}

	// Ahora calculamos la BRDF de phong
	// Primero obtenemos la normal, omega de o y de r
	
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

	double acum = 0.5/3.141592;
	acum += 0.5*((1+2)/(2*3.141592)) * pow(dotproduct2,1);
	
	// Por último obtenemos el coseno del angulo

	if(dotproduct<0) dotproduct = -dotproduct;

	rgb->r = power.r*acum*dotproduct*minimo->propiedades->color->r;
	rgb->g = power.g*acum*dotproduct*minimo->propiedades->color->g;
	rgb->b = power.b*acum*dotproduct*minimo->propiedades->color->b;
	return 1;
}



/*
 * Método para calcular el color de reflexión, los vectores deben estar normalizados
 */
color reflection (punto *point, vector *normal, vector *ray, int recursivo){

	// se calcula el factor para calcular el rayo reflectado y se calcula
	double factor = normal->x * -ray->x + normal->y * -ray->y + normal->z * -ray->z;
	vector reflection;
	reflection.x = -ray->x - 2 * (-ray->x - factor * normal->x);
	reflection.y = -ray->y - 2 * (-ray->y - factor * normal->y);
	reflection.z = -ray->z - 2 * (-ray->z - factor * normal->z);
	normalizar(&reflection);

	color reflectionColor={0.0,0.0,0.0};
	calcular_luz(reflection,&reflectionColor,*point,recursivo);
	return reflectionColor;
}

/*
 * http://asawicki.info/news_1301_reflect_and_refract_functions.html
 * https://www.flipcode.com/archives/reflection_transmission.pdf
 */
color refraction (lista *esfera, punto *point, vector *normal, vector *ray, int recursivo, int dentro){
	double n_refraction = esfera->propiedades->indice_ref; // Aire-Cristal 1.00/1.52=0.65
	if(dentro == 0) n_refraction = 1.0/n_refraction;
	double dot = -normal->x * ray->x + -normal->y * ray->y + -normal->z * ray->z;
	double k = 1.0 - n_refraction * n_refraction * (1.0 - dot * dot);
	vector refractado;
	if(k<0.0){
		refractado.x=0.0; refractado.y=0.0; refractado.z=0.0;
	}
	refractado.x = n_refraction * ray->x + (n_refraction * dot - sqrt(k)) * normal->x;
	refractado.y = n_refraction * ray->y + (n_refraction * dot - sqrt(k)) * normal->y;
	refractado.z = n_refraction * ray->z + (n_refraction * dot - sqrt(k)) * normal->z;
	normalizar(&refractado);
	color refractionColor = {0.0, 0.0, 0.0};

	calcular_luz(refractado,&refractionColor,*point,recursivo);
	return refractionColor;
}

/*
 * Método para calcular la luz de un pixel
 */
int calcular_luz(vector pixel, color * rgb, punto cam, int recursivo)
{
	// Primero buscamos el punto de intersección más cercano
	double min = 65535.0;
	double dist = 0.0;
	lista * aux = l;
	lista * minimo = NULL;
	char dir = 1;
	char dir_aux = 1;
	// Buscamos donde se choca el rayo
	while(1){
		dist = toca_esfera(cam,pixel,*aux->punto,aux->radio,&dir);
		if(dist>0.0 && dist<min){
			min = dist;
			minimo = aux;
			dir_aux = dir;
		}
		if(aux->l==NULL) break;
		aux = aux->l;
	}
	if(min == 65535.0){
		return 0;
	}

	// Obtenemos las coordenadas del punto en el espacio
	punto esfera;
	esfera.x = cam.x + pixel.x*min;
	esfera.y = cam.y + pixel.y*min;
	esfera.z = cam.z + pixel.z*min;

	// Calculamos la normal dependiendo si se esta dentro o fuera del circulo
	vector * normal = calloc(1,sizeof(vector));
	if(dir_aux==1){
		normal->x = esfera.x - minimo->punto->x;
		normal->y = esfera.y - minimo->punto->y;
		normal->z = esfera.z - minimo->punto->z;
	} else{
		normal->x = minimo->punto->x - esfera.x;
		normal->y = minimo->punto->y - esfera.y;
		normal->z = minimo->punto->z - esfera.z;
	}
	normalizar(normal);

	// Obtenemos la luz directa
	luces * aux2 = lights;
	while(1){
		color col={0.0,0.0,0.0};
		luz_directa(esfera, minimo, &col, aux2, pixel, *normal);
		rgb->r = rgb->r + col.r; rgb->g = rgb->g + col.g; rgb->b = rgb->b + col.b;
		if(aux2->l==NULL) break;
		aux2 = aux2->l;
	}
	
	// Obtenemos el porcentaje que corresponde por luz directa
	rgb->r = rgb->r * (1.0 - minimo->propiedades->Krfl->r - minimo->propiedades->Krfr->r);
	rgb->g = rgb->g * (1.0 - minimo->propiedades->Krfl->g - minimo->propiedades->Krfr->g);
	rgb->b = rgb->b * (1.0 - minimo->propiedades->Krfl->b - minimo->propiedades->Krfr->b);
	
	if(recursivo>0){
		recursivo--;
		color color_reflexion;
		color color_refraccion;
		color color_indirecta;
				
		// Se calcula la reflexion solo si es necesario
		if(minimo->propiedades->Krfl->r!=0.0 && 
			minimo->propiedades->Krfl->g!=0.0 && 
			minimo->propiedades->Krfl->b!=0.0)
		{
			color_reflexion = reflection(&esfera, normal, &pixel, recursivo);
			rgb->r = rgb->r + color_reflexion.r * minimo->propiedades->Krfl->r;
			rgb->g = rgb->g + color_reflexion.g * minimo->propiedades->Krfl->g;
			rgb->b = rgb->b + color_reflexion.b * minimo->propiedades->Krfl->b;
		}

		// Se calcula la refraccion solo si es necesario
		if(minimo->propiedades->Krfr->r!=0.0 && 
			minimo->propiedades->Krfr->g!=0.0 && 
			minimo->propiedades->Krfr->b!=0.0)
		{

			color_refraccion = refraction(minimo, &esfera, normal, &pixel, recursivo, dir_aux);
			rgb->r = rgb->r + color_refraccion.r * minimo->propiedades->Krfr->r;
			rgb->g = rgb->g + color_refraccion.g * minimo->propiedades->Krfr->g;
			rgb->b = rgb->b + color_refraccion.b * minimo->propiedades->Krfr->b;
		}

		color_indirecta = luz_indirecta(esfera, *normal, 0.8, 0.8, recursivo);
		rgb->r += color_indirecta.r * (1.0 - minimo->propiedades->Krfl->r - minimo->propiedades->Krfr->r);
		rgb->g += color_indirecta.g * (1.0 - minimo->propiedades->Krfl->g - minimo->propiedades->Krfr->g);
		rgb->b += color_indirecta.b * (1.0 - minimo->propiedades->Krfl->b - minimo->propiedades->Krfr->b);

	}
	free(normal);
	return 1;
}

double acumulativa_inversa_inclinacion(double x){
	return acos(sqrt(1-x));
}

double p_inclinacion(double x){
	return 2 * cos(x) * sin(x);
}

double acumulativa_inversa_acimut(double x){
	return 2 * M_PI * x;
}

double p_acimut(double x){
	return (double) 1 / (2 * M_PI);
}

vector global_desde_local(vector local, vector u, vector v, vector n){
	vector global = {0.0, 0.0, 0.0};
	global.x = u.x*local.x + v.x*local.y + n.x*local.z;
	global.y = u.y*local.x + v.y*local.y + n.y*local.z;
	global.z = u.z*local.x + v.z*local.y + n.z*local.z;
	return global;
}

color luz_indirecta (punto punto_mat, vector n, int ks, int kd, int recursivo){
	color luz_indirecta = {0.0, 0.0, 0.0};

	if (recursivo > 0){
		//primero se obtienen vectores perpendiculares para la geometria local
		double aleatorio = (double) (rand()%1000) / 1000;
		vector aleatoriov = {aleatorio, aleatorio, aleatorio};
		vector u;
		crossproduct(&n, &aleatoriov, &u);
		normalizar(&u);
		vector v;
		crossproduct(&n, &u, &v);
		normalizar(&v);

		int i;
		for (i = 0; i < rayos_indirectos; i++){
			//se eligen la inclinación y el acimut por montecarlo
			double aleatorio = (double) (rand()%1000) / 1000;
			double inclinacion = acumulativa_inversa_inclinacion(aleatorio);
			aleatorio = (double) (rand()%1000) / 1000;
			double acimut = acumulativa_inversa_acimut(aleatorio);

			//vector reflejado en geometría local
			vector reflejado = {sin(inclinacion) * cos(acimut), sin(inclinacion) * sin(acimut), cos(inclinacion)};

			//vector reflejado en geometría global
			reflejado = global_desde_local(reflejado, u, v, n);

			color luz_incidente;
			calcular_luz(reflejado, &luz_incidente, punto_mat, recursivo);
			luz_indirecta.r = luz_incidente.r * (kd + ks * (ALPHA + 2) / 2 * pow(dotproduct(&n, &reflejado), ALPHA));
			luz_indirecta.g = luz_incidente.g * (kd + ks * (ALPHA + 2) / 2 * pow(dotproduct(&n, &reflejado), ALPHA));
			luz_indirecta.b = luz_incidente.b * (kd + ks * (ALPHA + 2) / 2 * pow(dotproduct(&n, &reflejado), ALPHA));
			
		}
		//se divide por el número de muestras y ladistribución de probabilidad
		luz_indirecta.r = luz_indirecta.r/rayos_indirectos;
		luz_indirecta.g = luz_indirecta.g/rayos_indirectos;
		luz_indirecta.b = luz_indirecta.b/rayos_indirectos;
	}

	return luz_indirecta;
}

int saturacion_color(color * col)
{
	if(col->r>255) col->r = 255;
	if(col->g>255) col->g = 255;
	if(col->b>255) col->b = 255;

	return 1;
}

int main(int argc, char ** argv)
{
	int opt;
	// Con este bucle obtenemos los parametros dados
	while ((opt = getopt (argc, argv, "o:i:h")) != -1){
		switch(opt)
			{
				case 'o':
					img = optarg;
					break;
				case 'i':
					scn = optarg;
					break;
				case 'h':
					printf("Use -o para nombre de imagen y -i para nombre de escena\n");
					return 0;
			}
	}

	FILE * imagen;
	FILE * escena;

	imagen = fopen(img, "w");
	escena = fopen(scn, "r");
	parser(escena);

	fprintf(imagen, "P3 %d %d 255\n", ancho, alto);
	// i y d se usan para crear los rayos
	double i,d;
	// i_i y d_d se usan debido a comportamientos extraños con altas resoluciones
	int i_i,d_d;

	double i_ancho=1.0/ancho;
	double i_alto=1.0/alto;

	int incrementador = alto/100;
	int progreso = 0;
	int porcentaje = 0;
	for(i = 1.0, i_i=alto-1; i_i>0.0; i=i-i_alto,i_i--)
	{
		for (d = 0.0, d_d=0; d_d<ancho; d=d+i_ancho,d_d++)
		{
			vector pixel = {d-camara.x, i-camara.y, 0.0-camara.z};
			color col = {0.0,0.0,0.0};
			normalizar(&pixel);
			calcular_luz(pixel,&col,camara,5);
			// Desnormalizamos la luz
			col.r = col.r * 255.0; col.g = col.g * 255.0; col.b = col.b * 255.0;
			saturacion_color(&col);
			fprintf(imagen," %d %d %d ", (int)col.r, (int)col.g, (int)col.b);
		}

		progreso++;
		if(progreso>=incrementador){
			porcentaje++;progreso=0;
			int p;
			printf("\r[");
			for(p=0;p<=porcentaje/5;p++) printf("=");
			for(;p<20;p++) printf(" ");
			printf("][%02d%%]", porcentaje);
			fflush(stdout);
		}
		fprintf(imagen, "\n");
	}
	printf("\n");
	return 0;
}