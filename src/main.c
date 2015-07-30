/**
* The MIT License (MIT)
* 
* Copyright (c) 2015 HyperSequence
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*/

//
// Includes
#include <cbm.h>
//#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <mouse.h>
//#include "RsTypes.h"
//#include <peekpoke.h>

#define bufSize 5000

#define PETSCII_STOP  0x03
//#define PETSCII_RUN   0x83

#define PETSCII_F1  0x85
//#define PETSCII_F2  0x89
#define PETSCII_F3  0x86
//#define PETSCII_F4  0x8a
#define PETSCII_F5  0x87
//#define PETSCII_F6  0x8b
#define PETSCII_F7  0x88
//#define PETSCII_F8  0x8c

#define PETSCII_RVSON  0x12
#define PETSCII_RVSOFF 0x92

#define PETSCII_DOWN  0x11
#define PETSCII_UP    0x91
#define PETSCII_RIGHT 0x1D
#define PETSCII_LEFT  0x9D


#define SPRITE0_DATA    0x0340
#define SPRITE0_PTR     0x07F8
#define DRIVER          "c64-1351.mou"


//typedef void (*TerminalFunc)( char pByte );
//static TerminalFunc pchar = 0;

static int screenH = 24;
static int screenW = 40;

static int screenPixH = 200;
static int screenPixW = 320;

static char url[64];
static char sRecvBuf[bufSize] ;

static int linkTable[40][4];

static char sRunning = 1;


/* The mouse sprite (an arrow) */
static const unsigned char MouseSprite[64] = {
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x0F, 0xE0, 0x00,
    0x0F, 0xC0, 0x00,
    0x0F, 0x80, 0x00,
    0x0F, 0xC0, 0x00,
    0x0D, 0xE0, 0x00,
    0x08, 0xF0, 0x00,
    0x00, 0x78, 0x00,
    0x00, 0x3C, 0x00,
    0x00, 0x1E, 0x00,
    0x00, 0x0F, 0x00,
    0x00, 0x07, 0x80,
    0x00, 0x03, 0x80,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00
};




//
// Functions


void pchar( unsigned char charin ){

  putchar(charin);

}


/******************************************************************************/
/* screenRender                                                               */
/******************************************************************************/
unsigned char screenRender( int indx )
{

  int i;
  int x;
  char tag_buf[7];
  char tag_href[] = "A HREF";


  i=indx;
  clrscr();
  bordercolor(6);
  while(wherey() < screenH && i<=bufSize){

    //i=0;
    if(sRecvBuf[i]=='<'){
      puts("***tag check***");
      //Place the html tag in the tab_buf string
      i++;
      for (x=0; x < 6; x++){
        tag_buf[x]=sRecvBuf[x+i];
      }
      tag_buf[x+1]='\0';

      puts(tag_buf);
      puts(tag_href);
      //Check for "href" tag
      if (strcmp(tag_href,tag_buf) == 0){
        puts("***href start***");
        i=i+9;
        linkTable[wherey()][2]=i;
        while(sRecvBuf[i++]!='"'){
          linkTable[wherey()][0]=wherex();
          pchar(sRecvBuf[i]);
        }
        linkTable[wherey()][3]=i;
        i+2;
        puts("***href end***");
        x=i;;
        //putchar( PETSCII_RVSON );//Turn on reverse
        while(sRecvBuf[i]!='<' && sRecvBuf[i+1]!='/'){
          //putchar(sRecvBuf[i++]);
          pchar(sRecvBuf[i++]);
        }
        //putchar( PETSCII_RVSOFF );//Turn off reverse

        linkTable[wherey()][1]=i-x;
        i=i+3;
      }
      else{

        while(sRecvBuf[i]!='>'){
          i++;
        }
        i++;

      }
    }
    //<a href="http://www.w3schools.com">Visit W3Schools.com!</a>

    pchar(sRecvBuf[i++]);
    //putchar(sRecvBuf[i++]);
    //cputc(sRecvBuf[i++]);
  }
  bordercolor(0);
  return(i - indx); 
}

/******************************************************************************/
/* getData                                                                    */
/******************************************************************************/


unsigned char getData()
{
  bordercolor(2);
  //Make sure all devices are closed before opening again
  cbm_close( 2 );

  //If its a local file, use device 8
  if(url[0]=='/'){

    //remove leading '/' and add a ',s' to allow openning of a seq file
    memmove(url, url+1, strlen(url));
    strcat(url,",s");

    cbm_close( 8 );
    cbm_open( 2, 8, 2, url );
  }
  //Get local data for an already formated URL
  else if(url[strlen(url)-2]==',' && url[strlen(url)-1]=='s'){

    cbm_close( 8 );
    cbm_open( 2, 8, 2, url );
  }
  //For remote files use device 7 (Flyer)
  else{
    cbm_close( 7 );
    //
    if( cbm_open( 7, 7, 15, "" ) != 0 )
    {
      cputs("Flyer not connected!");
    }
    cbm_open( 2, 7, 2, url );
    cbm_write( 7, "http-transact:2",15);
  }

  //Clear the input buffer:
  memset(&sRecvBuf[0], 0, sizeof(sRecvBuf));

  //Read a chunk (or all if small) of the file:
  cbm_read( 2, sRecvBuf, sizeof(sRecvBuf));

  bordercolor(0);
  return(1);
}



/******************************************************************************/
/* Display the url entry prompt                                               */
/******************************************************************************/
int showAddrsBar()
{
  int Len;

  cclearxy(0,0,screenW*2);
  chlinexy(0,1,screenW);
  cputsxy(0,0,"URL:");

  //fflush (stdout);
  fgets (url, sizeof (url), stdin);

  /* Remove trailing white space including the line terminator */
  Len = strlen (url);
  while (Len > 0 && isspace (url[Len-1])) {
      --Len;
  }
  url[Len] = '\0';

  if (Len>0) {
      getData();
  }

  return(Len);
}













int main( void )
{

  int i = 0;
  int bufIndx;
  int bufPage;
  int charCnt;
  char cursorx;
  char cursory;


  memcpy ((void*) SPRITE0_DATA, MouseSprite, sizeof (MouseSprite));


  /* Load and install the mouse driver */
   mouse_load_driver (&mouse_def_callbacks, DRIVER);
/* Set the VIC sprite pointer */
    *(unsigned char*)SPRITE0_PTR = SPRITE0_DATA / 64;

    VIC.spr0_color = COLOR_WHITE;



  bgcolor (0);
  bordercolor (0);

  clrscr ();

  
  while( showAddrsBar()<1){bufPage=1;}

  bufPage=1;
  bufIndx=0;
  charCnt = screenRender(bufIndx);


  cursorx=20;
  cursory=11;

  mouse_show();
  mouse_move (cursorx , cursory);

  while( sRunning )
  {

    if( kbhit() )
    {
      //uint8_t pet = cgetc();
      char pet = cgetc();

      switch( pet )
      {
        //Move cursor down
        case PETSCII_DOWN:

          if(cursory < screenPixH){
            cursory++;
            mouse_move (cursorx , cursory);

          }
          break;

        //Move cursor up
        case PETSCII_UP:

          if(cursory > 0){
            cursory--;
            mouse_move (cursorx , cursory);

          }
          break;

        //Move cursor left
        case PETSCII_LEFT:

          if(cursorx > 0){
            cursorx--;
            mouse_move (cursorx , cursory);
          }
          break;

        //Move cursor right
        case PETSCII_RIGHT:

          if(cursorx < screenPixW-1){
            cursorx++;

            mouse_move (cursorx , cursory);
          }
          break;


        //Scroll UP
        case PETSCII_F1:

          bufIndx = bufIndx - charCnt;
          if(bufIndx <0){
            
            if(bufPage>1){
              getData();
              bufIndx=bufSize - charCnt;

              bufPage--;
              textcolor(bufPage);
            }
            else{bufIndx=0;}
          }
          charCnt = screenRender(bufIndx);
          break;

        //Scroll DOWN
        case PETSCII_F7:

          bufIndx = bufIndx + charCnt;
          if(bufIndx > bufSize){
            
            cbm_read( 2, sRecvBuf, sizeof(sRecvBuf));
            bufIndx=0;

            bufPage++;
            textcolor(bufPage);
          }
          charCnt = screenRender(bufIndx);
          break;

        //Show URL address entry bar
        case PETSCII_STOP:

          if(showAddrsBar() >0){
             bufPage=1;
             bufIndx=0;
             charCnt = screenRender(bufIndx);
          }
          break;

      }
    }

  }


  cbm_close( 2 );
  cbm_close( 7 );
  cbm_close( 8 );

  return(0);
}
