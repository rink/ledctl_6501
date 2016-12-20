/********************************************************************************
 *
 * Filename: ledctl_6501.c
 *
 * ledctl_6501
 * Program that allows control over the 'Ready' and 'Error' LEDs on the
 * Soekris Engineering net6501 computer.
 *
 *   Copyright (C) 2014  Stefan Rink, Victor Perez
 * 
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *******************************************************************************/

/*
http://lists.soekris.com/pipermail/soekris-tech/2011-October/017729.html

on the net6501:

Red Error LED:   I/O port 069C bit 0, 0=off, 1=on.
Green Ready LED: I/O port 069D bit 0, 0=off, 1=on.

Info to be included on next release of manual.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/param.h>
#include <time.h>

#if defined(__linux__)
#include <sys/io.h>
#elif defined(BSD)
#include <machine/cpufunc.h>
#include <machine/sysarch.h>
#endif

#define IOPORTS_6501_BASE 0x69C
#define IOPORTS_6501_NUM  2
#define IOPORTS_6501_RED   (IOPORTS_6501_BASE+0)
#define IOPORTS_6501_GREEN (IOPORTS_6501_BASE+1)

int verbose_flag=0;
int value_red=-1;   /* 1:on, 0:off, -1:unchanged */
int value_green=-1; /* 1:on, 0:off, -1:unchanged */
int demomode_flag=0;
int alternate_flag=0;
float blink_time=1;
float blink_time2=0;
int blink_timeout=60;

void usage(char *argv0)
{
  printf("Usage: %s [--red=0/1] [--green=0/1] [--demo] [--alternate] [--time=0.5] [--time2=1.6] [--timeout=60] [--verbose] [--help]\n\n", argv0);
  exit(2);
}

void parseopt(int argc, char **argv)
{
  while (1)
  {
    static struct option long_options[] =
    {
      /* These options set a flag. */
      {"verbose", no_argument,       &verbose_flag, 1},
      {"quiet",   no_argument,       &verbose_flag, 0},
      {"demo",    no_argument,       &demomode_flag, 1},
      {"alternate",no_argument,       &alternate_flag, 1},
      /* These options don't set a flag.
      We distinguish them by their indices. */
      {"time",    required_argument, 0, 't'},
      {"time2",    required_argument, 0, 'u'},
      {"timeout", required_argument, 0, 'v'},
      {"red",     required_argument, 0, 'r'},
      {"error",   required_argument, 0, 'r'},
      {"green",   required_argument, 0, 'g'},
      {"ready",   required_argument, 0, 'g'},
      {"help",    no_argument,       0, 'h'},
      {0, 0, 0, 0}
    };
    /* getopt_long stores the option index here. */
    int option_index = 0;
    char c;
     
    c = getopt_long (argc, argv, "t:r:g:h", long_options, &option_index);
     
    /* Detect the end of the options. */
    if (c == -1)
      break;
     
    switch (c)
    {
      case 0:
        /* If this option set a flag, do nothing else now. */
        if (long_options[option_index].flag != 0)
          break;
        if (verbose_flag)
          printf ("DEBUG option %s", long_options[option_index].name);
        if (optarg)
          printf (" with arg %s", optarg);
        printf ("\n");
        break;
     
      case 'r':
        if (verbose_flag)
          printf("DEBUG: red \"%s\"\n", optarg);
        value_red = strtol(optarg, NULL, 0);
        break;
        
      case 'g':
        if (verbose_flag)
          printf("DEBUG: green \"%s\"\n", optarg);
        value_green = strtol(optarg, NULL, 0);
        break;

      case 't':
        if (verbose_flag)
          printf("DEBUG: time \"%s\"\n", optarg);
        blink_time = strtof(optarg, NULL);
        break;

      case 'u':
        if (verbose_flag)
          printf("DEBUG: time \"%s\"\n", optarg);
        blink_time2 = strtof(optarg, NULL);
        break;

      case 'v':
        if (verbose_flag)
          printf("DEBUG: timeout \"%s\"\n", optarg);
        blink_timeout = strtol(optarg, NULL, 0);
        break;
     
      case 'h':
        /* getopt_long already printed an error message. */
        usage(argv[0]);
        break;
     
      default:
        usage(argv[0]);
    }
  }

  if (verbose_flag)
    printf("DEBUG: options: verbose:%d red:%d green:%d\n", verbose_flag, value_red, value_green);

  //validate options, abort upon mismatch
  if (!((verbose_flag==0 || verbose_flag==1) &&
        (value_red==-1   || value_red==0   || value_red==1) &&
        (value_green==-1 || value_green==0 || value_green==1)))
  {
    usage(argv[0]);
  }

  //stop operation if user did not supply options.
  if (value_red==-1 && value_green==-1)
  {
    printf("Not setting LED, arguments missing\n");
    usage(argv[0]);
  }

  if (verbose_flag)
  {
    printf("Setting LED:");
    if (value_red != -1)
    {
      printf(" red:%d", value_red);
    }
    if (value_green != -1)
    {
      printf(" green:%d", value_green);
    }
    printf("\n");
  }
}

void iowrite_single(int val, int port)
{
  if (val == 0 || val == 1)
  {
#if defined(__linux__) 
      outb(val, port);
#elif defined(BSD) 
      outb(port, val);
#endif
  }
}

void iowrite_prepare(void)
{
  //get I/O port permissions

#if defined(__linux__) 
    int status = ioperm(IOPORTS_6501_BASE, IOPORTS_6501_NUM, 1);
#elif defined(BSD) 
    int status = i386_set_ioperm(IOPORTS_6501_BASE, IOPORTS_6501_NUM, 1);
#endif

  if (status != 0)
  {
    perror("ledctl_6501 failed to get I/O port permissions");
    exit(1);
  }

  if (verbose_flag)
  {
    printf("ioperm went fine\n");
  }

}

void iowrite(void)
{
  //set the defined values, where needed
  iowrite_single(value_red,   IOPORTS_6501_RED);
  iowrite_single(value_green, IOPORTS_6501_GREEN);
}

void flip(int *p_val)
{
  *p_val = (*p_val) ? 0 : 1;
}

int main(int argc, char **argv)
{
  parseopt(argc, argv);
  iowrite_prepare();
  iowrite();

  if (demomode_flag)
  {
    int blink_value_green = value_green ;
    int blink_value_red = value_red ;
    int ublink_time = blink_time * 1000000;
    int ublink_time2 = blink_time2 * 1000000;
    int i = 0;
    time_t endwait;
    time_t start = time(NULL);
    time_t seconds = blink_timeout;
    endwait = start + seconds;

    while(start < endwait)
    {
      if (ublink_time2==0){
        usleep(ublink_time);
      }
      else {
	if (i%4!=1){
	  usleep(ublink_time);
	}
	else {
          if (alternate_flag) {
            iowrite_single(0,   IOPORTS_6501_RED);
            iowrite_single(0, IOPORTS_6501_GREEN);
          }
	  usleep(ublink_time2);
	}
      }
      i += 1;
      if (verbose_flag){
        printf("green : %d\n", blink_value_green);
        printf("red : %d\n", blink_value_red);
      }
      if (alternate_flag) {
	if (i%2==0){
	  blink_value_red = 1;
	  blink_value_green = 0;
	}
	else {
	  blink_value_red = 0;
	  blink_value_green = 1;
	}
      }
      else {
        if (value_red == 1){
          flip(&blink_value_red);
        }
        if (value_green == 1){
          flip(&blink_value_green);
        }
      }
      iowrite_single(blink_value_red,   IOPORTS_6501_RED);
      iowrite_single(blink_value_green, IOPORTS_6501_GREEN);
      start = time(NULL);
    }
    iowrite_single(0,   IOPORTS_6501_RED);
    iowrite_single(0, IOPORTS_6501_GREEN);
  }

  if (verbose_flag)
  {
    printf("LEDs have been set. Thank you for making a simple program very happy.\n");
  }

  return 0;
}

/* EOF */
