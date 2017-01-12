/**
 * \file      gtk_ui.hpp
 * \brief     Header for the mwtouch GTK UI
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-07 16:52 UTC+1
 * \copyright BSD
 */

#ifndef MWKINECT_GTK_UI_HPP
#define	MWKINECT_GTK_UI_HPP

#include <opencv2/opencv.hpp>
#include <gtkmm-3.0/gtkmm.h>
#include <glibmm-2.4/glibmm.h>
#include <cairomm/cairomm.h>
#include "nodeconfig.hpp"

class mwkinect_gui: public Gtk::Window {
public:
    mwkinect_gui(thread_com * com);
    ~mwkinect_gui();
    
    void update_preview(const cv::Mat & preview);
    
protected:
    
    // threads data interchange
    bool on_check_thread_signal_quit();
    
    // window close
    bool on_delete(GdkEventAny* event);
    
    // menu actions
    void on_quit_activate();
    void on_load_activate();
    void on_save_activate();
    void on_tuio_activate();
    
    // wrapper spins
    void on_depth_min();
    void on_depth_max();
    void on_blob_min();
    void on_blob_max();
    
    // apply button
    void on_apply_activate();
    
    void replace_buffer();
    
    bool on_redraw_preview(const Cairo::RefPtr<Cairo::Context>& cr);
    
    static const int DEFAULT_FRAME_WIDTH;
    static const int DEFAULT_FRAME_HEIGHT;
    
    static const double DEPTH_CLIMB_RATIO;
    static const double DEPTH_STEP;
    static const double DEPTH_PAGE;
    
    static const double BLOB_CLIMB_RATIO;
    static const double BLOB_STEP;
    static const double BLOB_PAGE;
    
    static const int MWKINECT_SAMPLE_BITS;

    thread_com * m_thread_com;
    sigc::connection stop_flag_connection;
    pthread_mutex_t m_preview_lock;
    
    int m_preview_width;
    int m_preview_height;
    
    Gtk::VBox m_main_vbox;
    Gtk::HBox m_statusbar_box;
    Gtk::HBox m_controls_box;

    // menu    
    Glib::RefPtr<Gtk::ActionGroup> m_actions;
    Glib::RefPtr<Gtk::Action> m_settings_action;
    Glib::RefPtr<Gtk::Action> m_save_action;
    Glib::RefPtr<Gtk::Action> m_load_action;
    Glib::RefPtr<Gtk::Action> m_quit_action;
    Gtk::ToolButton m_settings_button;
    Gtk::ToolButton m_save_button;
    Gtk::ToolButton m_load_button;
    Gtk::ToolButton m_quit_button;
    
    // apply button
    Glib::RefPtr<Gtk::Action> m_apply_action;
    Gtk::Button m_apply_button;
    
    Gtk::DrawingArea m_preview_rendering;
    Gtk::Toolbar m_controls_main;
    
    // wrapper control
    Gtk::Alignment m_wrapper_control_alignment;
    Gtk::Grid m_wrapper_control_grid;
    Gtk::Label m_depth_label;
    Gtk::SpinButton m_depth_button_min;
    Gtk::SpinButton m_depth_button_max;
    Gtk::Label m_blob_label;
    Gtk::SpinButton m_blob_button_min;
    Gtk::SpinButton m_blob_button_max;
    
    //Glib::RefPtr<Gtk::UIManager> m_ui_manager;
    Glib::RefPtr<Gdk::Pixbuf> m_preview_buffer;
    uint8_t * m_preview_data;
    
};

void * mwkinect_gui_thread_start(void * ptr_to_gui_ptr);

#endif	/* MWKINECT_GTK_UI_HPP */

