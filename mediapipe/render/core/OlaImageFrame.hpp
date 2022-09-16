#ifndef OlaImageFormat_hpp
#define OlaImageFormat_hpp

#include <mutex>

namespace Opipe {


    /**
    * 和 UPipeImageFormat相同
    */
    enum class OlaImageFormat {
        //TODO
        // initialized with this value.
        UNKNOWN = 0,

        // sRGB, interleaved: one byte for R, then one byte for G, then one
        // byte for B for each pixel.
        SRGB = 1,

        // sRGBA, interleaved: one byte for R, one byte for G, one byte for B,
        // one byte for alpha or unused.
        SRGBA = 2,

        // Grayscale, one byte per pixel.
        GRAY8 = 3,

        // Grayscale, one uint16 per pixel.
        GRAY16 = 4,

        // YCbCr420P (1 bpp for Y, 0.25 bpp for U and V).
        // NOTE: NOT a valid ImageFrame format, but intended for
        // ScaleImageCalculatorOptions, VideoHeader, etc. to indicate that
        // YUVImage is used in place of ImageFrame.
        YCBCR420P = 5,

        // Similar to YCbCr420P, but the data is represented as the lower 10bits of
        // a uint16. Like YCbCr420P, this is NOT a valid ImageFrame, and the data is
        // carried within a YUVImage.
        YCBCR420P10 = 6,

        // sRGB, interleaved, each component is a uint16.
        SRGB48 = 7,

        // sRGBA, interleaved, each component is a uint16.
        SRGBA64 = 8,

        // One float per pixel.
        VEC32F1 = 9,

        // LAB, interleaved: one byte for L, then one byte for a, then one
        // byte for b for each pixel.
        LAB8 = 10,

        // sBGRA, interleaved: one byte for B, one byte for G, one byte for R,
        // one byte for alpha or unused. This is the N32 format for Skia.
        SBGRA = 11,
    };

    struct OlaImageFrameDes {
        OlaImageFormat format = OlaImageFormat::UNKNOWN;
        int width = 0;
        int height = 0;
        int widthStep = 0;

        void fill(OlaImageFrameDes &des) {
            format = des.format;
            width = des.width;
            height = des.height;
            widthStep = des.widthStep;
        }

        void reset() {
            format = OlaImageFormat::UNKNOWN;
            width = 0;
            height = 0;
            widthStep = 0;
        }
    };

    /**
     * Quaramera中 CPU Image数据的格式
     */
    class OlaImageFrame {
    public :

        void createData(int width, int height, int widthStep, OlaImageFormat format);


        void copyData(char *data, int width, int height, int widthStep, OlaImageFormat format);

        /**
         * 释放数据的控制权限，释放后，数据不再由 OlaImageFrame 负责销毁
         *
         * 使用这种释放需要格外小心内存释放
         */
        void releaseDataControl(char **data, OlaImageFrameDes &des);

        void releaseData();

        char *getData() {
            return _data;
        }

        OlaImageFrameDes getImageDes() {
            return _des;
        }

        const std::mutex &getDataLock() {
            return _dataLock;
        }

        virtual ~OlaImageFrame();


    private :
        char *_data;
        OlaImageFrameDes _des;
        std::mutex _dataLock;
    };
}

#endif