/*
 * File:   event_storage.hpp
 * Author: Lukáš Ručka (359687@mail.muni.cz)
 * Organization: Masaryk University; Brno, Czech Republic
 * License: BSD
 * Created on 9th August 2011, 23:53
 *
 * Purpose: * define the macros present in newer kernels for backwards common
 *          * prototypes of functions utilized by both dumper and replay
 */

#ifndef MWTOUCH_EVENT_STORAGE_HPP
#define MWTOUCH_EVENT_STORAGE_HPP

#include <linux/input.h>
#include <stdint.h>
#include <cstring>
#include "axis.hpp"

// compatibility hacks based on 2.6.38's input.h

typedef input_absinfo storage_file_axis_record;

/**
 * Write the storage file magick number
 * @param storage_fd - output file handle
 */
void mwt_storage_file_write_header(int storage_fd);

/**
 * Test whether the storage file contains proper header
 * @param storage_fd - storage file header
 * @return true if ok, false otherwise
 */
bool mwt_storage_file_test_header(int storage_fd);

/**
 * Write the data to storage file
 * @param storage_fd - output file handle
 * @param event - axis entry to be written to the storage file
 */
void mwt_storage_file_write_event(int storage_fd, const input_event & event);

/**
 * Read formated data from storage file
 * @param storage_fd - file containing the storage entries
 * @param record - record to read the data into
 * @return 0 if read ok, -1 if something went wrong
 */
int mwt_storage_file_read_event(int storage_fd, input_event & event);


/**
 * Read the axis records from the input file
 * @param storage_fd - file containing the storage axis entries
 * @param map - map to fill the records into
 * @return 0 if read ok, -1 if something went wrong
 */
int mwt_storage_file_read_axis_map(int storage_fd, axis_map & map);

/**
 * Write the axis records to the output file
 * @param storage_fd - file to write into
 * @param map - map containing the axis records to be written
 */
void mwt_storage_file_write_axis_map(int storage_fd, const axis_map & map);

#endif  /* MWTOUCH_DUMP_FORMAT_HPP */
