#include "sevenzip/compressor.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "sevenzip/archive.hpp"
#include "sevenzip/error.hpp"

namespace sevenzip {

class Compressor::Impl {
   public:
    Format format;
    CompressionLevel level;

    Impl(Format fmt, CompressionLevel lvl) : format(fmt), level(lvl) {
        // 验证格式是否支持独立压缩
        if (format != Format::GZip && format != Format::BZip2 && format != Format::Xz) {
            throw std::runtime_error("Compressor only supports GZip, BZip2, and Xz formats");
        }
    }
};

Compressor::Compressor(Format format, CompressionLevel level)
    : impl_(std::make_unique<Impl>(format, level)) {}

Compressor::~Compressor() = default;

Compressor::Compressor(Compressor&& other) noexcept = default;
Compressor& Compressor::operator=(Compressor&& other) noexcept = default;

Compressor& Compressor::withLevel(CompressionLevel level) {
    impl_->level = level;
    return *this;
}

std::vector<uint8_t> Compressor::compress(const std::vector<uint8_t>& input) {
    std::vector<uint8_t> buffer;

    try {
        auto archive = Archive::createToMemory(buffer, impl_->format);
        archive.withCompressionLevel(impl_->level);
        archive.addFromMemory(input, "data.bin");
        archive.finalize();
    } catch (const Exception& e) {
        throw std::runtime_error(std::string("Failed to compress: ") + e.what());
    }

    return buffer;
}

std::vector<uint8_t> Compressor::decompress(const std::vector<uint8_t>& input) {
    try {
        // Use the known format for decompression
        auto archive = Archive::openFromMemory(input, impl_->format);

        // 获取第一个文件并解压到内存
        if (archive.itemCount() == 0) {
            throw std::runtime_error("Archive is empty");
        }

        return archive.extractItemToMemory(0);
    } catch (const Exception& e) {
        throw std::runtime_error(std::string("Failed to decompress: ") + e.what());
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to decompress: ") + e.what());
    } catch (...) {
        throw std::runtime_error("Failed to decompress: unknown error");
    }
}

void Compressor::compressFile(const std::filesystem::path& inputPath,
                              const std::filesystem::path& outputPath) {
    // 读取输入文件
    std::ifstream ifs(inputPath, std::ios::binary);
    if (!ifs) {
        throw Exception("Cannot open input file: " + inputPath.string());
    }

    std::vector<uint8_t> input((std::istreambuf_iterator<char>(ifs)),
                               std::istreambuf_iterator<char>());
    ifs.close();

    // 压缩
    auto output = compress(input);

    // 写入输出文件
    std::ofstream ofs(outputPath, std::ios::binary);
    if (!ofs) {
        throw Exception("Cannot create output file: " + outputPath.string());
    }

    ofs.write(reinterpret_cast<const char*>(output.data()), output.size());
}

void Compressor::decompressFile(const std::filesystem::path& inputPath,
                                const std::filesystem::path& outputPath) {
    // 读取输入文件
    std::ifstream ifs(inputPath, std::ios::binary);
    if (!ifs) {
        throw Exception("Cannot open input file: " + inputPath.string());
    }

    std::vector<uint8_t> input((std::istreambuf_iterator<char>(ifs)),
                               std::istreambuf_iterator<char>());
    ifs.close();

    // 解压
    auto output = decompress(input);

    // 写入输出文件
    std::ofstream ofs(outputPath, std::ios::binary);
    if (!ofs) {
        throw Exception("Cannot create output file: " + outputPath.string());
    }

    ofs.write(reinterpret_cast<const char*>(output.data()), output.size());
}

}  // namespace sevenzip
