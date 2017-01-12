/*
 * File:   event_storage.cpp
 * Author: Lukáš Ručka (359687@mail.muni.cz)
 * Organization: Masaryk University; Brno, Czech Republic
 * License: BSD
 * Created on 9th August 2011, 23:53
 *
 * Purpose: * implements functions utilized by both dumper and replay
 */

#include <unistd.h>
#include <iostream>

#include "event_storage.hpp"
#include "axis.hpp"

// this pretty much mirrors the input_event struct
struct storage_unifier {
    uint64_t tv_sec;
    uint64_t tv_usec;
    uint16_t type;
    uint16_t code;
    int32_t value;
};

struct storage_file_header {
    storage_file_header(){
        memcpy(magick_number, "/**mwtouch*/", MAGICK_NUMBER_LENGTH);
    }
    
    static const int MAGICK_NUMBER_LENGTH = 12;
    
    char magick_number[MAGICK_NUMBER_LENGTH];
};


static const int EMPTY_RECORDS_SEPARATOR_COUNT = 2;

/**
 * Processes the machine-native event to unified format
 */
static struct storage_unifier mwt_unifier_unify(const input_event & input);

/**
 * Processes the unified event to machine-native format
 */
static struct input_event mwt_unifier_nativate(storage_unifier input);

/**
 * Checks whether the given axis record is empty
 * @param record - record to check
 * @return true if empty
 */
static bool mwt_storage_file_axis_record_is_empty(const storage_file_axis_record & record);

/**
 * Write the axis record
 * @param storage_fd - output file handle
 * @param record - axis entry to be written to the storage file
 */
static void mwt_storage_file_write_axis_record(int storage_fd, storage_file_axis_record record);

/**
 * Read formated axis info record from storage file
 * @param storage_fd - file containing the storage entries
 * @param record - record to read the data into
 * @return 0 if read ok, -1 if something went wrong
 */
static int mwt_storage_file_read_axis_record(int storage_fd, storage_file_axis_record & record);

struct storage_unifier mwt_unifier_unify(const input_event & input){
    struct storage_unifier unifier;
        unifier.tv_sec  = input.time.tv_sec;
            unifier.tv_sec  = htobe64(unifier.tv_sec);

        unifier.tv_usec = input.time.tv_usec;
            unifier.tv_usec = htobe64(unifier.tv_usec);
        
        unifier.type    = input.type;
            unifier.type    = htobe16(unifier.type);
        
        unifier.code    = input.code;
            unifier.code    = htobe16(unifier.code);
        
        unifier.value   = input.value;
            unifier.value   = htobe32(unifier.value);
    return unifier;
}

struct input_event mwt_unifier_nativate(storage_unifier input){
    struct input_event native;
        
        input.tv_sec  = be64toh(input.tv_sec);
            native.time.tv_sec  = input.tv_sec;
        
        input.tv_usec = be64toh(input.tv_usec);
            native.time.tv_usec = input.tv_usec;
            
        input.code         = be16toh(input.code);
            native.code         = input.code;
        
        input.type         = be16toh(input.type);
            native.type         = input.type;
        
        input.value        = be32toh(input.value);
            native.value        = input.value;
        
    return native;
}

void mwt_storage_file_write_header(int storage_fd){
    storage_file_header header;
    if (write(storage_fd, &header, sizeof(storage_file_header)) <= 0){ 
        std::cerr << "Failed to dump store header!" << std::endl;
    }
}

bool mwt_storage_file_test_header(int storage_fd){
    storage_file_header header;
    storage_file_header tmp_header;
    int retval = read(storage_fd, &tmp_header, sizeof(storage_file_header));
    if (retval != sizeof(storage_file_header)){ return false; }
    if (memcmp(&tmp_header, &header, sizeof(tmp_header)) != 0){ return false; }
    return true;
}

void mwt_storage_file_write_axis_record(int storage_fd, storage_file_axis_record record){
	record.value = htobe32(record.value);
	record.minimum = htobe32(record.minimum);
	record.maximum = htobe32(record.maximum);
	record.fuzz = htobe32(record.fuzz);
	record.flat = htobe32(record.flat);
	record.resolution = htobe32(record.resolution);
    
    if (write(storage_fd, &record, sizeof(storage_file_axis_record)) <= 0){ 
        std::cerr << "Failed to dump axis record!" << std::endl;
    }
}

int mwt_storage_file_read_axis_record(int storage_fd, storage_file_axis_record & record){
    memset(&record, 0, sizeof(storage_file_axis_record));
    int retval = read(storage_fd, &record, sizeof(storage_file_axis_record));
    
	record.value = be32toh(record.value);
	record.minimum = be32toh(record.minimum);
	record.maximum = be32toh(record.maximum);
	record.fuzz = be32toh(record.fuzz);
	record.flat = be32toh(record.flat);
	record.resolution = be32toh(record.resolution);
    
    return (retval == sizeof(storage_file_axis_record))?0:-1;
}

int mwt_storage_file_read_event(int storage_fd, input_event & event){
    storage_unifier record;
    memset(&record, 0, sizeof(storage_unifier));
    int retval = read(storage_fd, &record, sizeof(storage_unifier));
    
    event = mwt_unifier_nativate(record);

    return (retval == sizeof(storage_unifier))?0:-1;
}

void mwt_storage_file_write_event(int storage_fd, const input_event & event){
    storage_unifier record = mwt_unifier_unify(event);
    if (write(storage_fd, &record, sizeof(storage_unifier)) <= 0){ 
        std::cerr << "Failed to dump event!" << std::endl;
    }
}

bool mwt_storage_file_axis_record_is_empty(const storage_file_axis_record & record){
    storage_file_axis_record empty_record;
    memset(&empty_record, 0, sizeof(storage_file_axis_record));
    return memcmp(&record, &empty_record, sizeof(storage_file_axis_record)) == 0;
}

int mwt_storage_file_read_axis_map(int storage_fd, axis_map & map){
    
    int retval = 0;
    int empty_records = 0;
    
    while ((retval == 0) && (empty_records < EMPTY_RECORDS_SEPARATOR_COUNT)){

        storage_file_axis_record record;
        memset(&record, 0, sizeof(record));
        
        retval = mwt_storage_file_read_axis_record(storage_fd, record);
        
        if (mwt_storage_file_axis_record_is_empty(record)){
            empty_records++;
        } else {
            if (empty_records == 1){
                storage_file_axis_record empty_record;
                memset(&empty_record, 0, sizeof(empty_record));
                map[0] = empty_record;
            }
            
            empty_records = 0;
            map[record.value] = record;
        }
    }
    
    return retval;
}

void mwt_storage_file_write_axis_map(int storage_fd, const axis_map & map){

    for (axis_map::const_iterator i = map.begin(); i != map.end(); i++){
        storage_file_axis_record tmp = i->second;
        tmp.value = i->first;
        mwt_storage_file_write_axis_record(storage_fd, tmp);
    }
    
    storage_file_axis_record empty_record;
    memset(&empty_record, 0, sizeof(empty_record));
    
    for (int empty_records = 0; empty_records < EMPTY_RECORDS_SEPARATOR_COUNT; empty_records++){
        mwt_storage_file_write_axis_record(storage_fd, empty_record);
    }
}



