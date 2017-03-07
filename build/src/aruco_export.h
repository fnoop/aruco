
#ifndef ARUCO_EXPORT_H
#define ARUCO_EXPORT_H

#ifdef ARUCO_STATIC_DEFINE
#  define ARUCO_EXPORT
#  define ARUCO_NO_EXPORT
#else
#  ifndef ARUCO_EXPORT
#    ifdef ARUCO_DSO_EXPORTS
        /* We are building this library */
#      define ARUCO_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define ARUCO_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef ARUCO_NO_EXPORT
#    define ARUCO_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef ARUCO_DEPRECATED
#  define ARUCO_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef ARUCO_DEPRECATED_EXPORT
#  define ARUCO_DEPRECATED_EXPORT ARUCO_EXPORT ARUCO_DEPRECATED
#endif

#ifndef ARUCO_DEPRECATED_NO_EXPORT
#  define ARUCO_DEPRECATED_NO_EXPORT ARUCO_NO_EXPORT ARUCO_DEPRECATED
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define ARUCO_NO_DEPRECATED
#endif

#endif
