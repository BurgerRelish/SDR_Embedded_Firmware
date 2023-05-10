#include "MessageSerializer.h"

/**
 * @brief Compresses a string using the Brotli Algorithm.
 * Dynamically allocates a temporary buffer to store the compressed string. Uses the 
 * Google Brotli Encoder to compress the inout string. Copys the output of the encoder to a 
 * new string and shrinks it to fit.
 * 
 * @note Returns an empty string on compression failure.
 * 
 * @param message std::string to be compressed.
 * @return std::string - Compressed string data.
*/
const std::string MessageSerializer::compressString(const std::string message)
{
    size_t outbytes = 0, data_size = message.size();

    uint8_t* outbuf = (uint8_t *) calloc(data_size + 1, sizeof(uint8_t)); // Allocate the output buffer

    int status = BrotliEncoderCompress(1, BROTLI_DEFAULT_WINDOW, BROTLI_DEFAULT_MODE, data_size, (uint8_t *)message.c_str(), &outbytes, outbuf); // Compress message with Brotli Compression

    if (status != BROTLI_TRUE) // Check that encoder was successful.
    {
        log_e("Brotli Encoder Failed. Error Code:%d\n", status);

        free(outbuf); // Free dynamically allocated memory

        return std::string("");
    }

    std::string ret((char *)outbuf); // Create a string containing the compressed data.

    free(outbuf); // Free dynamically allocated memory

    ret.shrink_to_fit(); // Ensure that the string is not oversized.

    log_i("Message Compressed. Compression Ratio: %f\n", (float) data_size / (float) ret.size());

    return ret;
}

/**
 * @brief Decompresses a string using the Brotli Algrotithm.
 * Dynamically allocates a buffer of BROTLI_DECODER_BUFFER_SIZE, uses the Google Brotli Decoder to decode the message
 * into the output buffer. Converts the output buffer to a std::string and shrinks it to fit.
 * 
 * @note Returns an empty string on decompression failure
 * .
 * @param message Brotli Compressed string to decompress.
 * @return std::string - Decompressed Data
*/
const std::string MessageSerializer::decompressString(const std::string message)
{
    size_t data_size = message.size(), decoded_size = BROTLI_DECODER_BUFFER_SIZE;

    uint8_t* inbuf = (uint8_t *) calloc(data_size + 1, sizeof(uint8_t)); // Allocate the input and output buffers
    uint8_t* outbuf = (uint8_t *) calloc(BROTLI_DECODER_BUFFER_SIZE, sizeof(uint8_t));

    BrotliDecoderResult status = BrotliDecoderDecompress(data_size, (uint8_t *)message.c_str(), &decoded_size, outbuf); // Decompress message with Brotli Decompression

    if (status != BROTLI_DECODER_RESULT_SUCCESS) // Check that decoder was successful.
    {
        log_e("Brotli Decoder Failed. Error Code:%d\n", (uint8_t)status);

        free(inbuf); // Free dynamically allocated memory
        free(outbuf);

        return std::string("");
    }

    std::string ret((char *)outbuf); // Create a string containing the decompressed data.

    log_i("Message Decompressed. Decompression Ratio: %f\n", (float) decoded_size / (float) data_size);

    ret.shrink_to_fit(); // Ensure that the string is not oversized.

    free(inbuf); // Free dynamically allocated memory
    free(outbuf);

    return ret;
}
