/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.5 */

#ifndef PB_PROTOCOL_TRACKER_PB_H_INCLUDED
#define PB_PROTOCOL_TRACKER_PB_H_INCLUDED
#include <proto/pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _protocol_TrackerPosition { 
    int64_t track; 
    int64_t vehicleId; 
    int64_t timestamp; 
    double latitude; 
    double longitude; 
    double speed; 
} protocol_TrackerPosition;

typedef struct _protocol_Report { 
    char token[256]; 
    bool has_timestamp;
    int64_t timestamp; 
    bool isMoving; 
    pb_size_t positions_count;
    protocol_TrackerPosition positions[20]; 
} protocol_Report;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define protocol_Report_init_default             {"", false, 0, 0, 0, {protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default, protocol_TrackerPosition_init_default}}
#define protocol_TrackerPosition_init_default    {0, 0, 0, 0, 0, 0}
#define protocol_Report_init_zero                {"", false, 0, 0, 0, {protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero, protocol_TrackerPosition_init_zero}}
#define protocol_TrackerPosition_init_zero       {0, 0, 0, 0, 0, 0}

/* Field tags (for use in manual encoding/decoding) */
#define protocol_TrackerPosition_track_tag       1
#define protocol_TrackerPosition_vehicleId_tag   2
#define protocol_TrackerPosition_timestamp_tag   3
#define protocol_TrackerPosition_latitude_tag    4
#define protocol_TrackerPosition_longitude_tag   5
#define protocol_TrackerPosition_speed_tag       6
#define protocol_Report_token_tag                1
#define protocol_Report_timestamp_tag            2
#define protocol_Report_isMoving_tag             3
#define protocol_Report_positions_tag            4

/* Struct field encoding specification for nanopb */
#define protocol_Report_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, STRING,   token,             1) \
X(a, STATIC,   OPTIONAL, INT64,    timestamp,         2) \
X(a, STATIC,   REQUIRED, BOOL,     isMoving,          3) \
X(a, STATIC,   REPEATED, MESSAGE,  positions,         4)
#define protocol_Report_CALLBACK NULL
#define protocol_Report_DEFAULT NULL
#define protocol_Report_positions_MSGTYPE protocol_TrackerPosition

#define protocol_TrackerPosition_FIELDLIST(X, a) \
X(a, STATIC,   REQUIRED, INT64,    track,             1) \
X(a, STATIC,   REQUIRED, INT64,    vehicleId,         2) \
X(a, STATIC,   REQUIRED, INT64,    timestamp,         3) \
X(a, STATIC,   REQUIRED, DOUBLE,   latitude,          4) \
X(a, STATIC,   REQUIRED, DOUBLE,   longitude,         5) \
X(a, STATIC,   REQUIRED, DOUBLE,   speed,             6)
#define protocol_TrackerPosition_CALLBACK NULL
#define protocol_TrackerPosition_DEFAULT NULL

extern const pb_msgdesc_t protocol_Report_msg;
extern const pb_msgdesc_t protocol_TrackerPosition_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define protocol_Report_fields &protocol_Report_msg
#define protocol_TrackerPosition_fields &protocol_TrackerPosition_msg

/* Maximum encoded size of messages (where known) */
#define protocol_Report_size                     1511
#define protocol_TrackerPosition_size            60

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif