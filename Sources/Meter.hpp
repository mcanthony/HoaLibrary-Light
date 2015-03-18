/*
// Copyright (c) 2012-2015 Eliott Paris, Julien Colafrancesco & Pierre Guillot, CICM, Universite Paris 8.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#ifndef DEF_HOA_METER_LIGHT
#define DEF_HOA_METER_LIGHT

#include "Planewaves.hpp"
#include "Voronoi.hpp"

namespace hoa
{
    template <Dimension D, typename T> class Meter;
    
    template <typename T> class Meter<Hoa2d, T> : public Planewave<Hoa2d, T>::Processor
    {
    private:
        ulong   m_ramp;
        ulong   m_vector_size;
        T*      m_channels_peaks;
        T*      m_channels_azimuth_mapped;
        T*      m_channels_azimuth_width;
        ulong*  m_over_leds;
        
    public:
        
        Meter(ulong numberOfPlanewaves) noexcept :
        Planewave<Hoa2d, T>::Processor(numberOfPlanewaves)
        {
            m_ramp                      = 0;
            m_vector_size               = 0;
            m_channels_peaks            = new T[Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves()];
            m_channels_azimuth_width    = new T[Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves()];
            m_channels_azimuth_mapped   = new T[Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves()];
            m_over_leds                 = new ulong[Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves()];
            for(ulong i = 0; i < Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves(); i++)
            {
                m_channels_peaks[i] = 0;
                m_over_leds[i]      = 0;
            }
        }
        
        ~Meter()
        {
            delete [] m_channels_peaks;
            delete [] m_channels_azimuth_width;
            delete [] m_channels_azimuth_mapped;
            delete [] m_over_leds;
        }
        
        inline void setVectorSize(ulong vectorSize) noexcept
        {
            m_vector_size   = vectorSize;
            m_ramp          = 0;
        }
        
        inline ulong getVectorSize() const noexcept
        {
            return m_vector_size;
        }
        
        void computeRendering()
        {
            if(Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves() == 1)
            {
                m_channels_azimuth_width[0] = HOA_2PI;
                m_channels_azimuth_mapped[0]= 0.;
            }
            else
            {
                vector<Planewave<Hoa2d, T> > channels;
                for(ulong i = 0; i < Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves(); i++)
                {
                    channels.push_back(Planewave<Hoa2d, T>(i, Math<T>::wrap_twopi(Planewave<Hoa2d, T>::Processor::getPlanewaveAzimuth(i) + Planewave<Hoa2d, T>::Processor::getPlanewavesRotation())));
                }
                std::sort(channels.begin(), channels.end(), Planewave<Hoa2d, T>::sort_azimuth);
                {
                    const T current_angle   = channels[0].getAzimuth();
                    const T previous_angle  = channels[channels.size() - 1].getAzimuth();
                    const T next_angle      = channels[1].getAzimuth();
                    const T previous_portion= (HOA_2PI - previous_angle) + current_angle;
                    const T next_portion    = next_angle - current_angle;
                    m_channels_azimuth_width[channels[0].getIndex()] = (previous_portion + next_portion) * 0.5;
                    m_channels_azimuth_mapped[channels[0].getIndex()]= Math<T>::wrap_twopi((current_angle - previous_portion * 0.5) + m_channels_azimuth_width[channels[0].getIndex()] * 0.5);
                }
                for(ulong i = 1; i < channels.size() - 1; i++)
                {
                    const T current_angle   = channels[i].getAzimuth();
                    const T previous_angle  = channels[i-1].getAzimuth();
                    const T next_angle      = channels[i+1].getAzimuth();
                    const T previous_portion= current_angle - previous_angle;
                    const T next_portion    = next_angle - current_angle;
                    m_channels_azimuth_width[channels[i].getIndex()] = (previous_portion + next_portion) * 0.5;
                    m_channels_azimuth_mapped[channels[i].getIndex()]= Math<T>::wrap_twopi((current_angle - previous_portion * 0.5) + m_channels_azimuth_width[channels[i].getIndex()] * 0.5);
                }
                {
                    const ulong index = channels.size() - 1;
                    const T current_angle   = channels[index].getAzimuth();
                    const T previous_angle  = channels[index - 1].getAzimuth();
                    const T next_angle      = channels[0].getAzimuth();
                    const T previous_portion= current_angle - previous_angle;
                    const T next_portion    = (HOA_2PI - current_angle) + next_angle;
                    m_channels_azimuth_width[channels[index].getIndex()] = (previous_portion + next_portion) * 0.5;
                    m_channels_azimuth_mapped[channels[index].getIndex()]= Math<T>::wrap_twopi((current_angle - previous_portion * 0.5) + m_channels_azimuth_width[channels[index].getIndex()] * 0.5);
                }
                channels.clear();
    
            }
            
        }
        
        inline T getPlanewaveAzimuthMapped(const ulong index) const noexcept
        {
            return m_channels_azimuth_mapped[index];
        }
        
        inline T getPlanewaveWidth(const ulong index) const noexcept
        {
            return m_channels_azimuth_width[index];
        }
        
        inline T getPlanewaveEnergy(const ulong index) const noexcept
        {
            if(m_channels_peaks[index] > 0.)
            {
                return 20. * log10(m_channels_peaks[index]);
            }
            else
            {
                return -90.;
            }
        }
        
        inline bool getPlanewaveOverLed(const ulong index) const noexcept
        {
            return m_over_leds[index];
        }
        
        inline void tick(const ulong time) noexcept
        {
            for(ulong i = 0; i < Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves(); i++)
            {
                if(m_channels_peaks[i] > 1.)
                {
                    m_over_leds[i] = time;
                }
                else if(m_over_leds[i])
                {
                    m_over_leds[i]--;
                }
            }
        }
        
        inline void process(const T* inputs) noexcept
        {
            if(m_ramp++ == m_vector_size)
            {
                m_ramp = 0;
                for(ulong i = 0; i < Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves(); i++)
                {
                    m_channels_peaks[i] = fabs(*inputs++);
                }
            }
            else
            {
                for(ulong i = 0; i < Planewave<Hoa2d, T>::Processor::getNumberOfPlanewaves(); i++)
                {
                    const T peak = fabs(*inputs++);
                    if(peak > m_channels_peaks[i])
                    {
                        m_channels_peaks[i] = peak;
                    }
                }
            }
        }
    };
    
    template <typename T> class Meter<Hoa3d, T> : public Planewave<Hoa3d, T>::Processor
    {
    public:
        typedef typename Voronoi<Hoa3d, T>::Point Point;
        typedef vector<Point> Path;
    private:
        ulong   m_ramp;
        ulong   m_vector_size;
        T*      m_channels_peaks;
        ulong*  m_over_leds;
        
        Path*   m_top;
        Path*   m_bottom;
        
        static void filterPath(Path& path, const bool top = true)
        {
            if(top)
            {
                bool valid = false;
                for(ulong i = 0; i < path.size(); i++)
                {
                    if(path[i].z > 0.)
                    {
                        valid = true;
                    }
                }
                if(!valid || path.size() < 3)
                {
                    path.clear();
                }
                else
                {
                    ulong size = path.size();
                    for(ulong i = 0; i < size;)
                    {
                        const ulong p = i ? i-1 : size-1;
                        const ulong n = (i == size-1) ? 0 : i+1;
                        if(path[i].z < 0. && path[p].z >= 0. && path[n].z >= 0.)
                        {
                            const T dist1 = path[p].z / (path[p].z - path[i].z);
                            Point temp1 = (path[i] - path[p]) * dist1 + path[p];
                            temp1.z = 0.;
                            temp1.normalize();
                            
                            const T dist2 = path[n].z / (path[n].z - path[i].z);
                            path[i] = (path[i] - path[n]) * dist2 + path[n];
                            path[i].z = 0.;
                            path[i].normalize();
                            path.insert(path.begin()+i, temp1);
                            size++;
                            i += 3;
                        }
                        else if(path[i].z < 0. && path[p].z >= 0.)
                        {
                            const T dist = path[p].z / (path[p].z - path[i].z);
                            Point temp = (path[i] - path[p]) * dist + path[p];
                            temp.z = 0.;
                            temp.normalize();
                            path.insert(path.begin()+i, temp);
                            size++;
                            i += 2;
                        }
                        else if(path[i].z < 0. && path[n].z >= 0.)
                        {
                            const T dist = path[n].z / (path[n].z - path[i].z);
                            Point temp = (path[i] - path[n]) * dist + path[n];
                            temp.z = 0.;
                            temp.normalize();
                            path.insert(path.begin()+n, temp);
                            size++;
                            i += 2;
                        }
                        else
                        {
                            i++;
                        }
                    }
                    size = path.size();
                    for(ulong i = 0; i < size;)
                    {
                        const ulong p = i ? i-1 : size-1;
                        const ulong n = (i == size-1) ? 0 : i+1;
                        if(path[i].z <= 0. && path[p].z <= 0. && path[n].z <= 0.)
                        {
                            path.erase(path.begin()+i);
                            size--;
                        }
                        else
                        {
                            i++;
                        }
                    }
                }
            }
            else
            {
                bool valid = false;
                for(ulong i = 0; i < path.size(); i++)
                {
                    if(path[i].z < 0.)
                    {
                        valid = true;
                    }
                }
                if(!valid || path.size() < 3)
                {
                    path.clear();
                }
                else
                {
                    ulong size = path.size();
                    for(ulong i = 0; i < size;)
                    {
                        const ulong p = i ? i-1 : size-1;
                        const ulong n = (i == size-1) ? 0 : i+1;
                        if(path[i].z > 0. && path[p].z <= 0. && path[n].z <= 0.)
                        {
                            const T dist1 = path[i].z / (path[i].z - path[p].z);
                            Point temp = (path[p] - path[i]) * dist1 + path[i];
                            temp.z = 0.;
                            temp.normalize();
                            
                            const T dist2 = path[i].z / (path[i].z - path[n].z);
                            path[i] = (path[n] - path[i]) * dist2 + path[i];
                            path[i].z = 0.;
                            path[i].normalize();
                            path.insert(path.begin()+i, temp);
                            size++;
                            i += 3;
                        }
                        else if(path[i].z > 0. && path[p].z <= 0.)
                        {
                            const T dist = path[i].z / (path[i].z - path[p].z);
                            Point temp = (path[p] - path[i]) * dist + path[i];
                            temp.z = 0.;
                            temp.normalize();
                            path.insert(path.begin()+i, temp);
                            size++;
                            i += 2;
                        }
                        else if(path[i].z > 0. && path[n].z <= 0.)
                        {
                            const T dist = path[i].z / (path[i].z - path[n].z);
                            Point temp = (path[n] - path[i]) * dist + path[i];
                            temp.z = 0.;
                            temp.normalize();
                            path.insert(path.begin()+n, temp);
                            size++;
                            i += 2;
                        }
                        else
                        {
                            i++;
                        }
                    }
                    size = path.size();
                    for(ulong i = 0; i < size;)
                    {
                        const ulong p = i ? i-1 : size-1;
                        const ulong n = (i == size-1) ? 0 : i+1;
                        if(path[i].z >= 0. && path[p].z >= 0. && path[n].z >= 0.)
                        {
                            path.erase(path.begin()+i);
                            size--;
                        }
                        else
                        {
                            i++;
                        }
                    }
                }
            }
        }
    public:
        
        Meter(const ulong numberOfPlanewaves) noexcept : Planewave<Hoa3d, T>::Processor(numberOfPlanewaves)
        {
            m_ramp                      = 0;
            m_vector_size               = 0;
            m_channels_peaks            = new T[Planewave<Hoa3d, T>::Processor::getNumberOfPlanewaves()];
            m_over_leds                 = new ulong[Planewave<Hoa3d, T>::Processor::getNumberOfPlanewaves()];
            m_top                       = new Path[Planewave<Hoa3d, T>::Processor::getNumberOfPlanewaves()];
            m_bottom                    = new Path[Planewave<Hoa3d, T>::Processor::getNumberOfPlanewaves()];
            for(ulong i = 0; i < Planewave<Hoa3d, T>::Processor::getNumberOfPlanewaves(); i++)
            {
                m_channels_peaks[i] = 0;
                m_over_leds[i]      = 0;
            }
        }
        
        ~Meter()
        {
            delete [] m_channels_peaks;
            delete [] m_over_leds;
            for(ulong i = 0; i < Planewave<Hoa3d, T>::Processor::getNumberOfPlanewaves(); i++)
            {
                m_top[i].clear();
                m_bottom[i].clear();
            }
            delete [] m_top;
            delete [] m_bottom;
        }
        
        inline void setVectorSize(const ulong vectorSize) noexcept
        {
            m_vector_size   = vectorSize;
            m_ramp          = 0;
        }
        
        inline ulong getVectorSize() const noexcept
        {
            return m_vector_size;
        }
        
        inline T getPlanewaveEnergy(const ulong index) const noexcept
        {
            if(m_channels_peaks[index] > 0.)
            {
                return 20. * log10(m_channels_peaks[index]);
            }
            else
            {
                return -90.;
            }
        }
        
        inline bool getPlanewaveOverLed(const ulong index) const noexcept
        {
            return m_over_leds[index];
        }
        
        inline void tick(const ulong time) noexcept
        {
            for(ulong i = 0; i < Planewave<Hoa3d, T>::Processor::getNumberOfPlanewaves(); i++)
            {
                if(m_channels_peaks[i] > 1.)
                {
                    m_over_leds[i] = time;
                }
                else if(m_over_leds[i])
                {
                    m_over_leds[i]--;
                }
            }
        }
        
        inline void process(const T* inputs) noexcept
        {
            if(m_ramp++ == m_vector_size)
            {
                m_ramp = 0;
                for(ulong i = 0; i < Planewave<Hoa3d, T>::Processor::getNumberOfPlanewaves(); i++)
                {
                    m_channels_peaks[i] = fabs(*inputs++);
                }
            }
            else
            {
                for(ulong i = 0; i < Planewave<Hoa3d, T>::Processor::getNumberOfPlanewaves(); i++)
                {
                    const T peak = fabs(*inputs++);
                    if(peak > m_channels_peaks[i])
                    {
                        m_channels_peaks[i] = peak;
                    }
                }
            }
        }
        
        void computeRendering()
        {
            Voronoi<Hoa3d, T> voronoi;
            
            for(ulong i = 0; i < Planewave<Hoa3d, T>::Processor::getNumberOfPlanewaves(); i++)
            {
                voronoi.add(Planewave<Hoa3d, T>::Processor::getPlanewaveAbscissa(i), Planewave<Hoa3d, T>::Processor::getPlanewaveOrdinate(i), Planewave<Hoa3d, T>::Processor::getPlanewaveHeight(i));
                m_bottom[i].clear();
            }
            //voronoi.add(0., 0., 1.);
            voronoi.compute();
            Path const& bottom = voronoi.getPoints();
            for(ulong i = 0; i < Planewave<Hoa3d, T>::Processor::getNumberOfPlanewaves(); i++)
            {
                for(ulong j = 0; j < bottom[i].bounds.size(); j++)
                {
                    m_bottom[i].push_back(bottom[i].bounds[j]);
                }
                filterPath(m_bottom[i], false);
            }
            
            voronoi.clear();
            
            for(ulong i = 0; i < Planewave<Hoa3d, T>::Processor::getNumberOfPlanewaves(); i++)
            {
                voronoi.add(Planewave<Hoa3d, T>::Processor::getPlanewaveAbscissa(i), Planewave<Hoa3d, T>::Processor::getPlanewaveOrdinate(i), Planewave<Hoa3d, T>::Processor::getPlanewaveHeight(i));
                m_top[i].clear();
            }
            
            for(ulong i = 0; i < Planewave<Hoa3d, T>::Processor::getNumberOfPlanewaves(); i++)
            {
                const double az = T(i) / T(Planewave<Hoa3d, T>::Processor::getNumberOfPlanewaves()) * HOA_2PI;
                voronoi.add(Math<T>::abscissa(1., az, -HOA_PI2 + HOA_EPSILON), Math<T>::ordinate(1., az, -HOA_PI2 + HOA_EPSILON), Math<T>::height(1., az, HOA_PI2 + HOA_EPSILON));
            }
            voronoi.compute();
            Path const& top = voronoi.getPoints();
            for(ulong i = 0; i < Planewave<Hoa3d, T>::Processor::getNumberOfPlanewaves(); i++)
            {
                for(ulong j = 0; j < top[i].bounds.size(); j++)
                {
                    m_top[i].push_back(top[i].bounds[j]);
                }

                filterPath(m_top[i], true);
            }
        }
        
        inline Path const& getPlanewavePath(const ulong index, const bool top) const noexcept
        {
            if(top)
            {
                return m_top[index];
            }
            else
            {
                return m_bottom[index];
            }
        }        
    };
}

#endif



