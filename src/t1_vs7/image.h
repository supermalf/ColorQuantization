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

#ifndef IMAGE_H
#define IMAGE_H

/************************************************************************/
/* Tipos Exportados                                                     */
/************************************************************************/
/**
 *   Imagem com um buffer rgb.
 */

typedef struct Image_imp Image;

/************************************************************************/
/* Funcoes Exportadas                                                   */
/************************************************************************/

/**
 *	Cria uma nova imagem com as dimensoes especificadas.
 *
 *	@param w Largura da imagem.
 *	@param h Altura da imagem.
 *
 *	@return Handle da imagem criada.
 */
Image  * imgCreate (int w, int h);

/**
 *	Destroi a imagem.
 *
 *	@param image imagem a ser destruida.
 */
void    imgDestroy (Image *image);

/**
 *	Cria uma nova nova copia imagem dada.
 *
 *	@param image imagem a ser copiada.
 *
 *	@return Handle da imagem criada.
 */
Image * imgCopy(Image * image);

/**
 *	Cria uma nova nova copia imagem dada em tons de cinza.
 *
 *	@param image imagem a ser copiada em tons de cinza.
 *
 *	@return Handle da imagem criada.
 */
Image * imgGrey(Image * image);


/**
 *	Obtem a largura (width) de uma imagem.
 *
 *	@param image Handle para uma imagem.
 *	@return  a largura em pixels (width) da imagem.
 */
int imgGetWidth(Image * image);

/**
 *	Obtem a altura (heigth) de uma imagem.
 *
 *	@param image Handle para uma imagem.
 *	@return  a altura em pixels (height) da imagem.
 */
int imgGetHeight(Image * image);

/**
 *	Obtem as dimensoes de uma imagem.
 *
 *	@param image Handle para uma imagem.
 *	@param w [out]Retorna a largura da imagem.
 *	@param h [out]Retorna a altura da imagem.
 */
unsigned char * imgGetRGBData(Image * image);

/**
 *	Ajusta o pixel de uma imagem com a cor especificada.
 *
 *	@param image Handle para uma imagem.
 *	@param x Posicao x na imagem.
 *	@param y Posicao y na imagem.
 *	@param color Cor do pixel(valor em float [0,1]).
 */
void imgSetPixel3fv(Image * image, int x, int y, float * color);

/**
 *	Ajusta o pixel de uma imagem com a cor especificada.
 *
 *	@param image Handle para uma imagem.
 *	@param x Posicao x na imagem.
 *	@param y Posicao y na imagem.
 *	@param color Cor do pixel (valor em unsigend char[0,255]).
 */
void imgSetPixel3ubv(Image * image, int x, int y, unsigned char * color);

/**
 *	Obtem o pixel de uma imagem na posicao especificada.
 *
 *	@param image Handle para uma imagem.
 *	@param x Posicao x na imagem.
 *	@param y Posicao y na imagem.
 *	@param color [out] Pixel da posicao especificada(valor em float [0,1]).
 */
void imgGetPixel3fv(Image * image, int x, int y, float *color);

/**
 *	Obtem o pixel de uma imagem na posicao especificada.
 *
 *	@param image Handle para uma imagem.
 *	@param x Posicao x na imagem.
 *	@param y Posicao y na imagem.
 *	@param color [out] Pixel da posicao especificada (valor em unsigend char[0,255]).
 */
void imgGetPixel3ubv(Image * image, int x, int y, unsigned char *color);

/**
 *	Redimensiona a imagem especificada.
 *
 *	@param image Handle para uma imagem.
 *	@param w1 Nova largura da imagem.
 *	@param h1 Nova altura da imagem.
 *  @return imagem criada.
 */
Image * imgResize(Image * img0, int w1, int h1);

/**
 *	Le a imagem a partir do arquivo especificado.
 *
 *	@param filename Nome do arquivo de imagem.
 *
 *	@return imagem criada.
 */
Image * imgLoad(char *filename);

/**
 *	Salva a imagem no arquivo especificado em formato TGA.
 *
 *	@param filename Nome do arquivo de imagem.
 *	@param image Handle para uma imagem.
 *
 *	@return retorna 1 caso nao haja erros.
 */
int imgWriteTGA(char *filename, Image * image);


/*** FUNCOES QUE DEVEM SER IMPLEMENTADAS NO TRABALHO 1 ***/

/**
 *	Conta o numero de cores diferentes na imagem
 *
 *	@param image Handle para uma imagem.
 *	@param w Nova largura da imagem.
 *	@param h Nova altura da imagem.
 */
unsigned int imgCountColors(Image * image);

/**
 *	Normaliza as cores da imagem (eliminando o 
 *  efeito da iluminacao.
 *
 *	@param image Handle para uma imagem.
 *  @return Handle para nova imagem normalizada.
 */
Image * imgNormalizeColors(Image * image);

/**
 *	Reduz o numero de cores distintas para 256
 *
 *	@param image Handle para uma imagem.
 *  @return Handle para nova imagem com 256 cores.
 */
Image * imgReduce256Colors(Image * image);


/* img0 =  |img0-img1| */
void imgSub(Image *im0, Image *img1);

/* |image| */
float imgMed(Image *image);

#endif
