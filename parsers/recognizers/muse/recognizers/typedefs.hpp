/**
 * \file      typedefs.hpp
 * \brief     Holds type definitions for recognizers
 * \author    Martin Luptak <374178@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2013-02-14 13:10 UTC+1
 * \copyright BSD
 */

#ifndef TYPEDEFS_HPP_
#define	TYPEDEFS_HPP_

namespace libreco {
    namespace recognizers {
        
        //! \brief Holds scores (in descending order) and names of templates after recognition process is done
        typedef std::multimap<float, std::string, std::greater<float> > recognized_gestures;
    }
    
}

#endif	/* TYPEDEFS_HPP */

