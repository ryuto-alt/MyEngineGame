#include "GLBStructures.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>

namespace GLB {
    // GLBファイルのマジックナンバー
    static const uint32_t GLB_MAGIC = 0x46546C67;  // "glTF"

    // File クラスの実装
    const Chunk* File::GetJsonChunk() const {
        for (const auto& chunk : chunks) {
            if (chunk.header.type == CHUNK_TYPE_JSON) {
                return &chunk;
            }
        }
        return nullptr;
    }

    const Chunk* File::GetBinaryChunk() const {
        for (const auto& chunk : chunks) {
            if (chunk.header.type == CHUNK_TYPE_BIN) {
                return &chunk;
            }
        }
        return nullptr;
    }

    // Parser クラスの実装
    Parser::Parser() {}

    Parser::~Parser() {}

    bool Parser::LoadFromFile(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            errorMessage_ = "Failed to open file: " + filePath;
            return false;
        }

        // ファイルサイズを取得
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        // ファイルデータを読み込み
        std::vector<uint8_t> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            errorMessage_ = "Failed to read file: " + filePath;
            return false;
        }

        return LoadFromMemory(buffer.data(), buffer.size());
    }

    bool Parser::LoadFromMemory(const uint8_t* data, size_t size) {
        if (!data || size == 0) {
            errorMessage_ = "Invalid input data";
            return false;
        }

        return ReadBinaryData(data, size);
    }

    bool Parser::ReadBinaryData(const uint8_t* data, size_t size) {
        size_t offset = 0;

        // ヘッダーの読み込み
        if (!ReadHeader(data, offset, size)) {
            return false;
        }
        offset += sizeof(Header);

        // チャンクの読み込み
        while (offset < size) {
            Chunk chunk;
            if (!ReadChunk(data, offset, size, chunk)) {
                return false;
            }
            
            file_.chunks.push_back(std::move(chunk));
            
            // 次のチャンクのオフセット計算
            offset += sizeof(ChunkHeader) + chunk.header.length;
            
            // 4バイト境界へのパディング
            size_t padding = CalculatePadding(chunk.header.length);
            offset += padding;
        }

        return true;
    }

    bool Parser::ReadHeader(const uint8_t* data, size_t offset, size_t size) {
        if (offset + sizeof(Header) > size) {
            errorMessage_ = "Insufficient data for GLB header";
            return false;
        }

        file_.header.magic = ReadUint32LE(data, offset);
        file_.header.version = ReadUint32LE(data, offset + 4);
        file_.header.length = ReadUint32LE(data, offset + 8);

        if (!ValidateHeader(file_.header)) {
            return false;
        }

        return true;
    }

    bool Parser::ReadChunk(const uint8_t* data, size_t offset, size_t size, Chunk& chunk) {
        if (offset + sizeof(ChunkHeader) > size) {
            errorMessage_ = "Insufficient data for chunk header";
            return false;
        }

        chunk.header.length = ReadUint32LE(data, offset);
        chunk.header.type = ReadUint32LE(data, offset + 4);

        size_t chunkDataOffset = offset + sizeof(ChunkHeader);
        if (chunkDataOffset + chunk.header.length > size) {
            errorMessage_ = "Insufficient data for chunk data";
            return false;
        }

        // チャンクデータをコピー
        chunk.data.resize(chunk.header.length);
        std::copy(data + chunkDataOffset, 
                  data + chunkDataOffset + chunk.header.length, 
                  chunk.data.begin());

        if (!ValidateChunk(chunk)) {
            return false;
        }

        return true;
    }

    uint32_t Parser::ReadUint32LE(const uint8_t* data, size_t offset) const {
        return static_cast<uint32_t>(data[offset]) |
               (static_cast<uint32_t>(data[offset + 1]) << 8) |
               (static_cast<uint32_t>(data[offset + 2]) << 16) |
               (static_cast<uint32_t>(data[offset + 3]) << 24);
    }

    bool Parser::ValidateHeader(const Header& header) {
        if (header.magic != GLB_MAGIC) {
            errorMessage_ = "Invalid GLB magic number";
            return false;
        }

        if (header.version != 2) {
            std::stringstream ss;
            ss << "Unsupported GLB version: " << header.version;
            errorMessage_ = ss.str();
            return false;
        }

        if (header.length < sizeof(Header)) {
            errorMessage_ = "Invalid GLB file length";
            return false;
        }

        return true;
    }

    bool Parser::ValidateChunk(const Chunk& chunk) {
        if (chunk.header.type != CHUNK_TYPE_JSON && chunk.header.type != CHUNK_TYPE_BIN) {
            std::stringstream ss;
            ss << "Unknown chunk type: 0x" << std::hex << chunk.header.type;
            errorMessage_ = ss.str();
            return false;
        }

        if (chunk.header.length == 0) {
            errorMessage_ = "Chunk has zero length";
            return false;
        }

        return true;
    }

    size_t Parser::CalculatePadding(size_t size) const {
        return (4 - (size % 4)) % 4;
    }
}