/*!
 * Macros for defining how VTK check the prefixes on class and file anmes etc.
 */


#define PREFIX_STRINGS_MATCH == 0
#define PREFIX_STRINGS_DONT_MATCH != 0

/* Base string checks - these are what vtk does already */
#define PREFIX_STRING_CHECK_BASE(ss) strncmp(ss, "vtk", 3)
#define PREFIX_STRING_CHECK_BASE_UC(ss) strncmp(ss, "VTK", 3)
#define PREFIX_STRING_CHECK_BASE_NS(ss) strncmp(ss, "vtk::", 5)

/* Additional string check - these are meant to be overridden at compile time */
#ifndef PREFIX_STRING_CHECK_ADDITIONAL
	#define PREFIX_STRING_CHECK_ADDITIONAL(ss) || strncmp(ss, "msf", 3)
#endif
#ifndef PREFIX_STRING_CHECK_ADDITIONAL_UC
	#define PREFIX_STRING_CHECK_ADDITIONAL_UC(ss) || strncmp(ss, "MSF", 3)
#endif
#ifndef PREFIX_STRING_CHECK_ADDITIONAL_NS
	#define PREFIX_STRING_CHECK_ADDITIONAL_NS(ss) || strncmp(ss, "msf::", 5)
#endif


/* Build conditionals used in code */
#define PREFIX_CHECK_STRINGS_MATCH(ss) \
	(PREFIX_STRING_CHECK_BASE(ss) PREFIX_STRINGS_MATCH PREFIX_STRING_CHECK_ADDITIONAL(ss) PREFIX_STRINGS_MATCH )
#define PREFIX_CHECK_STRINGS_MATCH_UC(ss) \
	(PREFIX_STRING_CHECK_BASE_UC(ss) PREFIX_STRINGS_MATCH PREFIX_STRING_CHECK_ADDITIONAL_UC(ss) PREFIX_STRINGS_MATCH )
#define PREFIX_CHECK_STRINGS_MATCH_NS(ss) \
	(PREFIX_STRING_CHECK_BASE_NS(ss) PREFIX_STRINGS_MATCH PREFIX_STRING_CHECK_ADDITIONAL_NS(ss) PREFIX_STRINGS_MATCH )

#define PREFIX_CHECK_STRINGS_DONT_MATCH(ss) ( !( PREFIX_CHECK_STRINGS_MATCH(ss) ) )
#define PREFIX_CHECK_STRINGS_DONT_MATCH_UC(ss) ( !( PREFIX_CHECK_STRINGS_MATCH_UC(ss) ) )
#define PREFIX_CHECK_STRINGS_DONT_MATCH_NS(ss) ( !( PREFIX_CHECK_STRINGS_MATCH_NS(ss) ) )





