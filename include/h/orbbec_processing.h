#ifndef ORBBEC_PROCESSING_H
#define ORBBEC_PROCESSING_H
#ifdef __cplusplus
extern "C" {
#endif

#include "orbbec_types.h"
#include "orbbec_sensor.h"

orbbec_processing_block* orbbec_create_colorizer(orbbec_error** error);
orbbec_processing_block* orbbec_create_sync_processing_block(orbbec_error** error);
orbbec_processing_block* orbbec_create_pointcloud(orbbec_error** error);
orbbec_processing_block* orbbec_create_processing_block(orbbec_frame_processor_callback* proc, orbbec_error** error);
orbbec_processing_block* orbbec_create_processing_block_fptr(orbbec_frame_processor_callback_ptr proc, void * context, orbbec_error** error);
void orbbec_start_processing(orbbec_processing_block* block, orbbec_frame_callback* on_frame, orbbec_error** error);
void orbbec_start_processing_fptr(orbbec_processing_block* block, orbbec_frame_callback_ptr on_frame, void* user, orbbec_error** error);
void orbbec_start_processing_queue(orbbec_processing_block* block, orbbec_frame_queue* queue, orbbec_error** error);
void orbbec_process_frame(orbbec_processing_block* block, orbbec_frame* frame, orbbec_error** error);
void orbbec_delete_processing_block(orbbec_processing_block* block);
orbbec_frame_queue* orbbec_create_frame_queue(int capacity, orbbec_error** error);
void orbbec_delete_frame_queue(orbbec_frame_queue* queue);
orbbec_frame* orbbec_wait_for_frame(orbbec_frame_queue* queue, unsigned int timeout_ms, orbbec_error** error);
int orbbec_poll_for_frame(orbbec_frame_queue* queue, orbbec_frame** output_frame, orbbec_error** error);
void orbbec_enqueue_frame(orbbec_frame* frame, void* queue);
orbbec_processing_block* orbbec_create_align(orbbec_stream align_to, orbbec_error** error);
orbbec_processing_block* orbbec_create_decimation_filter_block(orbbec_error** error);
orbbec_processing_block* orbbec_create_temporal_filter_block(orbbec_error** error);
orbbec_processing_block* orbbec_create_spatial_filter_block(orbbec_error** error);
orbbec_processing_block* orbbec_create_disparity_transform_block(unsigned char transform_to_disparity, orbbec_error** error);
orbbec_processing_block* orbbec_create_hole_filling_filter_block(orbbec_error** error);

#ifdef __cplusplus
}
#endif
#endif // ORBBEC_PROCESSING_H
