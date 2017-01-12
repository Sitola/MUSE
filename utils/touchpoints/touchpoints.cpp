/**
 * Display captured tuio pointers on the sage machine
 *
 * \file      tuiopoints.cpp
 * \author    Lukas Rucka <359687@mail.muni.cz>, Masaryk University, Brno, Czech Republic
 * \date      2011-09-08 20:24 UTC+1
 * \copyright BSD
 */

#define _BSD_SOURCE

#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <inttypes.h>
#include <iostream>
#include <map>
#include <list>
#include <algorithm>
#include <time.h>
#include <getopt.h>

#include <kerat/kerat.hpp>

#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#ifdef HAVE_MUSE
    #include <muse/muse.hpp>
    #include <tinyxml.h>
#endif
// headers for SAGE
#include <sail.h>
#include <misc.h>
#include <tinyxml.h>

#ifdef HAVE_FASTDXT
    #include <libdxt.h>
#endif

#ifdef HAVE_GLSLDXT
extern "C" {
    #include "dxt_encoder.h"
}
#endif

#include "point_renderer.hpp"

using std::cout;
using std::cerr;
using std::endl;

bool running = true;

/**
 * Struct that holds the runtime configuration
 */
struct touchpoints_config {
    typedef enum{ RENDER_RGBA, RENDER_FASTDXT, RENDER_GLSLDXT } rendering_t;

    const char * sail_config_path;
    rendering_t render_type;
    size_t win_width;
    size_t win_height;
    size_t snr_width;
    size_t snr_height;
    bool   snr_scale;
    bool   multiplexing;
    uint8_t line_width;
#ifdef HAVE_MUSE
    TiXmlDocument muse_config;
#endif
    unsigned short port;
    bool draw_track;
    bool top_to_bottom;
};

/**
 * Make default configuration
 * @return default configuration
 */
static touchpoints_config get_default_config(){
    touchpoints_config config;
    config.sail_config_path = "touchpoints.conf";
    config.render_type      = touchpoints_config::RENDER_RGBA;
    config.win_width        = 1920;
    config.win_height       = 1080;
    config.snr_width        = config.win_width;
    config.snr_height       = config.win_height;
    config.snr_scale        = false;
    config.multiplexing     = true;
    config.line_width       = 3;
    config.port             = 3333;
    config.draw_track       = false;
    config.top_to_bottom    = true;
    return config;
}

// commandline
static int parse_dim(const char * orig_dim, size_t & width, size_t & height);
static int parse_commandline(touchpoints_config * config, int argc, char ** argv);

static void register_signal_handlers();

/**
 * Sets the event loop stop flag
 * @param ev - do not care about this (viz. man signal)
 */
static void handle_kill_signal(int ev);


static void usage();

// buffer copy functions
#ifdef HAVE_FASTDXT
static void imprint_buffer_fastdxt(void * sage_buffer, const void * rgba_buffer, const size_t width, const size_t height);

static uint8_t * fastdxt_dxt_buffer = NULL;
static long fastdxt_num_threads = 2;

#endif

#ifdef HAVE_GLSLDXT

static void imprint_buffer_glsldxt(void * sage_buffer, const void * rgba_buffer, const size_t width, const size_t height);

static struct dxt_encoder* glsl_dxt_encoder = NULL;
static int glsl_dxt_compressed_image_size = 0;
static uint8_t * glsl_dxt_compressed_image = NULL;

#endif

#if defined HAVE_FASTDXT or defined HAVE_GLSLDXT
    static int tp_dxt_compressed_size_limit = 0;
#endif


static void imprint_buffer_rgba(void * sage_buffer, const void * rgba_buffer, const size_t width, const size_t height);
typedef void (*imprint_buffer_callback)(void *, const void *, const size_t, const size_t);

int main(int argc,char **argv){

#ifdef HAVE_MUSE
//    muse_init();
#endif

    srand(time(NULL));

    // for command line arguments

    // parse the command line auxiliary
    touchpoints_config config = get_default_config();
    if (parse_commandline(&config, argc, argv) != 0){
        cerr << "Parsing command line has failed, commiting suicide" << endl;
        exit(EXIT_FAILURE);
    }

    if (!running){ exit(EXIT_SUCCESS); }

    register_signal_handlers();

    // initialize SAIL
    sail sail_instance;
    sailConfig scfg;
    {
        char *tmpconf = getenv("SAGE_APP_CONFIG");
        if (tmpconf) {
            scfg.init(tmpconf);
        } else {
            scfg.init((char *)config.sail_config_path);
        }
    }



    scfg.setAppName("touchpoints");

    scfg.resX = config.win_width;
    scfg.resY = config.win_height;


    if (scfg.winWidth == -1 || scfg.winHeight == -1) {
        scfg.winWidth = config.win_width;
        scfg.winHeight = config.win_height;
    }

    imprint_buffer_callback imprint_buffer = NULL;

    switch (config.render_type){
#ifdef HAVE_FASTDXT
        case touchpoints_config::RENDER_FASTDXT: {
            scfg.pixFmt = PIXFMT_DXT;
            scfg.rowOrd = BOTTOM_TO_TOP;
            config.top_to_bottom = false;

            imprint_buffer = &imprint_buffer_fastdxt;
            tp_dxt_compressed_size_limit = (config.win_width * config.win_height) / (8/4);

            fastdxt_dxt_buffer = (uint8_t *)malloc(config.win_width * config.win_height * 4);
            if (fastdxt_dxt_buffer == NULL){
                cerr << "Failed to allocate buffer for FastDXT compression" << endl;
                return EXIT_FAILURE;
            }
            // #ifdef _SC_NPROCESSORS_ONLN
            //     fastdxt_num_threads = sysconf(_SC_NPROCESSORS_ONLN);
            //     if (fastdxt_num_threads > 0){
            //         fastdxt_num_threads *= 2;
            //     }
            // #endif
            cout << "FastDXT compression enabled, using " << fastdxt_num_threads << " threads" << endl;
            break;
        }
#endif
#ifdef HAVE_GLSLDXT
        case touchpoints_config::RENDER_GLSLDXT: {
            scfg.pixFmt = PIXFMT_DXT;
            scfg.rowOrd = BOTTOM_TO_TOP;
            config.top_to_bottom = false;

            imprint_buffer = &imprint_buffer_glsldxt;
            tp_dxt_compressed_size_limit = (config.win_width * config.win_height) / (8/4);

            if (dxt_init() != 0){
                cerr << "Failed to initialize GLSL DXT library!" << endl;
                return EXIT_FAILURE;
            }

            glsl_dxt_encoder = dxt_encoder_create(DXT_TYPE_DXT1, config.win_width, config.win_height, DXT_FORMAT_RGB);
            if (glsl_dxt_encoder == NULL) {
                cerr << "Failed to create GLSL DXT encoder!" << endl;
                return EXIT_FAILURE;
            }

            if ( dxt_encoder_buffer_allocate(glsl_dxt_encoder, &glsl_dxt_compressed_image, &glsl_dxt_compressed_image_size) != 0 ) {
                cerr << "Failed to allocate GLSL DXT buffer!" << endl;
                dxt_encoder_destroy(glsl_dxt_encoder);
                return EXIT_FAILURE;
            }

            break;
        }
#endif
        case touchpoints_config::RENDER_RGBA: {
            scfg.pixFmt = PIXFMT_8888;
            scfg.rowOrd = TOP_TO_BOTTOM;

            imprint_buffer = &imprint_buffer_rgba;

            break;
        }
        // this should never happen unles someone messes up the code
        default: {
            cerr << "Unsupported compression mode " << config.render_type << endl;
            break;
        }

    }

    sail_instance.init(scfg);

    // libkerat init
    libkerat::simple_client client(config.port);
    libkerat::client * last_client = &client;
    point_renderer renderer(config.win_width, config.win_height, config.line_width, config.draw_track, config.top_to_bottom);
    libkerat::adaptor * multiplexer = NULL;
    libkerat::adaptor * scaler = NULL;

#ifdef HAVE_MUSE
    muse::module_service::module_chain modules;

    if (config.muse_config.NoChildren()){
#endif
        if (config.multiplexing){
            multiplexer = new libkerat::adaptors::multiplexing_adaptor;
            last_client->add_listener(multiplexer);
            last_client = multiplexer;
        }
#ifdef HAVE_MUSE
    } else {
        muse::module_service * modserv = muse::module_service::get_instance();

        const TiXmlElement * e_config = config.muse_config.RootElement();
        assert(e_config != NULL);
        const TiXmlElement * e_chain = e_config->FirstChildElement("chain");

        if (modserv->create_module_chain(e_chain, modules)){
            if (modules.empty()){
                std::cerr << "Failed to create module chain!" << std::endl;
            } else {
                std::cerr << "Failed to create module from following module data:" <<
                    modules.back().second->GetText() << std::endl;
            }
            modserv->free_module_chain(modules);
        }

        if (!modules.empty()){
            last_client->add_listener(modules.front().first);
            last_client = modules.back().first;
        }
    }
#endif

    if (config.snr_scale){
        double scale_x = config.win_width; scale_x /= config.snr_width;
        double scale_y = config.win_height; scale_y /= config.snr_height;

        scaler = new libkerat::adaptors::scaling_adaptor(scale_x, scale_y);
        last_client->add_listener(scaler);
        last_client = scaler;
    }


    last_client->add_listener(&renderer);

    // initial grey screen
    renderer.render();

    while (running){
        imprint_buffer(sail_instance.getBuffer(), renderer.get_canvas(), config.win_width, config.win_height);
        sail_instance.swapBuffer(SAGE_NON_BLOCKING);

        struct timespec timeout;
        timeout.tv_sec = 0;
        timeout.tv_nsec = 500000;

        if (!last_client->load(20, timeout)){
            renderer.clear();
        }
        renderer.render();

        sageMessage msg;
        if (sail_instance.checkMsg(msg, false) > 0) {
            switch (msg.getCode()) {
                case APP_QUIT: {
                    running = false;
                    break;
                }
            }
        }
    }

    sail_instance.shutdown();

#ifdef HAVE_GLSLDXT
    if (glsl_dxt_compressed_image != NULL){
        dxt_encoder_buffer_free(glsl_dxt_compressed_image);
        glsl_dxt_compressed_image = NULL;
    }

    if (glsl_dxt_encoder != NULL){
        dxt_encoder_destroy(glsl_dxt_encoder);
        glsl_dxt_encoder = NULL;
    }
#endif

#ifdef HAVE_FASTDXT
    if (fastdxt_dxt_buffer != NULL){
        free(fastdxt_dxt_buffer);
        fastdxt_dxt_buffer = NULL;
    }
#endif


    if (scaler != NULL){ delete scaler; scaler = NULL; }
#ifdef HAVE_MUSE
    if (!modules.empty()){
        muse::module_service::get_instance()->free_module_chain(modules);
    }
#endif
    if (multiplexer != NULL){ delete multiplexer; multiplexer = NULL; }

    return 0;
}

void usage(){
    cout << "Usage:" << endl;
    cout << "touchpoints [options]\t\t" << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << "--raw                               \t\tDisable multiplexing client" << endl;
    cout << "--dim=<width>x<height>              \t\tSets both --sensor and --window" << endl;
    cout << "--sensor=<width>x<height>           \t\tSets sensor dimmensions for scaling" << endl;
    cout << "--window=<width>x<height>           \t\tSets window size" << endl;
    cout << "--sail-config=<path>                \t\tAbsolute path to sail config file" << endl;
    cout << "--port=<port>                       \t\tPort that TUIO client shall listen on" << endl;
    cout << "--tracks                            \t\tDraw track" << endl;
    cout << "--compress=<compression_type>       \t\tSelect compression type (if compiled with)" << endl;
#ifdef HAVE_MUSE
    cout << "--muse-config=<file.xml>            \t\tUse given MUSE framework configuration" << endl;
#endif
    cout << endl;
    cout << "Available compression types: " << endl;
#ifdef HAVE_FASTDXT
    cout << "    fastdxt   short: --fastdxt      \t\tUse the FastDXT implementation" << endl;
#endif
#ifdef HAVE_FASTDXT
    cout << "    glsldxt   short: --glsldxt      \t\tUse the GLSL DXT implementa tion" << endl;
#endif
    cout << "    none                            \t\tDo not use compression at all" << endl;
    cout.flush();
}

#ifdef HAVE_FASTDXT
void imprint_buffer_fastdxt(void * sage_buffer, const void * rgba_buffer, const size_t width, const size_t height){
    memset(sage_buffer, 0, tp_dxt_compressed_size_limit);

    int num_bytes = 0;

    num_bytes = CompressDXT((const byte *)rgba_buffer, (byte *)fastdxt_dxt_buffer, width, height, FORMAT_DXT1, fastdxt_num_threads);
//    CompressImageDXT1((const byte *)rgba_buffer, (byte *)fastdxt_dxt_buffer, width, height, num_bytes);

    memcpy(sage_buffer, fastdxt_dxt_buffer,
        (num_bytes<tp_dxt_compressed_size_limit)?num_bytes:tp_dxt_compressed_size_limit
    );
}
#endif

#ifdef HAVE_GLSLDXT
void imprint_buffer_glsldxt(void * sage_buffer, const void * rgba_buffer, const size_t width, const size_t height){
    // clean sage buffer
    memset(sage_buffer, 0, tp_dxt_compressed_size_limit);

    if (dxt_encoder_compress(glsl_dxt_encoder, (uint8_t *)rgba_buffer, glsl_dxt_compressed_image) != 0) {
        cerr << "GLSL DXT: error compressing image" << endl;
    }

    // write back
    memcpy(sage_buffer, glsl_dxt_compressed_image,
        (glsl_dxt_compressed_image_size<tp_dxt_compressed_size_limit)?glsl_dxt_compressed_image_size:tp_dxt_compressed_size_limit
    );
}
#endif

void imprint_buffer_rgba(void * sage_buffer, const void * rgba_buffer, const size_t width, const size_t height){
    memcpy(sage_buffer, rgba_buffer, width * height * 4);
}



int parse_commandline(touchpoints_config * config, int argc, char ** argv){

    struct option cmdline_opts[15];
    memset(&cmdline_opts, 0, sizeof(cmdline_opts));
    { int index = 0;

        cmdline_opts[index].name = "help";
        cmdline_opts[index].has_arg = 0;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'h';
        ++index;

        cmdline_opts[index].name = "window";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'w';
        ++index;

        cmdline_opts[index].name = "sensor";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 's';
        ++index;

        cmdline_opts[index].name = "dimmensions";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'd';
        ++index;

        cmdline_opts[index].name = "raw";
        cmdline_opts[index].has_arg = 0;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'r';
        ++index;

        cmdline_opts[index].name = "tracks";
        cmdline_opts[index].has_arg = 0;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 't';
        ++index;

        cmdline_opts[index].name = "sail-config";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'S';
        ++index;

        cmdline_opts[index].name = "port";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'p';
        ++index;

        cmdline_opts[index].name = "line-width";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'l';
        ++index;

        cmdline_opts[index].name = "compress";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'c';
        ++index;

#ifdef HAVE_FASTDXT
        cmdline_opts[index].name = "fastdxt";
        cmdline_opts[index].has_arg = 0;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'f';
        ++index;
#endif

#ifdef HAVE_GLSLDXT
        cmdline_opts[index].name = "glsldxt";
        cmdline_opts[index].has_arg = 0;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'g';
        ++index;
#endif

#ifdef HAVE_MUSE
        cmdline_opts[index].name = "muse-config";
        cmdline_opts[index].has_arg = 1;
        cmdline_opts[index].flag = NULL;
        cmdline_opts[index].val = 'm';
        ++index;
#endif

    }

  char opt = -1;
    while ((opt = getopt_long(argc, argv, "-hw:d:s:rl:S:p:c:m:fgt", cmdline_opts, NULL)) != -1){
        switch (opt){
            case 'h': {
                usage();
                running = false;
                return EXIT_FAILURE;
                break;
            }
            case 'w': {
                if (parse_dim(optarg, config->win_width, config->win_height) != 0){ return -1; }
                cout << "Setting window dimmensions to " << config->win_width << "x" << config->win_height << " pixels" << endl;
                break;
            }
            case 's': {
                if (parse_dim(optarg, config->snr_width, config->snr_height) != 0){ return -1; }

                config->snr_scale = true;
                cout << "Setting sensor dimmensions to " << config->snr_width << "x" << config->snr_height << " pixels" << endl;

                break;
            }
            case 'd': {
                if (parse_dim(optarg, config->win_width, config->win_height) != 0){ return -1; }

                config->snr_width = config->win_width;
                config->snr_height = config->win_height;
                config->snr_scale = true;
                cout << "Setting both sensor and window dimmensions to " << config->win_width << "x" << config->win_height << " pixels" << endl;

                break;
            }
            case 'r': { // ================ multiplexing
                config->multiplexing = false;
                break;
            }
            case 't': { 
                config->draw_track = true;
                break;
            }
            case 'S': { // ================ sail
                config->sail_config_path = optarg;
                break;
            }
            case 'p': { // ================ port
                config->port = strtol(optarg, NULL, 10);
                if (config->port > 0xffff){ cerr << "Invalid port " << config->port << endl; }
                break;
            }
            case 'c': { //================= compressions
                if (strcmp(optarg, "none")==0){
                    config->render_type = touchpoints_config::RENDER_RGBA;
                #ifdef HAVE_FASTDXT
                } else if (strcmp(optarg, "fastdxt")==0){
                    config->render_type = touchpoints_config::RENDER_FASTDXT;
                #endif
                #ifdef HAVE_GLSLDXT
                } else if (strcmp(optarg, "glsldxt")==0){
                    config->render_type = touchpoints_config::RENDER_GLSLDXT;
                #endif
                } else {
                    cerr << "Unrecognized compression type \"" << optarg << "\"!" << endl;
                    return -1;
                }

                break;
            }
            case 'l': { // ================ line width
                int32_t tmp = strtol(optarg, NULL, 10);
                if ((tmp < 1) || (tmp > 255)){
                    cerr << "Line width must fall within the range [1-255]! << endl";
                    return -1;
                }
                config->line_width = tmp;
                break;
            }
            #ifdef HAVE_FASTDXT
            case 'f': {
                config->render_type = touchpoints_config::RENDER_FASTDXT;
                break;
            }
            #endif
            #ifdef HAVE_GLSLDXT
            case 'g': {
                config->render_type = touchpoints_config::RENDER_GLSLDXT;
                break;
            }
            #endif
            #ifdef HAVE_MUSE
            case 'm': {
                config->muse_config.LoadFile(optarg);
                if (config->muse_config.Error()){
                    std::cerr << "Failed to parse given MUSE config file!" << std::endl;
                    std::cerr << config->muse_config.ErrorDesc() << std::endl;
                    return -1;
                }
                break;
            }
            #endif

            default: {
                std::cerr << "Unrecognized argument!" << std::endl;
            }
        }
    }

    return 0;
}
int parse_dim(const char * orig_dim, size_t & width, size_t & height){

    {
        char * dim = strdup(orig_dim);

        // width
        char * tmp_dim = strtok(dim, "x");
        if (tmp_dim == NULL){
            cerr << "Unable to find width in \"" << orig_dim << endl;
            free(dim);
            return -1;
        }
        double tmp = strtod(tmp_dim, NULL);
        width = tmp;

        // height
        tmp_dim = strtok(NULL, "x,:");
        if (tmp_dim == NULL){
            cerr << "Unable to find height in \"" << orig_dim << endl;
            free(dim);
            return -1;
        }
        tmp = strtod(tmp_dim, NULL);
        height = tmp;
        free(dim);
    }


    // errors
    if (width == 0){
        cerr << "Non-zero width is required!" << endl;
        return -1;
    }
    if (height == 0){
        cerr << "Non-zero height is required!" << endl;
        return -1;
    }

    return 0;
}


static void register_signal_handlers(){

    signal(SIGTERM, handle_kill_signal);
    // NOTE: SIGKILL is still uncaughtable
    //signal(SIGKILL, handle_kill_signal); // can't do anything about it, so at least correctly close
    signal(SIGABRT, handle_kill_signal);
    signal(SIGINT, handle_kill_signal);
    signal(SIGHUP, handle_kill_signal);

}

static void handle_kill_signal(int ev){
    cout << "Caught signal " << ev << ", closing up..." << endl;
    running = false; // this variable is checked in the main event loop
}

