/* 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
  Constants - Network Interface Constants - written by Stephane Vaxelaire <svax@free.fr>
*/

#ifndef __CONST_H
#define __CONST_H

#include "config.h"
#include <assert.h>
#include <stdio.h>

#define SERVERCONFIGFILE	"vegaserver.config"
#define ACCTCONFIGFILE		"accountserver.config"

#define MAXCLIENTS	512
#define SERVER_PORT 6777
#define CLIENT_PORT	6778
#define ACCT_PORT	6779
#define NAMELEN		32
#define MAXBUFFER	16384

#define MAXSERIAL 0xFFFF
#define TONET() htons()
typedef unsigned short ObjSerial;
typedef unsigned int InstSerial;

extern double NETWORK_ATOM;

#if defined(_WIN32) && defined(_MSC_VER) && defined(USE_BOOST_129) //wierd error in MSVC
#define COUT std::cout << __FILE__ << ":"
#else
#define COUT std::cout << __FILE__ << ":" << __LINE__ << " "
#endif

#if defined(_WIN32) && !defined(__CYGWIN__) || defined(__APPLE__)
	//#warning "Win32 platform"
	#define in_addr_t unsigned long
	#define socklen_t int
#else
	//#warning "GCC platform"
  #ifndef SOCKET_ERROR
  #define SOCKET_ERROR -1
  #endif
#endif

#ifndef NDEBUG
#define VSNET_DEBUG
#else
#undef VSNET_DEBUG
#endif

#undef VSNET_DEBUG

/* --- BEGIN memory debugging macros --- */
/** The following macros are intended for default versions a class validity
 *  debugging. The idea is to increase the likelyhood that uninitialized
 *  and re-initialized memory areas are captured as well as possible.
 */
#ifndef NDEBUG

#include <assert.h>
#include <stdio.h>

#define DECLARE_VALID \
private: \
    bool _valid; \
    bool _invalid; \
public: \
    void validate( const char* file, int line ) const { \
	if( !_valid || _invalid ) { \
	    fprintf( stderr, "object invalid in %s:%d\n", file, line ); \
	} \
        assert( _valid ); \
        assert( !_invalid ); \
    }

#define MAKE_VALID \
    _valid = true; \
    _invalid = false;

#define MAKE_INVALID \
    _valid = false; \
    _invalid = true;

#define CHECK_VALID \
    validate( __FILE__, __LINE__ );

#define CHECK_VALID_OBJ(a) \
    (a).validate( __FILE__, __LINE__ );

#define ASSERT(a) if(!(a)) { std::cerr << __FILE__ << ":" << __LINE__ << " assertion failed, forcing segfault for postmortem debugging"; int x = 1/0; }

#else /* NDEBUG */
#  define DECLARE_VALID
#  define MAKE_VALID
#  define MAKE_INVALID
#  define CHECK_VALID
#  define CHECK_VALID_OBJ(a)
#  define ASSERT(a)
#endif /* NDEBUG */
/* --- END memory debugging macros --- */

#ifndef _WIN32
#define LOCALCONST_DECL(Type,Name,Value) static const Type Name = Value;
#define LOCALCONST_DEF(Class,Type,Name,Value)
#else
#ifndef __PRETTY_FUNCTION
#define __PRETTY_FUNCTION__ "<Unknown>"
#endif

#ifndef __FUNCTION
#if (_MSC_VER < 1300)  // 1300 == VC++ 7.0
#define __FUNCTION__ "<Unknown>"
#endif
#endif

#define LOCALCONST_DECL(Type,Name,Value) static Type Name;
#define LOCALCONST_DEF(Class,Type,Name,Value) Type Class::Name = Value;
#endif

#endif /* __CONST_H */

