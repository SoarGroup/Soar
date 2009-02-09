
/*
 * Desc: Useful encoding/decoding routines
 * Author: Andrew Howard
 * Date: 16 Sep 2005
 * CVS: $Id: encode.h 2453 2004-09-23 05:42:04Z inspectorg $
 */

#ifndef ENCODE_H_
#define ENCODE_H_


/// Determine the size of the destination buffer for hex encoding
size_t EncodeHexSize(size_t src_len);

/// Encode binary data to ascii hex.
void EncodeHex(char *dst, size_t dst_len, const void *src, size_t src_len);

/// Determine the size of the destination buffer for hex decoding
size_t DecodeHexSize(size_t src_len);

/// Decodes ascii hex to binary data.  
void DecodeHex(void *dst, size_t dst_len, const char *src, size_t src_len);


#endif
