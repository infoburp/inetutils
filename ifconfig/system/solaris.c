/* solaris.c -- Solaris specific code for ifconfig
  Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,
  2010 Free Software Foundation, Inc.

  This file is part of GNU Inetutils.

  GNU Inetutils is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or (at
  your option) any later version.

  GNU Inetutils is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see `http://www.gnu.org/licenses/'. */

/* Written by Marcus Brinkmann.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if HAVE_STRING_H
# include <string.h>
#else
# include <strings.h>
#endif

#if STDC_HEADERS
# include <stdlib.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
#endif

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>

#include "../ifconfig.h"


/* Output format stuff.  */

const char *system_default_format = "unix";


/* Argument parsing stuff.  */

const char *system_help = "\
NAME [ADDR [DSTADDR]] [broadcast BRDADDR] [netmask MASK] [metric N] [mtu N]";

struct argp_child system_argp_child;

int
system_parse_opt (struct ifconfig **ifp, char option, char *optarg)
{
  return 0;
}

int
system_parse_opt_rest (struct ifconfig **ifp, int argc, char *argv[])
{
  int i = 0;
  enum
  {
    EXPECT_NOTHING,
    EXPECT_BROADCAST,
    EXPECT_NETMASK,
    EXPECT_METRIC,
    EXPECT_MTU
  } expect = EXPECT_NOTHING;

  *ifp = parse_opt_new_ifs (argv[0]);

  while (++i < argc)
    {
      switch (expect)
	{
	case EXPECT_BROADCAST:
	  parse_opt_set_brdaddr (*ifp, argv[i]);
	  break;

	case EXPECT_NETMASK:
	  parse_opt_set_netmask (*ifp, argv[i]);
	  break;

	case EXPECT_MTU:
	  parse_opt_set_mtu (*ifp, argv[i]);
	  break;

	case EXPECT_METRIC:
	  parse_opt_set_metric (*ifp, argv[i]);
	  break;

	case EXPECT_NOTHING:
	  break;
	}

      if (expect != EXPECT_NOTHING)
	expect = EXPECT_NOTHING;
      else if (!strcmp (argv[i], "broadcast"))
	expect = EXPECT_BROADCAST;
      else if (!strcmp (argv[i], "netmask"))
	expect = EXPECT_NETMASK;
      else if (!strcmp (argv[i], "metric"))
	expect = EXPECT_METRIC;
      else if (!strcmp (argv[i], "mtu"))
	expect = EXPECT_MTU;
      else
	{
	  /* Recognize AF here.  */
	  /* Recognize up/down.  */
	  /* Also auto-revarp, trailers, -trailers,
	     private, -private, arp, -arp, plumb, unplumb.  */
	  if (!((*ifp)->valid & IF_VALID_ADDR))
	    parse_opt_set_address (*ifp, argv[i]);
	  else if (!((*ifp)->valid & IF_VALID_DSTADDR))
	    parse_opt_set_dstaddr (*ifp, argv[i]);
	}
    }

  switch (expect)
    {
    case EXPECT_BROADCAST:
      error (0, 0, "option `broadcast' requires an argument");
      break;

    case EXPECT_NETMASK:
      error (0, 0, "option `netmask' requires an argument");
      break;

    case EXPECT_METRIC:
      error (0, 0, "option `metric' requires an argument");
      break;

    case EXPECT_MTU:
      error (0, 0, "option `mtu' requires an argument");
      break;

    case EXPECT_NOTHING:
      break;
    }
  return expect == EXPECT_NOTHING;
}

int
system_configure (int sfd, struct ifreq *ifr, struct system_ifconfig *ifs)
{
#ifdef IF_VALID_TXQLEN
  if (ifs->valid & IF_VALID_TXQLEN)
    {
# ifndef SIOCSIFTXQLEN
      error (0, 0, "don't know how to set the txqlen on this system");
      return -1;
# else
      int err = 0;

      ifr->ifr_qlen = ifs->txqlen;
      err = ioctl (sfd, SIOCSIFTXQLEN, ifr);
      if (err < 0)
	error (0, errno, "SIOCSIFTXQLEN failed");
      if (verbose)
	printf ("Set txqlen value of `%s' to `%i'.\n",
		ifr->ifr_name, ifr->ifr_qlen);
# endif
    }
  return 0;
#endif
}
