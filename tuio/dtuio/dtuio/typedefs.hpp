/**
 * \file      typedefs.hpp
 * \brief     Typedefs for this library that are (for some reason) included in libkerat
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-10-28 11:27 UTC+1
 * \copyright BSD
 */

#ifndef DTUIO_TYPEDEFS_HPP
#define DTUIO_TYPEDEFS_HPP

#ifdef HAVE_STDINT_H
    #include <stdint.h>
#endif

namespace dtuio {
    
    //! \brief typedef for uuid, libuuid compatible
    typedef uint8_t uuid_t[16];
    
    //! \brief capacity required to store uuid in cstring
    const size_t UUID_STRING_CAPACITY = 40;
    
}

#endif // DTUIO_TYPEDEFS_HPP
