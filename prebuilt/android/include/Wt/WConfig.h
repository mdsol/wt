#ifndef WCONFIG_H
#define WCONFIG_H

// Version defines
#define WT_SERIES 4
#define WT_MAJOR  1
#define WT_MINOR  0

/*! \brief A constant that encodes the library version of %Wt
 *
 * You may use this constant to check for the version of %Wt at build-time.
 */
#define WT_VERSION (((WT_SERIES & 0xff) << 24) | ((WT_MAJOR & 0xff) << 16) | ((WT_MINOR & 0xff) << 8))
#define WT_VERSION_STR "4.1.0"
#define WT_CLASS       "Wt4_1_0"
#define WT_INCLUDED_VERSION Wt_4_1_0

#define RUNDIR "/var/run/wt"
#define WT_CONFIG_XML "/etc/wt/wt_config.xml"
#define WTHTTP_CONFIGURATION "/etc/wt/wthttpd"

#define WT_STATIC
#define WTDBO_STATIC
#define WTDBOPOSTGRES_STATIC
#define WTDBOSQLITE3_STATIC
#define WTDBOFIREBIRD_STATIC
#define WTDBOMYSQL_STATIC
#define WTDBOMSSQLSERVER_STATIC
#define WTHTTP_STATIC

/* #undef WT_HAS_WRASTERIMAGE */
/* #undef WT_HAS_WPDFIMAGE */
/* #undef WT_WITH_SSL */

/* #undef WT_USE_OPENGL */
/* #undef WT_DEBUG_ENABLED */
#define WT_THREADED

#define WT_ANY_IS_THELINK2012_ANY
/* #undef WT_ANY_IS_EXPERIMENTAL_ANY */
/* #undef WT_ANY_IS_STD_ANY */

#define WT_ASIO_IS_BOOST_ASIO
/* #undef WT_ASIO_IS_STANDALONE_ASIO */

#define WT_WARN_HEADER_MISSING_H

// our win32: WIN32 (gcc) or _WIN32 (MSC)
#if defined(WIN32) || defined(_WIN32)
#define WT_WIN32 1
#endif

#endif
