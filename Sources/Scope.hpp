/*
// Copyright (c) 2012-2014 Eliott Paris, Julien Colafrancesco & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#ifndef DEF_HOA_SCOPE_LIGHT
#define DEF_HOA_SCOPE_LIGHT

#include "Encoder.hpp"
#include "Planewaves.hpp"

namespace hoa
{
    
    template <Dimension D, typename T> class Scope;
    
    //! The ambisonic scope.
    /** The scope discretize a circle by a set of point and uses a decoder to project the circular harmonics on it. This class should be used for graphical interfaces outside the digital signal processing if the number of points to discretize the circle is very large. Then you should prefer to record snapshot of the circular harmonics and to call the process method at an interval adapted to a graphical rendering.
     */
    template <typename T> class Scope<Hoa2d, T> : public Encoder<Hoa2d, T>, protected Planewave<Hoa2d, T>::Processor
    {
    private:
        T*  m_matrix;
        T*  m_vector;
        T   m_maximum;
    public:
        
        //! The scope constructor.
        /**	The scope constructor allocates and initialize the member values to computes circular harmonics projection on a circle depending on a decomposition order and a circle discretization. The circle is discretized by the number of points. The order must be at least 1. The number of points and column should be at least 3 (but it's very low).
         
         @param     order            The order.
         @param     numberOfPoints   The number of points.
         */
        Scope(ulong order, ulong numberOfPoints) :
        Encoder<Hoa2d, T>(order),
        Planewave<Hoa2d, T>::Processor(numberOfPoints)
        {
            m_matrix = new T[Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves() * Encoder<Hoa2d, T>::getNumberOfHarmonics()];
            m_vector = new T[Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves()];
            
            const T factor = 1. / (T)(Encoder<Hoa2d, T>::getDecompositionOrder() + 1.);
            for(ulong i = 0; i < Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves(); i++)
            {
                Encoder<Hoa2d, T>::setAzimuth(Planewave<Hoa2d, T>::Processor::getPlanewaveAzimuth(i));
                Encoder<Hoa2d, T>::process(&factor, m_matrix + i * Encoder<Hoa2d, T>::getNumberOfHarmonics());
                m_matrix[i * Encoder<Hoa2d, T>::getNumberOfHarmonics()] = factor * 0.5;
            }
            for(ulong i = 0; i < Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves(); i++)
            {
                m_vector[i] = 0.;
            }
            m_maximum = 0;
        }
        //! The Scope destructor.
        /**	The Scope destructor free the memory.
         */
        ~Scope()
        {
            delete [] m_matrix;
            delete [] m_vector;
        }
        
        //! Retrieve the number of points.
        /**	Retrieve the number of points used to discretize the ambisonic circle.
         @return     This method returns the number of points used to discretize the circle.
         */
        inline ulong getNumberOfPoints() const noexcept
        {
            return Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves();
        }
        
        //! Retrieve the value of a point of the circular harmonics projection.
        /**	Retrieve the result value of the circular harmonics projection for a given point defined by an index. The absolute of the value can be used as the radius of the point for a 2 dimentionnal representation. For the index, 0 is the 0 azimtuh of the circle. The maximum index must be the number of points - 1.
         @param     index   The point index of the point.
         @return    This method returns the value of a point of the ambisonic circle.
         */
        inline T getPointValue(const ulong index) const noexcept
        {
            return m_vector[index];
        }
        
        //! Retrieve the radius of a point of the circular harmonics projection.
        /**	Retrieve the radius of the circular harmonics projection for a given point defined by an index. This the absolute of the result of the projection. For the index, 0 is the 0 azimtuh of the circle. The maximum index must be the number of points - 1.
         @param     pointIndex   The point index of the point.
         @return    This method returns the radius of a point of the ambisonic circle.
         */
        inline T getPointRadius(const ulong index) const noexcept
        {
            return fabs(m_vector[index]);
        }
        
        //! Retrieve the azimuth of a point of the circular harmonics projection.
        /**	Retrieve the azimuth of the circular harmonics projection for a given point defined by an index.The maximum index must be the number of points - 1.
         @param     pointIndex   The point index of the point.
         @return    This method returns the azimuth of a point of the ambisonic circle.
         */
        inline T getPointAzimuth(const ulong index) const noexcept
        {
            return Planewave<Hoa2d, T>::Processor::getPlanewaveAzimuth(index);
        }
        
        //! Retrieve the abscissa of a point of the circular harmonics projection.
        /**	Retrieve the abscissa of the circular harmonics projection for a given point defined by an index.The maximum index must be the number of points - 1.
         
         @param     pointIndex   The point index of the point.
         @return    This method returns the abscissa of a point of the ambisonic circle.
         
         @see       getOrdinate
         */
        inline double getPointAbscissa(const ulong index) const noexcept
        {
            return fabs(m_vector[index]) * Planewave<Hoa2d, T>::Processor::getPlanewaveAbscissa(index);
        }
        
        //! Retrieve the ordinate of a point of the circular harmonics projection.
        /**	Retrieve the ordinate of the circular harmonics projection for a given point defined by an index.The maximum index must be the number of points - 1.
         
         @param     pointIndex   The point index of the point.
         @return    This method returns the ordinate of a point of the ambisonic circle.
         
         @see       getAbscissa
         */
        inline double getPointOrdinate(const ulong index) const noexcept
        {
            return fabs(m_vector[index]) * Planewave<Hoa2d, T>::Processor::getPlanewaveOrdinate(index);
        }
        
        //! This method performs the circular harmonics projection.
        /**	You should use this method to compute the projection of the circular harmonics over an ambisonics circle. The inputs array contains the circular harmonics samples and the minimum size must be the number of harmonics.
         @param     inputs   The inputs array.
         */
        inline void process(const T* inputs) noexcept
        {
            Signal<T>::matrix_vector_mul(Encoder<Hoa2d, T>::getNumberOfHarmonics(), Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves(), inputs, m_matrix, m_vector);
            m_maximum = fabs(Signal<T>::vector_max(Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves(), m_vector));
            if(m_maximum > 1.)
            {
                Signal<T>::vector_scale(Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves(), (1. / m_maximum), m_vector);
            }
        }
    };
    
    //! The ambisonic scope.
    /** The scope discretize a sphere by a set of point and uses a decoder to project the spherical harmonics on it. This class should be used for graphical interfaces outside the digital signal processing if the number of points to discretize the sphere is very large. Then you should prefer to record snapshot of the spherical harmonics and to call the process method at an interval adapted to a graphical rendering.
     */
    template <typename T> class Scope<Hoa3d, T> : public Encoder<Hoa3d, T>, protected Planewave<Hoa3d, T>::Processor
    {
    private:
        unsigned int m_number_of_rows;
        unsigned int m_number_of_columns;
        double*     m_harmonics;
        double*     m_matrix;
    public:
        
        //! The Scope constructor.
        /**	The Scope constructor allocates and initialize the member values to computes spherical harmonics projection on a sphere depending on a decomposition order and a sphere discretization. The sphere discretization is done by a set of points defined by rows and columns then the precision will be lower at the elevation center (0 radian) than at the top (1/2 Pi) or the bottom (-1/2 Pi) of the sphere. The number of row discretize the elevation then it set how many points are used between the bottom and the top. The number of column discretize the azimuth circle then it set how many points are used to make the turn from the front (O radian). Then the sphere is discretized by number of rows * number of columns points. The order must be at least 1. The number of rows and column should be at least 3 (but it's very low).
         
         @param     order            The order.
         @param     numberOfRow      The number of rows.
         @param     numberOfColumn	The number of columns.
         */
        Scope(unsigned int order, unsigned int numberOfRows, unsigned int numberOfColumns);
        
        //! The Scope destructor.
        /**	The Scope destructor free the memory.
         */
        ~Scope();
        
        //! Retrieve the number of rows.
        /**	Retrieve the number of rows used to discretize the ambisonic sphere.
         
         @return     This method returns the number of rows used to discretize the sphere.
         */
        inline unsigned int getNumberOfRows() const
        {
            return m_number_of_rows;
        }
        
        //! Retrieve the number of column.
        /**	Retrieve the number of column used to discretize the ambisonic sphere.
         
         @return     This method returns the number of column used to discretize the sphere.
         */
        inline unsigned int getNumberOfColumns() const
        {
            return m_number_of_columns;
        }
        
        //! Retrieve the value of a point of the spherical harmonics projection.
        /**	Retrieve the result value of the spherical harmonics projection for a given point defined by a row index and a column index. The absolute of the value can be used as the radius of the point for a 3 dimentionnal representation. For the row index, 0 is the bottom of the sphere, number of rows / 2 is at the center of the elevation and number of rows - 1 is at the top of the sphere. For the column index, 0 is the front (0 radian) and number of columns / 2 is the rear of the sphere. The maximum row index must be the number of row - 1 and the maximum column index must be the number of columns - 1.
         
         @param     rowIndex     The row index of the point.
         @param     columnIndex  The column index of the point.
         @return    This method returns the value of a point of the ambisonic sphere.
         @see       getradius
         @see       getAzimuth
         @see       getElevation
         */
        inline double getValue(unsigned int rowIndex, unsigned int columnIndex) const
        {
            assert(rowIndex < m_number_of_rows);
            assert(columnIndex < m_number_of_columns);
            return m_matrix[rowIndex * m_number_of_columns + columnIndex];
        }
        
        //! Retrieve the radius of a point of the spherical harmonics projection.
        /**	Retrieve the radius of the spherical harmonics projection for a given point defined by a row index and a column index. This the absolute of the result of the projection. For the row index, 0 is the bottom of the sphere, number of rows / 2 is at the center of the elevation and number of rows - 1 is at the top of the sphere. For the column index, 0 is the front (0 radian) and number of columns / 2 is the rear of the sphere. The maximum row index must be the number of row - 1 and the maximum column index must be the number of columns - 1.
         
         @param     rowIndex     The row index of the point.
         @param     columnIndex  The column index of the point.
         @return    This method returns the radius of a point of the ambisonic sphere.
         @see       getAzimuth
         @see       getElevation
         @see       getValue
         */
        inline double getRadius(unsigned int rowIndex, unsigned int columnIndex) const
        {
            assert(rowIndex < m_number_of_rows);
            assert(columnIndex < m_number_of_columns);
            return fabs(m_matrix[rowIndex * m_number_of_columns + columnIndex]);
        }
        
        //! Retrieve the azimuth of a point of the spherical harmonics projection.
        /**	Retrieve the azimuth of the spherical harmonics projection for a given point defined by a row index and a column index. For the column index, 0 is the front (0 radian) and number of columns / 2 is the rear of the sphere. The maximum column index must be the number of columns - 1.
         
         @param     rowIndex     The row index of the point.
         @param     columnIndex  The column index of the point.
         @return    This method returns the azimuth of a point of the ambisonic sphere.
         @see       getValue
         @see       getRadius
         @see       getElevation
         */
        inline double getAzimuth(unsigned int columnIndex) const
        {
            assert(columnIndex < m_number_of_columns);
            return (double)columnIndex * HOA_2PI / (double)m_number_of_columns;
        }
        
        //! Retrieve the elevation of a point of the spherical harmonics projection.
        /**	Retrieve the elevation of the spherical harmonics projection for a given point defined by a row index. For the row index, 0 is the bottom of the sphere, number of rows / 2 is at the center of the elevation and number of rows - 1 is at the top of the sphere. The maximum row index must be the number of row - 1.
         
         @param     rowIndex     The row index of the point.
         @param     columnIndex  The column index of the point.
         @return    This method returns the elevation of a point of the ambisonic sphere.
         @see       getValue
         @see       getRadius
         @see       getAzimuth
         */
        inline double getElevation(unsigned int rowIndex) const
        {
            assert(rowIndex < m_number_of_rows);
            return (double)rowIndex * HOA_PI / (double)(m_number_of_rows - 1) - HOA_PI2;
        }
        
        //! This method performs the spherical harmonics projection with single precision.
        /**	You should use this method to compute the projection of the spherical harmonics over an ambisonics sphere. The inputs array contains the spherical harmonics samples and the minimum size must be the number of harmonics.
         
         @param     inputs   The inputs array.
         */
        void process(const float* inputs);
        
        //! This method performs the spherical harmonics projection with double precision.
        /**	You should use this method to compute the projection of the spherical harmonics over an ambisonics sphere. The inputs array contains the spherical harmonics samples and the minimum size must be the number of harmonics.
         
         @param     inputs   The inputs array.
         */
        void process(const double* inputs);
    };

}

#endif



