////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief     Utility definitions (types, etc.).
/// \author    Sergey Shershakov, Anton Rigin
/// \version   0.1.0
/// \date      01.05.2017 -- 04.02.2018
///            This is a part of the course "Algorithms and Data Structures" 
///            provided by  the School of Software Engineering of the Faculty 
///            of Computer Science at the Higher School of Economics
///            and of the course work of Anton Rigin,
///            the HSE Software Engineering 3-rd year bachelor student.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef BTREE_UTILS_H_
#define BTREE_UTILS_H_

// To mark method as deprecated.
#ifdef __GNUC__
#define DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED __declspec(deprecated)
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED
#endif

namespace btree {

//==============================================================================
// Types
//==============================================================================

typedef unsigned char Byte;
typedef unsigned short UShort;
typedef unsigned int UInt;

#ifdef BTREE_WITH_REUSING_FREE_PAGES

typedef unsigned long ULong;

#endif

} // namespace btree

#endif // BTREE_UTILS_H_
