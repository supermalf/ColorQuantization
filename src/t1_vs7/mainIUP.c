/**
 *	@file CGT1.c Exibe uma imagem no formato TGA através do OpenGL.
 *
 *	@author 
 *			- Marcelo Gattass
 *
 *	@date
 *			Criado em:		    12 de Agosto de 2004
 *			Última Modificação:	1 de Agosto de 2004
 *
 *	@version 1.1 - Medidas de tempo e melhor feedback
 */
/*- Bibliotecas padrao usadas: --------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*- Inclusao das bibliotecas IUP e CD: ------------------------------------*/
#include <iup.h>                    /* interface das funcoes IUP */
#include <iupgl.h>                  /* interface da funcao do canvas do OpenGL no IUP */
#include <windows.h>                /* inclui as definicoes do windows para o OpenGL */
#include <gl/gl.h>                  /* prototypes do OpenGL */
#include <gl/glu.h>                 /* prototypes do OpenGL */


#include "image.h"                  /* TAD para imagem */

/* -- implemented in "iconlib.c" to load standard icon images into IUP */
void IconLibOpen(void);

/*- Contexto do Programa: -------------------------------------------------*/
Image   *image1, *image2;                 /* imagens dos dois canvas */
Ihandle *left_canvas, *right_canvas;      /* hadle dos dois canvas */
Ihandle *label;                           /* handle do label para colocar mensagens para usuario */

/*- Funcoes auxiliares ------------*/

/* Dialogo de selecao de arquivo  */
static char * get_file_name( void )
{
  Ihandle* getfile = IupFileDlg();
  char* filename = NULL;

  IupSetAttribute(getfile, IUP_TITLE, "Abertura de arquivo"  );
  IupSetAttribute(getfile, IUP_DIALOGTYPE, IUP_OPEN);
  IupSetAttribute(getfile, IUP_FILTER, "*.tga");
  IupSetAttribute(getfile, IUP_FILTERINFO, "Arquivo de imagem (*.tga)");
  IupPopup(getfile, IUP_CENTER, IUP_CENTER);  /* o posicionamento nao esta sendo respeitado no Windows */

  filename = IupGetAttribute(getfile, IUP_VALUE);
  return filename;
}


/*------------------------------------------*/
/* Callbacks do IUP.                        */
/*------------------------------------------*/


/* - Callback de mudanca de tamanho no canvas (mesma para ambos os canvas) */
int resize_cb(Ihandle *self, int width, int height)
{
 IupGLMakeCurrent(self);  /* torna o foco do OpenGL para este canvas */

 /* define a area do canvas que deve receber as primitivas do OpenGL */
 glViewport(0,0,width,height);

 /* transformacao de instanciacao dos objetos no sistema de coordenadas da camera */
 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();           /* identidade,  ou seja nada */

 /* transformacao de projecao */
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 gluOrtho2D (0.0, (GLsizei)(width), 0.0, (GLsizei)(height));  /* ortografica no plano xy de [0,w]x[0,h] */

 return IUP_DEFAULT; /* retorna o controle para o gerenciador de eventos */
}

/* - Callback de repaint do canvas 1 */
int repaint_cb1(Ihandle *self)
{
  int w = imgGetWidth(image1);
  int h = imgGetHeight(image1);
  unsigned char *rgbData = imgGetRGBData(image1);

  IupGLMakeCurrent(self);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glRasterPos2d(0.0,0.0);     /* define a origem da imagem */
  glDrawPixels (w, h, GL_RGB,GL_UNSIGNED_BYTE, (GLubyte *) rgbData);
  glFlush();               /* forca atualizacao do OpenGL  (que pode ser bufferizado) */ 
  return IUP_DEFAULT; /* retorna o controle para o gerenciador de eventos */
}

/* - Callback de repaint do canvas 2 */
/* - poderia ser a mesma do 1, feita diferente para exemplificar OpenGL */
int repaint_cb2(Ihandle *self)
{
  int w = imgGetWidth(image2);
  int h = imgGetHeight(image2);
  unsigned char *rgbData = imgGetRGBData(image2);
  int x,y;

  IupGLMakeCurrent(self);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glBegin(GL_POINTS);
    for (y=0;y<h;y++) {
      for (x=0; x<w; x++) {
         unsigned char rgb[3];
         imgGetPixel3ubv(image2, x, y, rgb );
         glColor3ubv(rgb);
         glVertex2i(x,y);
       }
    }
  glEnd();
  
  glFlush();
  return IUP_DEFAULT; /* retorna o controle para o gerenciador de eventos */
} 





/* - Callback do botao que conta cor */
int conta_cb(Ihandle* self,int but, int pressed, int x, int y, char* status)
{
    int finish_time;
	double duration;
	int start_time  = clock(); 
	int ncolors1 = imgCountColors(image1);
	int ncolors2 = imgCountColors(image2);
	finish_time = clock();
	duration = (double)(finish_time - start_time)/CLOCKS_PER_SEC;
	IupSetfAttribute(label, "TITLE", "Numero de cores. Esq: %d  Dir:%d - Tempo: %2.1f", ncolors1,ncolors2,duration);
    repaint_cb1(left_canvas);  /* redesenha o canvas 2 */
    repaint_cb2(right_canvas);  /* redesenha o canvas 2 */
	return IUP_DEFAULT; /* retorna o controle para o gerenciador de eventos */
}


int reduz_cb(void)
{
	int ncolors2;
	int start_time  = clock(); 
    int finish_time;
	double duration;
	image2=imgReduce256Colors(image2);
	finish_time = clock();
	duration = (double)(finish_time - start_time)/CLOCKS_PER_SEC;
    repaint_cb2(right_canvas);  /* redesenha o canvas 2 */
	ncolors2= imgCountColors(image2);
	IupSetfAttribute(label, "TITLE", "Numero reduzido de cores: %d - tempo de processamento: %2.1f segundos", ncolors2, duration);
	return IUP_DEFAULT;
}

int lum_cb(void)
{
	int ncolors2;
	int finish_time;
	double duration;
	int start_time  = clock(); 
    image2=imgNormalizeColors(image2);
	finish_time = clock();
	duration = (double)(finish_time - start_time)/CLOCKS_PER_SEC;
	ncolors2= imgCountColors(image2);
	IupSetfAttribute(label, "TITLE", "Novo numero de cores: %d - tempo de processamento: %2.1f segundos", ncolors2, duration);
    repaint_cb2(right_canvas);  /* redesenha o canvas 2 */
    return IUP_DEFAULT;
}

int tranf_cb(void)
{
	if (image2) imgDestroy(image2);
	image2=imgCopy(image1);
    repaint_cb2(right_canvas);  /* redesenha o canvas 2 */
	IupSetfAttribute(label, "TITLE", " ");
	return IUP_DEFAULT;
}

int dif_cb(void)
{
	imgSub(image2,image1);
    repaint_cb2(right_canvas);  /* redesenha o canvas 2 */
	return IUP_DEFAULT;
}

float med_cb(void)
{
	float med1=imgMed(image1);
	float med2=imgMed(image2);
	IupSetfAttribute(label, "TITLE", "Valor medio dos pixels. esq:%f  dir:%f", med1,med2);
	return IUP_DEFAULT;
}

static char * get_new_file_name( void )
{
  Ihandle* getfile = IupFileDlg();
  char* filename = NULL;

  IupSetAttribute(getfile, IUP_TITLE, "Salva arquivo"  );
  IupSetAttribute(getfile, IUP_DIALOGTYPE, IUP_SAVE);
  IupSetAttribute(getfile, IUP_FILTER, "*.tga");
  IupSetAttribute(getfile, IUP_FILTERINFO, "Arquivo de imagem (*.tga)");
  IupPopup(getfile, IUP_CENTER, IUP_CENTER);  /* o posicionamento nao esta sendo respeitado no Windows */

  filename = IupGetAttribute(getfile, IUP_VALUE);
  return filename;
}


int save_cb(Ihandle *self)
{
  char* filename = get_new_file_name( );  /* chama o dialogo de abertura de arquivo */
  if (filename==NULL) return 0;
  imgWriteTGA(filename,image2);
  return IUP_DEFAULT;
}


int load_cb(void) {
  char* filename = get_file_name();  /* chama o dialogo de abertura de arquivo */

  if (filename==NULL) return 0;
  /* le a imagem de fundo */
  if (image1) imgDestroy(image1);
  if (image2) imgDestroy(image2);

  image1 = imgLoad(filename);
  image2 = imgCopy(image1);

  IupSetFunction("repaint_cb1", (Icallback) repaint_cb1);
  IupSetFunction("repaint_cb2", (Icallback) repaint_cb2 );

  repaint_cb1(left_canvas);  /* redesenha o canvas 2 */
  repaint_cb2(right_canvas);  /* redesenha o canvas 2 */

  IupSetfAttribute(label, "TITLE", "%3dx%3d", imgGetWidth(image1), imgGetWidth(image2));

  IupSetFunction("tranf_cb", (Icallback)tranf_cb);
  IupSetFunction("conta_cb", (Icallback)conta_cb); 
  IupSetFunction("reduz_cb", (Icallback)reduz_cb);
  IupSetFunction("lum_cb", (Icallback)lum_cb);
  IupSetFunction("dif_cb", (Icallback)dif_cb);
  IupSetFunction("med_cb", (Icallback)med_cb);
  IupSetFunction("save_cb", (Icallback)save_cb);

  return IUP_DEFAULT;

}

/*-------------------------------------------------------------------------*/
/* Incializa o programa.                                                   */
/*-------------------------------------------------------------------------*/

int init(void)
{
  Ihandle *dialog, *statusbar,  *box;

  Ihandle *toolbar, *load, * tranf, *save;

  /* creates the toolbar and its buttons */
  load = IupButton("", "load_cb");
  IupSetAttribute(load,"TIP","Carrega uma imagem.");
  IupSetAttribute(load,"IMAGE","icon_lib_open");
  IupSetFunction("load_cb", (Icallback)load_cb);

 
  tranf = IupButton("", "tranf_cb");
  IupSetAttribute(tranf,"TIP","Transfere para o outro canvvas.");
  IupSetAttribute(tranf,"IMAGE","icon_lib_transfer");

  save = IupButton("", "save_cb");
  IupSetAttribute(save,"TIP","Salva no formato GIF.");
  IupSetAttribute(save,"IMAGE","icon_lib_save");
  
  
  toolbar = IupHbox(
       load, 
	   tranf,
	   IupFill(),
	   IupButton("Conta", "conta_cb"),  
       IupButton("Reduz", "reduz_cb"),
       IupButton("Lum", "lum_cb"),
       IupButton("Dif", "dif_cb"),
       IupButton("Med", "med_cb"),
       save,
     NULL);

  IupSetAttribute(toolbar, "ALIGNMENT", "ACENTER");
 

  /* cria dois canvas */
  left_canvas = IupGLCanvas("repaint_cb1"); 
  IupSetAttribute(left_canvas,IUP_RASTERSIZE,"320x240");
  IupSetAttribute(left_canvas, "RESIZE_CB", "resize_cb");

  right_canvas = IupGLCanvas("repaint_cb2"); 
  IupSetAttribute(right_canvas,IUP_RASTERSIZE,"320x240");
  IupSetAttribute(right_canvas, "RESIZE_CB", "resize_cb");

  /* associa o evento de repaint a funccao repaint_cb */
  IupSetFunction("repaint_cb1", NULL);
  IupSetFunction("repaint_cb2", NULL );
  IupSetFunction("resize_cb", (Icallback) resize_cb);

  /* the status bar is just a label to put some usefull information in run time */
  label = IupLabel("status");
  IupSetAttribute(label, "EXPAND", "HORIZONTAL");
  IupSetAttribute(label, "FONT", "COURIER_NORMAL_10");
  statusbar = IupSetAttributes(IupHbox(
                IupFrame(IupHbox(label, NULL)), 
                NULL), "MARGIN=5x5");

  /* this is the most external box that puts together
     the toolbar, the two canvas and the status bar */
  box = IupVbox(
          toolbar,
          IupSetAttributes(IupHbox(
            left_canvas, 
            right_canvas, 
            NULL), "GAP=5"),
          statusbar, 
          NULL);

  /* create the dialog and set its attributes */

  dialog = IupDialog(box);
  IupSetAttribute(dialog, "CLOSE_CB", "app_exit_cb");
  IupSetAttribute(dialog, "TITLE", "Trabalho 1");

  IupShowXY(dialog, IUP_CENTER, IUP_CENTER);

  return 1;
}

/*-------------------------------------------------------------------------*/
/* Rotina principal.                                                       */
/*-------------------------------------------------------------------------*/
void main(void) {
    IupOpen();
    IupGLCanvasOpen();
    IconLibOpen();
    if ( init() )
		IupMainLoop();
    IupClose();
}