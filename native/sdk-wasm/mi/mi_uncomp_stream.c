#include <nitro/mi/stream.h>
#include <nitro/mi/uncompress.h>
#include <nitro/mi/uncomp_stream.h>

void MI_InitUncompContextRL (MIUncompContextRL * context, u8 * dest, const MICompressionHeader * header)
{
    context->destp = dest;
    context->destCount = (s32)header->destSize;
    context->flags = 0;
    context->length = 0;
    context->destTmp = 0;
    context->destTmpCnt = 0;
}

void MI_InitUncompContextLZ (MIUncompContextLZ * context, u8 * dest, const MICompressionHeader * header)
{
    context->destp = dest;
    context->destCount = (s32)header->destSize;
    context->flags = 0;
    context->flagIndex = 0;
    context->length = 0;
    context->lengthFlg = 3;
    context->destTmp = 0;
    context->destTmpCnt = 0;
    context->exFormat = (u8)((header->compParam == 0)? 0U : 1U);
}

void MI_InitUncompContextHuffman (MIUncompContextHuffman * context, u8 * dest, const MICompressionHeader * header)
{
    context->destp = dest;
    context->destCount = (s32)header->destSize;
    context->bitSize = (u8)header->compParam;
    context->treeSize = -1;
    context->treep = &context->tree[0];
    context->destTmp = 0;
    context->destTmpCnt = 0;
    context->srcTmp = 0;
    context->srcTmpCnt = 0;
}

#include <nitro/code32.h>

#define TREE_END_MASK 0x80

#include <nitro/codereset.h>
