/****VER.2*****/

/*============================================================================
 * 1o trabalho                                                               *
 * Curso de Computacao Grafica 2004.2, prof. Marcelo Gattass.                *
 * Aluno(s): Mauricio Ferreira e Giovani Tadei                               *
 *===========================================================================*/


/**
 *	@file image.c Image: operações imagens. Escreve e le formato TGA.
 *
 *
 *	@author 
 *			- Marcelo Gattass
 *			- Maira Noronha
 *			- Thiago Bastos
 *
 *	@date
 *			Criado em:		 1 de Dezembro de 2002
 *			Última Modificação:	12 de Agosto de 2004
 *
 *	@version 3.0
 */


/*- Bibliotecas padrao usadas: --------------------------------------------*/
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

/*- Inclusao das bibliotecas IUP e CD: ------------------------------------*/
#include <iup.h>                    /* interface das funcoes IUP */
#include <iupgl.h>                  /* interface da funcao do canvas do OpenGL no IUP */
#include <windows.h>                /* inclui as definicoes do windows para o OpenGL */
#include <GL/gl.h>                  /* prototypes do OpenGL */
#include <GL/glu.h>                 /* prototypes do OpenGL */
#include "image.h"                  /* TAD para imagem */


#define ROUND(_) (int)floor( (_) + 0.5 )



/* Palheta Standard - Nome do arquivo */

#define ARQ_PALSTAND "Mauricio&Giovani.pal"



/************************************************************************/
/* Funcoes Exportadas                                                   */
/************************************************************************/


   typedef struct tgPalheta tpPalheta;
   typedef struct tgcolor   tpcolor ;

   int imgAlocaPalheta (tpPalheta ** pPalhetaAloc);
   int imgCriaPalheta();
   int imgPalStandard (void);
   Image * imgRmNearestColor(Image * img0);
   void imgAddPixel3fv(Image * image, int x, int y, float * color);
   int imgPalPopularity (Image * img);
   tpcolor * buscaColor (unsigned char rgb_desejada[3], tpcolor * palheta, unsigned int final);
   tpcolor * bubble_sort( tpcolor * palheta, int n );
   Image * imgNormalizeCube(Image * img0);
   Image * imgNormalizeLumMed(Image * img0);
   Image * imgNormalizeColors3(Image * img0);
   Image * imgNormalizeColorsDiff(Image * img);
   void imgRGBtoXYZ (float rgb[3], float XYZ[3]);
   void imgXYZtoLUV (float XYZ[3], float LUV[3]);
   void imgLUVtoXYZ (float LUV[3], float XYZ[3]);
   void imgXYZtoRGB (float XYZ[3], float rgb[3]);
   float imgGetLum (float rgb_ori[3], float rgb_viz[3]);
   int imgPalUniforme (Image * img);
   Image * imgRmErrorDiffusion(Image * img0);
   int imgPalMedianCut (Image * img);
   void DivideCubo (unsigned char r_min, unsigned char r_max, unsigned char g_min, unsigned char g_max, unsigned char b_min, unsigned char b_max, unsigned char*** cubo_rgb, int Divisoes);
 
/************************************************************************/
/* Variaveis Globais                                                    */
/************************************************************************/


   static tpPalheta * pPalheta = NULL;
   static float total_viz      = 0;  //Total de vizinhos de um mesmo objeto de um pixel
   static int diff             = 0; 
   /* Seleciona o metodo do diff 
     - 1 sem add
     - 2 com add
   */ 

  
   static tpPalheta * pPosPalheta = NULL;   //Posicao corrente na palheta (median cut)

/*********************************
  Struct Image
*********************************/


   struct Image_imp 
   {
     unsigned short width;
     unsigned short height;
     unsigned char  *buf;              
   };  


/*********************************
  Struct Palheta
*********************************/


	typedef struct tgPalheta 
	{
		unsigned char rgb[3];
		struct tgPalheta * pProx;

	} tpPalheta;  


/*********************************
  Struct Color
*********************************/

   typedef struct tgcolor 
	{
		unsigned char rgb[3];
		int qtde;

	} tpcolor ;  



/************************************************************************/
/* Definicao das Funcoes Privadas                                       */
/************************************************************************/

/*  getuint e putuint:
 * Funcoes auxiliares para ler e escrever inteiros na ordem (lo-hi)
 * Note que no Windows as variaveis tipo "unsigned short int" sao armazenadas 
 * no disco em dois bytes na ordem inversa.  Ou seja, o numero 400, 
 * por exemplo, que pode ser escrito como 0x190, fica armazenado em 
 * dois bytes consecutivos 0x90 e 0x01. Nos sistemas UNIX e Mac 
 * este mesmo inteiro seria armazenado na ordem 0x01 e 0x90.  
 *  O armazenamento do Windows e'  chamado de "little endian"  
 * (i.e., lowest-order byte stored first), e no sitemas Unix sao
 * "big-endian" (i.e., highest-order byte stored first). 
 */  

   static int getuint(unsigned short *uint, FILE *input)
   {
     int got;
     unsigned char temp[2];
     unsigned short tempuint;

     got = (int) fread(&temp, 1, 2, input);
     if (got != 2) return 0;

     tempuint = ((unsigned short)(temp[1])<<8) | ((unsigned short)(temp[0]));

     *uint = tempuint;

     return 1;
   }


   /********************************/

   static int putuint(unsigned short uint, FILE *output)
   {
     int put;
     unsigned char temp[2];

     temp[0] = uint & 0xff;
     temp[1] = (uint >> 8) & 0xff;

     put = (int) fwrite(&temp, 1, 2, output);
     if (put != 2) return 0;
 
     return 1;
   }

/************************************************************************/
/* Definicao das Funcoes Exportadas                                     */
/************************************************************************/

/* OBS: Nova Create definida abaixo */


   void imgDestroy (Image * image)
   {
      if (image)
      {
         if (image->buf) free (image->buf);
         free(image);
      }
   }


   /********************************/


   Image * imgCopy(Image * image)
   {
	   int w = imgGetWidth(image);
	   int h = imgGetHeight(image);
	   Image * img1=imgCreate(w,h);
	   int x,y;
	   unsigned char rgb[3];

	   for (y=0;y<h;y++){
		   for (x=0;x<w;x++) {
			   imgGetPixel3ubv(image,x,y,rgb);
			   imgSetPixel3ubv(img1,x,y,rgb);
		   }
	   }

	   return img1;
   }


   /********************************/


   Image * imgGrey(Image * image)
   {
	   int w = imgGetWidth(image);
	   int h = imgGetHeight(image);
	   Image * img1=imgCreate(w,h);
	   int x,y;
	   float rgb[3],grey[3];

	   for (y=0;y<h;y++){
		   for (x=0;x<w;x++) {
			   imgGetPixel3fv(image,x,y,rgb);
			   grey[0]=0.2999f*rgb[0]+0.587f*rgb[1]+0.114f*rgb[2];
			   grey[1]=grey[0];
			   grey[2]=grey[0];
			   imgSetPixel3fv(img1,x,y,grey);
		   }
	   }

	   return img1;
   }


   /********************************/


   Image * imgResize(Image * img0, int w1, int h1) 
   {
	   Image * img1 = imgCreate(w1,h1);
       float w0 = (float) img0->width;  /* passa para float para fazer contas */
       float h0 = (float) img0->height;

       int x0,y0,x1,y1;
	   unsigned char color[3];

	   for (y1=0;y1<h1;y1++)
		   for (x1=0;x1<w1;x1++)
		   {
			   x0=ROUND(w0*x1/w1);   /* pega a cor do pixel mais proxima */
			   y0=ROUND(h0*y1/h1);
			   imgGetPixel3ubv(img0,x0,y0,color);
			   imgSetPixel3ubv(img1,x1,y1,color);
		   }
		   return img1;
   }


   /********************************/


   int imgGetWidth(Image * image)
   {
	   return image->width;
   }


   /********************************/


   int imgGetHeight(Image * image)
   {
	   return image->height;
   }


   /********************************/


   unsigned char *imgGetRGBData(Image * image)
   {
       return image->buf;
   }


   /********************************/


   void imgSetPixel3fv(Image * image, int x, int y, float * color)
   {
      int pos = (y*image->width*3) + (x*3);

      image->buf[pos  ] = (unsigned char)(color[0] * 255);
      image->buf[pos+1] = (unsigned char)(color[1] * 255);
      image->buf[pos+2] = (unsigned char)(color[2] * 255);
   }


   /********************************/


   void imgGetPixel3fv(Image * image, int x, int y, float *color)
   {
      int pos = (y*image->width*3) + (x*3);

      color[0] = (float)(image->buf[pos]) / 255.0f;
      color[1] = (float)(image->buf[pos+1]) / 255.0f;
      color[2] = (float)(image->buf[pos+2]) / 255.0f;
   }


   /********************************/


   void imgSetPixel3ubv(Image * image, int x, int y, unsigned char * color)
   {
      int pos = (y*image->width*3) + (x*3);

      image->buf[pos  ] = color[0];
      image->buf[pos+1] = color[1];
      image->buf[pos+2] = color[2];
   }


   /********************************/


   void imgGetPixel3ubv(Image * image, int x, int y, unsigned char *color)
   {
      int pos = (y*image->width*3) + (x*3);

      color[0] = image->buf[pos  ];
      color[1] = image->buf[pos+1];
      color[2] = image->buf[pos+2];
   }


   Image * imgLoad (char *filename) 
   {
      FILE        *filePtr;

      Image       *image;             /* imagem a ser criada */

      unsigned char imageType;        /* 2 para imagens RGB */ 
      unsigned short int imageWidth;  /* largura da imagem */
      unsigned short int imageHeight; /* altura da imagem */
      unsigned char bitCount;         /* numero de bits por pixel */
 
      long        imageIdx;         /* contadore de laco */
      unsigned char colorSwap;      /* variavel para traca */
   
      unsigned char ucharSkip;      /* dado lixo unsigned char */
      short int     sintSkip;       /* dado lixo short int */

      /* abre o arquivo com a imagem TGA */
      filePtr = fopen(filename, "rb");
      assert(filePtr);

      /* pula os primeiros dois bytes que devem ter valor zero */
      ucharSkip = getc(filePtr); /* tamanho do descritor da imagem (0) */
      if (ucharSkip != 0) printf("erro na leitura de %s: imagem com descritor\n", filename);

      ucharSkip = getc(filePtr); 
      if (ucharSkip != 0) printf("erro na leitura de %s: imagem com tabela de cores\n", filename);
   
      /* le o tipo de imagem (que deve ser obrigatoriamente 2).  
         nao estamos tratando dos outros tipos */
      imageType=getc(filePtr);
      assert(imageType == 2);

      /* pula 9 bytes relacionados com a tabela de cores 
        (que nao existe quando a imagem e' RGB, imageType=2) */
      getuint((short unsigned int *)&sintSkip,filePtr);
      getuint((short unsigned int *)&sintSkip,filePtr);
      ucharSkip = getc(filePtr); 

      /* especificacao da imagem */
      getuint((short unsigned int *)&sintSkip,filePtr);      /* origem em x (por default = 0) */
      getuint((short unsigned int *)&sintSkip,filePtr);      /* origem em y (por default = 0) */ 
      getuint(&imageWidth,filePtr);   /* largura */
      getuint(&imageHeight,filePtr);   /* altura */

      /* read image bit depth */
      bitCount=getc(filePtr);
      assert(bitCount == 24);  /* colorMode -> 3 = BGR (24 bits) */

      /* read 1 byte of garbage data */
      ucharSkip = getc(filePtr); 

      /* cria uma instancia do tipo Imagem */
      image = imgCreate(imageWidth,imageHeight);
      assert(image);

      /* read in image data */
      fread(image->buf, sizeof(unsigned char), 3*imageWidth*imageHeight, filePtr);
   
      /* change BGR to RGB so OpenGL can read the image data */
      for (imageIdx = 0; imageIdx < 3*imageWidth*imageHeight; imageIdx += 3)
      {
         colorSwap = image->buf[imageIdx];
         image->buf[imageIdx] = image->buf[imageIdx + 2];
         image->buf[imageIdx + 2] = colorSwap;
      }

      fclose(filePtr);
      return image;
   }


   /********************************/


   int imgWriteTGA(char *filename, Image * image)
   {
      unsigned char imageType=2;      /* RGB(A) sem compressÃ£o */
      unsigned char bitDepth=24;      /* 24 bits por pixel */

      FILE         *filePtr;         /* ponteiro do arquivo */
      long        imageIdx;         /* indice para varrer os pixels */
      unsigned char colorSwap;      /* variavel temporaria para trocar de RGBA para BGRA */ 

      unsigned char byteZero=0;      /* usado para escrever um byte zero no arquivo */
      short int     shortZero=0;     /* usado para escrever um short int zero no arquivo */


      /* cria um arquivo binario novo */
      filePtr = fopen(filename, "wb");
      assert(filePtr);

      /* escreve o cabecalho */
      putc(byteZero,filePtr);     /* 0, no. de caracteres no campo de id da imagem */
      putc(byteZero,filePtr);     /* = 0, imagem nao tem palheta de cores */
      putc(imageType,filePtr);    /* = 2 -> imagem "true color" (RGB) */
      putuint(shortZero,filePtr); /* info sobre a tabela de cores (inexistente) */
      putuint(shortZero,filePtr);              /* idem */
      putc(byteZero,filePtr);                  /* idem */
      putuint(shortZero,filePtr);    /* =0 origem em x */
      putuint(shortZero,filePtr);    /* =0 origem em y */
      putuint(image->width,filePtr);   /* largura da imagem em pixels */
      putuint(image->height,filePtr);  /* altura da imagem em pixels */
      putc(bitDepth,filePtr);      /* numero de bits de um pixel */
      putc(byteZero, filePtr);   /* =0 origem no canto inf esquedo sem entrelacamento */

      /* muda os pixels de RGB para BGR */ 
      for (imageIdx = 0; imageIdx < 3*image->width*image->height ; imageIdx += 3) 
      {
         colorSwap = image->buf[imageIdx];
         image->buf[imageIdx] = image->buf[imageIdx + 2];
         image->buf[imageIdx + 2] = colorSwap;
      }

      /* escreve o buf de cores da imagem */
      fwrite(image->buf, sizeof(unsigned char), 3*image->width*image->height, filePtr);

      /* muda os pixels de BGR para RGB novamente */ 
      for (imageIdx = 0; imageIdx < 3*image->width*image->height ; imageIdx += 3) 
      {
         colorSwap = image->buf[imageIdx];
         image->buf[imageIdx] = image->buf[imageIdx + 2];
         image->buf[imageIdx + 2] = colorSwap;
      }

      fclose(filePtr);
      return 1;
   }



/* --- Funcoes do primeiro trabalho ----*/


   
	/**********************************************************************

	  Funcoes auxiliares criadas pelo Gattass

	**********************************************************************/


   void imgSub(Image *img0, Image *img1)
   {
	   int w = imgGetWidth(img0);
	   int h = imgGetHeight(img0);
	   int x,y;
	   unsigned char rgb0[3],rgb1[3];

	   for (y=0;y<h;y++){
		   for (x=0;x<w;x++) {
			   imgGetPixel3ubv(img0,x,y,rgb0);
			   imgGetPixel3ubv(img1,x,y,rgb1);
				   rgb0[0]=(rgb1[0]>rgb0[0])? rgb1[0]-rgb0[0] : rgb0[0]-rgb1[0] ;
				   rgb0[1]=(rgb1[1]>rgb0[1])? rgb1[1]-rgb0[1] : rgb0[1]-rgb1[1] ;
				   rgb0[2]=(rgb1[2]>rgb0[2])? rgb1[2]-rgb0[2] : rgb0[2]-rgb1[2] ;
			   imgSetPixel3ubv(img0,x,y,rgb0);
		   }
	   }
   }


   /********************************/


   float imgMed(Image * img)
   {
	   int w = imgGetWidth(img);
	   int h = imgGetHeight(img);
	   int x,y;
	   unsigned char rgb[3];
	   float  norm=0.0f;

	   for (y=0;y<h;y++){
		   for (x=0;x<w;x++) {
			   imgGetPixel3ubv(img,x,y,rgb);
			   norm += (rgb[0] + rgb[1] + rgb[2])/3.0f;
		   }
	   }
	   return norm/(w*h);
   }


/**************************************************************************
    -- PARTE I --

	Funcao:		imgCountColors

	Descricao:	Funcao para contar o numero de cores de uma imagem 
	          qualquer.

	Ideia:      Criar um cubo 256x256x256 que representa os eixos RED, GREEN
             e blue. Varer a imagem pixel a pixel e para cada cor encontrada
             da imagem setar o cubo criado com "1" no ponto dos eixos que 
             represente tal cor. Após esse processo basta varrer o cubo e 
             contar a quantidade de "1"s.
	
**************************************************************************/


   unsigned int imgCountColors(Image * img)
   {
	  int w = imgGetWidth(img);
	  int h = imgGetHeight(img);
      int r,g,b=0;
      int x,y;

      unsigned int total_cores=0;
      unsigned char rgb[3];
      

   /* Criando o cubo RGB */

      unsigned char * cubo_rgb [256][256];

      for (r=0; r<256; r++)
        for (g=0; g<256; g++)
       {
           cubo_rgb[r][g] = (char *) malloc (sizeof (char) * 256);
           memset ( cubo_rgb[r][g],  0,  256);
       }


   /* Setando os bits de cores usados para o Cubo */

	   for (y=0;y<h;y++)
      {
		   for (x=0;x<w;x++) 
         {
			   imgGetPixel3ubv(img,x,y,rgb);

            r = rgb[0];
            g = rgb[1];
            b = rgb[2];
            
            cubo_rgb[r][g][b] = '1';
		   }
	   }


   /* Contando Cores */

      for (r=0; r<256; r++)
        for (g=0; g<256; g++)
           for (b=0; b<256; b++)
           {
               if(cubo_rgb[r][g][b] == '1')
                  total_cores++;
           }


   /* Libreando Bloco */
           
       for (r=0; r<256; r++)
          for (g=0; g<256; g++)
             free(cubo_rgb[r][g]);

	   return total_cores;
   }



/**************************************************************************
    -- PARTE II --

	Funcao:		imgNormalizeColors
	Descricao:	Funcao que cria janela para a selecao do metodo de 
              normalizacao a ser usado.

	Ideia:      A idéia para se fazer uma normalizacao e' eliminar da imagem
              original o efeito de mudanças de iluminação.
	
**************************************************************************/


   Image * imgNormalizeColors(Image * img0)
   {
      int w = imgGetWidth(img0);
	   int h = imgGetHeight(img0);
	   Image *img1=imgCreate(w,h);


      switch (IupAlarm ("Selecionar Metodo de Normalizacao", 
           "== METODOS DE NORMALIZACAO ==", "RGB Normalizado (Norma)", "Luminancia Media", "+"))
      {
         case 1:
            img1 = imgNormalizeCube(img0);
         break;

         case 2:
            img1 = imgNormalizeLumMed(img0);
         break;

         case 3:
            
            switch (IupAlarm ("Selecionar Metodo de Normalizacao", 
                    "== LUMINANCIA PELOS VIZINHOS ==", "1) Sem adicao de luminancia", "2) Com adicao de luminancia", "Cancelar"))
              {
                case 1:
                  diff = 0;
                  img1 = imgNormalizeColorsDiff(img0);
                break ;

                case 2:
                  diff = 1;
                  img1 = imgNormalizeColorsDiff(img0);
                break ;

                case 3:
                   img1 = img0;
                break ;
              }

       break ;
     }

	   return img1;
   }



	/**********************************************************************
	
			===================================
				Normalizar: Cubo RGB / Norma 
			===================================

	  Funcao:  imgNormalizeCube

	  Ideia:   Para buscar uma normalizacao da imagem, utilizamos a norma dos
             vetores. Onde:
            
              r=RED  /sqrt(RED^2+GREEN^2+BLUE^2)
              g=GREEN/sqrt(RED^2+GREEN^2+BLUE^2)
              b=BLUE /sqrt(RED^2+GREEN^2+BLUE^2)

	**********************************************************************/


   Image * imgNormalizeCube(Image * img0)
   {
     int w = imgGetWidth(img0);
     int h = imgGetHeight(img0);
     Image *img1=imgCreate(w,h);
     int x,y;
     float rgb[3],norm[3];
     float norma;

     for (y=0;y<h;y++){
        for (x=0;x<w;x++) 
        {
           imgGetPixel3fv(img0,x,y,rgb);
           norma = (float) sqrt(pow(rgb[0], 2)+pow(rgb[1], 2)+pow(rgb[2], 2));
                  
           norm[0]=rgb[0]/norma;
           norm[1]=rgb[1]/norma;
           norm[2]=rgb[2]/norma;
           
           imgSetPixel3fv(img1,x,y,norm);
        }
     }

     return img1;
   }


	/**********************************************************************
	
			===================================
				Normalizar: Luminancia Media
			===================================

	  Funcao:  imgNormalizeLumMed

	  Ideia:   Varrer toda a imagem e obter o valor da luminancia media, 
             apartir dela setar para todos os pixeis o valor encontrado.

	**********************************************************************/


   Image * imgNormalizeLumMed(Image * img0)
   {
	   int w = imgGetWidth(img0);
	   int h = imgGetHeight(img0);
	   Image *img1=imgCreate(w,h);
	   int x,y;
	   float rgb[3];
      float rgb_xyz[3], rgb_luv[3];
      float luminancia=0;



	   for (y=0;y<h;y++)
      {
		   for (x=0;x<w;x++) 
         {
			   imgGetPixel3fv(img0,x,y,rgb);

            imgRGBtoXYZ (rgb,rgb_xyz);
            imgXYZtoLUV (rgb_xyz,rgb_luv);

            luminancia += rgb_luv[0];
		   }
	   }
      
      luminancia = luminancia / (w*h);
       //luminancia = -15.985f ;


      for (y=0;y<h;y++){
         for (x=0;x<w;x++) 
         {
            imgGetPixel3fv(img0,x,y,rgb);

            imgRGBtoXYZ (rgb,rgb_xyz);
            imgXYZtoLUV (rgb_xyz,rgb_luv);

            rgb_luv[0] = luminancia;

            imgLUVtoXYZ (rgb_luv, rgb_xyz);
            imgXYZtoRGB (rgb_xyz, rgb);
            
            imgSetPixel3fv(img1,x,y,rgb);
         }
      }
      
	   return img1;
   }

   
   /**********************************************************************
	
			========================================
				Normalizar: Luminancia por vizinhos
			========================================

	  Funcao:  imgNormalizeColorsDiff

	  Ideia:   Varrer cada pixel da imagem original olhando os seus 8 
             vizinhos e pegando a media da luminancia dos pixels que 
             pertencem a mesma figura, e com essa luminancia alterar o
             pixel atual.
              Para esse método foi feito duas verões:
              
                1) Sem adicao de luminancia da img original, ou seja,
                cada media é calculada sem se basear na alteracao do
                pixel anterior.

                2) Com adicao de luminancia, ou seja, para cada alteracao
                o novo valor de luminancia ja' entra na nova conta da 
                media.

	**********************************************************************/




   Image * imgNormalizeColorsDiff(Image * img)
   {
      int w = imgGetWidth(img);
      int h = imgGetHeight(img);
      int x,y;
      
      Image *img1=imgCreate(w,h);
      Image *img2=imgCreate(w,h); //para retirar lum

      float luminancia=0;
      float rgb_xyz[3], rgb_luv[3];
      float rgb_ori[3], rgb_viz[3];

      total_viz=0;

 
      /* Copiando imagem */
      for (y=0; y<h; y++)
         for (x=0; x<w; x++)
         {
            imgGetPixel3fv(img,x,y,rgb_ori);
            imgSetPixel3fv(img1,x,y,rgb_ori);
         }


	   for (y=1; y<h-1; y++)
      {
		   for (x=1; x<w-1; x++) 
         {
            imgGetPixel3fv(img,x,y,rgb_ori);

            /* Vizinho 1 */

            imgGetPixel3fv(img,x-1,y+1,rgb_viz);
            luminancia += imgGetLum (rgb_ori, rgb_viz);

            /* Vizinho 2 */
            imgGetPixel3fv(img,x,y+1,rgb_viz);
            luminancia += imgGetLum (rgb_ori, rgb_viz);
            
            /* Vizinho 3 */ 
            imgGetPixel3fv(img,x+1,y+1,rgb_viz);
            luminancia += imgGetLum (rgb_ori, rgb_viz);

            /* Vizinho 4 */
            imgGetPixel3fv(img,x-1,y,rgb_viz);
            luminancia += imgGetLum (rgb_ori, rgb_viz);

            /* Vizinho 5 */
            imgGetPixel3fv(img,x+1,y,rgb_viz);
            luminancia += imgGetLum (rgb_ori, rgb_viz);

            /* Vizinho 6 */
            imgGetPixel3fv(img,x-1,y-1,rgb_viz);
            luminancia += imgGetLum (rgb_ori, rgb_viz);

            /* Vizinho 7 */
            imgGetPixel3fv(img,x,y-1,rgb_viz);
            luminancia += imgGetLum (rgb_ori, rgb_viz);

            /* Vizinho 8 */
            imgGetPixel3fv(img,x+1,y-1,rgb_viz);
            luminancia += imgGetLum (rgb_ori, rgb_viz);


            /* Fazer media das luminancias */

            luminancia = luminancia/total_viz;


            /* Passar pixel atual para formato de luminancia
             e mudar sua luminancia para o valor encontrado */

            imgRGBtoXYZ (rgb_ori,rgb_xyz);
            imgXYZtoLUV (rgb_xyz,rgb_luv);

            rgb_luv[0] = luminancia;

            imgLUVtoXYZ (rgb_luv, rgb_xyz);
            imgXYZtoRGB (rgb_xyz, rgb_ori);

            imgSetPixel3fv(img1,x,y,rgb_ori);

            

            imgAddPixel3fv(img,x,y,rgb_ori);

            /* Corrigir luminosidade */
            imgGetPixel3fv(img,x,y,rgb_ori);

			   rgb_ori[0] = rgb_ori[0] / 2.0f ;
			   rgb_ori[1] = rgb_ori[1] / 2.0f ;
			   rgb_ori[2] = rgb_ori[2] / 2.0f ;

			   imgSetPixel3fv(img2,x,y,rgb_ori);


            
            /* Repetir para cada pixel */

            luminancia = 0;
            total_viz  = 0;

		   }
	   }

      if (diff == 0)
       return img1;

      if (diff == 1)
       return img2;

      //erro?
      return NULL;
   }


	/**********************************************************************

	  Funcoes auxiliares usadas no processo de normalizacao

	**********************************************************************/



    float imgGetLum (float rgb_ori[3], float rgb_viz[3])
    {
       float erro = 50.0f/255.0f; /* valor de diff entre pixels para estar no mesmo obj */

       float r_ori,g_ori,b_ori=0;
       float r_viz,g_viz,b_viz=0;
       float luminancia=0;
       
       float rgb_xyz[3], rgb_luv[3];


      /* Se a cor for parecida com o do pixel original,
      "significa" q esse vizinho pertence ao mesmo objeto
      entao pega-se a luminancia desse pixel para fazer a 
      media dos vizinhos */
       
       r_ori = rgb_ori[0];
       g_ori = rgb_ori[1];
       b_ori = rgb_ori[2];
       
       r_viz = rgb_viz[0];
       g_viz = rgb_viz[1];
       b_viz = rgb_viz[2];


      /* Verificando se a cor é parecida - Criterio +- 20?? */
      

      if ( (fabs(r_viz-r_ori) <= erro) && 
           (fabs(g_viz-g_ori) <= erro) && 
           (fabs(b_viz-b_ori) <= erro)) 
      {
         /* Pega a sua luminancia */
         
         imgRGBtoXYZ (rgb_viz,rgb_xyz);
         imgXYZtoLUV (rgb_xyz,rgb_luv);

         imgLUVtoXYZ (rgb_luv, rgb_xyz);
         imgXYZtoRGB (rgb_xyz, rgb_viz);

         luminancia = rgb_luv[0];
         
         /* Adicona o total de vizinhos para fazer a media */

         total_viz ++;
         
      }
      
      return luminancia;
    }


   /********************************/

   void imgRGBtoXYZ (float rgb[3], float XYZ[3])
   {
      float X, Y, Z ;

      X = 0.412f * rgb[0] + 0.358f * rgb[1] + 0.180f * rgb[2];
      Y = 0.213f * rgb[0] + 0.715f * rgb[1] + 0.072f * rgb[2];
      Z = 0.019f * rgb[0] + 0.119f * rgb[1] + 0.950f * rgb[2];

      XYZ[0] = X;
      XYZ[1] = Y;
      XYZ[2] = Z;

      return ;
   }


   /********************************/

   void imgXYZtoLUV (float XYZ[3], float LUV[3])
   {

     float X, Y, Z ;
     float var_U, var_V, var_Y ;
     float ref_X, ref_Y, ref_Z, ref_U, ref_V ;
     float CIE_L, CIE_u, CIE_v ;

     X = XYZ[0];
     Y = XYZ[1];
     Z = XYZ[2];


     var_U = ( 4.0f * X ) / ( X + ( 15.0f * Y ) + ( 3.0f * Z ) ) ;
     var_V = ( 9.0f * Y ) / ( X + ( 15.0f * Y ) + ( 3.0f * Z ) ) ;
     var_Y = Y / 100.0f ;


      if ( var_Y > 0.008856f )
         var_Y = (float) pow(var_Y,  1.0f/3.0f ) ;
      
      else
         var_Y = ( 7.787f * var_Y ) + ( 16.f / 116.f ) ;


      ref_X =  95.047f ;        //Observer= 2°, Illuminant= D65
      ref_Y = 100.000f ;
      ref_Z = 108.883f ;


      ref_U = ( 4.0f * ref_X ) / ( ref_X + ( 15.0f * ref_Y ) + ( 3.0f * ref_Z ) ) ;
      ref_V = ( 9.0f * ref_Y ) / ( ref_X + ( 15.0f * ref_Y ) + ( 3.0f * ref_Z ) ) ;


      CIE_L = ( 116.0f * var_Y ) - 16.0f ;
      CIE_u = 13.0f * CIE_L * ( var_U - ref_U ) ;
      CIE_v = 13.0f * CIE_L * ( var_V - ref_V ) ;
      

      LUV[0] = CIE_L;
      LUV[1] = CIE_u;  
      LUV[2] = CIE_v;
      
      return;
            
   }

   
   /********************************/

   void imgLUVtoXYZ (float LUV[3], float XYZ[3])
   {
      float X, Y, Z ;
      float var_U, var_V, var_Y ;
      float ref_X, ref_Y, ref_Z, ref_U, ref_V ;
      float CIE_L, CIE_u, CIE_v ;

      CIE_L = LUV[0];
      CIE_u = LUV[1]; 
      CIE_v = LUV[2];


      var_Y = ( CIE_L + 16.0f ) / 116.0f ;

      if ( pow(var_Y, 3) > 0.008856f )
         var_Y = (float) pow(var_Y, 3) ;

      else
         var_Y = ( var_Y - 16.0f / 116.0f ) / 7.787f ;


      ref_X =  95.047f ;     //Observer= 2°, Illuminant= D65
      ref_Y = 100.000f ;
      ref_Z = 108.883f ;


      ref_U = ( 4.0f * ref_X ) / ( ref_X + ( 15.0f * ref_Y ) + ( 3.0f * ref_Z ) ) ;
      ref_V = ( 9.0f * ref_Y ) / ( ref_X + ( 15.0f * ref_Y ) + ( 3.0f * ref_Z ) ) ;


      var_U = CIE_u / ( 13.0f * CIE_L ) + ref_U ;
      var_V = CIE_v / ( 13.0f * CIE_L ) + ref_V ;


      Y = var_Y * 100.0f ;
      X =  - ( 9.0f * Y * var_U ) / ( ( var_U - 4.0f ) * var_V  - var_U * var_V );
      Z =    ( 9.0f * Y - ( 15.0f * var_V * Y ) - ( var_V * X ) ) / ( 3.0f * var_V );


      XYZ[0] = X;
      XYZ[1] = Y;
      XYZ[2] = Z;

      return;
   }

   /********************************/


   void imgXYZtoRGB (float XYZ[3], float rgb[3])
   {

     float X, Y, Z ;

     X = XYZ[0];
     Y = XYZ[1];
     Z = XYZ[2];

     rgb[0] = ( 3.240f * X + -1.537f * Y + -0.499f * Z) ;
     rgb[1] = (-0.969f * X +  1.876f * Y +  0.042f * Z) ;
     rgb[2] = ( 0.056f * X + -0.200f * Y +  1.057f * Z) ;

     return;

   }



/*************************************************************************
    -- PARTE III --

	Funcao:		imgReduce256Colors

	Descricao:	Funcao para reduzir cores de uma imagem para 256 cores

	Ideia: PALHETA(1) Selecionar uma palheta de 256 cores para representar 
	       a imagem escolhida.
		       Exs: - Standard/Web-Safe
			         - Popularity
					   - Median Cut
					   - Octree

		   MODO DE REDUCAO(2) A partir da palheta selecionada mapear 
		   para a imagem dada, ou seja, escolher para cada pixel da imagem 
		   uma cor da palheta que melhor o representa.
			    Exs: - Cor mais proxima (norma euclediana)
			         - Error Diffusion
					   - Ordered Dithering
	
**************************************************************************/


	Image * imgReduce256Colors(Image * img0)
	{
      int erro=0;
      int w = imgGetWidth(img0);
	   int h = imgGetHeight(img0);
	   Image *img1=imgCreate(w,h);

  
     switch (
        IupAlarm ("Selecionar metodo de reducao de cores",
       "==  PALHETA STANDARD (Web)  ==", "Nearest Color", "Error Diffusion (Floyd Steinberg)", "+"))
     {
       case 1:
         erro = imgPalStandard();
         img1 = imgRmNearestColor(img0);
       break ;

       case 2:
         erro = imgPalStandard();
         img1 =imgRmErrorDiffusion(img0);
       break ;

       case 3:
          
          switch (IupAlarm ("Selecionar metodo de reducao de cores", 
           "== PALHETA POR POPULARIDADE ==", "Nearest Color", "Error Diffusion (Floyd Steinberg)", "+"))
         {
             case 1:
               erro = imgPalPopularity(img0);
               img1 = imgRmNearestColor(img0);
             break ;

             case 2:
               erro = imgPalPopularity(img0);
               img1 = imgRmErrorDiffusion(img0);
             break ;

             case 3:
          
                switch (IupAlarm ("Selecionar metodo de reducao de cores", 
                 "== PALHETA POR UNIFORMIDADE ==", "Nearest Color", "Error Diffusion (Floyd Steinberg)", "+"))
                {
                  case 1:
                     erro = imgPalUniforme(img0);
                     img1 = imgRmNearestColor(img0);
                  break ;

                  case 2:
                     erro = imgPalUniforme(img0);
                     img1 = imgRmErrorDiffusion(img0);
                  break ;

                  case 3:
                     switch (IupAlarm ("Selecionar metodo de reducao de cores", 
                      "== PALHETA POR CORTE MEDIANO ==", "Nearest Color", "Error Diffusion (Floyd Steinberg)", "Cancelar"))
                     {
                        case 1:
                           erro = imgPalMedianCut(img0);
                           img1 = imgRmNearestColor(img0);
                        break ;

                        case 2:
                           erro = imgPalMedianCut(img0);
                           img1 = imgRmErrorDiffusion(img0);
                        break ;

                        case 3:
                           img1=img0;
                        break ; 
                     }
                  break ; 
               }
            break ;
          }
          break ;
     }
     
     return img1;
   
   }




	/**********************************************************************
	
			===================================
				Palheta: Standard/Web-Safe
			===================================

	  Funcao:  imgPalStandard

	  Ideia:   Ler as cores atraves do arquivo de palhetas Standard e
	        completar cada no' da estrutura de palhetas com uma cor 
			  diferente.

	**********************************************************************/


	int imgPalStandard (void)
	{
		int i,erro=0;
		unsigned int r=0,b=0,g=0;
		char buffer[13];
		char msgErro[200];
		
		tpPalheta * pPalhetaAux;

	    
		/* Abrindo arquivo .pal de palheta Standard */

		FILE *input = fopen( ARQ_PALSTAND, "r" ); 
											
		if ( input == NULL )
		{
		  sprintf(msgErro, "Nao foi possivel abrir o arquivo de palheta Standard.\nVerifique se o arquivo %s esta' no diretorio corrente.", ARQ_PALSTAND);
          IupMessage("Erro Encontrado!", msgErro);
		  return 1;
		}


		/* Criando o contexto da estrutura de palheta */

		erro = imgCriaPalheta();

		if ( erro == 1 )
		{
		  sprintf(msgErro,"Memoria insuficiente para realizar o processo.");
          IupMessage("Erro Encontrado!", msgErro);
		  return 1;
		}

		/* Preenchendo a palheta com os valores do arquivo */

		pPalhetaAux = pPalheta;

		for(i=0 ; i<255 ; i++)
		{	
			fgets( buffer, 15, input );
			sscanf(buffer,"%d%d%d",&r,&g,&b);

			pPalhetaAux->rgb[0] = r;
			pPalhetaAux->rgb[1] = g;
			pPalhetaAux->rgb[2] = b;

			pPalhetaAux = pPalhetaAux->pProx;
		}
		
		fclose (input);
      return 0;
	}


   

	/**********************************************************************
	
			===================================
				Palheta: Popularidade
			===================================

	  Funcao:  imgPalPopularity

	  Ideia:   Varer toda a imagem e buscar as 256 cores mais populares da
             imagem.

	**********************************************************************/



   int imgPalPopularity (Image * img)
	{    
      unsigned int i;
      unsigned int total_colors = imgCountColors(img);
      int w = imgGetWidth(img);
	   int h = imgGetHeight(img);
      int erro=0;
      int x,y;
      unsigned char rgb[3];
      char msgErro[200];
      //tpCubo cubo_rgb;


		tpPalheta * pPalhetaAux  = NULL;
      tpPalheta * pPalhetaCorr = NULL;

      tpcolor * palheta_img;
      tpcolor * palheta_aux;

   
      //image->buf = (unsigned char *) malloc (w * h * 3);
      palheta_img = (tpcolor *) calloc (total_colors,(sizeof(struct tgcolor)));
      palheta_aux = palheta_img ;



		for (y=0;y<h;y++){
			for (x=0;x<w;x++) 
			{
				imgGetPixel3ubv(img,x,y,rgb);
            
            palheta_aux = buscaColor (rgb, palheta_img,total_colors);

            //tratar null?
         }
      }

      palheta_img = bubble_sort(palheta_img,total_colors);



      /* Criando o contexto da estrutura de palheta */

		erro = imgCriaPalheta();

		if ( erro == 1 )
		{
		  sprintf(msgErro,"Memoria insuficiente para realizar o processo.");
          IupMessage("Erro Encontrado!", msgErro);
		  return 1;
		}


	/* Preenchendo a palheta com os valores do arquivo */

		pPalhetaAux = pPalheta;


      if (total_colors <= 256)
      {
         for(i=total_colors-1 ; i > 0 ; i--)
		   {	
         
			   pPalhetaAux->rgb[0] = (palheta_img + i)->rgb[0];
			   pPalhetaAux->rgb[1] = (palheta_img + i)->rgb[1];
			   pPalhetaAux->rgb[2] = (palheta_img + i)->rgb[2];

			   pPalhetaAux = pPalhetaAux->pProx;
		   }
      
      }

      else
      {
		   for(i=total_colors-1 ; i >total_colors-256 ; i--)
		   {	
         
			   pPalhetaAux->rgb[0] = (palheta_img + i)->rgb[0];
			   pPalhetaAux->rgb[1] = (palheta_img + i)->rgb[1];
			   pPalhetaAux->rgb[2] = (palheta_img + i)->rgb[2];

			   pPalhetaAux = pPalhetaAux->pProx;
		   }
      }
		
      return 0;
   }



	/**********************************************************************
	
			===================================
				Palheta: Uniforme
			===================================

	  Funcao:  imgPalUniforme

	  Ideia:   Quantização Uniforme:

               O espaço de cores é dividido uniformemente em:
               - 8 divisões em RED
               - 8 divisões em GREEN
               - 4 divisões em BLUE (Olhos humanos são menos sensíveis 
               as variações do azul)
               
               [  8x8x4 = 256 regiões  ]
               
               Cada região definirá uma cor representativa, média das cores 
            mapeadas para aquela região.

	**********************************************************************/




   int imgPalUniforme (Image * img)
   {
      int w = imgGetWidth(img);
	   int h = imgGetHeight(img);
      int x,y,erro=0;
      unsigned int total_cores=0, total_red=0, total_green=0, total_blue=0;

      unsigned int r,g,b=0;
      unsigned int r_color,g_color,b_color=0;
      unsigned char rgb[3], rgb_result[3];
      
      unsigned char * cubo_rgb [256][256];
  
		char msgErro[200];


		tpPalheta * pPalhetaAux;



	/* Criando o contexto da estrutura de palheta */

		erro = imgCriaPalheta();

		if ( erro == 1 )
		{
		  sprintf(msgErro,"Memoria insuficiente para realizar o processo.");
          IupMessage("Erro Encontrado!", msgErro);
		  return 1;
		}

      pPalhetaAux = pPalheta;


   /* Criando o cubo RGB */


      for (x=0; x<256; x++)
        for (y=0; y<256; y++)
       {
           cubo_rgb[x][y] = (char *) malloc (sizeof (char) * 256);
           memset ( cubo_rgb[x][y],  0,  256);
       }


   /* Setando os bits de cores usados para o Cubo */

	   for (y=0;y<h;y++)
      {
		   for (x=0;x<w;x++) 
         {
			   imgGetPixel3ubv(img,x,y,rgb);

            r = rgb[0];
            g = rgb[1];
            b = rgb[2];
            
            cubo_rgb[r][g][b] = '1';

		   }
	   }


   /*  */

      for (b=0; b<=192; b += 64)
      {
        for (r=0; r<=224; r += 32 )
        {
           for (g=0; g<=224; g += 32)
           {

                 //Pega um intervalo de cores e retorna a media representativa


               for(b_color=b; b_color<b+64; b_color++)
                  for(r_color=r; r_color<r+32; r_color++)
                     for(g_color=g; g_color<g+32; g_color++)
                     {
         
                        if(cubo_rgb[r_color][g_color][b_color] == '1')
                        {
                           total_red   += r_color;
                           total_green += g_color;
                           total_blue  += b_color;

                           total_cores++;
                        }
                     }


               if (total_cores != 0)
               //Calculando a media
               {
                  rgb_result[0] = total_red   / total_cores;
                  rgb_result[1] = total_green / total_cores;
                  rgb_result[2] = total_blue  / total_cores;
               

                  /* Preenchendo a palheta */

			         pPalhetaAux->rgb[0] = rgb_result[0];
			         pPalhetaAux->rgb[1] = rgb_result[1];
			         pPalhetaAux->rgb[2] = rgb_result[2];
                  
                  pPalhetaAux = pPalhetaAux->pProx;
               }

               //zerando
			      
               total_red   = 0;
               total_green = 0;
               total_blue  = 0;
               total_cores = 0;
           }
        }
      }

      return 0;
  
   }




	/**********************************************************************
	
			===================================
				Palheta: Median Cut
			===================================

	  Funcao:  imgPalMedianCut

	  Ideia:   

	**********************************************************************/



        int imgPalMedianCut (Image * img)
        {
                int i=0;
                int w = imgGetWidth(img);
                int h = imgGetHeight(img);
                
                int x,y, Divisoes, rint, gint;
                
                unsigned int total_cores=0;
                unsigned char rgb[3], r, g, b, r_min = 255, r_max = 0, g_min = 255, g_max = 0, b_min = 255, b_max = 0;
                
                
                int aresta_r=0,aresta_g=0,aresta_b=0, aresta=0; 
                
                
                unsigned char *** cubo_rgb;


      
                imgCriaPalheta();
                
                /* Criando o cubo RGB */
                
                cubo_rgb = (char ***) malloc (sizeof (char**) * 256);


                for (rint = 0 ; rint < 256 ; rint++)
                {
                        cubo_rgb[rint] = (char **) malloc (sizeof (char*) * 256);
                }


                
                for (rint=0; rint<256; rint++)
                        for (gint=0; gint<256; gint++)
                        {
                                cubo_rgb[rint][gint] = (char *) malloc (sizeof (char) * 256);
                                memset ( cubo_rgb[rint][gint],  0,  256);
                        }
                
                /* Passo 0: Buscar os MAX e MIN */
                
                for (y=0;y<h;y++)
                {
                        for (x=0;x<w;x++) 
                        {
                                imgGetPixel3ubv(img,x,y,rgb);
                                
                                r = rgb[0];
                                g = rgb[1];
                                b = rgb[2];
                                
                                /* Procurando Maximos */
                                if(r > r_max)
                                        r_max = r;
                                
                                if(g > g_max)
                                        g_max = g;
                                
                                if(b > b_max)
                                        b_max = b;
                                
                                
                                /* Procurando Minimos */
                                if(r < r_min)
                                        r_min = r;
                                
                                if(g < g_min)
                                        g_min = g;
                                
                                if(b < b_min)
                                        b_min = b;
                                
                                cubo_rgb[r][g][b] += 1;
                        }
                }
                
                Divisoes = 8 ; // Numero de vezes que o cubo sera recursivamente dividido em 2 (2^8 = 256)
                pPosPalheta = pPalheta ;
                DivideCubo (r_min, r_max, g_min, g_max, b_min, b_max, cubo_rgb, Divisoes) ;
                
                return 0;
        }
        
        


        
        
        
        
        
        
        void DivideCubo (unsigned char r_min, unsigned char r_max, unsigned char g_min, unsigned char g_max, unsigned char b_min, unsigned char b_max, unsigned char*** cubo_rgb, int Divisoes)
        {
                int ArestaR = (r_max - r_min), ArestaG = (g_max - g_min), ArestaB = (b_max - b_min) ;
                int Aresta, posicao, midpoint ;
                unsigned char r, g, b, GuardaCor[256] ;
                int rint, gint, bint ;
                
                
                if (Divisoes == 0)
                {
                        r = r_min + ((r_max - r_min) /2) ;
                        g = g_min + ((g_max - g_min) /2) ;
                        b = b_min + ((b_max - b_min) /2) ;


                        pPosPalheta->rgb[0] = r ;
                        pPosPalheta->rgb[1] = g ;
                        pPosPalheta->rgb[2] = b ;


                        pPosPalheta = pPosPalheta->pProx ;


                }
                else
                {
                        
                        if (ArestaR > ArestaG)
                        {
                                if (ArestaR > ArestaB)
                                {
                                        Aresta = 0 ;
                                }
                                
                                else
                                {
                                        Aresta = 2 ;
                                }
                        }
                        else
                        {
                                if (ArestaB > ArestaG)
                                {
                                        Aresta = 2 ;
                                }
                                else
                                {
                                        Aresta = 1 ;
                                }
                        }
                        
                        
                        posicao = 0 ;
                        if (Aresta == 0) // aresta r
                        {
                                for (rint = r_min ; rint <= r_max ; rint++)
                                {
                                        for (gint = g_min ; gint <= g_max ; gint++)
                                        {
                                                for (bint = b_min ; bint <= b_max ; bint++)
                                                {
                                                        if (cubo_rgb[rint][gint][bint] != 0)
                                                                if (posicao == 0)
                                                                {
                                                                        GuardaCor[posicao] = rint ;
                                                                        posicao++ ;
                                                                }
                                                                else
                                                                        if(GuardaCor[posicao - 1] != rint)
                                                                        {
                                                                                GuardaCor[posicao] = rint ;
                                                                                posicao ++ ;
                                                                        }
                                                }
                                        }
                                }
                                if (posicao > 1)
                                {
                                        Divisoes-- ;
                                        midpoint = (posicao / 2) ;
                                        DivideCubo(GuardaCor[0], GuardaCor[midpoint - 1], g_min, g_max, b_min, b_max, cubo_rgb, Divisoes) ; 
                                        DivideCubo(GuardaCor[midpoint], GuardaCor[posicao - 1], g_min, g_max, b_min, b_max, cubo_rgb, Divisoes) ;
                                }
                                else
                                {
                                        pPosPalheta->rgb[0] = r_min ;
                                        pPosPalheta->rgb[1] = g_min ;
                                        pPosPalheta->rgb[2] = b_min ;
                                        pPosPalheta = pPosPalheta->pProx ;
                                }
                                
                                
                                
                                
                                
                        }
                        
                        if (Aresta == 1) // aresta g
                        {
                                for (gint = g_min ; gint <= g_max ; gint++)
                                {
                                        for (rint = r_min ; rint <= r_max ; rint++)
                                        {
                                                for (bint = b_min ; bint <= b_max ; bint++)
                                                {
                                                        if (cubo_rgb[rint][gint][bint] != 0)
                                                                if (posicao == 0)
                                                                {
                                                                        GuardaCor[posicao] = gint ;
                                                                        posicao++ ;
                                                                }
                                                                else
                                                                        if(GuardaCor[posicao - 1] != gint)
                                                                        {
                                                                                GuardaCor[posicao] = gint ;
                                                                                posicao ++ ;
                                                                        }                                       
                                                }
                                        }
                                }
                                if (posicao > 1)
                                {
                                        Divisoes-- ;
                                        midpoint = (posicao / 2) ;
                                        DivideCubo(r_min, r_max, GuardaCor[0], GuardaCor[midpoint - 1], b_min, b_max, cubo_rgb, Divisoes) ; 
                                        DivideCubo(r_min, r_max, GuardaCor[midpoint], GuardaCor[posicao - 1], b_min, b_max, cubo_rgb, Divisoes) ;
                                }
                                else
                                {
                                        pPosPalheta->rgb[0] = r_min ;
                                        pPosPalheta->rgb[1] = g_min ;
                                        pPosPalheta->rgb[2] = b_min ;
                                        pPosPalheta = pPosPalheta->pProx ;
                                }
                        }
                        
                        if (Aresta == 2) // aresta b
                        {
                                for (bint = b_min ; bint <= b_max ; bint++)
                                {
                                        for (gint = g_min ; gint <= g_max ; gint++)
                                        {
                                                for (rint = r_min ; rint <= r_max ; rint++)
                                                {
                                                        if (cubo_rgb[rint][gint][bint] != 0)
                                                                if (posicao == 0)
                                                                {
                                                                        GuardaCor[posicao] = bint ;
                                                                        posicao++ ;
                                                                }
                                                                else
                                                                        if(GuardaCor[posicao - 1] != bint)
                                                                        {
                                                                                GuardaCor[posicao] = bint ;
                                                                                posicao ++ ;
                                                                        }
                                                }
                                        }
                                }
                                if (posicao > 1)
                                {
                                        Divisoes-- ;
                                        midpoint = (posicao / 2) ;
                                        DivideCubo(r_min, r_max, g_min, g_max, GuardaCor[0], GuardaCor[midpoint - 1], cubo_rgb, Divisoes) ; 
                                        DivideCubo(r_min, r_max, g_min, g_max, GuardaCor[midpoint], GuardaCor[posicao - 1], cubo_rgb, Divisoes) ;
                                }
                                else
                                {
                                        pPosPalheta->rgb[0] = r_min ;
                                        pPosPalheta->rgb[1] = g_min ;
                                        pPosPalheta->rgb[2] = b_min ;
                                        pPosPalheta = pPosPalheta->pProx ;
                                }
                        }
                }
        }


        



	/**********************************************************************

	  Funcoes auxiliares usadas no processo de reducao de cores (palhetas)

	**********************************************************************/


   
   int imgCriaPalheta()
	{
		int i,erro=0;
		tpPalheta * pPalhetaAux  = NULL;
      tpPalheta * pPalhetaCorr = NULL;
	
      if(pPalheta == NULL)
      {
		   for(i=0 ; i<256 ; i++)  //colocar EOF?
		   {	
			   pPalhetaAux = (tpPalheta *) malloc (1 * (sizeof (tpPalheta)));

            /* Tratar falta de memoria */

            if (pPalhetaAux == NULL)
            {
               //Valor Erro = 1
               return 1; 
            }

            if(pPalhetaCorr == NULL)
            {
               pPalheta     = pPalhetaAux;
               pPalhetaCorr = pPalhetaAux;
            }
				       
			   pPalhetaAux->rgb[0] = 0;
			   pPalhetaAux->rgb[1] = 0;
			   pPalhetaAux->rgb[2] = 0;
            pPalhetaAux->pProx = NULL;

            if(pPalhetaCorr != pPalhetaAux )
            {
               pPalhetaCorr->pProx = pPalhetaAux;
               pPalhetaCorr = pPalhetaAux;
            }
         }
      }

		return 0; 
	}


   /********************************/


   tpcolor * bubble_sort( tpcolor * palheta, int n )
   {

      int i,j;
      unsigned int qtde;
      unsigned char rgb[3];

      //tpcolor * palheta_aux ;
   
      //palheta_aux = palheta;

      for(i=0;i<n;i++)
      {
         for(j=i+1;j<n;j++)
         {
            //palheta_aux[i]

            if ((palheta + i)->qtde > (palheta + j)->qtde)
            {
            
               //guardando o temp
               rgb[0] = (palheta + i)->rgb[0];
               rgb[1] = (palheta + i)->rgb[1];
               rgb[2] = (palheta + i)->rgb[2];
               qtde   = (palheta + i)->qtde;
               
               
               //mudando posicao i
               (palheta + i)->rgb[0] = (palheta + j)->rgb[0];
               (palheta + i)->rgb[1] = (palheta + j)->rgb[1];
               (palheta + i)->rgb[2] = (palheta + j)->rgb[2];
               (palheta + i)->qtde   = (palheta + j)->qtde;
               
               
               //mudando a posicao j
               (palheta + j)->rgb[0] = rgb[0];
               (palheta + j)->rgb[1] = rgb[1];
               (palheta + j)->rgb[2] = rgb[2];
               (palheta + j)->qtde   = qtde;
              
            }

            else
               continue;
         }
      }

      return palheta;

   }


   /********************************/

   tpcolor * buscaColor (unsigned char rgb_desejada[3], tpcolor * palheta, unsigned int final)
   {

      unsigned int i;
      //unsigned char rgb[3];
      tpcolor * palheta_aux ;


      palheta_aux = palheta;

      /* buscar a cor desejada */

      for (i=0; i<= final; i++)
      {

         //Se o lugar for vago - insere
         if ((palheta_aux->rgb[0] == 0) && 
             (palheta_aux->rgb[1] == 0) &&
             (palheta_aux->rgb[2] == 0) &&
             (palheta_aux->qtde   == 0))
         {

            palheta_aux->rgb[0] = rgb_desejada[0];
            palheta_aux->rgb[1] = rgb_desejada[1];
            palheta_aux->rgb[2] = rgb_desejada[2];

            palheta_aux->qtde = 1;

            return palheta;
            
         }


         //Buscando se a cor existe - aumenta a quantidade
         else if  ((palheta_aux->rgb[0] == rgb_desejada[0]) && 
                   (palheta_aux->rgb[1] == rgb_desejada[1]) &&
                   (palheta_aux->rgb[2] == rgb_desejada[2]))
         {
            
            palheta_aux->qtde += 1;
            return palheta;
         }

         else
            palheta_aux ++;

      }

      //Erro!?
      return NULL;
   }



	/**********************************************************************
	
			========================================
				Modo de Reducao: Cor mais proxima
			========================================

	  Funcao:  imgRmNearestColor

	  Ideia:   Para cada pixel da imagem calcular a norma euclediana em 
			relacao a cada cor da palheta escolhida, e substituir pela 
			menor norma encontrada.

	**********************************************************************/


	Image * imgRmNearestColor(Image * img0)
	{
		double d=0,best_d=10000, r_distance=0,g_distance=0,b_distance=0;
		int w = imgGetWidth(img0);
		int h = imgGetHeight(img0);
		Image * img1=imgCreate(w,h);
		int x,y;
		unsigned char rgb_img[3],rgb_best[3];
		
		tpPalheta * pPalhetaAux;

		pPalhetaAux = pPalheta;


      /* Varrendo para toda a imagem */

		for (y=0;y<h;y++)
		{
			for (x=0;x<w;x++) 
			{
				imgGetPixel3ubv(img0,x,y,rgb_img);

            
            /* Zerando a menor distancia */

            best_d=10000;


            /* Varrer a palheta e buscar a cor mais proxima do pixel atual */

				while (pPalhetaAux->pProx != NULL )
				{
               r_distance = pPalhetaAux->rgb[0] - rgb_img[0];
               g_distance = pPalhetaAux->rgb[1] - rgb_img[1];
               b_distance = pPalhetaAux->rgb[2] - rgb_img[2];

					d = (double) (r_distance * r_distance) + 
                            (g_distance * g_distance) +
                            (b_distance * b_distance);

					if (d < best_d)
					{
						best_d = d;
						rgb_best[0] = pPalhetaAux->rgb[0];
						rgb_best[1] = pPalhetaAux->rgb[1];
						rgb_best[2] = pPalhetaAux->rgb[2];

					}

					pPalhetaAux = pPalhetaAux->pProx;

				}


            /* Substituir o pixel atual pelo mais proximo da palheta */

				imgSetPixel3ubv(img1,x,y,rgb_best);
				pPalhetaAux = pPalheta;
				
			}
		}

		return img1;
	}


	/**********************************************************************
	
			========================================
				Modo de Reducao: Difusao do Erro
               (Algoritmo Floyd-Steinberg)
			========================================

	  Funcao:  imgRMErrorDiffusion

     Teoria:  Esse modo de reducao considera as grandes diferenças de 
            intensidades, este método considera o a diferença entre o valor 
            original e um limiar previamente definido. Este erro é distribuído
            para os pixels vizinhos.
              Esta técnica de Floyd-Steinberg é um aprimoramento da técnica de 
            Threshold. Quando o valor da intensidade (para cada pixel) da 
            imagem passa de um determinado limite, este é pintado com a 
            intensidade mais alta do dispositivo. Caso contrário, ele será 
            pintado com a intensidade mais baixa. Em relação ao algoritmo de 
            Floyd-Steinberg, ele busca "suavizar" este erro, distribuindo-o entre 
            os pixels adjacentes. Assim, o erro de quantização local é 
            distribuído, minimizando globalmente as diferenças de intensidade 
            entre a imagem original e a processada.

	  Ideia:   Para cada pixel da nova imagem buscar o valor que mais se aproxima
            da palheta selecionada, após isso subtrair a cor encontrada da imagem
            original e guardar esse valor como o erro. Repassar esse erro para os 
            vizinhos proporcionalmente: (w+1,h)-> 7/16, (w,h+1)-> 5/16, 
            (w+1,h+1)-> 3/16 e (w-1,h+1)-> 1/16. Esses valores serao adicionados
            aos valores das cores da imagem original, e repetir o processo até
            o último pixel.

        **********************************************************************/



     Image * imgRmErrorDiffusion(Image * img0)
     {
        int t=0,x,y;    
        int w = imgGetWidth(img0);
        int h = imgGetHeight(img0);

        float d=0,best_d=10000, r_distance=0,g_distance=0,b_distance=0;
        float r_error=0, g_error=0, b_error=0;
        float rgb_error[3],rgb_img[3],rgb_best[3];

        Image * img1 = imgCreate(w,h);
        tpPalheta * pPalhetaAux = pPalheta;


        /* Copiando a img0 para a img1*/

        for (y=0;y<h;y++){
		   for (x=0;x<w;x++){
			   imgGetPixel3fv(img0,x,y,rgb_img);
			   imgSetPixel3fv(img1,x,y,rgb_img);
         }
        }


        for (y=0;y<h;y++){
           for (x=0;x<w;x++)
           {
              /* Pegar pixel da img original */
              imgGetPixel3fv(img1,x,y,rgb_img);


              /* Resetando o best para cada iteracao */
              best_d=10000000000.0;


              /* Procuro a cor da palheta que melhor representa */
              while (pPalhetaAux->pProx != NULL )
              {
                 r_distance = ((float)pPalhetaAux->rgb[0] / 255.0f) - rgb_img[0];
                 g_distance = ((float)pPalhetaAux->rgb[1] / 255.0f) - rgb_img[1];
                 b_distance = ((float)pPalhetaAux->rgb[2] / 255.0f) - rgb_img[2];

                 d = (float) (r_distance * r_distance) + (g_distance * g_distance) + (b_distance * b_distance);

                 if (d < best_d)
                 {
                    best_d = d;
                    rgb_best[0] = (float)pPalhetaAux->rgb[0] / 255.0f;
                    rgb_best[1] = (float)pPalhetaAux->rgb[1] / 255.0f;
                    rgb_best[2] = (float)pPalhetaAux->rgb[2] / 255.0f;

                    if(rgb_best[0] == 255/ 255.0f)
                       rgb_best[0] = 1.0;
                 }

                 pPalhetaAux = pPalhetaAux->pProx;
              }


              /* Pintando o melhor pixel na imagem nova */
              imgSetPixel3fv(img1,x,y,rgb_best);


              /* calculando o erro */
              r_error = (rgb_img[0] - rgb_best[0]);
              g_error = (rgb_img[1] - rgb_best[1]);
              b_error = (rgb_img[2] - rgb_best[2]);


              /* Pintando o erro nos vizinhos da imagem erro*/
              rgb_error[0] = r_error * (7.0f/16.0f);
              rgb_error[1] = g_error * (7.0f/16.0f);
              rgb_error[2] = b_error * (7.0f/16.0f);             
              if(x+1 < w)
              imgAddPixel3fv(img1,x+1,y,rgb_error);

              rgb_error[0] = r_error * (5.0f/16.0f);
              rgb_error[1] = g_error * (5.0f/16.0f);
              rgb_error[2] = b_error * (5.0f/16.0f);
              if(y+1 < h)
              imgAddPixel3fv(img1,x,y+1,rgb_error);


              rgb_error[0] = r_error * (3.0f/16.0f);
              rgb_error[1] = g_error * (3.0f/16.0f);
              rgb_error[2] = b_error * (3.0f/16.0f);
              if((x+1 < w) && (y+1 < h))
              imgAddPixel3fv(img1,x+1,y+1,rgb_error);

              rgb_error[0] = r_error * (1.0f/16.0f);
              rgb_error[1] = g_error * (1.0f/16.0f);
              rgb_error[2] = b_error * (1.0f/16.0f);
              if((x-1 >= 0) && (y+1 < h))
              imgAddPixel3fv(img1,x-1,y+1,rgb_error);

              pPalhetaAux = pPalheta;
           }
        }
        return img1;
     }



	/**********************************************************************

	  Funcoes auxiliares usadas no processo de reducao de cores (reducao)

	**********************************************************************/


   ///Alterei a create do gattass

   Image * imgCreate (int w, int h)
   {
      Image * image = (Image*) malloc (sizeof(Image));
        
      assert(image);
      image->width  =(unsigned int) w;
      image->height =(unsigned int) h;
      //image->buf = (unsigned char *) malloc (w * h * 3);
      image->buf = (unsigned char *) calloc (w * h * 3,(sizeof(char)));
      assert(image->buf);
      return image;
   }

   /********************************/

   void imgAddPixel3fv(Image * image, int x, int y, float * color)
   {
      int pos = (y*image->width*3) + (x*3);


      /* Tratar "estouro" do char */

      if      (((image->buf[pos] / 255.0) + color[0]) >= 1.0 )
         image->buf[pos  ] = (unsigned char) 255;

      else if (((image->buf[pos] / 255.0) + color[0]) <= 0.0 )
         image->buf[pos  ] = (unsigned char) 0;
      
      else
         image->buf[pos  ] += (unsigned char)(color[0] * 255);


      if      (((image->buf[pos+1] / 255.0) + 	color[1]) >= 1.0)
         image->buf[pos+1] = (unsigned char) 255;

      else if (((image->buf[pos+1] / 255.0) + color[1]) <= 0.0 )
         image->buf[pos+1] = (unsigned char) 0;

      else
         image->buf[pos+1] += (unsigned char)(color[1] * 255);


      if      (((image->buf[pos+2] / 255.0) + 	color[2]) >= 1.0)
         image->buf[pos+2] = (unsigned char) 255;
      
      else if (((image->buf[pos+2] / 255.0) + color[2]) <= 0.0 )
         image->buf[pos+2] = (unsigned char) 0;

      else
         image->buf[pos+2] += (unsigned char)(color[2] * 255);

   }