/*!
 * Macros for defining how VTK check the prefixes on class and file anmes etc.
 */


#define PREFIX_CHECK_MACRO_BASE(ss) strncmp(ss, "vtk", 3) == 0

#ifndef PREFIX_CHECK_MACRO_ADDITIONAL
//#define PREFIX_CHECK_MACRO_ADDITIONAL(ss) || (0==0)
#define PREFIX_CHECK_MACRO_ADDITIONAL(ss) || strncmp(ss, "msf", 3) == 0
#endif

#define PREFIX_CHECK_MACRO(ss) 					\
			PREFIX_CHECK_MACRO_BASE(ss)			\
			PREFIX_CHECK_MACRO_ADDITIONAL(ss)
