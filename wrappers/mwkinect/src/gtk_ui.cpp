/**
 * \file      gtk_ui.cpp
 * \brief     Implement the mwkinect GTK UI
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2012-11-07 16:52 UTC+1
 * \copyright BSD
 */

#include <iostream>
#include <fstream>
#include <cstring>
#include <pthread.h>
#include <gtk-3.0/gtk/gtk.h>
#include <gtkmm-3.0/gtkmm.h>
#include <gdkmm-3.0/gdkmm.h>
#include <gdkmm/general.h>
#include <glibmm-2.4/glibmm.h>
#include <sigc++-2.0/sigc++/trackable.h>
#include <cairomm/context.h>

#include "gtk_ui.hpp"
#include "nodeconfig.hpp"

const int mwkinect_gui::DEFAULT_FRAME_WIDTH = 640;
const int mwkinect_gui::DEFAULT_FRAME_HEIGHT = 480;

const double mwkinect_gui::DEPTH_CLIMB_RATIO = 2.0;
const double mwkinect_gui::DEPTH_STEP = 1.0;
const double mwkinect_gui::DEPTH_PAGE = 10.0;

const double mwkinect_gui::BLOB_CLIMB_RATIO = 2.0;
const double mwkinect_gui::BLOB_STEP = 10.0;
const double mwkinect_gui::BLOB_PAGE = 100.0;

mwkinect_gui::mwkinect_gui(thread_com * com)
    :m_thread_com(com),
    m_preview_width(mwkinect_gui::DEFAULT_FRAME_WIDTH), 
    m_preview_height(mwkinect_gui::DEFAULT_FRAME_HEIGHT),
    m_wrapper_control_alignment(Gtk::ALIGN_START, Gtk::ALIGN_END),
    m_depth_button_min(mwkinect_gui::DEPTH_CLIMB_RATIO, 0), 
    m_depth_button_max(mwkinect_gui::DEPTH_CLIMB_RATIO, 0),
    m_blob_button_min(mwkinect_gui::BLOB_CLIMB_RATIO, 0), 
    m_blob_button_max(mwkinect_gui::BLOB_CLIMB_RATIO, 0),
    m_preview_data(NULL)
{
    pthread_mutex_init(&m_preview_lock, NULL);
    
    stop_flag_connection = Glib::signal_timeout().connect(
        sigc::mem_fun(*this, &mwkinect_gui::on_check_thread_signal_quit), 20
    );

    // action group for the whole window
    m_actions = Gtk::ActionGroup::create("mwkinect_actions");
    
    { // preview
        replace_buffer();
        m_preview_buffer = Gdk::Pixbuf::create_from_data(m_preview_data, Gdk::COLORSPACE_RGB, false, 8, m_preview_width, m_preview_height, m_preview_width*3);
        
        m_preview_rendering.set_double_buffered(true);
        m_preview_rendering.set_size_request(m_preview_width, m_preview_height);
        m_preview_rendering.signal_draw().connect(
            sigc::mem_fun(this, &mwkinect_gui::on_redraw_preview)
        );
        m_main_vbox.pack_start(m_preview_rendering, true, true);
    }
    
    signal_delete_event().connect(sigc::mem_fun(this, &mwkinect_gui::on_delete), false);
    
    { // wrapper controls
        m_wrapper_control_grid.set_row_homogeneous(false);
        m_wrapper_control_grid.set_column_homogeneous(false);
        m_wrapper_control_grid.set_column_spacing(3);
        m_wrapper_control_grid.set_row_spacing(3);
        
        m_depth_label.set_text("<depth range>");
        m_depth_button_min.set_alignment(Gtk::ALIGN_END);
        m_depth_button_min.signal_changed().connect(
            sigc::mem_fun(*this, &mwkinect_gui::on_depth_min)
        );
        m_depth_button_min.set_range(0, 1 << 12);
        m_depth_button_min.set_increments(DEPTH_STEP, DEPTH_PAGE);
        m_depth_button_min.set_value(m_thread_com->config->depth_threshold_min);
        m_depth_button_max.set_alignment(Gtk::ALIGN_END);
        m_depth_button_max.signal_changed().connect(
            sigc::mem_fun(*this, &mwkinect_gui::on_depth_max)
        );
        m_depth_button_max.set_range(0, 1 << 12);
        m_depth_button_max.set_value(m_thread_com->config->depth_threshold_max);
        m_depth_button_max.set_increments(DEPTH_STEP, DEPTH_PAGE);
        
        m_blob_label.set_text("<blob size>");
        m_blob_button_min.set_alignment(Gtk::ALIGN_END);
        m_blob_button_min.signal_changed().connect(
            sigc::mem_fun(*this, &mwkinect_gui::on_blob_min)
        );
        m_blob_button_min.set_range(0, m_preview_width * m_preview_height / 2);
        m_blob_button_min.set_increments(20, 50);
        m_blob_button_min.set_value(m_thread_com->config->blob_area_min);
        m_blob_button_max.set_alignment(Gtk::ALIGN_END);
        m_blob_button_max.signal_changed().connect(
            sigc::mem_fun(*this, &mwkinect_gui::on_blob_max)
        );
        m_blob_button_max.set_range(0, m_preview_width * m_preview_height / 2);
        m_blob_button_max.set_increments(20, 50);
        m_blob_button_max.set_value(m_thread_com->config->blob_area_max);
        
        m_wrapper_control_grid.attach(m_depth_button_min, 0, 0, 1, 1);
        m_wrapper_control_grid.attach(m_depth_label,      1, 0, 1, 1);
        m_wrapper_control_grid.attach(m_depth_button_max, 2, 0, 1, 1);

        m_wrapper_control_grid.attach(m_blob_button_min, 0, 1, 1, 1);
        m_wrapper_control_grid.attach(m_blob_label,      1, 1, 1, 1);
        m_wrapper_control_grid.attach(m_blob_button_max, 2, 1, 1, 1);
        
        // apply button
        m_apply_action = Gtk::Action::create(
            "apply",
            Gtk::Stock::OK,
            "_Apply",
            "Applies the modified wrapper settings"
        );
        m_apply_action->signal_activate().connect(
            sigc::mem_fun(*this, &mwkinect_gui::on_apply_activate)
        );
        m_actions->add(m_apply_action, Gtk::AccelKey("<control><enter>"));

        m_apply_button.set_use_action_appearance(true);
        m_apply_button.set_use_stock(true);
        m_apply_button.set_related_action(m_apply_action);
        m_wrapper_control_grid.attach(m_apply_button, 2, 2, 1, 1);
        
        m_wrapper_control_alignment.add(m_wrapper_control_grid);
        m_controls_box.pack_start(m_wrapper_control_alignment, true, true);
    }

    { // menu

        { // quit
            m_quit_action = Gtk::Action::create(
                "quit",
                Gtk::Stock::QUIT,
                "_Quit",
                "Closes the window and kills the underlying wrapper"
            );

            m_quit_action->signal_activate().connect(
                sigc::mem_fun(*this, &mwkinect_gui::on_quit_activate)
            );
            m_actions->add(m_quit_action, Gtk::AccelKey("F10"));

            m_quit_button.set_use_action_appearance(true);
            m_quit_button.set_related_action(m_quit_action);

            m_controls_main.append(m_quit_button);
        }

        { // save
            m_save_action = Gtk::Action::create(
                "save",
                Gtk::Stock::SAVE_AS,
                "_Save",
                "Save wrapper configuration"
            );

            m_save_action->signal_activate().connect(
                sigc::mem_fun(*this, &mwkinect_gui::on_save_activate)
            );
            m_actions->add(m_save_action, Gtk::AccelKey("<control>s"));

            m_save_button.set_use_action_appearance(true);
            m_save_button.set_related_action(m_save_action);

            m_controls_main.append(m_save_button);
        }

        { // load        
            m_load_action = Gtk::Action::create(
                "open",
                Gtk::Stock::OPEN,
                "_Load",
                "Load wrapper configuration"
            );

            m_load_action->signal_activate().connect(
                sigc::mem_fun(*this, &mwkinect_gui::on_load_activate)
            );
            m_actions->add(m_load_action, Gtk::AccelKey("<control>o"));

            m_load_button.set_use_action_appearance(true);
            m_load_button.set_related_action(m_load_action);
            m_load_button.set_border_width(1);

            m_controls_main.append(m_load_button);
        }

#if 0
        { // tuio properties
            m_settings_action = Gtk::Action::create(
                "settings",
                Gtk::Stock::PREFERENCES,
                "_TUIO",
                "Show TUIO configuration dialog"
            );
            
            m_settings_action->signal_activate().connect(
                sigc::mem_fun(*this, &mwkinect_gui::on_tuio_activate)
            );
            m_actions->add(m_settings_action, Gtk::AccelKey("F12"));
            
            m_settings_button.set_use_action_appearance(true);
            m_settings_button.set_related_action(m_settings_action);
            
            m_controls_main.append(m_settings_button);
        }
#endif
        
        m_controls_main.set_toolbar_style(Gtk::TOOLBAR_BOTH_HORIZ);
        m_controls_main.check_resize();
        m_controls_main.compute_expand(Gtk::ORIENTATION_VERTICAL);
        m_controls_main.rebuild_menu();
        m_controls_main.set_show_arrow(false);
        m_controls_main.set_halign(Gtk::ALIGN_START);
        
        GtkOrientable * tb = (GtkOrientable *)m_controls_main.gobj();
        gtk_orientable_set_orientation(tb, GTK_ORIENTATION_VERTICAL);

        m_controls_box.pack_end(m_controls_main, false, false);
        m_main_vbox.pack_start(m_controls_box);
    }

    add(m_main_vbox);
    show_all_children(true);
    resize_children();
    
}

bool mwkinect_gui::on_redraw_preview(const Cairo::RefPtr<Cairo::Context>& cr){
    pthread_mutex_lock(&m_preview_lock);
    Gdk::Cairo::set_source_pixbuf(cr, m_preview_buffer, 0, 0);
    cr->paint();
    pthread_mutex_unlock(&m_preview_lock);
    return true;
}

void mwkinect_gui::replace_buffer(){
    if (m_preview_data == NULL){
        delete m_preview_data;
        m_preview_data = NULL;
    }        
    m_preview_data = new uint8_t[m_preview_height*m_preview_width*3];
    memset(m_preview_data, 0, m_preview_height*m_preview_width*3);
}

void mwkinect_gui::update_preview(const cv::Mat& preview){
    // resize
    cv::Mat tmp_resized(m_preview_width, m_preview_height, CV_32FC3);
    cv::resize(preview, tmp_resized, tmp_resized.size());
    
    // convert
    cv::Mat tmp_converted(m_preview_width, m_preview_height, CV_8UC3);
    tmp_resized.convertTo(tmp_converted, CV_8UC3, 255.0);

    if (m_thread_com->keep_running == false){ return; }
    // the get_window sometimes tends to segfault, but why?
    if (pthread_mutex_lock(&m_preview_lock) != 0){ return; }
    
    // one does not simply use conversion inplace...
    memcpy(m_preview_data, tmp_converted.datastart, m_preview_width*m_preview_height*3);

    Glib::RefPtr<Gdk::Window> win = get_window();
    if (win) {
        Gdk::Rectangle r(0, 0, get_allocation().get_width(), get_allocation().get_height());
        win->invalidate_rect(r, true);
    }
    pthread_mutex_unlock(&m_preview_lock);

}

mwkinect_gui::~mwkinect_gui(){
    pthread_mutex_destroy(&m_preview_lock);
    if (m_preview_data == NULL) {
        delete m_preview_data;
        m_preview_data = NULL;
    }
}

void mwkinect_gui::on_quit_activate(){
    //assert(m_thread_com->keep_running != NULL);
    m_thread_com->keep_running = false;
}

bool mwkinect_gui::on_delete(GdkEventAny* event __attribute__((unused))){
    //assert(m_thread_com->keep_running != NULL);
    m_thread_com->keep_running = false;
    return false;
}

void mwkinect_gui::on_load_activate(){
    //! \todo implement
}

void mwkinect_gui::on_save_activate(){
    Gtk::FileChooserDialog save_dialog("mwkinect - save config", Gtk::FILE_CHOOSER_ACTION_SAVE);
    save_dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    save_dialog.add_button(Gtk::Stock::SAVE_AS, Gtk::RESPONSE_OK);
    
    int result = save_dialog.run();
    if (result == Gtk::RESPONSE_OK) {
        std::string filename = save_dialog.get_filename();
        if (filename.empty()) {
            std::cerr << "Filename expected!" << std::endl;
            return;
        }
        
        std::cout << "Dumping config into \"" << filename << "\"... ";
        std::fstream output(filename.c_str(), std::ios_base::out);
        if (output.good()){
            save_config_file(output, *node_config::get_instance());
            std::cout << "done." << std::endl;
        } else {
            std::cout << "failed!" << std::endl;
        }
        output.close();
    }
}

void mwkinect_gui::on_tuio_activate(){
    //! \todo implement
}

void mwkinect_gui::on_apply_activate(){
    pthread_spin_lock(&m_thread_com->update_lock);
    
    m_thread_com->config->blob_area_min = m_blob_button_min.get_value_as_int();
    m_thread_com->config->blob_area_max = m_blob_button_max.get_value_as_int();

    m_thread_com->config->depth_threshold_min = m_depth_button_min.get_value_as_int();
    m_thread_com->config->depth_threshold_max = m_depth_button_max.get_value_as_int();
    
    m_thread_com->config->force_update = true;
    
    pthread_spin_unlock(&m_thread_com->update_lock);
}

void mwkinect_gui::on_depth_min(){
    if (m_depth_button_min.get_value_as_int() > m_depth_button_max.get_value_as_int()){
        m_depth_button_max.set_value(m_depth_button_min.get_value_as_int());
    }
}

void mwkinect_gui::on_depth_max(){
    if (m_depth_button_max.get_value_as_int() < m_depth_button_min.get_value_as_int()){
        m_depth_button_min.set_value(m_depth_button_max.get_value_as_int());
    }
}

void mwkinect_gui::on_blob_min(){
    if (m_blob_button_min.get_value_as_int() > m_blob_button_max.get_value_as_int()){
        m_blob_button_max.set_value(m_blob_button_min.get_value_as_int());
    }
}

void mwkinect_gui::on_blob_max(){
    if (m_blob_button_max.get_value_as_int() < m_blob_button_min.get_value_as_int()){
        m_blob_button_min.set_value(m_blob_button_max.get_value_as_int());
    }
}

bool mwkinect_gui::on_check_thread_signal_quit(){
    //assert(m_thread_com->keep_running != NULL);
    
    if (m_thread_com->keep_running == false){
        Gtk::Main::quit();
    }

    return true;
}

void * mwkinect_gui_thread_start(void * ptr_to_gui_com){
    thread_com * com_ptr = static_cast<thread_com *>(ptr_to_gui_com);

    Gtk::Main gtk_main(com_ptr->argc, com_ptr->argv);

    // gui thread main
    com_ptr->gui_instance = new mwkinect_gui(com_ptr);
    Gtk::Main::run(*com_ptr->gui_instance);
    delete com_ptr->gui_instance;
    
    return NULL;
}