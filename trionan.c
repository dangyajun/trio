/*************************************************************************
 *
 * $Id$
 *
 * Copyright (C) 2001 Bjorn Reese <breese@users.sourceforge.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS AND
 * CONTRIBUTORS ACCEPT NO RESPONSIBILITY IN ANY CONCEIVABLE MANNER.
 *
 ************************************************************************/

static const char rcsid[] = "@(#)$Id$";

#include "trionan.h"

/*************************************************************************
 * Platform and compiler support detection
 */
#if defined(unix)
# define PLATFORM_UNIX
#elif defined(__xlC__) || defined(_AIX)
# define PLATFORM_UNIX
#elif defined(__QNX__)
# defined PLATFORM_UNIX
#endif

#if defined(_MSC_VER)
# define COMPILER_MSVC
#endif

#if defined(__STDC__) && defined(__STDC_VERSION__)
# if (__STDC_VERSION__ >= 199901L)
#  define COMPILER_SUPPORTS_C99
# endif
#endif

#if defined(_XOPEN_SOURCE)
# if defined(_XOPEN_SOURCE_EXTENDED)
#  define COMPILER_SUPPORTS_UNIX95
# endif
#endif

/*************************************************************************
 * Include files
 */
#include <math.h>
#include <float.h>
#if defined(PLATFORM_UNIX)
# include <signal.h>
#endif

/* Some IBM xlC compiler fu */
#if defined(__xlC__)
# pragma options float=nans
#endif

/*************************************************************************
 * NumberDivide
 */
static double
InternalNumberDivide(double dividend, double divisor)
{
#if defined(PLATFORM_UNIX)
  double return_value;
  void (*signal_handler)(int) = signal(SIGFPE, SIG_IGN);
  return_value = dividend / divisor;
  signal(SIGFPE, signal_handler);
  return return_value;
  
#else
  return dividend / divisor;
  
#endif
}

/*************************************************************************
 * trio_nan
 */
double
trio_nan()
{
#if defined(COMPILER_SUPPORTS_C99)
  return nan(NULL);
  
#elif defined(DBL_QNAN)
  return DBL_QNAN;
  
#else
  return InternalNumberDivide(0.0, 0.0);
  
#endif
}

/*************************************************************************
 * trio_pinf
 */
double
trio_pinf()
{
  return InternalNumberDivide(1.0, 0.0);
}

/*************************************************************************
 * trio_ninf
 */
double
trio_ninf()
{
  return InternalNumberDivide(-1.0, 0.0);
}

/*************************************************************************
 * trio_isnan
 */
int
trio_isnan(double number)
{
#if defined(isnan)
  /*
   * C99 defines isnan() as a macro
   */
  return isnan(number);
  
#elif defined(COMPILER_SUPPORTS_UNIX95)
  /*
   * UNIX95 defines isnan() as a function. This function was already
   * present in XPG4, but this is a bit tricky to detect with compiler
   * defines, so we choose the conservative approach and only use it
   * for UNIX95.
   */
  return isnan(number);
  
#elif defined(COMPILER_MSVC)
  /*
   * MSC has an _isnan() function
   */
  return _isnan(number);
  
#else
  /*
   * Fallback solution
   */
  int status;
  double integral, fraction;
# if defined(PLATFORM_UNIX)
  void (*signal_handler)(int) = signal(SIGFPE, SIG_IGN);
# endif
  status = (/*
	     * NaN is the only number which does not compare to itself
	     */
	    (number != number) ||
	    /*
	     * Fallback solution if NaN compares to NaN
	     */
	    ((number != 0.0) &&
	     (fraction = modf(number, &integral),
	      integral == fraction)));
# if defined(PLATFORM_UNIX)
  signal(SIGFPE, signal_handler);
#endif
  return status;
  
#endif
}

/*************************************************************************
 * trio_isinf
 */
int
trio_isinf(double number)
{
#if defined(isinf)
  /*
   * C99 defines isinf() as a macro
   */
  return isinf(number);
  
#elif defined(COMPILER_MSVC)
  /*
   * MSC has an _fpclass() function that can be used to detect infinty
   */
  return ((_fpclass(number) == _FPCLASS_PINF)
	  ? 1
	  : ((_fpclass(number) == _FPCLASS_NINF) ? -1 : 0));
  
#else
  /*
   * Fallback solution
   */
  int status;
# if defined(PLATFORM_UNIX)
  void (*signal_handler)(int) = signal(SIGFPE, SIG_IGN);
# endif
  status = ((number == HUGE_VAL)
	    ? 1
	    : ((number == -HUGE_VAL) ? -1 : 0));
# if defined(PLATFORM_UNIX)
  signal(SIGFPE, signal_handler);
# endif
  return status;
  
#endif
}

/*************************************************************************
 */
#if defined(STANDALONE)
# include <stdio.h>

int main(int argc, char *argv[])
{
  double my_nan;
  double my_pinf;
  double my_ninf;

  my_nan = trio_nan();
  my_pinf = trio_pinf();
  my_ninf = trio_ninf();

  printf("NaN : %4g (%2d, %2d)\n",
	 my_nan, trio_isnan(my_nan), trio_isinf(my_nan));
  printf("PInf: %4g (%2d, %2d)\n",
	 my_pinf, trio_isnan(my_pinf), trio_isinf(my_pinf));
  printf("NInf: %4g (%2d, %2d)\n",
	 my_ninf, trio_isnan(my_ninf), trio_isinf(my_ninf));
  
  return 0;
}
#endif
