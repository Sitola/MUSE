/**
 * \file      scaling_adaptor.hpp
 * \brief     Provides the means of client-side axis scaling.
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-12-21 22:36 UTC+1
 * \copyright BSD
 */

#ifndef KERAT_SCALING_ADAPTOR_HPP
#define KERAT_SCALING_ADAPTOR_HPP

#include <kerat/typedefs.hpp>
#include <kerat/adaptor.hpp>
#include <kerat/server_adaptor.hpp>
#include <lo/lo.h>

namespace libkerat {

    namespace adaptors {

        //! \brief Adaptor that scales the coordinates, velocities and possibly acceleration
        class scaling_adaptor: public adaptor, public server_adaptor {
        public:

            /**
             * \brief Create new scaling adaptor with enabled autoconfiguration to fit
             * the given dimmensions. Z axis is not scaled.
             *
             * \param width - scale x axis to width
             * \param height - scale y axis to height
             * \param scale_accel - whether to scale the acceleration value, defaults to true
             */
            scaling_adaptor(dimmension_t width, dimmension_t height, bool scale_accel = true);

            /**
             * \brief Create new scaling adaptor with given scaling factors and possibly
             * disable (enable) accleration scaling. Does not use autoconfigure
             *
             * \param x_axis_scaling - scale factor to use for x axis
             * \param y_axis_scaling - scale factor to use for y axis
             * \param z_axis_scaling - scale factor to use for z axis
             * \param scale_accel - whether to scale the acceleration information, defaults to true
             */
            scaling_adaptor(double x_axis_scaling, double y_axis_scaling, double z_axis_scaling = 1.0, bool scale_accel = true);

            ~scaling_adaptor();

            /**
             * \brief Get scaling factor for x axis
             * \return factor to scale by, 0 means project all x coordinates
             * to YZ plane, 1 ignores the scaling for this axis
             */
            inline double get_x_scaling() const { return m_x_scaling; }

            /**
             * \brief Set scaling factor for x axis
             * \note choose the factor carefully to prevent overflow
             * \param factor - factor to scale by, 0 to project all x coordinates
             * to YZ plane, 1 to ignore the scaling for this axis
             * \return previous setting
             */
            double set_x_scaling(const double factor);


            /**
             * \brief Get scaling factor for y axis
             * \return factor to scale by, 0 means project all y coordinates
             * to XZ plane, 1 ignores the scaling for this axis
             */
            inline double get_y_scaling() const { return m_y_scaling; }

            /**
             * \brief Set scaling factor for y axis
             * \note choose the factor carefully to prevent overflow
             * \param factor - factor to scale by, 0 to project all y coordinates
             * to XZ plane, 1 to ignore the scaling for this axis
             * \return previous setting
             */
            double set_y_scaling(const double factor);


            /**
             * \brief Get scaling factor for z axis
             * \return factor to scale by, 0 means project all z coordinates
             * to XY plane, 1 ignores the scaling for this axis
             */
            inline double get_z_scaling() const { return m_z_scaling; }

            /**
             * \brief Set scaling factor for z axis
             * \note choose the factor carefully to prevent overflow
             * \param factor - factor to scale by, 0 to project all z coordinates
             * to XY plane, 1 to ignore the scaling for this axis
             * \return previous setting
             */
            double set_z_scaling(const double factor);


            /**
             * \brief Check whether automatic scaling factor computation is enabled
             * \return true if enabled
             */
            inline bool get_auto() const { return m_autoconf; }

            /**
             * \brief Enable/disable automatic scaling factor computation
             * \param autoconf - true to enable, false to disable, defaults to true
             * \return previous setting
             */
            bool set_auto(const bool autoconf = true);


            /**
             * \brief Check whether the accleration value should be scaled as well
             * \return true if scale
             */
            inline bool get_scale_accel() const { return m_sc_accel; }

            /**
             * \brief Enable/disable scaling of the acceleration value
             * \param scale - true to enable, false to disable, defaults to true
             * \return previous setting
             */
            bool set_scale_accel(const bool scale = true);


            /**
             * \brief Get the length of x axis (used for automatic scaling factor computation)
             * \return length or 0 if unset
             */
            inline dimmension_t get_x_length() const { return m_x_axis_length; }

            /**
             * \brief Set the length of x axis (used for automatic scaling factor computation)
             * \return previous setting
             */
            dimmension_t set_x_length(const dimmension_t length);


            /**
             * \brief Get the length of y axis (used for automatic scaling factor computation)
             * \return length or 0 if unset
             */
            inline dimmension_t get_y_length() const { return m_y_axis_length; }

            /**
             * \brief Set the length of y axis (used for automatic scaling factor computation)
             * \return previous setting
             */
            dimmension_t set_y_length(const dimmension_t length);

            // commented out because someone ignores every email I send him...
            // inline dimmension_t get_z_length() const { return m_z_axis_length; }
            // dimmension_t set_z_length(const dimmension_t length);

            void notify(const client * notifier);

            bundle_stack get_stack() const { return m_processed_frames; }

            
            /**
             * \brief Commance scaling on given bundle handle.
             * \param to_process - bundle handle to be processed by adaptor
             * \param output_bundle - output bundle handle
             * \return 0 if ok, -1 if scaling set to automatic and no sensor dimmensions arrived in this bundle
             */
            int process_bundle(const bundle_handle & to_process, bundle_handle & output_bundle);
            
            /**
             * \brief Commance scaling on given bundle handle (server-side adaptor variant).
             * \param to_process - bundle handle to be processed by adaptor
             * \return 0 if ok, -1 if scaling set to automatic
             */
            int process_bundle(bundle_handle & to_process);

            void purge();

        private:
            //! \brief Holds the scaling factor for x axis
            double m_x_scaling;

            //! \brief Holds the scaling factor for y axis
            double m_y_scaling;

            //! \brief Holds the scaling factor for z axis
            double m_z_scaling;

            /**
             * \brief Determine scaling factors based on given dimensions and sensor
             * dimmensions that were received in given frame handle
             */
            bool m_autoconf;

            /**
             * \brief Whether the acceleration information should be scalled as well
             */
            bool m_sc_accel;

            //! \brief X axis length
            dimmension_t m_x_axis_length;
            
            //! \brief Y axis length
            dimmension_t m_y_axis_length;
//            dimmension_t m_z_axis_length;

            bundle_stack m_processed_frames;

            /**
             * \brief Compute scalled accleration (3D)
             *
             * The scaled accleration is computed by decomposing the acceleration
             * into individual accelerations in direction of movement axes, which
             * are then scaled using scaling factors and recomposed to form the
             * new acceleration value.
             *
             * \param vel_x - velocity in direction of x axis
             * \param vel_y - velocity in direction of y axis
             * \param vel_z - velocity in direction of z axis
             * \param acc - acceleration value
             * \return scaled acceleration value
             */
            accel_t scale_accel(const velocity_t vel_x, const velocity_t vel_y, const velocity_t vel_z, const accel_t acc);

            /**
             * \brief Compute scalled accleration (2D)
             *
             * The scaled accleration is computed by decomposing the acceleration
             * into individual accelerations in direction of movement axes, which
             * are then scaled using scaling factors and recomposed to form the
             * new acceleration value.
             *
             * \param vel_x - velocity in direction of x axis
             * \param vel_y - velocity in direction of y axis
             * \param acc - acceleration value
             * \return scaled acceleration value
             */
            accel_t scale_accel(const velocity_t vel_x, const velocity_t vel_y, const accel_t acc);
            
            //! \brief internal processing core
            int internal_process_bundle(bundle_handle & to_process);
            
        };

    }

}

#endif // KERAT_SCALING_ADAPTOR_HPP
