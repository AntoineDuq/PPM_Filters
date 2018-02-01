#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#define SIZE 256
#define SIZE2 1024

/*
 * Taille maximale utilisée pour le chargement d'image
 */
#define TAILLE_MAX 512 //par défaut, utilisé pour définir la taille de votre image

/*
 * Intensité maximale, pour PGM/PPM les intensités sont codées sur
 * un octet donc [0;255]
 */
#define INTENSITE_MAX 255

/*
 * Pour le format utilisé pour ce projet, les couleurs
 * sont stockées sur 3 octets (1 Rouge, 1 Vert, 1 Bleu)
 */
#define RVB_PPM 3

/*********************************************************
 * Fonctions de base : chargement et sauvegarde d'images
 *********************************************************
 */


int chargeImage(const char * nom_fichier, unsigned char * tableau);
int sauvegardeImage(const char * nom_fichier, unsigned char * tableau, int largeur, int hauteur);
void test(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_test[TAILLE_MAX][TAILLE_MAX][3]);
void blackwhite(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_noire[TAILLE_MAX][TAILLE_MAX][3], int s);
void gris(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_grise[TAILLE_MAX][TAILLE_MAX][3]);
void rgb_to_tsv(unsigned char image_rgb[TAILLE_MAX][TAILLE_MAX][3], double image_tsv[TAILLE_MAX][TAILLE_MAX][3]);
void tsv_to_rgb(double image_tsv[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_rgb[TAILLE_MAX][TAILLE_MAX][3]);
double getmax(double a, double b, double c);
double getmin(double a, double b, double c);
void blur_count(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_flou[TAILLE_MAX][TAILLE_MAX][3], int b_size, int b_count, int type);
void blur(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_flou[TAILLE_MAX][TAILLE_MAX][3], int b_size, int type);
void blur_dir(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_flou[TAILLE_MAX][TAILLE_MAX][3], int b_size, int type);
void copy(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_copy[TAILLE_MAX][TAILLE_MAX][3]);
void resize(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_resize[SIZE][SIZE][3], int blur);
void reverse(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_reverse[TAILLE_MAX][TAILLE_MAX][3]);
void hue(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_hue[TAILLE_MAX][TAILLE_MAX][3], double new_h);
void sat(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_sat[TAILLE_MAX][TAILLE_MAX][3], int new_s);
void val(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_val[TAILLE_MAX][TAILLE_MAX][3], int new_v);
void extrapol(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_extrapol[TAILLE_MAX][TAILLE_MAX][3],float u);
int ui_choice(char nom_fichier[30], char nom_out[30], unsigned char image[TAILLE_MAX][TAILLE_MAX][3]);
void resize2(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_resize[SIZE2][SIZE2][3]);

/*
 \brief Fonction pour charger une image
 \param nom_fichier Le fichier que l'on veut charger
 \param vecteur adresse ou l'image va être stockée
 \return 1 si tout c'est bien passé, 0 sinon
 */
int chargeImage(const char * nom_fichier, unsigned char * vecteur)
{
	/* Declaration de variables */
	int largeur, hauteur;
	FILE * fp;
	unsigned int type;
	int imax, tailleImage, debug;
	char buffer[10];

	/* Ouverture du fichier */
	if( (fp = fopen(nom_fichier, "rb")) == NULL )
	{
		printf("[chargeImage] Impossible d'ouvrir le fichier %s\n", nom_fichier);
		return 0;
	}
	printf("[chargeImage] Fichier %s ouvert\n", nom_fichier);
	/* 1- entête */
	if(fgets(buffer, 3, fp) == NULL)
	{
		printf("[chargeImage] Erreur de lecture du fichier\n");
		return 0;
	}

	if( (type = buffer[1]-48) != 6 )
	{
		printf("[chargeImage] L'image n'est pas de type P6 (P%d)\n", type);
	}
	///* 2- On passe les commentaires */
	//do
	//{
	//	if(fgets(buffer, TAILLE_MAX, fp) == NULL)
	//	{
	//		printf("[chargeImage] Erreur de lecture du fichier\n");
	//		return 0;
	//	}
	//
	//}
	//while (buffer[0] == '#');

	/* 3- Dimensions */
	if(fscanf(fp, "%d %d\n%d\n", &largeur, &hauteur, &imax) == 0)
	{
		printf("[chargeImage] Erreur pour accéder aux paramètres de l'image\n");
		return 0;
	}
	printf("[chargeImage] Entete = Image %d x %d pixels, intensite maximale = %d\n", largeur, hauteur, imax);
	/* Allocation d'un tableau suffisamment grand pour contenir les pixels de l'image */
	// on suppose que la taille du tableau image est correcte par rapport à l'image chargée
	tailleImage = TAILLE_MAX * TAILLE_MAX * RVB_PPM;

	/* Recuperation des pixels de l'image */
	if( (fread (vecteur, 1, tailleImage, fp)) != tailleImage )
	{
		printf("[chargeImage] Les pixels n'ont pas été correctement récupérés\n");
		return 0;
	}

	/* Si on arrive jusque là, tout s'est bien passé :) */
	fclose(fp);
	printf("[chargeImage] Le fichier a été correctement fermé\n");
	return 1;
}

/*
 \brief Fonction pour sauvegarder une image
 \param nom_fichier Le fichier que l'on veut sauvegarder
 \param tableau tableau de char (= endroit où les pixels sont stockés)
 \return 1 si tout c'est bien passé, 0 sinon
 */

int sauvegardeImage(const char * nom_fichier, unsigned char * vecteur, int largeur, int hauteur)
{
	/* Declaration de variables */
	FILE * fp;
	int tailleImage;
	//int largeur=TAILLE_MAX, hauteur=TAILLE_MAX;

	/* Ouverture du fichier */
	if( (fp = fopen(nom_fichier, "wb")) == NULL )
	{
		printf("[sauvegardeImage] Impossible d'ouvrir le fichier %s\n", nom_fichier);
		return 0;
	}
	printf("[sauvegardeImage] Fichier %s ouvert en ecriture\n", nom_fichier);

	/* Ecriture de l'entete */
	if(!fprintf(fp, "P6\n%d %d\n%d\n", largeur, hauteur, INTENSITE_MAX))
	{
		printf("[sauvegardeImage] Erreur pour écrire les paramètres de l'image\n");
		return 0;
	}
	printf("[sauvegardeImage] Entete = Image %d x %d pixels, intensite maximale = %d\n", largeur, hauteur, INTENSITE_MAX);

	/* Ecriture des pixels de l'image */
	tailleImage = largeur * hauteur * RVB_PPM;
	if( fwrite (vecteur, 1, tailleImage, fp) != tailleImage )
	{
		printf("[sauvegardeImage] Les pixels n'ont pas été correctement écrits\n");
		return 0;
	}
	/* Si on arrive jusque là, tout s'est bien passé :) */
	fclose(fp);
	printf("[sauvegardeImage] Le fichier a été correctement fermé\n");
	return 1;
}

// -------------------------------------------------------------------------------------------------

void resize(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_resize[SIZE][SIZE][3], int blur)
{

	int i,j;

    if (blur == 1)
    {
        blur_count(image_source,image_source,1,1,4);
    }

	for(i=0;i<TAILLE_MAX;i+=2)
	{
		for(j=0;j<TAILLE_MAX;j+=2)
		{

			image_resize[i/(TAILLE_MAX/SIZE)][j/(TAILLE_MAX/SIZE)][0] = image_source[i][j][0];
			image_resize[i/(TAILLE_MAX/SIZE)][j/(TAILLE_MAX/SIZE)][1] = image_source[i][j][1];
			image_resize[i/(TAILLE_MAX/SIZE)][j/(TAILLE_MAX/SIZE)][2] = image_source[i][j][2];

		}
	}

}

void resize2(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_resize[SIZE2][SIZE2][3])
{

	int i,j,k;

	for(i=0;i<TAILLE_MAX;i+=1)
	{
		for(j=0;j<TAILLE_MAX;j+=1)
		{
            for(k=0;k<3;k+=1)
            {
                image_resize[2*i][2*j][k] = image_source[i][j][k];
                image_resize[2*i+1][2*j+1][k] = image_source[i][j][k];
                image_resize[2*i][2*j+1][k] = image_source[i][j][k];
                image_resize[2*i+1][2*j][k] = image_source[i][j][k];
            }

		}
	}

}

void copy(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_copy[TAILLE_MAX][TAILLE_MAX][3])
{

	int i,j,k;

	for(i=0;i<TAILLE_MAX;i++)
	{
		for(j=0;j<TAILLE_MAX;j++)
		{
			for(k=0;k<=2;k++)
			{
				image_copy[i][j][k] = image_source[i][j][k];
			}

		}
	}

}

void blur(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_flou[TAILLE_MAX][TAILLE_MAX][3], int b_size, int type)
{

	int i,j,k,b;
	int avg;
	float sum;
	float cpt=1.0;
	float coeff;

	for(i=0;i<TAILLE_MAX;i++)
	{
		for(j=0;j<TAILLE_MAX;j++)
		{
			for(k=0;k<=2;k++)
			{
				sum=image_source[i][j][k];

				for(b=1;b<=b_size;b++)
				{

                    if(type==1)
                    {
                        coeff = 1.0/log(b+2.0);
                    }

                    else if(type==2)
                    {
                        coeff = 1.0/(float)pow(2,b);
                    }

                    else if(type==3)
                    {
                        coeff = 1.0/b+1.0;
                    }

                    else
                    {
                        coeff = 1.0;
                    }

					if(((i>=0) && (j+b>=0)) && ((i<TAILLE_MAX) && (j+b<TAILLE_MAX)))
					{
						sum+=image_source[i][j+b][k]*coeff;
						cpt+=coeff;
					}

					if(((i+b>=0) && (j+b>=0)) && ((i+b<TAILLE_MAX) && (j+b<TAILLE_MAX)))
					{
						sum+=image_source[i+b][j+b][k]*coeff;
						cpt+=coeff;
					}

					if(((i+b>=0) && (j>=0)) && ((i+b<TAILLE_MAX) && (j<TAILLE_MAX)))
					{
						sum+=image_source[i+b][j][k]*coeff;
						cpt+=coeff;
					}

					if(((i+b>=0) && (j-b>=0)) && ((i+b<TAILLE_MAX) && (j-b<TAILLE_MAX)))
					{
						sum+=image_source[i+b][j-b][k]*coeff;
						cpt+=coeff;
					}

					if(((i>=0) && (j-b>=0)) && ((i<TAILLE_MAX) && (j-b<TAILLE_MAX)))
					{
						sum+=image_source[i][j-b][k]*coeff;
						cpt+=coeff;
					}

					if(((i-b>=0) && (j-b>=0)) && ((i-b<TAILLE_MAX) && (j-b<TAILLE_MAX)))
					{
						sum+=image_source[i-b][j-b][k]*coeff;
						cpt+=coeff;
					}

					if(((i-b>=0) && (j>=0)) && ((i-b<TAILLE_MAX) && (j<TAILLE_MAX)))
					{
						sum+=image_source[i-b][j][k]*coeff;
						cpt+=coeff;
					}

					if(((i-b>=0) && (j+b>=0)) && ((i-b<TAILLE_MAX) && (j+b<TAILLE_MAX)))
					{
						sum+=image_source[i-b][j+b][k]*coeff;
						cpt+=coeff;
					}

				}

				avg = sum/cpt;
				image_flou[i][j][k] = avg;
				cpt = 1.0;

			}

		}
	}

}

void blur_dir(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_flou[TAILLE_MAX][TAILLE_MAX][3], int b_size, int type)
{
	int i,j,k,b;
	int avg;
	float sum;
	float cpt=1.0;
	float coeff;

	for(i=0;i<TAILLE_MAX;i++)
	{
		for(j=0;j<TAILLE_MAX;j++)
		{
			for(k=0;k<=2;k++)
			{
				sum=image_source[i][j][k];

				for(b=1;b<=b_size;b++)
				{

                    if(type==1)
                    {
                        coeff = 1.0/log(b+2.0);
                    }

                    else if(type==2)
                    {
                        coeff = 1.0/(float)pow(2,b);
                    }

                    else if(type==3)
                    {
                        coeff = 1.0/b+1.0;
                    }

                    else
                    {
                        coeff = 1.0;
                    }

					if((j-b>=0) && (j+b<TAILLE_MAX))
					{
						sum+=(image_source[i][j-b][k]*coeff)+(image_source[i][j+b][k]*coeff);
						cpt+=2.0*coeff;
					}


				}

				avg = sum/cpt;
				image_flou[i][j][k] = avg;
				cpt = 1.0;

			}

		}
	}

}

void blur_count(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_flou[TAILLE_MAX][TAILLE_MAX][3], int b_size, int b_count, int type)
{
	int i;
	for(i=1;i<=b_count;i++)
	{

		blur(image_source,image_flou,b_size,type);
		copy(image_flou,image_source);

	}

}

double getmin(double a, double b, double c)
{
	if(a<b)
	{
		if(a<c)
		{
			return a;
		}
		else
		{
			return c;
		}
	}
	else
	{
		if(b<c)
		{
			return b;
		}
		else
		{
			return c;
		}
	}
}

double getmax(double a, double b, double c)
{

	if(a<b)
	{

		if(b<=c)
		{
			return c;
		}
		else
		{
			return b;
		}

	}
	else
	{
		if(a<=c)
		{
			return c;
		}
		else
		{
			return a;
		}

	}


}

void rgb_to_tsv(unsigned char image_rgb[TAILLE_MAX][TAILLE_MAX][3], double image_tsv[TAILLE_MAX][TAILLE_MAX][3])
{
	int i,j;
	double h,s,l,r,v,b,min,max,q;
	for(i=0;i<TAILLE_MAX;i++)
	{
		for(j=0;j<TAILLE_MAX;j++)
		{

			r = image_rgb[i][j][0]/255.0;
			v = image_rgb[i][j][1]/255.0;
			b = image_rgb[i][j][2]/255.0;
            		//printf("%lf,%lf,%lf\n",r*255,v*255,b*255);
			max = getmax(r,v,b);
			min = getmin(r,v,b);
            		//printf("%lf,%lf\n",max,min);
			if(max == min)
			{
				h=0;
			}
			else if(max == r)
			{
			    q = (60.0*((v-b)/(max-min))+360);
			    h = fmod(q,360.0);
			}

			else if(max == v)
			{
				h=60.0*(((b-r)/(max-min))+2.0);
			}
			else if(max == b)
			{
				h=60.0*(((r-v)/(max-min))+4.0);
			}
			if(max == 0)
			{
				s=0.0;
			}
			else
			{
				s=((max-min)/max);
			}

			l=max;

			//printf("%lf,%lf,%lf\n\n",h,s,l);

			image_tsv[i][j][0] = h;
			image_tsv[i][j][1] = s;
			image_tsv[i][j][2] = l;


		}
	}
}

void tsv_to_rgb(double image_tsv[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_rgb[TAILLE_MAX][TAILLE_MAX][3])
{
	int i,j,p_ent,n;
	double h,s,l,r,v,b,f,p,q,u;
	for(i=0;i<TAILLE_MAX;i++)
	{
		for(j=0;j<TAILLE_MAX;j++)
		{
			h = image_tsv[i][j][0];
			s = image_tsv[i][j][1];
			l = image_tsv[i][j][2];
			p_ent = (int)(h/60.0);
			n = p_ent%6;
			f = (h/60.0)-p_ent;
			p = l*(1.0-s);
			q = l*(1.0-f*s);
			u = l*(1.0-(1.0-f)*s);

			p*=255;
			l*=255;
			u*=255;
			q*=255;

			if(n==0)
			{
				image_rgb[i][j][0] = l;
				image_rgb[i][j][1] = u;
				image_rgb[i][j][2] = p;
			}
			else if(n==1)
			{
				image_rgb[i][j][0] = q;
				image_rgb[i][j][1] = l;
				image_rgb[i][j][2] = p;
			}
			else if(n==2)
			{
				image_rgb[i][j][0] = p;
				image_rgb[i][j][1] = l;
				image_rgb[i][j][2] = u;
			}
			else if(n==3)
			{
				image_rgb[i][j][0] = p;
				image_rgb[i][j][1] = q;
				image_rgb[i][j][2] = l;
			}
			else if(n==4)
			{
				image_rgb[i][j][0] = u;
				image_rgb[i][j][1] = p;
				image_rgb[i][j][2] = l;
			}
			else if(n==5)
			{
				image_rgb[i][j][0] = l;
				image_rgb[i][j][1] = p;
				image_rgb[i][j][2] = q;
			}

			//printf("%lf,%lf,%lf,%lf\n",l,u,p,q);

		}
	}
}

void gris(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_grise[TAILLE_MAX][TAILLE_MAX][3])
{
    int i,j,k,sum,avg;
    sum = 0;
	for(i=0;i<TAILLE_MAX;i++)
	{
		for(j=0;j<TAILLE_MAX;j++)
		{
			for(k=0;k<=2;k++)
			{
			    sum+=image_source[i][j][k];
			}
			avg = sum/3;
			image_grise[i][j][0] = avg;
			image_grise[i][j][1] = avg;
			image_grise[i][j][2] = avg;
			sum = 0;
		}
	}
}

void blackwhite(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_noire[TAILLE_MAX][TAILLE_MAX][3], int s)
{
    int i,j,k,moy,seuil = s;

    if(s == -1)
    {
        seuil = 0;
        for(i=0;i<TAILLE_MAX;i++)
        {
            for(j=0;j<TAILLE_MAX;j++)
            {
                for(k=0;k<3;k++)
                {
                    seuil += image_source[i][j][k];
                }

            }
        }

        seuil = seuil/(TAILLE_MAX*TAILLE_MAX*3);
    }

	for(i=0;i<TAILLE_MAX;i++)
	{
		for(j=0;j<TAILLE_MAX;j++)
		{

            moy = (image_source[i][j][0]+image_source[i][j][1]+image_source[i][j][2])/3;

		    if(moy <= seuil)
		    {
                image_noire[i][j][0] = 0;
                image_noire[i][j][1] = 0;
                image_noire[i][j][2] = 0;
		    }
		    else
		    {
                image_noire[i][j][0] = 255;
                image_noire[i][j][1] = 255;
                image_noire[i][j][2] = 255;
		    }

		}
	}
}

void test(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_test[TAILLE_MAX][TAILLE_MAX][3])
{
    int i,j;
    unsigned char imageflou[TAILLE_MAX][TAILLE_MAX][3];
    unsigned char imagenoire[TAILLE_MAX][TAILLE_MAX][3];

    copy(image_source,imageflou);
    copy(image_source,imagenoire);

    blur_count(imageflou,imageflou,5,5,4);
    blackwhite(imagenoire,imagenoire,-1);

	for(i=0;i<TAILLE_MAX;i++)
	{
		for(j=0;j<TAILLE_MAX;j++)
		{
		    if(imagenoire[i][j][0] == 0)
		    {
			image_test[i][j][0] = imageflou[i][j][0];
			image_test[i][j][1] = imageflou[i][j][1];
			image_test[i][j][2] = imageflou[i][j][2];
		    }
		    else
		    {
			image_test[i][j][0] = image_source[i][j][0];
			image_test[i][j][1] = image_source[i][j][1];
			image_test[i][j][2] = image_source[i][j][2];
		    }
		}
	}

}

void reverse(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_reverse[TAILLE_MAX][TAILLE_MAX][3])
{

    int i,j,k;
	for(i=0;i<TAILLE_MAX;i++)
	{
		for(j=0;j<TAILLE_MAX;j++)
		{
			for(k=0;k<=2;k++)
			{
			    image_reverse[i][j][k] = 255 - image_source[i][j][k];
			}

		}
	}
}

void hue(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_hue[TAILLE_MAX][TAILLE_MAX][3], double new_h)
{
	int i,j;
	double tsv[TAILLE_MAX][TAILLE_MAX][3];
	rgb_to_tsv(image_source,tsv);
	for(i=0;i<TAILLE_MAX;i++)
	{
		for(j=0;j<TAILLE_MAX;j++)
		{
			tsv[i][j][0] = new_h;
			//printf("%lf,%lf,%lf\n",tsv[i][j][0],tsv[i][j][1],tsv[i][j][2]);
		}
	}
	tsv_to_rgb(tsv,image_hue);
}

void sat(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_sat[TAILLE_MAX][TAILLE_MAX][3], int new_s)
{
	int i,j;
	double tsv[TAILLE_MAX][TAILLE_MAX][3];
	rgb_to_tsv(image_source,tsv);
	for(i=0;i<TAILLE_MAX;i++)
	{
		for(j=0;j<TAILLE_MAX;j++)
		{
			//printf("%lf,%lf,%lf\n",tsv[i][j][0],tsv[i][j][1],tsv[i][j][2]);
			//printf("%lf   ,   %lf",new_s,new_s/100.0);
			tsv[i][j][1] = new_s/100.0;
		}
	}
	tsv_to_rgb(tsv,image_sat);
}

void val(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_val[TAILLE_MAX][TAILLE_MAX][3], int new_v)
{
	int i,j;
	double tsv[TAILLE_MAX][TAILLE_MAX][3];
	rgb_to_tsv(image_source,tsv);
	for(i=0;i<TAILLE_MAX;i++)
	{
		for(j=0;j<TAILLE_MAX;j++)
		{
			tsv[i][j][2] = new_v/100.0;
		}
	}
	tsv_to_rgb(tsv,image_val);
}

void extrapol(unsigned char image_source[TAILLE_MAX][TAILLE_MAX][3], unsigned char image_extrapol[TAILLE_MAX][TAILLE_MAX][3], float u)
{
	unsigned char original[TAILLE_MAX][TAILLE_MAX][3];
	copy(image_source,original);
	int i,j,k,sumR,sumV,sumB,avg;
	sumR = 0;
	sumV = 0;
	sumB = 0;
	for(i=0;i<TAILLE_MAX;i++)
	{
		for(j=0;j<TAILLE_MAX;j++)
		{

			sumR += image_source[i][j][0];
			sumV += image_source[i][j][1];
			sumB += image_source[i][j][2];

		}
	}

	for(i=0;i<TAILLE_MAX;i++)
	{
		for(j=0;j<TAILLE_MAX;j++)
		{
			image_extrapol[i][j][0] = u*original[i][j][0]+(1.0-u)*(sumR/(TAILLE_MAX*TAILLE_MAX));
			image_extrapol[i][j][1] = u*original[i][j][1]+(1.0-u)*(sumV/(TAILLE_MAX*TAILLE_MAX));
			image_extrapol[i][j][2] = u*original[i][j][2]+(1.0-u)*(sumB/(TAILLE_MAX*TAILLE_MAX));
		}
	}

}

int ui_choice(char nom_fichier[30], char nom_out[30], unsigned char image[TAILLE_MAX][TAILLE_MAX][3])
{
    int load = chargeImage(nom_fichier,(unsigned char *)image);
    printf("\n");

    int choice;

    if (load == 1)
    {
        printf("(1)  Dupliquer l'image\n");
        printf("(2)  Reduire la taille de l'image en 256x256\n");
        printf("(3)  Appliquer un flou\n");
        printf("(4)  Passer l'image en niveaux de gris\n");
        printf("(5)  Passer l'image en noir et blanc\n");
        printf("(6)  Filtre test\n");
        printf("(7)  Inverser les couleurs\n");
        printf("(8)  Extrapoler l'image\n");
        printf("(9)  Changer la teinte\n");
        printf("(10) Changer la saturation\n");
        printf("(11) Changer l'intensite lumineuse\n");
        printf("(12) Flou directionnel\n");
        printf("(13) Agrandir l'image en 1024x1024\n\n");
        printf("Que voulez-vous faire : ");
        scanf("%d", &choice);
        printf("\n");

        if(choice == 1)
        {
            sauvegardeImage(nom_out,(unsigned char *)image,TAILLE_MAX,TAILLE_MAX);
        }

        else if(choice == 2)
        {
            unsigned char resized[SIZE][SIZE][3];

            char rep_blur[3];
            printf("\nSouhaitez-vous lisser l'image (oui/non) : ");
            scanf("%s",rep_blur);
            printf("\n");
            if (rep_blur[0] == 'o')
            {
                resize(image,resized,1);
            }
            else
            {
                resize(image,resized,0);
            }

            sauvegardeImage(nom_out,(unsigned char *)resized,SIZE,SIZE);
        }

        else if(choice == 3)
        {
            int taille,passe,type;
            printf("Taille de la zone du flou : ");
            scanf("%d",&taille);
            printf("Nombre de passe(s) : ");
            scanf("%d",&passe);
            printf("Type de flou (1: 1/log(x) || 2: 1/pow(2,x) || 3: 1/x || 4: 1) : ");
            scanf("%d",&type);
            blur_count(image,image,taille/2,passe,type);
            sauvegardeImage(nom_out,(unsigned char *)image,TAILLE_MAX,TAILLE_MAX);
        }

        else if(choice == 4)
        {
            gris(image,image);
            sauvegardeImage(nom_out,(unsigned char *)image,TAILLE_MAX,TAILLE_MAX);
        }

        else if(choice == 5)
        {
            int seuil;
            printf("Entrer le seuil entre 0 et 255 (ou -1: automatiqe) : ");
            scanf("%d",&seuil);
            blackwhite(image,image,seuil);
            sauvegardeImage(nom_out,(unsigned char *)image,TAILLE_MAX,TAILLE_MAX);
        }

        else if(choice == 6)
        {
            test(image,image);
            sauvegardeImage(nom_out,(unsigned char *)image,TAILLE_MAX,TAILLE_MAX);
        }

        else if(choice == 7)
        {
            reverse(image,image);
            sauvegardeImage(nom_out,(unsigned char *)image,TAILLE_MAX,TAILLE_MAX);
        }

        else if(choice == 8)
        {
            float extra;
            printf("Coefficient u : ");
            scanf("%f",&extra);
            extrapol(image,image,extra);
            sauvegardeImage(nom_out,(unsigned char *)image,TAILLE_MAX,TAILLE_MAX);
        }

        else if(choice == 9)
        {
            double teinte;
            printf("Valeur de la teinte : ");
            scanf("%lf",&teinte);
            hue(image,image,teinte);
            sauvegardeImage(nom_out,(unsigned char *)image,TAILLE_MAX,TAILLE_MAX);
        }

        else if(choice == 10)
        {
            int saturation;
            printf("Valeur de la saturation : ");
            scanf("%d",&saturation);
            sat(image,image,saturation);
            sauvegardeImage(nom_out,(unsigned char *)image,TAILLE_MAX,TAILLE_MAX);
        }

        else if(choice == 11)
        {
            int valeur;
            printf("Valeur de l'intensite lumineuse: ");
            scanf("%d",&valeur);
            val(image,image,valeur);
            sauvegardeImage(nom_out,(unsigned char *)image,TAILLE_MAX,TAILLE_MAX);
        }

        else if(choice == 12)
        {
            int dir,type2;
            printf("Puissance du flou directionnel : ");
            scanf("%d",&dir);
            printf("Type de flou (1: 1/log(x) || 2: 1/pow(2,x) || 3: 1/x || 4: 1) : ");
            scanf("%d",&type2);
            blur_dir(image,image,dir/2,type2);
            sauvegardeImage(nom_out,(unsigned char *)image,TAILLE_MAX,TAILLE_MAX);
        }

        else if(choice == 13)
        {
            unsigned char resized2[SIZE2][SIZE2][3];
            resize2(image,resized2);
            sauvegardeImage(nom_out,(unsigned char *)resized2,SIZE2,SIZE2);
        }

        else
        {
            printf("Cette action n'existe pas !\n");
            return 0;
        }

        return choice;
    }

    else
    {
        return 0;
    }

}


// -------------------------------------------------------------------------------------------------


int main(void)
{

    printf("############ TOOLKIT IMAGE ############\n");
    printf("#                                     #\n");
    printf("#           Projet de C par           #\n");
    printf("#         Antoine.D & Antoine.G       #\n");
    printf("#                                     #\n");
    printf("############## 2016/2017 ##############\n\n");

    char nom_fichier[30];
    char nom_out[30];
    unsigned char image[TAILLE_MAX][TAILLE_MAX][3];

    printf("Nom de l'image a modifier : ");
    scanf("%s",nom_fichier);
    printf("\n");
    printf("Nom de l'image modifiee : ");
    scanf("%s",nom_out);
    printf("\n");

    int choice = ui_choice(nom_fichier,nom_out,image);

    int continu = 1;

    while(continu == 1)
    {
        if(choice == 2 || choice == 13)
        {
            printf("\nVous ne pouvez pas modifier une image redimensionnee !\n");
            continu = 0;
        }

        else if(choice == 0)
        {
            continu = 0;
        }

        else
        {
            char rep[3];
            printf("\nSouhaitez-vous appliquer un autre filtre (oui/non) : ");
            scanf("%s",rep);
            printf("\n");
            if (rep[0] == 'o')
            {
                choice = ui_choice(nom_out,nom_out,image);
            }
            else
            {
                continu = 0;
            }
        }

    }

    printf("\nA bientot !\n");

}

