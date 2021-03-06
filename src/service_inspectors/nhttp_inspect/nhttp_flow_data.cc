/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
// nhttp_flow_data.cc author Tom Peters <thopeter@cisco.com>

#include "nhttp_enum.h"
#include "nhttp_test_manager.h"
#include "nhttp_flow_data.h"
#include "nhttp_transaction.h"

using namespace NHttpEnums;

unsigned NHttpFlowData::nhttp_flow_id = 0;

NHttpFlowData::NHttpFlowData() : FlowData(nhttp_flow_id) {
    /* FIXIT-L Temporary printf while we shake out stream interface */
    if (!NHttpTestManager::use_test_input() && NHttpTestManager::use_test_output()) {
        printf("Flow Data construct %p\n", (void*)this);
        fflush(nullptr);
    }
}

NHttpFlowData::~NHttpFlowData() {
    /* FIXIT-L Temporary printf while we shake out stream interface */
    if (!NHttpTestManager::use_test_input() && NHttpTestManager::use_test_output()) {
        printf("Flow Data destruct %p\n", (void*)this);
        fflush(nullptr);
    }
    for (int k=0; k <= 1; k++) {
        if (section_buffer_owned[k]) {
            delete[] section_buffer[k];
        }
        delete[] chunk_buffer[k];
        delete transaction[k];
    }
    delete_pipeline();
}

void NHttpFlowData::half_reset(SourceId source_id) {
    assert((source_id == SRC_CLIENT) || (source_id == SRC_SERVER));

    version_id[source_id] = VERS__NOTPRESENT;
    if (source_id == SRC_CLIENT) {
        method_id = METH__NOTPRESENT;
    }
    else {
        status_code_num = STAT_NOTPRESENT;
    }
    data_length[source_id] = STAT_NOTPRESENT;
    body_octets[source_id] = STAT_NOTPRESENT;
}

void NHttpFlowData::show(FILE* out_file) const {
    assert(out_file != nullptr);
    fprintf(out_file, "Diagnostic output from NHttpFlowData (Client/Server):\n");
    fprintf(out_file, "Version ID: %d/%d\n", version_id[0], version_id[1]);
    fprintf(out_file, "Method ID: %d\n", method_id);
    fprintf(out_file, "Status code: %d\n", status_code_num);
    fprintf(out_file, "Type expected: %d/%d\n", type_expected[0], type_expected[1]);
    fprintf(out_file, "Data length: %" PRIi64 "/%" PRIi64 "\n", data_length[0], data_length[1]);
    fprintf(out_file, "Body octets: %" PRIi64 "/%" PRIi64 "\n", body_octets[0], body_octets[1]);
    fprintf(out_file, "Unused octets visible: %u/%u\n", unused_octets_visible[0], unused_octets_visible[1]);
    fprintf(out_file, "Header octets visible: %u/%u\n", header_octets_visible[0], header_octets_visible[1]);
    fprintf(out_file, "Section buffer length: %d/%d\n", section_buffer_length[0], section_buffer_length[1]);
    fprintf(out_file, "Chunk buffer length: %d/%d\n", chunk_buffer_length[0], chunk_buffer_length[1]);
    fprintf(out_file, "Pipelining: front %d back %d overflow %d underflow %d\n", pipeline_front, pipeline_back,
       pipeline_overflow, pipeline_underflow);
}

bool NHttpFlowData::add_to_pipeline(NHttpTransaction* latest) {
    assert(!pipeline_overflow && !pipeline_underflow);
    int new_back = (pipeline_back+1) % MAX_PIPELINE;
    if (new_back == pipeline_front) {
        pipeline_overflow = true;
        return false;
    }
    pipeline[pipeline_back] = latest;
    pipeline_back = new_back;
    return true;
}

NHttpTransaction* NHttpFlowData::take_from_pipeline() {
    assert(!pipeline_underflow);
    if (pipeline_back == pipeline_front) {
        return nullptr;
    }
    int old_front = pipeline_front;
    pipeline_front = (pipeline_front+1) % MAX_PIPELINE;
    return pipeline[old_front];
}

void NHttpFlowData::delete_pipeline() {
   for (int k=pipeline_front; k != pipeline_back; k = (k+1) % MAX_PIPELINE) {
       delete pipeline[k];
   }
}













