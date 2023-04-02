#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "pngwriter.h"

//odkomentujte nasledovny riadok ak kamera nepodporuje BGR format
//pozri v4l2-ctl -d /dev/videoX --list-formats

//#define POUZI_YUV

uint8_t *buffer;

int sirka = 320;
int vyska = 240;
 
long long usec()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return (1000000L * (long long)tv.tv_sec) + tv.tv_usec;
}

static int xioctl(int fd, int request, void *arg)
{
        int r;
 
        do r = ioctl (fd, request, arg);
        while (-1 == r && EINTR == errno);
 
        return r;
}
 
int setup_format(int fd)
{
        struct v4l2_format fmt = {0};
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = sirka;
        fmt.fmt.pix.height = vyska;
        
        // ak vasa kamera nepodporuje BGR24, m ozno podporuje YUV420,
        // ale v tom pripade bude treba obrazok spracovavat v tom
        // formate, alebo si ho skonvertovat...

#ifdef POUZI_YUV
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
#else
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
#endif

        fmt.fmt.pix.field = V4L2_FIELD_NONE;
        
        if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
        {
            perror("nepodarilo sa nastavit format");
            return 1;
        }

        return 0;
}
 
int init_mmap(int fd)
{
    struct v4l2_requestbuffers req = {0};
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
 
    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
    {
        perror("nepodarilo sa inicializovat mmap buffer");
        return 1;
    }
 
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
    {
        perror("nepodarilo sa ziskat mmap buffer");
        return 1;
    }
 
    buffer = mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
 
    return 0;
}

int je_lopta (uint8_t r, uint8_t g, uint8_t b)
{

  float h, s, v, max, min;
  h = 0;
 
  if ((uint16_t)r + (uint16_t)g + (uint16_t)b == 0) return 0;
   if (r < b){
    if(r < g){
      min = r;
    }
    else{
      min = g;
    }
  }
  else{
    if (b < g){
      min = b;
    }
    else{
      min = g;
    }
  }


  if (r > b){
    if(r > g){
      max = r;
      if (max!= min){
        h = 60*((g-b)/(max-min));
      }
    }
    else{
      max = g;
      if (max!= min){
        h = 60*(2+(b-r)/(max-min));
      }
    }
  }
  else{
    if (b > g){
      max = b;
      if (max!= min){
        h = 60*(4+(r-g)/(max - min));
      } 
    }
    else{
      max = g;
      if (max!= min){
        h = 60*(2+(b-r)/(max - min));
      }
    }
  }
 
  v = max;

  s = (max-min)/max;
  
  if (h < 0){
    h = h + 360;
  }
  if (((h < 28) || (h > 348)) && s > 0.5 && v > 100){
    return 1;
  }
  return 0;
}


int minr, mins, maxr, maxs;


void zisti_rgb(int riadok, int stlpec, uint8_t *r, uint8_t *g, uint8_t *b)
{
#ifdef POUZI_YUV
  	      *r = buffer[riadok * sirka * 3 + stlpec * 3];
  	      *g = buffer[riadok * sirka * 3 + stlpec * 3 + 1];
  	      *b = buffer[riadok * sirka * 3 + stlpec * 3 + 2];
#else
  	      *b = buffer[riadok * sirka * 3 + stlpec * 3];
  	      *g = buffer[riadok * sirka * 3 + stlpec * 3 + 1];
  	      *r = buffer[riadok * sirka * 3 + stlpec * 3 + 2];
#endif
}

int fill(int riadok, int stlpec)
{
  if (riadok < minr) minr = riadok;
  if (riadok > maxr) maxr = riadok;
  if (stlpec < mins) mins = stlpec;
  if (stlpec > maxs) maxs = stlpec;
  
  buffer[riadok * sirka * 3 + stlpec * 3] = 70;
  buffer[riadok * sirka * 3 + stlpec * 3 + 1] = 255;
  buffer[riadok * sirka * 3 + stlpec * 3 + 2] = 70;
  
  uint8_t r, g, b;
  
  zisti_rgb(riadok, stlpec + 1, &r, &g, &b);
  int kolko = 1;
  
  if (je_lopta(r, g, b))
    kolko += fill(riadok, stlpec + 1);

  zisti_rgb(riadok, stlpec - 1, &r, &g, &b);
  
  if (je_lopta(r, g, b))
    kolko += fill(riadok, stlpec - 1);

  zisti_rgb(riadok - 1, stlpec, &r, &g, &b);
    
  if (je_lopta(r, g, b))
    kolko += fill(riadok - 1, stlpec);

  zisti_rgb(riadok + 1, stlpec, &r, &g, &b);
  
  if (je_lopta(r, g, b))
    kolko += fill(riadok + 1, stlpec);

  return kolko;
}
 
struct v4l2_buffer buf = {0};
uint8_t *rgb; 
int kamera_fd;

int init_sledovanie()
{
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    
#ifdef POUZI_YUV
    *rgb = (uint8_t *)malloc(sirka * vyska * 3);
#endif

    if(-1 == xioctl(kamera_fd, VIDIOC_STREAMON, &buf.type))
    {
        perror("nepodarilo sa zapnut snimanie obrazu");
        return 1;
    }
}
 
void najdi_loptu(int *sirka_lopty, int *vyska_lopty, int *velkost_lopty, int *riadok, int *stlpec)
{
      if(-1 == xioctl(kamera_fd, VIDIOC_QBUF, &buf))
      {
        perror("nepodarilo sa poziadat o buffer");
        return;
      }
      
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(kamera_fd, &fds);
      struct timeval tv = {0};
      tv.tv_sec = 2;
      int rv = select(kamera_fd+1, &fds, NULL, NULL, &tv);
      if(-1 == rv)
      {
          perror("pocas cakania na obrazok doslo k chybe");
          return;
      }
  
      if(-1 == xioctl(kamera_fd, VIDIOC_DQBUF, &buf))
      {
          perror("nepodarilo sa ziskat obrazok");
          return;
      }

#ifdef POUZI_YUV
      yuv422_to_rgb((uint8_t *)buffer, rgb, sirka, vyska);
      uint8_t *p = rgb;
#else
      uint8_t *p = (uint8_t *)buffer;
#endif
      double xs = 0;
      double ys = 0;
      int cnt = 0;
      // prechadzame cely obrazok bod po bode...
      // na tomto mieste chcete program upravit podla svojich potrieb...


      // najskor vynulujeme vsetky 4 okraje, aby fill nevybehol mimo rozsah pola
      for (int i = 0; i < vyska; i++)
      {
         // lavy okraj
         buffer[i*sirka*3] = 0;
         buffer[i*sirka*3 + 1] = 0;
         buffer[i*sirka*3 + 2] = 0;
         
         // pravy okraj
         buffer[(i + 1)*sirka*3 - 3] = 0;
         buffer[(i + 1)*sirka*3 - 2] = 0;
         buffer[(i + 1)*sirka*3 - 1] = 0;
      }
      
      int index_zaciatku_dolneho_riadku = (vyska - 1) * sirka * 3;
      for (int i = 0; i < sirka; i++)
      {
         // horny okraj
         buffer[i*3] = 0;
         buffer[i*3 + 1] = 0;
         buffer[i*3 + 2] = 0;
         
         // dolny okraj
         buffer[index_zaciatku_dolneho_riadku + i * 3] = 0;
         buffer[index_zaciatku_dolneho_riadku + i * 3 + 1] = 0;
         buffer[index_zaciatku_dolneho_riadku + i * 3 + 2] = 0;
      }      

      int doteraz_najvacsi = 0;
      int doteraz_najv_sirka = 0;
      int doteraz_najv_vyska = 0;
      int doteraz_najv_riadok = 0;
      int doteraz_najv_stlpec = 0;
      
      for (int i = 0; i < vyska; i++)
        for (int j = 0; j < sirka; j++)
        {
#ifdef POUZI_YUV
  	      uint8_t r = *(p++);
  	      uint8_t g = *(p++);
  	      uint8_t b = *(p++);
#else
  	      uint8_t b = *(p++);
  	      uint8_t g = *(p++);
  	      uint8_t r = *(p++);
#endif
  	      if (je_lopta(r, g, b))
  	      {
                  mins = sirka, minr = vyska, maxs = -1, maxs = -1;
                  int pocet = fill(i, j);
                  if (pocet > doteraz_najvacsi)
                  {
                      doteraz_najvacsi = pocet;
                      doteraz_najv_sirka = maxs - mins + 1;
                      doteraz_najv_vyska = maxr - minr + 1;
                      doteraz_najv_riadok = (maxr + minr) / 2;
                      doteraz_najv_stlpec = (maxs + mins) / 2;
                  }
  	      }
        }
      //printf("velkost: %d, sirka: %d, vyska: %d\n", doteraz_najvacsi, 
      //         doteraz_najv_sirka, doteraz_najv_vyska);

      *sirka_lopty = doteraz_najv_sirka;
      *vyska_lopty = doteraz_najv_vyska;
      *velkost_lopty = doteraz_najvacsi;
      *riadok = doteraz_najv_riadok;
      *stlpec = doteraz_najv_stlpec;
      
      static int iter = 0;
      
      //if (iter++ == 1)
      //~ {
        //~ iter = 0;      
        //~ static int counter = 0;
        //~ char filename[30];
        //~ sprintf(filename, "image%d.png", counter++);
      
//~ #ifdef POUZI_YUV
        //~ write_yuv422_png_image((uint8_t *)buffer, filename, 320, 280);
//~ #else
        //~ write_bgr_png_image((uint8_t *)buffer, filename, 320, 200);
//~ #endif
      //~ }
}

void test_kamery()
{
   int sirka, vyska, velkost, riadok, stlpec;

   najdi_loptu(&sirka, &vyska, &velkost, &riadok, &stlpec);
   
   printf("s: %d, v: %d, P: %d, R: %d, S: %d\n", sirka, vyska, velkost, riadok, stlpec);
}
 
int setup_kamera()
{
    const char *device = "/dev/video0";
 
    kamera_fd = open(device, O_RDWR);
    if (kamera_fd == -1)
    {
        perror("nepodarilo sa otvorit zariadenie /dev/videoN ...");
        return 1;
    }
    if(setup_format(kamera_fd))
        return 1;
    
    if(init_mmap(kamera_fd))
        return 1;

    init_sledovanie();

    return 0;
}

void ukonci_kameru()
{
    free(rgb);
    close(kamera_fd);
}
