/*
 * debug_trace.h
 *
 *  Created on: Jun 23, 2013
 *      Author: mazzin
 */

#ifndef DEBUG_TRACE_H_
#define DEBUG_TRACE_H_

#pragma once

namespace tracey
{
    // API
    void watch(const void* ptr, size_t size);
    void forget(const void* ptr);
    
    // Control
    void enable(bool on = true);
    void disable();
    bool is_enabled();
    void invalidate();
    std::string demangle(const std::string& name);
    
    // Report
    void report();
    
    // Versioning
    const char* version();
}

#endif /* DEBUG_TRACE_H_ */
