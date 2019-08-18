#pragma once

//-----------------------------------------------------------------------------
// the following macros can be used to identifiy the type of the field
#define IS_NODE_FIELD(m) ((m) & 0x00010000? true:false )
#define IS_FACE_FIELD(m) ((m) & 0x00020000? true:false )
#define IS_ELEM_FIELD(m) ((m) & 0x00040000? true:false )
#define IS_EDGE_FIELD(m) ((m) & 0x00080000? true:false )

#define IS_VALID(m) ((IS_NODE_FIELD(m))||(IS_FACE_FIELD(m))||(IS_ELEM_FIELD(m))||(IS_EDGE_FIELD(m)))

#define FIELD_CODE(n) (((n) & 0xFF00) >> 8)
#define FIELD_COMP(n) ((n) & 0x00FF)
#define BUILD_FIELD(a,b,c) (((a)<<16)|((b)<<8)|(c))
