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
#include <sys/io.h>


#define IOPORTS_6501_BASE 0x69C
#define IOPORTS_6501_NUM  2
#define IOPORTS_6501_RED   (IOPORTS_6501_BASE+0)
#define IOPORTS_6501_GREEN (IOPORTS_6501_BASE+1)

int verbose_flag=0;
int value_red=-1;   /* 1:on, 0:off, -1:unchanged */
int value_green=-1; /* 1:on, 0:off, -1:unchanged */
int demomode_flag=0;

void usage(char *argv0)
{
  printf("Usage: %s [--red=0/1] [--green=0/1] [--verbose] [--help]\n\n", argv0);
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
      /* These options don't set a flag.
      We distinguish them by their indices. */
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
     
    c = getopt_long (argc, argv, "r:g:h", long_options, &option_index);
     
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
    outb(val, port);
  }
}

void iowrite_prepare(void)
{
  //get I/O port permissions
  int status = ioperm(IOPORTS_6501_BASE, IOPORTS_6501_NUM, 1);

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
    while(1)
    {
      sleep(1);
      flip(&value_red);
      flip(&value_green);
      iowrite();
    }
  }

  if (verbose_flag)
  {
    printf("LEDs have been set. Thank you for making a simple program very happy.\n");
  }

  return 0;
}

/* EOF */
